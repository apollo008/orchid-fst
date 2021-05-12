/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       common.h
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           3/11/21
  *Description:    Some macros definitions interface for common use
**********************************************************************************/
#ifndef _COMMON_COMMON_H_
#define _COMMON_COMMON_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory>
#include <iostream>
#include <string>
#include <cassert>
#include <tulip/TLogDefine.h>

#define COMMON_BEGIN_NAMESPACE namespace cppfst_common {
#define COMMON_END_NAMESPACE }
#define COMMON_USE_NAMESPACE using namespace cppfst_common
#define COMMON_NS cppfst_common

#define STD_USE_NAMESPACE using namespace std

STD_USE_NAMESPACE;
COMMON_BEGIN_NAMESPACE


#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

#define UNUSED(x)  (void)(x)

#define DELETE_AND_SET_NULL(x) delete [] x; x = NULL
#define CHECK_DELETE_AND_SET_NULL(x) do {       \
        if(x){                                  \
            delete x;                           \
            x = NULL;                           \
        }                                       \
    } while(0)


#define COPY_CONSTRUCTOR(T)                      \
    T(const T &);                                \
    T & operator=(const T &);

#define TYPEDEF_PTR(x) typedef std::shared_ptr<x> x##Ptr

class TLoggerGuard {
public:
    TLoggerGuard() {
        // configurate TLog from logger config file 'logger.conf'
        TLOG_CONFIG("logger.conf");
    }

    ~TLoggerGuard() {
        TLOG_LOG_FLUSH();
        sleep(1);
        TLOG_LOG_SHUTDOWN();
    }
};


COMMON_END_NAMESPACE

#endif //_COMMON_COMMON_H_
