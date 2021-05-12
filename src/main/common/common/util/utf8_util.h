/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       utf8_util.h
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           5/2/21
  *Description:    file defines class to resolve utf8 code utility interfaces
**********************************************************************************/
#ifndef __COMMON_UTF8_UTIL__H__
#define __COMMON_UTF8_UTIL__H__
#include "common/common.h"
#include <iostream>

STD_USE_NAMESPACE;
COMMON_BEGIN_NAMESPACE

/**
 *@brief     class to convert for utf8 code convert
 *           UTF8是以8bits即1Bytes为编码的最基本单位，当然也可以有基于16bits和32bits的形式，
 *           分别称为UTF16和UTF32，但目前用得不多，而UTF8则被广泛应用在文件储存和网络传输中
 *   　       UCS-4 range (hex.) UTF-8 octet sequence (binary)
 *   　　     0000 0000-0000 007F            0xxxxxxx
 *   　　     0000 0080-0000 07FF            110xxxxx 10xxxxxx
 *   　　     0000 0800-0000 FFFF            1110xxxx 10xxxxxx 10xxxxxx
 *   　　     0001 0000-001F FFFF            11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 *   　　     0020 0000-03FF FFFF            111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
 *   　　     0400 0000-7FFF FFFF            1111110x 10xxxxxx ... 10xxxxxx
 *   　　     编码步骤：
 *   　　     1) 首先确定需要多少个8bits(octets)
 *   　　     2) 按照上述模板填充每个octets的高位bits
 *   　　     3) 把字符的bits填充至x中，字符顺序：低位→高位，UTF8顺序：最后一个octet的最末位x→第一个octet最高位x
 *   　　     根据UTF8编码,最多可由6个字节组成,所以UTF8是1-6字节编码组成
 */
class Utf8Util {
public:
    ///judge whether is an ascii code for an byte
    static bool IsAscii(uint8_t ch);
    ///judge whether stars with a valid utf8 byte for a byte sequence
    static bool IsUtf8Beginning(uint8_t ch,uint32_t& nByteCnt);
    ///convert a string into utf8 represents
    static void String2utf8(const string& str, vector<string>& utf8strVec);
    ///convert a byte sequence into  utf8 encoding string
    static void Bytes2utf8(const vector<uint8_t>& bytes, vector<string>& utf8strVec);
    ///only used fst dot to decide how a byte sequence for a utf8 string
    static void UTF8Visible(vector<pair<uint8_t,string> >& byte2strVec);
};
COMMON_END_NAMESPACE
#endif //__COMMON_UTF8_UTIL__H__
