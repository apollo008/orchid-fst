/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       string_util.cpp
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           4/27/21
  *Description:    file implements class to implements string utility interfaces
**********************************************************************************/
#include "common/util/string_util.h"

STD_USE_NAMESPACE;
COMMON_BEGIN_NAMESPACE

void StringUtil::Split( const string& text, const string &sep, vector<string>& result, bool ignoreEmpty) {
    result.clear();

    string separator(sep);
    string str(text);

    size_t pos = 0, last = 0;
    while (pos != string::npos) {
        pos = str.find(separator,pos);
        if (pos != string::npos) {
            if (!ignoreEmpty || pos != last)
                result.push_back(str.substr(last, pos-last));
            pos += separator.length();
            last = pos;
        }
    }
    if (!ignoreEmpty || last < str.length()) {
        result.push_back(str.substr(last, str.length() - last));
    }
}



string
StringUtil::TrimString(const string& str) {
    if (str.empty()) return str;
    static const char* whiteSpace = " \t\r\n";
    string::size_type st = str.find_first_not_of(whiteSpace);
    if (st == string::npos) return "";
    string::size_type ed = str.find_last_not_of(whiteSpace);
    return string(str, st, ed - st + 1);
}

COMMON_END_NAMESPACE
