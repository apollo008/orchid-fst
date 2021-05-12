/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       CLI11_unittest.cpp
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           4/27/21
  *Description:
**********************************************************************************/
#include <common/test/test.h>
#include <cassert>
#include "common/util/test/CLI11_unittest.h"
STD_USE_NAMESPACE;
COMMON_BEGIN_NAMESPACE

CPPUNIT_TEST_SUITE_REGISTRATION(CLI11Test);
TLOG_SETUP(COMMON_NS,CLI11Test);

void CLI11Test::testCLI11() {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("",true,true);
}

COMMON_END_NAMESPACE