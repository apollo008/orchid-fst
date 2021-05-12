/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       fst_unittest.h
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           5/5/21
  *Description:    file defines fst unittest class
**********************************************************************************/
#ifndef _FST_CORE_MODULE_TEST_FST_UNITTEST__H_
#define _FST_CORE_MODULE_TEST_FST_UNITTEST__H_

#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include <cppunit/extensions/HelperMacros.h>
#include "common/common.h"
#include "tulip/TLogDefine.h"
#include "fst/fst_core/fst.h"

COMMON_BEGIN_NAMESPACE


class FstTest: public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(FstTest);
    CPPUNIT_TEST(testFst);
    CPPUNIT_TEST_SUITE_END();
public:
    void testFst();
private:
    TLOG_DECLARE();
};

COMMON_END_NAMESPACE
#endif //_FST_CORE_MODULE_TEST_FST_UNITTEST__H_
