/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       output_stream_util.h
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           5/12/21
  *Description:    file defines class to implements output stream utility interfaces
**********************************************************************************/
#ifndef __COMMON_OUTPUT_STREAM_UTIL__H__
#define __COMMON_OUTPUT_STREAM_UTIL__H__
#include "common/common.h"

STD_USE_NAMESPACE;
COMMON_BEGIN_NAMESPACE

/// base class to dump data out
class DataPieceBase {
public:
    DataPieceBase() {}
    virtual ~DataPieceBase() {}
public:
    virtual uint8_t * GetData() = 0;
    virtual size_t GetDataLength() = 0;
    virtual bool Sync() = 0;
};
TYPEDEF_PTR(DataPieceBase);

/// data piece implemented by  memory map technology
class MMapDataPiece : public DataPieceBase {
public:
    MMapDataPiece()
            : fd_(-1),
              pData_ (NULL),
              nLength_ (0)
    {}
    virtual ~MMapDataPiece() { if (pData_ != NULL) { Close(); } }

public:
    virtual uint8_t * GetData() { return pData_; }
    virtual size_t GetDataLength() { return nLength_; }
    virtual bool Sync();
public:
    bool Open(const char * filePath, int mmapProt, int mmapFlags, int fileFlags);
    bool Close();
    bool OpenReadWrite(const char * fileName);
    bool OpenRead(const char * filePath, bool isMapPrivate);
private:
    int             fd_;
    uint8_t*        pData_;
    size_t          nLength_;
private:
    TLOG_DECLARE();
};
TYPEDEF_PTR(MMapDataPiece);

class OutputStreamBase {
public:
    OutputStreamBase(): writenSize_ (0) {}
    virtual ~OutputStreamBase() {}
public:
    size_t GetTotalBytesCnt() { return writenSize_; }
    virtual bool Write(const uint8_t* pData, size_t nSize ) = 0;
    virtual bool WriteAt(size_t offset, const uint8_t *pData, size_t nSize) = 0;
    virtual void Flush() = 0;
protected:
    size_t writenSize_;
private:
    TLOG_DECLARE();
};
TYPEDEF_PTR(OutputStreamBase);

class StdostreamOutputStream : public OutputStreamBase {
public:
    StdostreamOutputStream(std::ostream& os) : os_(os) {}

    virtual bool Write(const uint8_t* pData, size_t nSize );
    virtual bool WriteAt(size_t offset, const uint8_t *pData, size_t nSize);
    virtual void Flush() { os_.flush(); }

    std::ostream&  GetOs() { return os_; }
protected:
    std::ostream&                os_;
private:
    TLOG_DECLARE();
};
TYPEDEF_PTR(StdostreamOutputStream);

class FileOutputStream : public OutputStreamBase {
public:
    FileOutputStream() : OutputStreamBase() , fd_ (-1) { }
    virtual ~FileOutputStream();

    bool Write(const uint8_t *pData, size_t nSize);
    void Flush();
    bool WriteAt(size_t offset, const uint8_t *pData, size_t nSize);

    bool Open(const std::string & filePath);
    void Close();
private:
    int fd_;
private:
    TLOG_DECLARE();
};
TYPEDEF_PTR(FileOutputStream);




COMMON_END_NAMESPACE
#endif //__COMMON_OUTPUT_STREAM_UTIL__H__
