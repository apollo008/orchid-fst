/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       common.h
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           25/3/21
  *Description:    Some macros definitions interface for common use
**********************************************************************************/
#ifndef _CPPFST_FST_COMMON_H_
#define _CPPFST_FST_COMMON_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory>
#include <iostream>
#include <string>

#define CPPFST_FST_BEGIN_NAMESPACE namespace cppfst_fst {
#define CPPFST_FST_END_NAMESPACE }
#define CPPFST_FST_USE_NAMESPACE using namespace cppfst_fst
#define CPPFST_FST_NS cppfst_fst

#define STD_USE_NAMESPACE using namespace std

STD_USE_NAMESPACE;
CPPFST_FST_BEGIN_NAMESPACE


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

CPPFST_FST_END_NAMESPACE
#endif //_CPPFST_FST_COMMON_H_
