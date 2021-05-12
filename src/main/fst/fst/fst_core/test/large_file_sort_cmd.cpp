/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       large_file_sort_cmd.cpp
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           5/10/21
  *Description:    file implements class to implements large_file_sort command
**********************************************************************************/
#include "common/common.h"
#include <common/util/CLI11.hpp>
#include "fst/fst_core/large_file_sorter.h"

using namespace std;
COMMON_USE_NAMESPACE;

int main(int argc, char** argv) {
    TLoggerGuard tLoggerGuard;

    //declare and setup tlog variable
    TLOG_DECLARE_AND_SETUP_LOGGER(lfsort, MAIN);

    CLI::App app("lfsort: A smart large file sort command line tool", "lfsort"); // 软件描述出现在第一行打印
    app.footer("Please contact dingbinthu@163.com for related questions and other matters not covered. Enjoy it!"); // 最后一行打印
    app.get_formatter()->column_width(40); // 列的宽度

    string inputFile, outputFile, workDir;
    uint32_t threadNum,splitFileNum, parallelTaskNum;
    bool ignoreEmptyLines;
    app.add_option("-f,--input-file",inputFile,"input file which is often a huge large file to sort")->check(CLI::ExistingFile)->required(true);
    app.add_option("-o,--output-file",outputFile,"output file which store result sorted from input file")->check(CLI::NonexistentPath)->required(true);
    app.add_option("-w,--work-directory",workDir,"work directory specified for total processing,default /tmp if not set")->default_val("/tmp")->check(CLI::ExistingDirectory)->required(false);
    app.add_option("-t,--thread-count",threadNum,"threads count used for sort large file,default 4 if not set")->default_val(4)->check(CLI::Range(1,32))->required(false);
    app.add_option("-s,--split-file-count",splitFileNum,"count number of large file split,default 8 if not set")->default_val(6)->check(CLI::Range(1,1000))->required(false);
    app.add_option("-p,--parallel-task-count",parallelTaskNum,"paralleled running task count for merge sorted intermediate files, default 3 if not set")->default_val(3)->check(CLI::Range(2,20))->required(false);
    app.add_flag("-i,--ignore-empty-line",ignoreEmptyLines,"whether ignore all empty lines to result file,default false if not set")->required(false);
    CLI11_PARSE(app, argc, argv);
    bool ret = false;
    if (app.parsed()) {
        Random<uint32_t>::seedDefault();
        Random<uint64_t>::seedDefault();
        LargeFileSorter largeFileSorter(inputFile,outputFile,workDir,threadNum,splitFileNum,parallelTaskNum,!ignoreEmptyLines);
        bool ret = largeFileSorter.Run();
    }
    return ret?0:1;
}

