/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       large_file_sorter_unittest.h
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           5/9/21
  *Description:
**********************************************************************************/
#ifndef _FST_CORE_MODULE_TEST_LARGE_FILE_SORTER_UNITTEST__H_
#define _FST_CORE_MODULE_TEST_LARGE_FILE_SORTER_UNITTEST__H_

#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include <cppunit/extensions/HelperMacros.h>
#include "common/common.h"
#include "tulip/TLogDefine.h"

COMMON_BEGIN_NAMESPACE


class LargeFileSorterTest: public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(LargeFileSorterTest);
    CPPUNIT_TEST(testLargeFileSorterContainEmptyLine);
    CPPUNIT_TEST(testLargeFileSorterIgnoreEmptyLine);
    CPPUNIT_TEST_SUITE_END();
public:
    void testLargeFileSorterContainEmptyLine();
    void testLargeFileSorterIgnoreEmptyLine();
private:
    TLOG_DECLARE();
};

COMMON_END_NAMESPACE
#endif //_FST_CORE_MODULE_TEST_LARGE_FILE_SORTER_UNITTEST__H_
