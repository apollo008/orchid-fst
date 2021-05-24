/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       fst_unittest.cpp
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           5/5/21
  *Description:    file implements fst unittest class
**********************************************************************************/
#include <fst/test/test.h>
#include "fst/fst_core/test/fst_unittest.h"
#include "common/util/file_util.h"
#include <string>
#include <iostream>
#include <cassert>
#include "fst/fst_core/large_file_sorter.h"

STD_USE_NAMESPACE;
COMMON_BEGIN_NAMESPACE

CPPUNIT_TEST_SUITE_REGISTRATION(FstTest);
TLOG_SETUP(COMMON_NS,FstTest);

void FstTest::testFstFuzzy() {

    string standardFile = string() + TEST_DATA_PATH + "/fst_test_dict2_standard.txt";

    string dictFile = string() + TEST_DATA_PATH + "/fst_test_dict2.txt";
    string sortOutputFile = string() + TEST_DATA_PATH + "/" +  Random<uint32_t>::RandomString(32);
    string fstOutputFile = string() + TEST_DATA_PATH + "/" +  Random<uint32_t>::RandomString(32);
    RemoveFileRAII removeFileRaii1(sortOutputFile);
    RemoveFileRAII removeFileRaii2(fstOutputFile);

    /////////////////////////////////////////////////
    //sort
    LargeFileSorter largeFileSorter(dictFile,sortOutputFile,"/tmp",4,10,3,false);
    bool bSortSucc = largeFileSorter.Run();
    CPPUNIT_ASSERT_EQUAL(true,bSortSucc);
    /////////////////////////////////////////////////
    //map/set
    bool isMap = false;
    FileOutputStreamPtr outputStream = std::make_shared<FileOutputStream>();
    outputStream->Open(fstOutputFile);
    FstBuilder builder(outputStream.get(),isMap, 1000000);
    ifstream ifs;
    string line;
    ifs.open(sortOutputFile);
    while (getline(ifs,line)) {
        if (line.empty()) continue;
        vector<string> arr;
        StringUtil::Split( line, ",",arr,false);
        if (arr.size() < 2 && isMap) {
            TLOG_LOG(ERROR, "invalid input data line:[%s],items count < 2!omit it", line.c_str());
            continue;
        }
        if (arr.size() < 1 && !isMap) {
            TLOG_LOG(ERROR, "invalid input data line:[%s],items count < 1!omit it", line.c_str());
            continue;
        }
        string key = arr[0];
//            TLOG_LOG(INFO,"key:[%s] in string,while [%ws] in wstring.",key.c_str(), s2ws(key).c_str());
        uint64_t value = 0;
        if (isMap) {
            stringstream ss;
            ss << arr[1];
            ss >> value;
        }
        builder.Insert((uint8_t*)key.c_str(), strlen(key.c_str()),value);
    }
    builder.Finish();
    outputStream->Close();
    /////////////////////////////////////////////////
    ///fuzzy query
    MMapDataPiece mMapDataPiece;
    bool openOk = mMapDataPiece.OpenRead(fstOutputFile.c_str(), true);
    assert(openOk);
    FstReader fstReader(mMapDataPiece.GetData());
    FstReader::Iterator it = fstReader.GetFuzzyIterator("hair",2,0,false);
    uint64_t hitCount = 0;
    isMap = fstReader.HasOutput();
    CPPUNIT_ASSERT_EQUAL(false,isMap);
    vector<string> fuzzyResults;
    while (true) {
        FstReader::IteratorResultPtr item = it.Next();
        if (nullptr == item) break;
        fuzzyResults.push_back(item->GetInputStr());
        ++hitCount;
    }
    CPPUNIT_ASSERT_EQUAL(214ul,hitCount);
    ifstream ifs2(standardFile);
    assert(ifs2);
    size_t lineNo = 0;
    while (getline(ifs2,line)) {
        CPPUNIT_ASSERT_EQUAL(fuzzyResults[lineNo++],line);
    }
    CPPUNIT_ASSERT_EQUAL(fuzzyResults.size(),lineNo);
}

void FstTest::testFst() {
    string standardFile = string() + TEST_DATA_PATH + "/fst_test_dict1_standard.dot";
    string dictFile = string() + TEST_DATA_PATH + "/fst_test_dict1.txt";
    string sortOutputFile = string() + TEST_DATA_PATH + "/" +  Random<uint32_t>::RandomString(32);
    string fstOutputFile = string() + TEST_DATA_PATH + "/" +  Random<uint32_t>::RandomString(32);
    string dotOutputFile = string() + TEST_DATA_PATH + "/" +  Random<uint32_t>::RandomString(32);
    RemoveFileRAII removeFileRaii1(sortOutputFile);
    RemoveFileRAII removeFileRaii2(fstOutputFile);
    RemoveFileRAII removeFileRaii3(dotOutputFile);


    /////////////////////////////////////////////////
    //sort
    LargeFileSorter largeFileSorter(dictFile,sortOutputFile,"/tmp",4,10,3,false);
    bool bSortSucc = largeFileSorter.Run();
    CPPUNIT_ASSERT_EQUAL(true,bSortSucc);
    /////////////////////////////////////////////////
    //map/set
    bool isMap = false;
    FileOutputStreamPtr outputStream = std::make_shared<FileOutputStream>();
    outputStream->Open(fstOutputFile);
    FstBuilder builder(outputStream.get(),isMap, 1000000);
    ifstream ifs;
    string line;
    ifs.open(sortOutputFile);
    while (getline(ifs,line)) {
        if (line.empty()) continue;
        vector<string> arr;
        StringUtil::Split( line, ",",arr,false);
        if (arr.size() < 2 && isMap) {
            TLOG_LOG(ERROR, "invalid input data line:[%s],items count < 2!omit it", line.c_str());
            continue;
        }
        if (arr.size() < 1 && !isMap) {
            TLOG_LOG(ERROR, "invalid input data line:[%s],items count < 1!omit it", line.c_str());
            continue;
        }
        string key = arr[0];
//            TLOG_LOG(INFO,"key:[%s] in string,while [%ws] in wstring.",key.c_str(), s2ws(key).c_str());
        uint64_t value = 0;
        if (isMap) {
            stringstream ss;
            ss << arr[1];
            ss >> value;
        }
        builder.Insert((uint8_t*)key.c_str(), strlen(key.c_str()),value);
    }
    builder.Finish();
    outputStream->Close();
    /////////////////////////////////////////////////
    //dot
    ofstream  ofs(dotOutputFile);
    if (!ofs) {
        TLOG_LOG(ERROR,"Error! failed to open dot output file:[%s],please check!", dotOutputFile.c_str());
        CPPUNIT_ASSERT(false);
    }
    MMapDataPiece mMapDataPiece;
    bool openOk = mMapDataPiece.OpenRead(fstOutputFile.c_str(), true);
    assert(openOk);
    FstReader fstReader(mMapDataPiece.GetData());
    fstReader.DotDraw(ofs);
    ofs.flush();
    ofs.close();
    mMapDataPiece.Close();
    /////////////////////////////////////////////////
    //compare files
    ostringstream oss1,oss2;
    ifstream ifs1(dotOutputFile);
    assert(ifs1);
    while (getline(ifs1,line)) {
        oss1 << line << std::endl;
    }
    ifstream ifs2(standardFile);
    assert(ifs2);
    while (getline(ifs2,line)) {
        oss2 << line << std::endl;
    }
    CPPUNIT_ASSERT_EQUAL(oss1.str(),oss2.str());
}

COMMON_END_NAMESPACE