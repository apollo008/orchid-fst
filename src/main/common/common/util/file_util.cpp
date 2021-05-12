/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       file_util.cpp
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           2021/4/26
  *Description:    file implements class to implements file utility interfaces
**********************************************************************************/
#include "common/util/file_util.h"
#include <sys/stat.h>
#include <dirent.h>
#include <fstream>

STD_USE_NAMESPACE;
COMMON_BEGIN_NAMESPACE

TLOG_SETUP(COMMON_NS,FileUtility);

uint64_t FileUtility::GetFileLineNumber(const string& file) {
    ifstream ifs(file);
    if (!ifs) return 0;
    char ch;
    uint64_t count = 0;
    while (ifs.get(ch)) {
        if (ch == '\n')
            ++count;
    }
    return count;
}

bool FileUtility::IsFileExists(const std::string& file){
    return access(file.c_str(), F_OK) == 0;
}

bool FileUtility::IsDirExists(const std::string& dir){
    struct stat st;
    if (stat(dir.c_str(), &st) != 0) { return false; }
    if (!S_ISDIR(st.st_mode)) { return false; }
    return true;
}

bool FileUtility::RecurMakeLocalDir(const string & dirPath ) {
    if (dirPath.empty()) {
        return false;
    }
    if (IsDirExists(dirPath)) {
        return true;
    }
    string curDir = dirPath;
    string parentDir = GetParentDir(curDir);
    if (!parentDir.empty() && !IsDirExists(parentDir)) {
        if (!RecurMakeLocalDir(parentDir)) {
            TLOG_LOG(ERROR, "recursive make local dir error, dir:[%s]", parentDir.c_str());
            return false;
        }
    }
    if (mkdir(curDir.c_str(), S_IRWXU) != 0) {
        TLOG_LOG(ERROR, "mkdir error, dir:[%s], errno:%d", curDir.c_str(), errno);
        return false;
    }
    return true;
}

std::string FileUtility::GetParentDir(const std::string& dir) {
    if (dir.empty()) return "";

    size_t delimPos = string::npos;
    if (DIR_SEPARATOR == *(dir.rbegin())) {
        delimPos = dir.rfind(DIR_SEPARATOR, dir.size() - 2);
    } else {
        delimPos = dir.rfind(DIR_SEPARATOR);
    }
    if (string::npos == delimPos) return "";
    return dir.substr(0, delimPos);
}

bool FileUtility::MakeLocalDir(const string & dirPath, bool bRecursive) {
    if (bRecursive) {
        return RecurMakeLocalDir(dirPath);
    }
    if (IsDirExists(dirPath)) {
        TLOG_LOG(ERROR, "the directory[%s] is already exist", dirPath.c_str());
        return false;
    }
    if (mkdir(dirPath.c_str(), S_IRWXU)) {
        TLOG_LOG(ERROR, "mkdir call error, dir:[%s], errno:%d", dirPath.c_str(), errno);
        return false;
    }
    return true;
}

bool FileUtility::DeleteLocalFile(const std::string & dirPath) {
    return (0 == unlink(dirPath.c_str()));
}


bool FileUtility::RemoveLocalDir(const std::string & dirPath, bool bRemoveNoneEmptyDir)
{
    if (!FileUtility::IsDirExists(dirPath)) {
        TLOG_LOG(ERROR, "the dir:[%s] is not exist", dirPath.c_str());
        return false;
    }
    if (bRemoveNoneEmptyDir) {
        return RecurRemoveLocalDir(dirPath);
    } else if (rmdir(dirPath.c_str()) != 0) {
        TLOG_LOG(ERROR, "dirPath [%s] may not an empty dir", dirPath.c_str());
        return false;
    }
    return true;
}

bool FileUtility::RecurRemoveLocalDir(const std::string & dirPath){
    DIR *dir = NULL;
    struct dirent *dp = NULL;

    if ((dir = opendir(dirPath.c_str())) == NULL) {
        TLOG_LOG(ERROR, "opendir error, dirPath:[%s]", dirPath.c_str());
        return false;
    }

    bool ret = false;
    while((dp = readdir(dir)) != NULL) {
        if (dp->d_type & DT_DIR) {
            if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
                ret = true;
                continue;
            } else {
                ret = RecurRemoveLocalDir(dirPath + DIR_SEPARATOR + string(dp->d_name));
            }
        } else {
            ret = DeleteLocalFile(dirPath + DIR_SEPARATOR + string(dp->d_name));
        }
        if (!ret) {
            break;
        }
    }
    closedir(dir);

    if (ret && rmdir(dirPath.c_str()) != 0) {
        TLOG_LOG(ERROR, "rmdir call error, dirPath [%s]", dirPath.c_str());
        return false;
    }
    return ret ;
}

COMMON_END_NAMESPACE
