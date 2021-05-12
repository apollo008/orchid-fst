/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       fst_core_main.cpp
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           25/3/21
  *Description:    File to implement main entrance for cppfst_fst code
**********************************************************************************/
#include "fst/common.h"
#include "tulip/TLogDefine.h"

STD_USE_NAMESPACE;
CPPFST_FST_USE_NAMESPACE;


int main(int argc, char** argv) {

    // configurate TLog from logger config file 'logger.conf'
    TLOG_CONFIG("logger.conf");

    //declare and setup tlog variable
    TLOG_DECLARE_AND_SETUP_LOGGER(CPPUNITDEMO, MAIN);

    //xxxx
    int v = 1;
    if (v) {
        TLOG_LOG(INFO,"1e2 to int is:[%d]",v);
    }
    else {
        TLOG_LOG(ERROR,"failed to translate '1e2' to int");
    }

    TLOG_LOG_FLUSH();
    TLOG_LOG_SHUTDOWN();
    return 0;
}
