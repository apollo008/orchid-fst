/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       time_util.cpp
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           2021/4/26
  *Description:
**********************************************************************************/
#include <sys/time.h>
#include "common/util/time_util.h"

STD_USE_NAMESPACE;
COMMON_BEGIN_NAMESPACE

int64_t TimeUtility::CurrentTime() {
    struct timeval tval;
    gettimeofday(&tval, NULL);
    return (tval.tv_sec * 1000000LL + tval.tv_usec);
}

int64_t TimeUtility::CurrentTimeInSeconds() {
    return CurrentTime() / 1000000;
}

int64_t TimeUtility::CurrentTimeInMs(){
    return CurrentTime() / 1000;
}
int64_t TimeUtility::CurrentTimeInMicroSeconds() {
    return CurrentTime();
}

int64_t TimeUtility::GetTime(int64_t usecOffset) {
    return CurrentTime() + usecOffset;
}

timeval TimeUtility::GetTimeval(int64_t usecOffset) {
    timeval tval;
    int64_t uTime = GetTime(usecOffset);
    tval.tv_sec = uTime / 1000000;
    tval.tv_usec = uTime % 1000000;
    return tval;
}

timespec TimeUtility::GetTimespec(int64_t usecOffset) {
    timespec tspec;
    int64_t uTime = GetTime(usecOffset);
    tspec.tv_sec = uTime / 1000000;
    tspec.tv_nsec = (uTime % 1000000) * 1000;
    return tspec;
}

string TimeUtility::CurrentTimeInSecondsReadable(time_t secOffset){
    tm tmNow;
    time_t tNow = time(NULL);
    tNow -= secOffset;
    localtime_r(&tNow, &tmNow);

    char szNow[32];
    sprintf(szNow,
            "%04d%02d%02d%02d%02d%02d",
            tmNow.tm_year + 1900,
            tmNow.tm_mon + 1,
            tmNow.tm_mday,
            tmNow.tm_hour,
            tmNow.tm_min,
            tmNow.tm_sec);
    return string(szNow);
}

COMMON_END_NAMESPACE
