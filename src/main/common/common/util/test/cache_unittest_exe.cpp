/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       cache_unittest_exe.cpp
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           5/6/21
  *Description:
**********************************************************************************/
#include "common/common.h"
#include <cassert>
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TextTestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <string>
#include <sstream>
#include "common/util/lru_cache.h"
#include "common/util/lfu_cache.h"
#include "common/util/hash_util.h"
#include <map>

using namespace std;
using namespace CppUnit;
COMMON_USE_NAMESPACE;

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

int main(int argc, char** argv) {
//    TextTestRunner testRunner;
//    testRunner.setOutputter(new CompilerOutputter(&testRunner.result(), std::cerr));
//    TestFactoryRegistry& registry = TestFactoryRegistry::getRegistry();
//    testRunner.addTest(registry.makeTest());
//    bool ok = testRunner.run("",false);
//    return ok ?0:1;

    TLoggerGuard tLoggerGuard;

    //declare and setup tlog variable
    TLOG_DECLARE_AND_SETUP_LOGGER(CPPUNITDEMO, MAIN);

    uint64_t  totalMemSize = 1e8;
    uint64_t initialHashSize = 1e6;
    LRUCache<string,uint32_t,GetStringSize,GetUInt32Size> lruCache(initialHashSize,totalMemSize);
    LFUCache<string,uint32_t,GetStringSize,GetUInt32Size> lfuCache(initialHashSize,totalMemSize);

    unordered_map<string,uint32_t > m(initialHashSize);
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

    TLOG_LOG(INFO,"------totally inserted element count:[%zu]-----", m.size());
    for (auto it = m.begin(); it != m.end(); ++it) {
        uint32_t v;
        lruCache.Get(it->first,v);
        lfuCache.Get(it->first,v);
    }

    isEqual = lfuCache.IsEqual(lruCache);
    TLOG_LOG(INFO,"after query operations,lfucache whether is equal to lrucache:[%d]", isEqual);

    TLOG_LOG(INFO,"lrucache keyCount:[%lu],cacheSize:[%lu], cacheSizeUsed:[%lu],totalQueryTimes:[%lu],hitRatio:[%f]",
             lruCache.GetKeyCount(),lruCache.GetCacheSize(),lruCache.GetCacheSizeUsed(),lruCache.GetTotalQueryTimes(),lruCache.GetHitRatio());
    TLOG_LOG(INFO,"lfuCache keyCount:[%lu],cacheSize:[%lu], cacheSizeUsed:[%lu],totalQueryTimes:[%lu],hitRatio:[%f]",
             lfuCache.GetKeyCount(), lfuCache.GetCacheSize(),lfuCache.GetCacheSizeUsed(),lfuCache.GetTotalQueryTimes(),lfuCache.GetHitRatio());

return 0;
}

