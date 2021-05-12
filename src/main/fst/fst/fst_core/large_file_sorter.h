/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       large_file_sorter.h
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           5/4/21
  *Description:    files defines class to implements sort large file data.
  *                It directory reads all lines of file data and sort it in memory
  *                if line number of input large file is smaller than some specified limit
  *                threshold (such as 1000000 lines), and if line number execceeds the limit,
  *                external disk merge sort will be used:
  *                1. split large file into many small files by hash every line
  *                2. read every small file into memory to sort for per thread and re-output to
  *                   external disk file, use multiple threads.
  *                3. at last merge the sorted disk file content by multiple ways by multiple threads
  *
  *                some parameters such as threadNum,splitFileNum,paralledMergeFileNum and so on can be
  *                customized for different performance.
**********************************************************************************/
#ifndef __CPPFST_FST_CORE_LARGE_FILE_SORTER__H__
#define __CPPFST_FST_CORE_LARGE_FILE_SORTER__H__
#define __CPPFST_FST_CORE_LARGE_FILE_SORTER__H__
#include <atomic>
#include <thread>
#include <mutex>
#include <list>
#include <deque>
#include <cassert>
#include <fstream>
#include "common/common.h"
#include "common/util/hash_util.h"
#include "tulip/TLogDefine.h"
#include "common/util/file_util.h"
#include "common/util/time_util.h"

STD_USE_NAMESPACE;
COMMON_BEGIN_NAMESPACE

///Large file sort main class
class LargeFileSorter {
public:
    class Task {
    public:
        enum TASK_TYPE_ENUM {
            TASK_TYPE_SPLIT = 0,   //split large file to smale files by hash
            TASK_TYPE_SORT,        //sort every split smale file
            TASK_TYPE_MERGE,       //external multiple ways to merge sorted files
            TASK_TYPE_COUNT
        };
        enum TASK_STATE_ENUM {
            TASK_STATE_WAIT_START = 0,
            TASK_STATE_RUNNING,
            TASK_STATE_SUCCESS,
            TASK_STATE_FAILED,
            TASK_STATE_COUNT
        };

        Task() = default;
        Task(const string& file,TASK_TYPE_ENUM type,TASK_STATE_ENUM state)
        : file_(file)
        , taskType_(type)
        , taskState_(state)
        {}
    public:
        void SetTaskFinish(bool success) { taskState_ = (success?TASK_STATE_SUCCESS:TASK_STATE_FAILED); }
        void SetTaskRunning() { taskState_ = TASK_STATE_RUNNING; }
        void SetTaskWaitStart() { taskState_ = TASK_STATE_WAIT_START; }

        bool IsWaitStart() { return Task::TASK_STATE_WAIT_START == taskState_; }
        bool IsFinished() { return Task::TASK_STATE_SUCCESS == taskState_ || Task::TASK_STATE_FAILED == taskState_; }
        bool IsFailed() { return Task::TASK_STATE_FAILED == taskState_; }

        TASK_STATE_ENUM GetTaskState() const { return taskState_; }
        TASK_TYPE_ENUM GetTaskType() const { return taskType_; }
        string GetFile() const { return file_; }

    private:
        string                file_;
        TASK_TYPE_ENUM        taskType_;
        TASK_STATE_ENUM       taskState_;
    };
    TYPEDEF_PTR(Task);
    /**
     *@brief     Construction method for large file sort main class
     *@param     largeFilePath        --- input large file path
     *@param     resultFile           ---- result output file,which will store all sorted file data
     *@param     resultFile           ---- result output file,which will store all sorted file data
     *@param     workDirPath          ---- work directory for all processing
     *@param     threadNum            ---- thread numbers used
     *@param     splitFileNum         ---- how many files of input large file will be split
     *@param     parallelTaskNum      ---- how many files to external merge once
     *@param     isOutputEmptyLine    ---- whether ignore or output the empty line
     *@author    dingbinthu@163.com
     *@date      5/3/21, 12:46 AM
     */
public:
    LargeFileSorter(const string& largeFilePath,
                    const string& resultFile,
                    const string& workDirPath = "/tmp",
                    uint32_t threadNum = 4,
                    uint32_t splitFileNum = 6,
                    uint32_t parallelTaskNum = 3,
                    bool isOutputEmptyLine = true )
                    : largeFilePath_(largeFilePath)
                    , resultFilePath_(resultFile)
                    , workDirPath_(workDirPath)
                    , isOutputEmptyLine_(isOutputEmptyLine)
                    , threadNum_(threadNum)
                    , splitFileNum_(splitFileNum)
                    , parallelTaskNum_(parallelTaskNum)
                    , threadStartedIndex_(0)
                    {
                        inputLineNum_ = abandonLineNum_ = outputLineNum_ = 0;
                        randomTmpDirName_ = TimeUtility::CurrentTimeInSecondsReadable() + "_" +  Random<uint64_t>::RandomString(8);
                    }
    ~LargeFileSorter() {}
    bool Run();
private:
    bool Check();
    bool CheckIfFinishedAllTasks();
    bool DispatchTask(Task::TASK_TYPE_ENUM& gotTaskType, vector<size_t>& gotTaskIds);

    //Step1: split large file into small files
    bool SplitLargeFile(size_t splitTaskId, uint32_t threadId);
    //Step2: sort every small file in memory
    bool SortSplitFile(size_t splitTaskId, uint32_t threadId);
    //Step3: external merge
    bool MergeSortedFile(const vector<size_t>& mergeTaskIds,uint32_t threadId);

    uint32_t CountRunningOrWaitStartTasks();
private:
    string            largeFilePath_;
    string            resultFilePath_;
    string            workDirPath_;
    bool              isOutputEmptyLine_;

    atomic<uint64_t>          inputLineNum_;
    atomic<uint64_t>          abandonLineNum_;
    atomic<uint64_t>          outputLineNum_;

    uint32_t          threadNum_;
    uint32_t          splitFileNum_;
    uint32_t          parallelTaskNum_;

    string            randomTmpDirName_;

    vector<Task>       taskList_;
    deque<uint32_t>    freeThreadIdsList_;
    mutex              mutex_;
    atomic<uint32_t>   threadStartedIndex_;

private:
    TLOG_DECLARE();
};

COMMON_END_NAMESPACE

#endif //__CPPFST_FST_CORE_LARGE_FILE_SORTER__H__
