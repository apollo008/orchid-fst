/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       utf8_util.cpp
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           5/12/21
  *Description:    file implements class to resolve utf8 code utility interfaces
**********************************************************************************/
#include "common/util/utf8_util.h"
#include <algorithm>

STD_USE_NAMESPACE;
COMMON_BEGIN_NAMESPACE


void Utf8Util::UTF8Visible(vector<pair<uint8_t,string> >& byte2strVec) {
    if (byte2strVec.empty()) return;
    vector<uint8_t> mayUtf8vec;
    for (int i = byte2strVec.size()-1; i >= 0; --i) {
        mayUtf8vec.push_back(byte2strVec[i].first);
        if ( (byte2strVec[i].first & 0x80) == 0) {
            //ascii
            mayUtf8vec.clear();
            string s;
            s.push_back(byte2strVec[i].first);
            byte2strVec[i].second = s;
            return;
        }
        else if (byte2strVec[i].first >= 0xC0) {
            uint8_t chr = byte2strVec[i].first;
            size_t nBytes = 0;
            if(chr>=0xFC&&chr<=0xFD)
                nBytes=6;
            else if(chr>=0xF8)
                nBytes=5;
            else if(chr>=0xF0)
                nBytes=4;
            else if(chr>=0xE0)
                nBytes=3;
            else if(chr>=0xC0)
                nBytes=2;
            if (nBytes > 0 && mayUtf8vec.size() == nBytes) {
                string s;
                s.reserve(mayUtf8vec.size());
                reverse(mayUtf8vec.begin(),mayUtf8vec.end());
                for (uint8_t u : mayUtf8vec) s.push_back(u);
                for (size_t j = 0; j < mayUtf8vec.size(); ++j) {
                    ostringstream oss;
                    oss << s << "[" << j + 1 <<"]";
                    if (byte2strVec[i+j].second.empty()) {
                        byte2strVec[i+j].second = oss.str();
                    }
                    else {
                        byte2strVec[i+j].second.append(",");
                        byte2strVec[i+j].second.append(oss.str());
                    }
                }
                return;
            }
            else {
                return;
            }
        }
    }
}

void Utf8Util::Bytes2utf8(const vector<uint8_t>& bytes, vector<string>& utf8strVec) {
    string s;
    s.reserve(bytes.size());
    for (uint8_t ch: bytes) {
        s.push_back(ch);
    }
    String2utf8(s,utf8strVec);
}

void Utf8Util::String2utf8(const string& str, vector<string>& utf8strVec) {
    utf8strVec.clear();
    string s;
    uint32_t nByte;
    for (char ch: str) {
        if (IsUtf8Beginning(ch,nByte)) {
            if (!s.empty()) utf8strVec.push_back(s);
            s.clear();
        }
        s.push_back(ch);
    }
    utf8strVec.push_back(s);
}

bool Utf8Util::IsAscii(uint8_t ch) {
    return (ch & 0x80) == 0 ;
}

/*
 *  Unicode符号范围     |        UTF-8编码方式
 *  (十六进制)        |              （二进制）
 *  ----------------------+---------------------------------------------
 *  0000 0000-0000 007F | 0xxxxxxx
 *  0000 0080-0000 07FF | 110xxxxx 10xxxxxx
 *  0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
 *  0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 */
bool Utf8Util::IsUtf8Beginning(uint8_t ch,uint32_t& nByteCnt) {
    if ((ch & 0xFC) == 0xFC) {
        nByteCnt = 6;
        return true;
    }
    else if ((ch &0xF8) == 0xF8) {
        nByteCnt = 5;
        return true;
    }
    else if ((ch &0xF0) == 0xF0) {
        nByteCnt = 4;
        return true;
    }
    else if ((ch &0xE0) == 0xE0) {
        nByteCnt = 3;
        return true;
    }
    else if ((ch &0xC0) == 0xC0) {
        nByteCnt = 2;
        return true;
    }
    else if ((ch &0x80) == 0) {
        nByteCnt = 1;
        return true;
    }
    return false;
}

COMMON_END_NAMESPACE
