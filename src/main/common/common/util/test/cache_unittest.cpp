/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       cache_unittest.cpp
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           5/6/21
  *Description:
**********************************************************************************/
#include <common/test/test.h>
#include <cassert>
#include "common/util/test/cache_unittest.h"
#include "common/util/lru_cache.h"
#include "common/util/lfu_cache.h"
#include "common/util/hash_util.h"
#include <map>
#include <vector>
#include <stdlib.h>
#include <thread>

STD_USE_NAMESPACE;
COMMON_BEGIN_NAMESPACE

CPPUNIT_TEST_SUITE_REGISTRATION(CacheTest);
TLOG_SETUP(COMMON,CacheTest);

class GetStringSize {
public:
    uint64_t operator()(const string& str) const {
        return str.size();
    }
};
class GetUInt32Size {
public:
    uint64_t operator()(const uint32_t & v) const {
        return sizeof(v);
    }
};

void CacheTest::testLRUCache() {

    uint64_t  totalMemSize = 1e8;
    uint64_t initialHashSize = 1e7;
    LRUCache<string,uint32_t,GetStringSize,GetUInt32Size,DBKeyHash<string>, DBKeyEqual<string> > lruCache(initialHashSize,totalMemSize);
    LFUCache<string,uint32_t,GetStringSize,GetUInt32Size,DBKeyHash<string>, DBKeyEqual<string> > lfuCache(initialHashSize,totalMemSize);

    unordered_map<string,uint32_t,DBKeyHash<string>, DBKeyEqual<string> > m(initialHashSize);

    TLOG_LOG(INFO,"begin put------------");
    for (uint32_t i = 0; i < 1e7; ++i) {
        if ((i % (uint32_t)1e6) == 0) {
            TLOG_LOG(INFO,"Begin %uth put------------",i);
        }
        uint32_t ix = Random<uint32_t>::RandomIntBetween(0,25);
        string s;
        for (uint32_t k = 0; k < 96; ++k ) {
            uint32_t kx = Random<uint32_t>::RandomIntBetween(0,25);
            s.push_back('A' + kx);
        }
        lruCache.Put(s,ix);
        lfuCache.Put(s,ix);
        m[s] = ix;
    }
    TLOG_LOG(INFO,"Finished put------------");

    bool isEqual = lfuCache.IsEqual(lruCache);
    TLOG_LOG(INFO,"before query operations,lfucache whether is equal to lrucache:[%d]", isEqual);

    TLOG_LOG(INFO,"------totally inserted element count:[%zu]-----", m.size());
    for (auto it = m.begin(); it != m.end(); ++it) {
        uint32_t v;
        lruCache.Get(it->first,v);
        lfuCache.Get(it->first,v);
    }

    isEqual = lfuCache.IsEqual(lruCache);
    TLOG_LOG(INFO,"after query operations,lfucache whether is equal to lrucache:[%d]", isEqual);

    TLOG_LOG(INFO,"lrucache keyCount:[%lu],keyCountInHash:[%lu],cacheSize:[%lu], cacheSizeUsed:[%lu],totalQueryTimes:[%lu],hitRatio:[%f]",
             lruCache.GetKeyCount(),lruCache.GetKeyCountInHash(),lruCache.GetCacheSize(),lruCache.GetCacheSizeUsed(),lruCache.GetTotalQueryTimes(),lruCache.GetHitRatio());
    TLOG_LOG(INFO,"lfuCache cacheSize:[%lu], cacheSizeUsed:[%lu],totalQueryTimes:[%lu],hitRatio:[%f]",
             lfuCache.GetCacheSize(),lfuCache.GetCacheSizeUsed(),lfuCache.GetTotalQueryTimes(),lfuCache.GetHitRatio());


    CPPUNIT_ASSERT_EQUAL(lruCache.GetCacheSize(),lfuCache.GetCacheSize());
    CPPUNIT_ASSERT_EQUAL(lruCache.GetCacheSizeUsed(),lfuCache.GetCacheSizeUsed());
    CPPUNIT_ASSERT_EQUAL(lruCache.GetTotalQueryTimes(),lfuCache.GetTotalQueryTimes());
    CPPUNIT_ASSERT_EQUAL(lruCache.GetHitQueryTimes(),lfuCache.GetHitQueryTimes());

    CPPUNIT_ASSERT_EQUAL(lruCache.GetKeyCount(),lfuCache.GetKeyCount());
    CPPUNIT_ASSERT_EQUAL(lruCache.GetKeyCountInHash(),lfuCache.GetKeyCountInHash());
    CPPUNIT_ASSERT_EQUAL(lruCache.GetKeyCount(),lruCache.GetKeyCountInHash());
    CPPUNIT_ASSERT_EQUAL(lfuCache.GetKeyCount(),lfuCache.GetKeyCountInHash());

    CPPUNIT_ASSERT_EQUAL(lruCache.GetHitRatio(),lfuCache.GetHitRatio());

}

void CacheTest::testLFUCache() {

    uint64_t  totalMemSize = 1e8;
    uint64_t initialHashSize = 1e6;
    LRUCache<string,uint32_t,GetStringSize,GetUInt32Size,DBKeyHash<string>, DBKeyEqual<string> > lruCache(initialHashSize,totalMemSize);
    LFUCache<string,uint32_t,GetStringSize,GetUInt32Size,DBKeyHash<string>, DBKeyEqual<string> > lfuCache(initialHashSize,totalMemSize);

    unordered_map<string,uint32_t,DBKeyHash<string>, DBKeyEqual<string> > m(initialHashSize);
    TLOG_LOG(INFO,"begin put------------");
    for (uint32_t i = 0; i < 1e6; ++i) {
        if ((i % (uint32_t)1e6) == 0) {
            TLOG_LOG(INFO,"Begin %uth put------------",i);
        }
        uint32_t ix = Random<uint32_t>::RandomIntBetween(0,25);
        string s;
        for (uint32_t k = 0; k < 96; ++k ) {
            uint32_t kx = Random<uint32_t>::RandomIntBetween(0,25);
            s.push_back('A' + kx);
        }
        lruCache.Put(s,ix);
        lfuCache.Put(s,ix);
        m[s] = ix;
    }
    TLOG_LOG(INFO,"Finished put------------");

    bool isEqual = lfuCache.IsEqual(lruCache);
    TLOG_LOG(INFO,"before query operations,lfucache whether is equal to lrucache:[%d]", isEqual);

    CPPUNIT_ASSERT_EQUAL(lruCache.GetKeyCount(),lfuCache.GetKeyCount());
    CPPUNIT_ASSERT_EQUAL(lruCache.GetKeyCountInHash(),lfuCache.GetKeyCountInHash());
    CPPUNIT_ASSERT_EQUAL(lfuCache.GetKeyCount(),lfuCache.GetKeyCountInHash());
    CPPUNIT_ASSERT_EQUAL(lruCache.GetKeyCount(),lruCache.GetKeyCountInHash());
    CPPUNIT_ASSERT_EQUAL(lruCache.GetKeyCount(),lfuCache.GetKeyCount());
    CPPUNIT_ASSERT_EQUAL(lruCache.GetKeyCountInHash(),lfuCache.GetKeyCountInHash());
    CPPUNIT_ASSERT_EQUAL(lfuCache.GetKeyCount(),lfuCache.GetKeyCountInHash());

    TLOG_LOG(INFO,"------totally inserted element count:[%zu]-----", m.size());
    for (auto it = m.begin(); it != m.end(); ++it) {
        uint32_t v;
        lruCache.Get(it->first,v);
        lfuCache.Get(it->first,v);
    }

    isEqual = lfuCache.IsEqual(lruCache);
    TLOG_LOG(INFO,"after query operations,lfucache whether is equal to lrucache:[%d]", isEqual);

    CPPUNIT_ASSERT_EQUAL(lruCache.GetKeyCount(),lfuCache.GetKeyCount());
    CPPUNIT_ASSERT_EQUAL(lruCache.GetKeyCountInHash(),lfuCache.GetKeyCountInHash());
    CPPUNIT_ASSERT_EQUAL(lfuCache.GetKeyCount(),lfuCache.GetKeyCountInHash());
    CPPUNIT_ASSERT_EQUAL(lruCache.GetKeyCount(),lruCache.GetKeyCountInHash());
    CPPUNIT_ASSERT_EQUAL(lruCache.GetKeyCount(),lfuCache.GetKeyCount());
    CPPUNIT_ASSERT_EQUAL(lruCache.GetKeyCountInHash(),lfuCache.GetKeyCountInHash());
    CPPUNIT_ASSERT_EQUAL(lfuCache.GetKeyCount(),lfuCache.GetKeyCountInHash());

    TLOG_LOG(INFO,"lrucache keyCount:[%lu],keyCountInHash:[%lu],cacheSize:[%lu], cacheSizeUsed:[%lu],totalQueryTimes:[%lu],hitRatio:[%f]",
             lruCache.GetKeyCount(),lruCache.GetKeyCountInHash(),lruCache.GetCacheSize(),lruCache.GetCacheSizeUsed(),lruCache.GetTotalQueryTimes(),lruCache.GetHitRatio());

    TLOG_LOG(INFO,"lfuCache keyCount:[%lu],keyCountInHash:[%lu],cacheSize:[%lu], cacheSizeUsed:[%lu],totalQueryTimes:[%lu],hitRatio:[%f]",
             lfuCache.GetKeyCount(), lfuCache.GetKeyCountInHash(),lfuCache.GetCacheSize(),lfuCache.GetCacheSizeUsed(),lfuCache.GetTotalQueryTimes(),lfuCache.GetHitRatio());
}

COMMON_END_NAMESPACE