/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       output_stream_util.cpp
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           5/12/21
  *Description:    file implements class to implements output stream utility interfaces
**********************************************************************************/
#include "common/util/output_stream_util.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <cerrno>
#include <ctype.h>
#include <sys/mman.h>

STD_USE_NAMESPACE;
COMMON_BEGIN_NAMESPACE

TLOG_SETUP(COMMON_NS,MMapDataPiece);
TLOG_SETUP(COMMON_NS,OutputStreamBase);
TLOG_SETUP(COMMON_NS,FileOutputStream);
TLOG_SETUP(COMMON_NS,StdostreamOutputStream);

bool StdostreamOutputStream::Write(const uint8_t *pData, size_t nSize) {
    if(nSize <= 0)
        return true;
    os_.write((const char*)pData,nSize);
    writenSize_ += nSize;
    return true;
}

bool StdostreamOutputStream::WriteAt(size_t offset, const uint8_t *pData, size_t nSize) {
    os_.seekp(offset,iostream::beg);
    bool ret = Write(pData, nSize);
    os_.seekp(0,iostream::end);
    return ret;
}

bool MMapDataPiece::Sync() {
    return msync(pData_ , nLength_, MS_SYNC) == 0;
}

bool MMapDataPiece::OpenRead(const char * filePath, bool isMapPrivate) {
    return Open(filePath, PROT_READ, isMapPrivate ? MAP_PRIVATE : MAP_SHARED, O_RDONLY);
}

bool MMapDataPiece::OpenReadWrite(const char * filePath) {
    return Open(filePath, PROT_READ | PROT_WRITE, MAP_SHARED , O_RDWR);
}

bool MMapDataPiece::Open(const char * filePath, int mmapProt, int mmapFlags, int fileFlags) {
    fd_ = open(filePath, fileFlags);
    if (fd_ < 0) {
        TLOG_LOG(ERROR, "open call failed, file:[%s], errno:[%d]", filePath, errno);
        return false;
    }
    nLength_ = lseek(fd_, 0, SEEK_END);
    lseek(fd_, 0, SEEK_SET);
    pData_ = (uint8_t*)mmap(0, nLength_, mmapProt, mmapFlags, fd_, 0);
    if (pData_ == MAP_FAILED) {
        TLOG_LOG(ERROR, "mmap call failed, file:[%s], length:[%zu], errno:[%d]", filePath, nLength_, errno);
        return false;
    }
    return true;
}

bool MMapDataPiece::Close() {
    if (munmap(pData_, nLength_) || close(fd_)) {
        return false;
    }
    pData_ = nullptr;
    nLength_ = 0;
    fd_ = -1;
    return true;
}

FileOutputStream::~FileOutputStream() {
    if (fd_ != -1) {
        Close();
    }
}

bool FileOutputStream::Open(const std::string & filePath)
{
    fd_ = open(filePath.c_str(), O_RDWR|O_CREAT, 00644);
    return fd_ >= 0;
}

void FileOutputStream::Close()
{
    close(fd_);
    fd_ = -1;
}


bool FileOutputStream::Write(const uint8_t *pData, size_t nSize)
{
    assert(fd_ != -1);
    if(nSize <= 0)
        return true;
    size_t dumpSize = nSize;
    size_t sumSize = 0;
    uint8_t * pTmpBuf =(uint8_t*) pData;
    while(dumpSize > 0) {
        sumSize = write(fd_, pTmpBuf, dumpSize);
        if(sumSize < 0)
            return false;
        dumpSize -= sumSize;
        pTmpBuf += sumSize;
    }
    writenSize_ += nSize;
    return true;
}

void FileOutputStream::Flush() {
    fsync(fd_);
}

bool FileOutputStream::WriteAt(size_t offset, const uint8_t *pData, size_t nSize) {
    lseek(fd_, offset, SEEK_SET);
    bool ret = Write(pData, nSize);
    lseek(fd_, 0, SEEK_END);
    return ret;
}


COMMON_END_NAMESPACE
