/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       time_util.h
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           2021/4/26
  *Description:
**********************************************************************************/
#ifndef __COMMON_TIME_UTIL__H__
#define __COMMON_TIME_UTIL__H__
#include "common/common.h"
#include <iostream>
#include <thread>

STD_USE_NAMESPACE;
COMMON_BEGIN_NAMESPACE

class ThreadDetachRAII {
    std::thread m_thread;
public:
    ThreadDetachRAII(std::thread &threadObj) : m_thread(std::move(threadObj)) {}
    ~ThreadDetachRAII() {
        if (m_thread.joinable()) {
            m_thread.detach();
        }
    }
};


class TimeUtility
{
public:
    static int64_t CurrentTime();
    static int64_t CurrentTimeInSeconds();
    static int64_t CurrentTimeInMs();
    static int64_t CurrentTimeInMicroSeconds();
    static int64_t GetTime(int64_t usecOffset = 0);
    static timeval GetTimeval(int64_t usecOffset = 0);
    static timespec GetTimespec(int64_t usecOffset = 0);
    static string CurrentTimeInSecondsReadable(time_t secOffset = 0);
};


COMMON_END_NAMESPACE
#endif //__COMMON_TIME_UTIL__H__
