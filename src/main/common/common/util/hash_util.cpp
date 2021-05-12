/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       hash_util.cpp
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           4/27/21
  *Description:    file implements hash function and other method on hash
**********************************************************************************/
#include "common/util/hash_util.h"
#include "common/util/string_util.h"

STD_USE_NAMESPACE;
COMMON_BEGIN_NAMESPACE


uint32_t DBHashImpl::HashStringWithLen(const char* str, uint32_t dwHashType, uint32_t len)
{
    uint32_t vChar;
    uint8_t* ch = (uint8_t*)str;
    uint32_t seed1 = 0x7FED7FED, seed2 = 0xEEEEEEEE;

    for(uint32_t i=0; i<len; i++)
    {
        vChar = *ch++;
        seed1 = DB_CRYPT_TABLE[(dwHashType << 8) + vChar] ^ (seed1 + seed2);
        seed2 = vChar + seed1 + seed2 + (seed2 << 5) + 3;
    }

    return seed1;
}

uint32_t DBHashImpl::HashString(const char* str, uint32_t dwHashType)
{
    uint32_t vChar;
    uint8_t* ch = (uint8_t*)str;
    uint32_t seed1 = 0x7FED7FED, seed2 = 0xEEEEEEEE;

    while (*ch != 0) {
        vChar = *ch++;
        seed1 = DB_CRYPT_TABLE[(dwHashType << 8) + vChar] ^ (seed1 + seed2);
        seed2 = vChar + seed1 + seed2 + (seed2 << 5) + 3;
    }

    return seed1;
}

uint32_t DBHashImpl::HashString(const char* str, size_t len, uint32_t dwHashType)
{
    uint32_t vChar;
    uint8_t* ch = (uint8_t*)str;
    uint32_t seed1 = 0x7FED7FED, seed2 = 0xEEEEEEEE;

    for (size_t i = 0; i < len; ++i) {
        vChar = *ch++;
        seed1 = DB_CRYPT_TABLE[(dwHashType << 8) + vChar] ^ (seed1 + seed2);
        seed2 = vChar + seed1 + seed2 + (seed2 << 5) + 3;
    }

    return seed1;
}

uint64_t DBHashImpl::HashString64WithLen(const char* str, uint32_t len)
{
    uint64_t h = HashStringWithLen(str, 0, len);
    h <<= 32;
    h |= HashStringWithLen(str, 1, len);
    return h;
}

uint64_t DBHashImpl::HashString64(const char* str)
{
    uint64_t h = HashString(str, 0);
    h <<= 32;
    h |= HashString(str, 1);
    return h;
}

uint64_t DBHashImpl::HashString64(const char* str, size_t len)
{
    uint64_t h = HashString(str, len, 0);
    h <<= 32;
    h |= HashString(str, len, 1);
    return h;
}

uint64_t DBHashImpl::HashNumber64(int32_t number) {
    uint64_t nHash;
    if (number < 0) {
        nHash = (uint64_t)(-number);
        nHash = ~nHash & 0x7FFFFFFFFFFFFFFFLL;
    } else {
        nHash = (uint64_t)number;
        nHash |= 0x8000000000000000LL;
    }
    return nHash;
}

COMMON_END_NAMESPACE
