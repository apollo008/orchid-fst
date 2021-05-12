/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       large_file_sorter_unittest.cpp
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           5/9/21
  *Description:
**********************************************************************************/
#include <fst/test/test.h>
#include "fst/fst_core/test/large_file_sorter_unittest.h"
#include "fst/fst_core/large_file_sorter.h"
#include "common/util/time_util.h"


STD_USE_NAMESPACE;
COMMON_BEGIN_NAMESPACE

CPPUNIT_TEST_SUITE_REGISTRATION(LargeFileSorterTest);
TLOG_SETUP(COMMON_NS,LargeFileSorterTest);


void LargeFileSorterTest::testLargeFileSorterContainEmptyLine() {

    string standardFile = string() + TEST_DATA_PATH + "/large_file_sort_test1_standard_contain_empty.txt";
    string inputFile = string() + TEST_DATA_PATH + "/large_file_sort_test1_input.txt";
    string outputFile = string() + TEST_DATA_PATH + "/" +  Random<uint32_t>::RandomString(128);
    RemoveFileRAII removeFileRaii(outputFile);

    LargeFileSorter largeFileSorter(inputFile,
                                    outputFile,
                                    "/tmp",
                                    4,8,3,true);
    bool ret = largeFileSorter.Run();
    CPPUNIT_ASSERT_EQUAL(true,ret);
    string line;
    ostringstream oss1,oss2;
    ifstream ifs1(outputFile);
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

void LargeFileSorterTest::testLargeFileSorterIgnoreEmptyLine() {
    string standardFile = string() + TEST_DATA_PATH + "/large_file_sort_test1_standard_ignore_empty.txt";
    string inputFile = string() + TEST_DATA_PATH + "/large_file_sort_test1_input.txt";
    string outputFile = string() + TEST_DATA_PATH + "/" +  Random<uint32_t>::RandomString(128);
    RemoveFileRAII removeFileRaii(outputFile);

    LargeFileSorter largeFileSorter(inputFile,
                                    outputFile,
                                    "/tmp",
                                    4,8,3,false);
    bool ret = largeFileSorter.Run();
    CPPUNIT_ASSERT_EQUAL(true,ret);
    string line;
    ostringstream oss1,oss2;
    ifstream ifs1(outputFile);
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