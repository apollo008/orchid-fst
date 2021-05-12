/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       cache_unittest.h
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           5/6/21
  *Description:
**********************************************************************************/
#ifndef _COMMON_MODULE_TEST_CACHE_UNITTEST__H_
#define _COMMON_MODULE_TEST_CACHE_UNITTEST__H_

#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include <cppunit/extensions/HelperMacros.h>
#include "common/common.h"
#include "tulip/TLogDefine.h"
#include "common/util/file_util.h"

COMMON_BEGIN_NAMESPACE


class CacheTest: public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(CacheTest);
    CPPUNIT_TEST(testLRUCache);
    CPPUNIT_TEST(testLFUCache);
    CPPUNIT_TEST_SUITE_END();
public:
    void testLRUCache();
    void testLFUCache();
private:
private:
    TLOG_DECLARE();
};

COMMON_END_NAMESPACE
#endif //_COMMON_MODULE_TEST_CACHE_UNITTEST__H_
