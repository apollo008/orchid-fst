/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       fst_unittest_exe.cpp
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           5/5/21
  *Description:    file implements class to implements fst data structure: Finite state transducer
**********************************************************************************/
#include "common/common.h"
#include <common/util/time_util.h>
#include <common/util/string_util.h>
#include <common/util/CLI11.hpp>
#include "common/util/file_util.h"
#include "fst/fst_core/large_file_sorter.h"
#include <fst/fst_core/fst.h>

using namespace std;
COMMON_USE_NAMESPACE;


int main(int argc, char** argv) {
    TLoggerGuard tLoggerGuard;

    //declare and setup tlog variable
    TLOG_DECLARE_AND_SETUP_LOGGER(COMMON_NS, MAIN);

    Random<uint64_t >::seedDefault();
    Random<uint32_t>::seedDefault();


    CLI::App app(fs("ofst: Orchid-Fst is a smart Fst command line tool",95), "ofst"); // 软件描述出现在第一行打印
    app.footer(fs("Please contact dingbinthu@163.com for related questions and other matters not covered. Enjoy it!",95)); // 最后一行打印
    app.get_formatter()->column_width(35); // 列的宽度
    app.require_subcommand(1); // 表示运行命令需要且仅需要一个子命令

    auto mapSubCmd = app.add_subcommand("map", fs("construct fst data file from a key-value(separated with comma every line) dictionary file."));
    auto setSubCmd = app.add_subcommand("set", fs("construct fst data file from a only key(no value) dictionary file."));
    auto dotSubCmd = app.add_subcommand("dot", fs("generate dot file from fst data file, which can be converted to png file using dot command like: dot -Tpng < a.dot > a.png, then you can view the picture generated."));
    auto matchQuerySubCmd = app.add_subcommand("match", fs("execute accurate match query for a term text in the fst."));
    auto prefixQuerySubCmd = app.add_subcommand("prefix", fs("execute prefix query starts with a term text in the fst."));
    auto rangeQuerySubCmd = app.add_subcommand("range", fs("execute range query in the fst."));
    auto fuzzyQuerySubCmd = app.add_subcommand("fuzzy", fs("execute fuzzy query in the fst,it works by building a Levenshtein or Damerau-Levenshtein automaton within a edit distance."));

    string dictFile, fstFile, dotFile, matchstr,prefixstr, gt,ge,lt,le,  fuzzyStr;
    uint32_t editDistance, fuzzyPrefixLen;
    uint64_t maxCacheSize;
    bool isFileSorted;
    bool isUseDamerauLevenshtein;
    string workDir;
    uint32_t threadNum,splitFileNum, parallelTaskNum;
    if (mapSubCmd) {
        mapSubCmd->add_option("-f,--dict-file",dictFile,fs("dictionary file which with format like:`key,value` for every line."))->check(CLI::ExistingFile)->required(true);
        mapSubCmd->add_option("-o,--fst-file",fstFile,fs("output fst data file will be generated."))->check(CLI::NonexistentPath)->required(true);
        mapSubCmd->add_option("-c,--cache-size",maxCacheSize,fs("max cache size used with unit MB bytes,default 1000M if not set"))->default_val(1000)->check(CLI::NonNegativeNumber)->required(false);

        mapSubCmd->add_flag("-s,--sorted",isFileSorted,fs("Set this if the input data is already lexicographically sorted. This will make fst construction much faster."))->default_val(false)->required(false);
        mapSubCmd->add_option("-w,--work-directory",workDir,fs("work directory specified for sort input dictionary file if necessary,default /tmp if not set"))->default_val("/tmp")->check(CLI::ExistingDirectory)->required(false);
        mapSubCmd->add_option("-t,--thread-count",threadNum,fs("threads count specified for sort input dictionary file if necessary,default 4 if not set"))->default_val(4)->check(CLI::Range(1,32))->required(false);
        mapSubCmd->add_option("-l,--split-file-count",splitFileNum,fs("count number of large file split specified for sort input dictionary file if necessary,default 8 if not set"))->default_val(6)->check(CLI::Range(1,1000))->required(false);
        mapSubCmd->add_option("-p,--parallel-task-count",parallelTaskNum, fs("paralleled running task count for merge sorted intermediate files specified for sort input dictionary file if necessary, default 3 if not set"))->default_val(3)->check(CLI::Range(2,20))->required(false);
    }
    if (setSubCmd) {
        setSubCmd->add_option("-f,--dict-file",dictFile,fs("dictionary file which with format like:`key,value` for every line."))->check(CLI::ExistingFile)->required(true);
        setSubCmd->add_option("-o,--fst-file",fstFile,fs("output fst data file will be generated."))->check(CLI::NonexistentPath)->required(true);
        setSubCmd->add_option("-c,--cache-size",maxCacheSize, fs("max cache size used with unit MB bytes,default 1000M if not set"))->default_val(1000)->check(CLI::NonNegativeNumber)->required(false);

        setSubCmd->add_flag("-s,--sorted",isFileSorted,fs("Set this if the input data is already lexicographically sorted. This will make fst construction much faster."))->default_val(false)->required(false);
        setSubCmd->add_option("-w,--work-directory",workDir,fs("work directory specified for sort input dictionary file if necessary,default /tmp if not set"))->default_val("/tmp")->check(CLI::ExistingDirectory)->required(false);
        setSubCmd->add_option("-t,--thread-count",threadNum,fs("threads count specified for sort input dictionary file if necessary,default 4 if not set"))->default_val(4)->check(CLI::Range(1,32))->required(false);
        setSubCmd->add_option("-l,--split-file-count",splitFileNum,fs("count number of large file split specified for sort input dictionary file if necessary,default 8 if not set"))->default_val(6)->check(CLI::Range(1,1000))->required(false);
        setSubCmd->add_option("-p,--parallel-task-count",parallelTaskNum,fs("paralleled running task count for merge sorted intermediate files specified for sort input dictionary file if necessary, default 3 if not set"))->default_val(3)->check(CLI::Range(2,20))->required(false);
    }
    if (dotSubCmd) {
        dotSubCmd->add_option("-f,--fst-file",fstFile,fs("fst data file constructed before."))->check(CLI::ExistingFile)->required(true);
        dotSubCmd->add_option("-o,--dot-file",dotFile,fs("output dot file will be generated,which can be converted to png file using dot command like: dot -Tpng < a.dot > a.png, then you can view the picture generated."))->check(CLI::NonexistentPath)->required(true);
    }
    if (matchQuerySubCmd) {
        matchQuerySubCmd->add_option("-f,--fst-file",fstFile,fs("fst data file constructed before."))->check(CLI::ExistingFile)->required(true);
        matchQuerySubCmd->add_option("-s,--greater-than",gt,fs("only show results greater than this, indicates left unbound if not specified"));
        matchQuerySubCmd->add_option("-a,--greater-equal-than",ge,fs("only show results greater than OR EQUAL TO this, indicates left unbound if not specified"));
        matchQuerySubCmd->add_option("-e,--less-than",lt,fs("only show results less than this, indicates right unbound if not specified"));
        matchQuerySubCmd->add_option("-b,--less-equal-than",le,fs("only show results less than OR EQUAL TO this, indicates right unbound if not specified"));
        matchQuerySubCmd->add_option("-q,--match-str",matchstr,fs("string to be matched."))->required(true);
    }
    if (prefixQuerySubCmd) {
        prefixQuerySubCmd->add_option("-f,--fst-file",fstFile,fs("fst data file constructed before."))->check(CLI::ExistingFile)->required(true);
        prefixQuerySubCmd->add_option("-s,--greater-than",gt,fs("only show results greater than this, indicates left unbound if not specified"));
        prefixQuerySubCmd->add_option("-a,--greater-equal-than",ge,fs("only show results greater than OR EQUAL TO this, indicates left unbound if not specified"));
        prefixQuerySubCmd->add_option("-e,--less-than",lt,fs("only show results less than this, indicates right unbound if not specified"));
        prefixQuerySubCmd->add_option("-b,--less-equal-than",le,fs("only show results less than OR EQUAL TO this, indicates right unbound if not specified"));
        prefixQuerySubCmd->add_option("-p,--prefix-str",prefixstr,fs("prefix string starts with to be searched."))->required(true);
    }
    if (rangeQuerySubCmd) {
        rangeQuerySubCmd->add_option("-f,--fst-file",fstFile,fs("fst data file constructed before."))->check(CLI::ExistingFile)->required(true);
        rangeQuerySubCmd->add_option("-s,--greater-than",gt,fs("only show results greater than this, indicates left unbound if not specified"));
        rangeQuerySubCmd->add_option("-a,--greater-equal-than",ge,fs("only show results greater than OR EQUAL TO this, indicates left unbound if not specified"));
        rangeQuerySubCmd->add_option("-e,--less-than",lt,fs("only show results less than this, indicates right unbound if not specified"));
        rangeQuerySubCmd->add_option("-b,--less-equal-than",le,fs("only show results less than OR EQUAL TO this, indicates right unbound if not specified"));
    }

    if (fuzzyQuerySubCmd) {
        fuzzyQuerySubCmd->add_option("-f,--fst-file",fstFile,fs("fst data file constructed before."))->check(CLI::ExistingFile)->required(true);
        fuzzyQuerySubCmd->add_option("-z,--fuzzy-str",fuzzyStr,fs("string to be fuzzy matched."))->required(true);
        fuzzyQuerySubCmd->add_option("-d,--distance",editDistance,fs("edit distance for levenshtein similarity search used."))->check(CLI::Range(0,100))->required(true);
        fuzzyQuerySubCmd->add_option("-l,--prefix-len",fuzzyPrefixLen,fs("same prefix length ignored for levenshtein similarity search used."))->default_val(0);
        fuzzyQuerySubCmd->add_flag("-m,--damerau-levenshtein",
                                   isUseDamerauLevenshtein,
                                   fs("Set this if use Damerau-Levenshtein Distance to measure similarity. Levenshtein Distance will be used to measure similarity if not set this option."))->default_val(false)->required(false);
    }

    CLI11_PARSE(app, argc, argv);

    // 判断哪个子命令被使用
    if (mapSubCmd->parsed() || setSubCmd->parsed()) {
        FileOutputStreamPtr outputStream = std::make_shared<FileOutputStream>();
        outputStream->Open(fstFile);
        FstBuilder builder(outputStream.get(),mapSubCmd->parsed(),maxCacheSize * 1000000);
        ifstream ifs;
        string line;
        RemoveFileRAIIPtr removeFileRaii = std::make_shared<RemoveFileRAII>();
        if (!isFileSorted) {
            string outputSortFile = TimeUtility::CurrentTimeInSecondsReadable() + "_" +  Random<uint32_t>::RandomString(8);
            removeFileRaii->SetFilePath(outputSortFile);

            LargeFileSorter largeFileSorter(dictFile,outputSortFile,workDir,threadNum,splitFileNum,parallelTaskNum,false);
            bool bSortSucc = largeFileSorter.Run();
            if (!bSortSucc) {
                TLOG_LOG(ERROR,"failed to sort dictionary file:[%s],please check!", dictFile.c_str());
                return -1;
            }
            ifs.open(outputSortFile);
            if (!ifs) {
                TLOG_LOG(ERROR,"failed to read data from sorted output dictionary file:[%s],please check!", outputSortFile.c_str());
                return -1;
            }
        }
        else {
            //dictionary file has been sorted
            ifs.open(dictFile);
        }
        while (getline(ifs,line)) {
            if (line.empty()) continue;
            vector<string> arr;
            StringUtil::Split( line, ",",arr,false);
            if (arr.size() < 2 && mapSubCmd->parsed()) {
                TLOG_LOG(ERROR, "invalid input data line:[%s],items count < 2!omit it", line.c_str());
                continue;
            }
            if (arr.size() < 1 && setSubCmd->parsed()) {
                TLOG_LOG(ERROR, "invalid input data line:[%s],items count < 1!omit it", line.c_str());
                continue;
            }
            string key = arr[0];
//            TLOG_LOG(INFO,"key:[%s] in string,while [%ws] in wstring.",key.c_str(), s2ws(key).c_str());
            uint64_t value = 0;
            if (mapSubCmd->parsed()) {
                stringstream ss;
                ss << arr[1];
                ss >> value;
            }
            builder.Insert((uint8_t*)key.c_str(), strlen(key.c_str()),value);
        }
        builder.Finish();
        outputStream->Close();
    }
    else if (dotSubCmd->parsed()) {
        ofstream  ofs(dotFile);
        if (!ofs) {
            TLOG_LOG(ERROR,"Error! failed to open output file:[%s],please check!", dotFile.c_str());
            return 1;
        }
        MMapDataPiece mMapDataPiece;
        bool openOk = mMapDataPiece.OpenRead(fstFile.c_str(), true);
        assert(openOk);
        FstReader fstReader(mMapDataPiece.GetData());
        fstReader.DotDraw(ofs);
        ofs.flush();
        ofs.close();
        mMapDataPiece.Close();
    }
    else if (matchQuerySubCmd->parsed()) {

        FstReader::FstIterBound leftBound, rightBound;
        if (gt.empty() && ge.empty()) {
            leftBound.m_bound.clear();
            leftBound.m_type = FstReader::FstIterBound::FST_ITER_BOUND_TYPE_UNBOUNDED;
        }
        else if (!ge.empty()) {
            for (char ch : ge) {
                leftBound.m_bound.push_back(ch);
            }
            leftBound.m_type = FstReader::FstIterBound::FST_ITER_BOUND_TYPE_INCLUDED;
        }
        else if (!gt.empty()) {
            for (char ch : gt) {
                leftBound.m_bound.push_back(ch);
            }
            leftBound.m_type = FstReader::FstIterBound::FST_ITER_BOUND_TYPE_EXCLUDED;
        }

        if (lt.empty() && le.empty()) {
            rightBound.m_bound.clear();
            rightBound.m_type = FstReader::FstIterBound::FST_ITER_BOUND_TYPE_UNBOUNDED;
        }
        else if (!le.empty()) {
            for (char ch : le) {
                rightBound.m_bound.push_back(ch);
            }
            rightBound.m_type = FstReader::FstIterBound::FST_ITER_BOUND_TYPE_INCLUDED;
        }
        else if (!lt.empty()) {
            for (char ch : lt) {
                rightBound.m_bound.push_back(ch);
            }
            rightBound.m_type = FstReader::FstIterBound::FST_ITER_BOUND_TYPE_EXCLUDED;
        }

        MMapDataPiece mMapDataPiece;
        bool openOk = mMapDataPiece.OpenRead(fstFile.c_str(), true);
        assert(openOk);
        FstReader fstReader(mMapDataPiece.GetData());

        int64_t  stTime = TimeUtility::CurrentTimeInMicroSeconds();
        FstReader::Iterator it = fstReader.GetMatchIterator(leftBound, rightBound,matchstr);
        FstReader::IteratorResultPtr item = it.Next();
        int64_t  edTime = TimeUtility::CurrentTimeInMicroSeconds();
        if (nullptr == item) {
            TLOG_LOG(INFO,"Can not found any key in dictionary! time consumed:[%lu] us.", edTime-stTime);
            return 1;
        }
        else {
            bool isMap = fstReader.HasOutput();
            if (isMap) {
                TLOG_LOG(INFO,"Found result:[%s]->[%lu], time consumed:[%lu] us.", item->GetInputStr().c_str(),item->m_output, edTime-stTime);
            }
            else {
                TLOG_LOG(INFO,"Found result:[%s], time consumed:[%lu] us.", item->GetInputStr().c_str(),edTime-stTime);
            }
        }
    }
    else if (prefixQuerySubCmd->parsed()) {
        FstReader::FstIterBound leftBound, rightBound;
        if (gt.empty() && ge.empty()) {
            leftBound.m_bound.clear();
            leftBound.m_type = FstReader::FstIterBound::FST_ITER_BOUND_TYPE_UNBOUNDED;
        }
        else if (!ge.empty()) {
            for (char ch : ge) {
                leftBound.m_bound.push_back(ch);
            }
            leftBound.m_type = FstReader::FstIterBound::FST_ITER_BOUND_TYPE_INCLUDED;
        }
        else if (!gt.empty()) {
            for (char ch : gt) {
                leftBound.m_bound.push_back(ch);
            }
            leftBound.m_type = FstReader::FstIterBound::FST_ITER_BOUND_TYPE_EXCLUDED;
        }

        if (lt.empty() && le.empty()) {
            rightBound.m_bound.clear();
            rightBound.m_type = FstReader::FstIterBound::FST_ITER_BOUND_TYPE_UNBOUNDED;
        }
        else if (!le.empty()) {
            for (char ch : le) {
                rightBound.m_bound.push_back(ch);
            }
            rightBound.m_type = FstReader::FstIterBound::FST_ITER_BOUND_TYPE_INCLUDED;
        }
        else if (!lt.empty()) {
            for (char ch : lt) {
                rightBound.m_bound.push_back(ch);
            }
            rightBound.m_type = FstReader::FstIterBound::FST_ITER_BOUND_TYPE_EXCLUDED;
        }

        MMapDataPiece mMapDataPiece;
        bool openOk = mMapDataPiece.OpenRead(fstFile.c_str(), true);
        assert(openOk);
        FstReader fstReader(mMapDataPiece.GetData());

        int64_t  stTime = TimeUtility::CurrentTimeInMicroSeconds();
        FstReader::Iterator it = fstReader.GetPrefixIterator(leftBound, rightBound,prefixstr);

        uint64_t  hitCount = 0;
        bool isMap = fstReader.HasOutput();
        while (true) {
            FstReader::IteratorResultPtr item = it.Next();
            if (nullptr == item) break;
            if (isMap) {
                TLOG_LOG(INFO,"[%s]->[%lu]", item->GetInputStr().c_str(),item->m_output);
            }
            else {
                TLOG_LOG(INFO,"[%s]", item->GetInputStr().c_str());
            }
            ++hitCount;
        }
        int64_t  edTime = TimeUtility::CurrentTimeInMicroSeconds();
        TLOG_LOG(INFO,"Totally got [%lu] results, time consumed:[%lu] us.", hitCount, edTime-stTime);
    }
    else if (rangeQuerySubCmd->parsed()) {
        MMapDataPiece mMapDataPiece;
        bool openOk = mMapDataPiece.OpenRead(fstFile.c_str(), true);
        assert(openOk);
        FstReader fstReader(mMapDataPiece.GetData());


        FstReader::FstIterBound leftBound, rightBound;
        if (gt.empty() && ge.empty()) {
            leftBound.m_bound.clear();
            leftBound.m_type = FstReader::FstIterBound::FST_ITER_BOUND_TYPE_UNBOUNDED;
        }
        else if (!ge.empty()) {
            for (char ch : ge) {
                leftBound.m_bound.push_back(ch);
            }
            leftBound.m_type = FstReader::FstIterBound::FST_ITER_BOUND_TYPE_INCLUDED;
        }
        else if (!gt.empty()) {
            for (char ch : gt) {
                leftBound.m_bound.push_back(ch);
            }
            leftBound.m_type = FstReader::FstIterBound::FST_ITER_BOUND_TYPE_EXCLUDED;
        }

        if (lt.empty() && le.empty()) {
            rightBound.m_bound.clear();
            rightBound.m_type = FstReader::FstIterBound::FST_ITER_BOUND_TYPE_UNBOUNDED;
        }
        else if (!le.empty()) {
            for (char ch : le) {
                rightBound.m_bound.push_back(ch);
            }
            rightBound.m_type = FstReader::FstIterBound::FST_ITER_BOUND_TYPE_INCLUDED;
        }
        else if (!lt.empty()) {
            for (char ch : lt) {
                rightBound.m_bound.push_back(ch);
            }
            rightBound.m_type = FstReader::FstIterBound::FST_ITER_BOUND_TYPE_EXCLUDED;
        }

        int64_t stTime = TimeUtility::CurrentTimeInMicroSeconds();
        FstReader::Iterator it = fstReader.GetRangeIterator(leftBound,rightBound);

        uint64_t hitCount = 0;
        bool isMap = fstReader.HasOutput();
        while (true) {
            FstReader::IteratorResultPtr item = it.Next();
            if (nullptr == item) break;
            if (isMap){
                TLOG_LOG(INFO, "[%s]->[%lu]", item->GetInputStr().c_str(), item->m_output);
            }
            else {
                TLOG_LOG(INFO, "[%s]", item->GetInputStr().c_str());
            }
            ++hitCount;
        }
        int64_t edTime = TimeUtility::CurrentTimeInMicroSeconds();
        TLOG_LOG(INFO, "Totally got [%lu] results, time consumed:[%lu] us.", hitCount, edTime - stTime);
    }
    else if (fuzzyQuerySubCmd->parsed()) {
        MMapDataPiece mMapDataPiece;
        bool openOk = mMapDataPiece.OpenRead(fstFile.c_str(), true);
        assert(openOk);
        FstReader fstReader(mMapDataPiece.GetData());

        int64_t  stTime = TimeUtility::CurrentTimeInMicroSeconds();
        FstReader::Iterator it = fstReader.GetFuzzyIterator(fuzzyStr,editDistance,fuzzyPrefixLen,isUseDamerauLevenshtein);

        uint64_t hitCount = 0;
        bool isMap = fstReader.HasOutput();
        while (true) {
            FstReader::IteratorResultPtr item = it.Next();
            if (nullptr == item) break;
            if (isMap) {
                TLOG_LOG(INFO, "[%s]->[%lu]", item->GetInputStr().c_str(), item->m_output);
            }
            else {
                TLOG_LOG(INFO, "[%s]", item->GetInputStr().c_str());
            }
            ++hitCount;
        }
        int64_t edTime = TimeUtility::CurrentTimeInMicroSeconds();
        TLOG_LOG(INFO, "Totally got [%lu] results, time consumed:[%lu] us.", hitCount, edTime - stTime);
    }
    return 0;
}

