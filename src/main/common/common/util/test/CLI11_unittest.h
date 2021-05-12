/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       CLI11_unittest.h
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           4/27/21
  *Description:
**********************************************************************************/
#ifndef _COMMON_MODULE_TEST_CLI11_UNITTEST__H_
#define _COMMON_MODULE_TEST_CLI11_UNITTEST__H_

#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include <cppunit/extensions/HelperMacros.h>
#include "common/common.h"
#include "tulip/TLogDefine.h"
#include "common/util/CLI11.hpp"

COMMON_BEGIN_NAMESPACE


class CLI11Test: public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(CLI11Test);
    CPPUNIT_TEST(testCLI11);
    CPPUNIT_TEST_SUITE_END();
public:
    void testCLI11();
private:
private:
    TLOG_DECLARE();
};

COMMON_END_NAMESPACE
#endif //_COMMON_MODULE_TEST_CLI11_UNITTEST__H_
