/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       string_util.h
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           4/27/21
  *Description:    file defines class to implements string utility interfaces
**********************************************************************************/
#ifndef __COMMON_STRING_UTIL__H__
#define __COMMON_STRING_UTIL__H__
#include "common/common.h"

STD_USE_NAMESPACE;
COMMON_BEGIN_NAMESPACE

class StringUtil {
public:
    ///split a string by separator
    static void Split( const string& text, const string &sep, vector<string>& result, bool ignoreEmpty);
    ///trim space characters at the beginning and ends of a string
    static string TrimString(const string& s);
};

COMMON_END_NAMESPACE
#endif //__COMMON_STRING_UTIL__H__
