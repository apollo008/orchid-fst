/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       large_file_sorter.cpp
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           5/4/21
  *Description:    files implements class to implements sort large file data.
**********************************************************************************/
#include "fst/fst_core/large_file_sorter.h"
#include "common/util/hash_util.h"
#include "common/util/string_util.h"
#include "common/util/time_util.h"
#include <chrono>
#include <fstream>
#include <cassert>
#include <algorithm>
#include <map>

STD_USE_NAMESPACE;
COMMON_BEGIN_NAMESPACE

TLOG_SETUP(COMMON_NS,LargeFileSorter);

/// remove randomed generated work sub directory at last, USE RAII idea
class RandomDirectoryGenAndRemoveRAII {
public:
    RandomDirectoryGenAndRemoveRAII(const string& dir)
    : dir_(dir)
    {
        bool ret = FileUtility::MakeLocalDir(dir_);
        assert(ret);
    }
    ~RandomDirectoryGenAndRemoveRAII(){
        bool ret = FileUtility::RemoveLocalDir(dir_,true);
//        assert(ret);
    }
private:
    string      dir_;
};

//Main process method for sort large file
bool LargeFileSorter::Run() {
    uint64_t bTime = TimeUtility::CurrentTimeInMs();
    if (!Check()) return false;
    RandomDirectoryGenAndRemoveRAII randomDirectoryGenAndRemoveRaii(workDirPath_ + "/" + randomTmpDirName_);
    TLOG_LOG(INFO,"Begin to sort large file under random generated directory:[%s/%s] for input file:[%s]", workDirPath_.c_str(),randomTmpDirName_.c_str(), largeFilePath_.c_str())

    for (uint32_t t = 1; t <= threadNum_; ++t) {
        freeThreadIdsList_.push_back(t);
    }

    //check whole line count of input large file
    uint64_t inputFileLineNum = FileUtility::GetFileLineNumber(largeFilePath_);
    TLOG_LOG(DEBUG,"totally [%lu] lines for input file:[%s]",inputFileLineNum,largeFilePath_.c_str());

    //if line count smaller just sort it in memory
    if (inputFileLineNum < (uint64_t)1e6) {
        //small line count for input file, do not split
        taskList_.emplace_back(largeFilePath_,Task::TASK_TYPE_SORT,Task::TASK_STATE_WAIT_START);
        inputLineNum_ = inputFileLineNum;
    }
    //more lines, use external sort
    else {
        taskList_.emplace_back(largeFilePath_,Task::TASK_TYPE_SPLIT,Task::TASK_STATE_WAIT_START);
    }

    uint64_t lastPrintWaitTime = TimeUtility::CurrentTimeInMs();
    while (!CheckIfFinishedAllTasks()) {
        Task::TASK_TYPE_ENUM gotTaskType;
        vector<size_t> gotTaskIds;
        //try to got available tasks
        bool ret = DispatchTask(gotTaskType,gotTaskIds);
        if (!gotTaskIds.empty()) {
            assert(ret);
            {
                mutex_.lock();
                //try got available thread
                if (!freeThreadIdsList_.empty()) {
                    uint32_t tid = freeThreadIdsList_.front();
                    freeThreadIdsList_.pop_front();
                    mutex_.unlock();

                    switch (gotTaskType) {
                        //split large file to small files
                        case Task::TASK_TYPE_SPLIT:
                        {
                            uint32_t lastThreadIndex = threadStartedIndex_.load();
                            thread t([&,tid,gotTaskIds]() {
                                threadStartedIndex_.fetch_add(1);
                                SplitLargeFile(gotTaskIds[0],tid);
                            });
                            ThreadDetachRAII threadRaii(t);
                            while (threadStartedIndex_.load() <= lastThreadIndex) {
                                this_thread::sleep_for(std::chrono::microseconds (10));
                            }
                            break;
                        }
                        //sort every small files
                        case Task::TASK_TYPE_SORT:
                        {
                            uint32_t lastThreadIndex = threadStartedIndex_.load();
                            thread t([&,tid,gotTaskIds]() {
                                threadStartedIndex_.fetch_add(1);
                                SortSplitFile(gotTaskIds[0],tid);
                            });
                            ThreadDetachRAII threadRaii(t);
                            while (threadStartedIndex_.load() <= lastThreadIndex) {
                                this_thread::sleep_for(std::chrono::microseconds (10));
                            }
                            break;
                        }
                        //external merge sorted files
                        case Task::TASK_TYPE_MERGE:
                        {
                            uint32_t lastThreadIndex = threadStartedIndex_.load();
                            thread t([&,tid,gotTaskIds]() {
                                threadStartedIndex_.fetch_add(1);
                                MergeSortedFile(gotTaskIds,tid);
                            });
                            ThreadDetachRAII  threadRaii(t);
                            while (threadStartedIndex_.load() <= lastThreadIndex) {
                                this_thread::sleep_for(std::chrono::microseconds (10));
                            }
                            break;
                        }
                        default:
                            break;
                    }
                }
                else {
                    mutex_.unlock();
                    //no thread and wait
                    uint64_t curPrintWaitTime = TimeUtility::CurrentTimeInMs();
                    if (curPrintWaitTime - lastPrintWaitTime >= 5000) {
                        TLOG_LOG(INFO,"waiting available thread...")
                        lastPrintWaitTime = curPrintWaitTime;
                    }
                    this_thread::sleep_for(std::chrono::microseconds (10));
                }
            }
        }
        else {
            //no task and also wait
            uint64_t curPrintWaitTime = TimeUtility::CurrentTimeInMs();
            if (curPrintWaitTime - lastPrintWaitTime >= 5000) {
                TLOG_LOG(INFO,"waiting available task...")
                lastPrintWaitTime = curPrintWaitTime;
            }
            this_thread::sleep_for(std::chrono::microseconds (10));
        }
    }
    uint64_t eTime = TimeUtility::CurrentTimeInMs();
    TLOG_LOG(INFO,"Totally consumed:[%lu]ms on [%lu] lines read,abandon:[%lu] empty lines, for large file:[%s],result output file is:[%s].",
             (eTime-bTime),inputLineNum_.load(),abandonLineNum_.load(),largeFilePath_.c_str(), resultFilePath_.c_str());
    return true;
}

bool LargeFileSorter::DispatchTask(Task::TASK_TYPE_ENUM& gotTaskType, vector<size_t>& gotTaskIds) {
    do {
        {
            lock_guard lockGuard(mutex_);
            gotTaskIds.clear();
            vector<size_t> freeTasks;
            for (size_t i = 0; i < taskList_.size(); ++i) {
                if (taskList_[i].IsWaitStart()) {
                    freeTasks.push_back(i);
                }
            }
            if (freeTasks.empty()) {
                break;
            }
            gotTaskType = taskList_[freeTasks[0] ].GetTaskType();
            if (gotTaskType == Task::TASK_TYPE_SPLIT) {
                //only handle in only one thread
                taskList_[freeTasks[0] ].SetTaskRunning();
                gotTaskIds.push_back(freeTasks[0]);
            }
            else if (gotTaskType == Task::TASK_TYPE_SORT) {
                //only handle in only one thread
                taskList_[freeTasks[0] ].SetTaskRunning();
                gotTaskIds.push_back(freeTasks[0]);
            }
            else if (gotTaskType == Task::TASK_TYPE_MERGE) {
                for (size_t i = 0; i < std::min<uint32_t>(parallelTaskNum_,freeTasks.size()); ++i) {
                    gotTaskIds.push_back(freeTasks[i]);
                    taskList_[freeTasks[i] ].SetTaskRunning();
                    assert(taskList_[freeTasks[i] ].GetTaskType() == gotTaskType);
                }
            }
        }
    } while (false);
    return !gotTaskIds.empty();
}

bool LargeFileSorter::SortSplitFile(size_t sortTaskId,uint32_t threadId) {
    Task sortTask;
    {
        lock_guard lockGuard(mutex_);
        sortTask = taskList_[sortTaskId];
    }
    uint64_t bTime = TimeUtility::CurrentTimeInMs();
    TLOG_LOG(DEBUG,"begin to sort split file:[%s] in thread %u",sortTask.GetFile().c_str(),threadId);
    string splitFile = sortTask.GetFile();
    ifstream ifs(splitFile);
    if (!ifs) {
        TLOG_LOG(ERROR, "Error!!! Failed to open split file:[%s] to sort", splitFile.c_str());
        {
            lock_guard lockGuard(mutex_);
            taskList_[sortTaskId].SetTaskFinish(false);
            freeThreadIdsList_.push_back(threadId);
        }
        return false;
    }
    string outputDir = workDirPath_ + "/" + randomTmpDirName_ + "/";
    string outputFile = outputDir + TimeUtility::CurrentTimeInSecondsReadable() + "_" + Random<uint64_t>::RandomString(20);
    assert(!FileUtility::IsFileExists(outputFile));
    ofstream  ofs(outputFile);
    if (!ofs) {
        TLOG_LOG(ERROR,"Failed to open intermediate result file:[%s]",outputFile.c_str());
        {
            lock_guard lockGuard(mutex_);
            taskList_[sortTaskId].SetTaskFinish(false);
            freeThreadIdsList_.push_back(threadId);
        }
        return false;
    }
    multimap<string,string> tmpmap;
    string line,trimedLine;
    uint64_t handleLineNum = 0;
    while (getline(ifs,line)) {
        trimedLine = StringUtil::TrimString(line);
        if (!isOutputEmptyLine_ && trimedLine.empty()) continue;
        tmpmap.insert(std::make_pair(trimedLine,line));
        ++handleLineNum;
    }
    assert(tmpmap.size() == handleLineNum);
    for (auto& p: tmpmap) {
        ofs << p.second << endl;
    }
    ofs.flush();
    ofs.close();
    {
        lock_guard lockGuard(mutex_);
        taskList_.emplace_back(
                outputFile,
                Task::TASK_TYPE_MERGE,
                Task::TASK_STATE_WAIT_START
                );
        taskList_[sortTaskId].SetTaskFinish(true);
        freeThreadIdsList_.push_back(threadId);
    }
    uint64_t eTime = TimeUtility::CurrentTimeInMs();
    TLOG_LOG(DEBUG,"Totally consumed:[%lu]ms,Finished sort split file:[%s] with [%lu] line handled in thread %u.",
             (eTime-bTime),splitFile.c_str(),handleLineNum,threadId);
    return true;
}

bool LargeFileSorter::Check() {
    assert(threadNum_ > 0);
    assert(splitFileNum_ > 0);
    assert(parallelTaskNum_ >= 2);
    if (!FileUtility::IsFileExists(largeFilePath_)) {
        TLOG_LOG(ERROR,"large file:[%s] does not exist!", largeFilePath_.c_str());
        return false;
    }
    if (!FileUtility::IsDirExists(workDirPath_)) {
        TLOG_LOG(ERROR,"workDir directory:[%s] does not exist!", workDirPath_.c_str());
        return false;
    }

    if (FileUtility::IsDirExists(resultFilePath_)) {
        TLOG_LOG(ERROR,"result file:[%s] is a directory,please check!", resultFilePath_.c_str());
        return false;
    }
    if (FileUtility::IsFileExists(resultFilePath_)) {
        TLOG_LOG(ERROR,"result file:[%s] already exist,please check!", resultFilePath_.c_str());
        return false;
    }
    return true;
}

bool LargeFileSorter::SplitLargeFile(size_t splitTaskId,uint32_t threadId) {
    uint64_t bTime = TimeUtility::CurrentTimeInMs();
    Task splitTask ;
    {
        lock_guard lockGuard(mutex_);
        splitTask = taskList_[splitTaskId];
    }
    TLOG_LOG(DEBUG,"begin to split large file:[%s] in thread %u",splitTask.GetFile().c_str(),threadId);
    string largeFile = splitTask.GetFile();
    ifstream ifs(largeFile);
    if (!ifs) {
        TLOG_LOG(ERROR, "Error!!! Failed to open large file:[%s]", largeFile.c_str());
        {
            lock_guard lockGuard(mutex_);
            taskList_[splitTaskId].SetTaskFinish(false);
            freeThreadIdsList_.push_back(threadId);
        }
        return false;
    }
    string outputDir = workDirPath_ + "/" + randomTmpDirName_ + "/";
    vector<std::shared_ptr<ofstream> > ofsVecs;
    vector<uint64_t> linesNumVecs;
    vector<string> splitFileNamesVec;
    for (uint32_t fileId = 0; fileId < splitFileNum_; ++fileId) {
        ostringstream oss;
        oss << fileId;
        string splitFile = outputDir + oss.str();
        assert(!FileUtility::IsFileExists(splitFile));
        shared_ptr<ofstream> ofs = std::make_shared<ofstream>(splitFile);
        if (!ofs->operator bool()) {
            TLOG_LOG(ERROR, "Error!!! Failed to open split file:[%s]", splitFile.c_str());
            {
                lock_guard lockGuard(mutex_);
                taskList_[splitTaskId].SetTaskFinish(false);
                freeThreadIdsList_.push_back(threadId);
            }
            return false;
        }
        ofsVecs.push_back(ofs);
        linesNumVecs.push_back(0);
        splitFileNamesVec.push_back(splitFile);
    }
    string line,trimedLine;
    while (getline(ifs, line)) {
        inputLineNum_.fetch_add(1);
        trimedLine = StringUtil::TrimString(line);
        if (!isOutputEmptyLine_ && trimedLine.empty()) {
            abandonLineNum_.fetch_add(1);
            continue;
        }
        uint64_t hash = DBKeyHash<string>()(trimedLine);
        uint32_t fileId = hash % splitFileNum_;
        (*ofsVecs[fileId]) << line << endl;
        linesNumVecs[fileId]++;
    }
    for (auto &a: ofsVecs) {
        a->flush();
        a->close();
    }
    TLOG_LOG(DEBUG, "Totally read:[%lu] lines,abandon:[%lu] empty lines with ratio:[%lf].", inputLineNum_.load(),
             abandonLineNum_.load(),
             ((inputLineNum_ == 0) ? 0 : (1.0f * abandonLineNum_ / inputLineNum_)));

    {
        lock_guard lockGuard(mutex_);
        for (uint32_t fileId = 0; fileId < ofsVecs.size(); ++fileId) {
            TLOG_LOG(DEBUG, "[%lu] lines outputted for split file:[%s]", linesNumVecs[fileId], splitFileNamesVec[fileId].c_str());
            if (linesNumVecs[fileId] > 0) {
                taskList_.emplace_back(splitFileNamesVec[fileId],
                                       Task::TASK_TYPE_SORT,
                                       Task::TASK_STATE_WAIT_START);
            }
            else {
                FileUtility::DeleteLocalFile(splitFileNamesVec[fileId]);
            }
        }
        taskList_[splitTaskId].SetTaskFinish(true);
        freeThreadIdsList_.push_back(threadId);
    }
    uint64_t eTime = TimeUtility::CurrentTimeInMs();
    TLOG_LOG(DEBUG,"Totally consumed:[%lu]ms,Finished split large file task for:[%s] in thread %u.",
             (eTime-bTime),splitTask.GetFile().c_str(),threadId);
    return true;
}

struct SortedFileLineScanner {
    SortedFileLineScanner(shared_ptr<ifstream> ifs,const string& file,const string& curLine, const string& trimedLine)
    : ifs_(ifs)
    , file_(file)
    , curLine_(curLine)
    , trimedLine_(trimedLine)
    {
    }

    shared_ptr<ifstream>     ifs_;
    string                   file_;
    string                   curLine_;
    string                   trimedLine_;
};
TYPEDEF_PTR(SortedFileLineScanner);

class SortedFileLineScannerPtrCompare {
public:
    bool operator()(const SortedFileLineScannerPtr& lhs,const SortedFileLineScannerPtr& rhs) const {
        return lhs->trimedLine_ > rhs->trimedLine_;
    }
};

bool LargeFileSorter::MergeSortedFile(const vector<size_t>& mergeTaskIds,uint32_t threadId) {
    uint64_t bTime = TimeUtility::CurrentTimeInMs();
    string fileNames;
    {
        lock_guard lockGuard(mutex_);
        for (size_t taskId: mergeTaskIds) {
            fileNames += taskList_[taskId].GetFile();
            fileNames += "\n";
        }
    }
    TLOG_LOG(DEBUG,"begin to merge [%zu] sorted files[%s]: in thread %u",mergeTaskIds.size(),fileNames.c_str(),threadId);
    if (mergeTaskIds.size() == 1) {
        {
            bool allTaskFinished = true;
            {
                lock_guard lockGuard(mutex_);
                for (size_t i = 0; i < taskList_.size(); ++i) {
                    if (i == mergeTaskIds[0]) continue;
                    if (!taskList_[i].IsFinished()) {
                        allTaskFinished = false;
                        break;
                    }
                }
            }
            if (allTaskFinished) {
                {
                    lock_guard lockGuard(mutex_);
                    string srcFileName = taskList_[mergeTaskIds[0] ].GetFile();

                    //transfer srcFileName to resultFilePath_
                    std::ifstream in(srcFileName, std::ios::in | std::ios::binary);
                    std::ofstream out(resultFilePath_, std::ios::out | std::ios::binary);
                    out << in.rdbuf();
                    out.flush();
                    out.close();
                    in.close();

                    taskList_[mergeTaskIds[0] ].SetTaskFinish(true);
                    freeThreadIdsList_.push_back(threadId);
                }
                uint64_t eTime = TimeUtility::CurrentTimeInMs();
                TLOG_LOG(DEBUG,"Only 1 files need to merge.Copy it as final result file:[%s],totally consumed:[%lu]ms in thread %u.",
                         resultFilePath_.c_str(), (eTime-bTime),threadId);
                return true;
            }
            else {
                {
                    lock_guard lockGuard(mutex_);
                    taskList_[mergeTaskIds[0] ].SetTaskWaitStart();
                    freeThreadIdsList_.push_back(threadId);
                }
                uint64_t eTime = TimeUtility::CurrentTimeInMs();
                TLOG_LOG(DEBUG,"Only 1 merge task,wait other merger together,totally consumed:[%lu]ms in thread %u.",
                         (eTime-bTime),threadId);
                return true;
            }
        }
    }

    string outputDir = workDirPath_ + "/" + randomTmpDirName_ + "/";
    string outputFile = outputDir + TimeUtility::CurrentTimeInSecondsReadable() + "_" +  Random<uint64_t>::RandomString(20);
    assert(!FileUtility::IsFileExists(outputFile));
    ofstream  ofs(outputFile);
    if (!ofs) {
        TLOG_LOG(ERROR,"Failed to open intermediate result file:[%s]",outputFile.c_str());
        {
            lock_guard lockGuard(mutex_);
            for (auto id: mergeTaskIds) {
                taskList_[id].SetTaskFinish(false);
            }
            freeThreadIdsList_.push_back(threadId);
        }
        return false;
    }
    vector<SortedFileLineScannerPtr> sortScanners;
    {
        lock_guard lockGuard(mutex_);
        for(auto id : mergeTaskIds) {
            shared_ptr<ifstream> tmpifs = std::make_shared<ifstream>(taskList_[id].GetFile());
            assert(tmpifs);
            string curLine;
            assert(getline(*tmpifs.get(),curLine));
            sortScanners.push_back(std::make_shared<SortedFileLineScanner>(
                    tmpifs,
                    taskList_[id].GetFile(),
                    curLine,
                    StringUtil::TrimString(curLine)
            ));
        }
    }
    //use heap to merge sort files
    make_heap(sortScanners.begin(),sortScanners.end(),SortedFileLineScannerPtrCompare{});
    string tmpCurLine, tmpOutputLine;
    uint64_t handleLineNum = 0;
    while (!sortScanners.empty()) {
        ++handleLineNum;
        pop_heap(sortScanners.begin(),sortScanners.end(),SortedFileLineScannerPtrCompare{});
        SortedFileLineScannerPtr minElem = sortScanners.back();
        sortScanners.pop_back();

        if (isOutputEmptyLine_ || !minElem->trimedLine_.empty()) {
            ofs << minElem->curLine_ << endl;
        }
        if (getline(*(minElem->ifs_),tmpCurLine)) {
            minElem->curLine_ = tmpCurLine;
            minElem->trimedLine_ = StringUtil::TrimString(tmpCurLine);
            sortScanners.push_back(minElem);
            push_heap(sortScanners.begin(),sortScanners.end(),SortedFileLineScannerPtrCompare{});
        }
    }
    ofs.flush();
    ofs.close();
    {
        lock_guard lockGuard(mutex_);
        taskList_.emplace_back(
                outputFile,Task::TASK_TYPE_MERGE,Task::TASK_STATE_WAIT_START );
        for (auto id: mergeTaskIds) {
            taskList_[id].SetTaskFinish(true);
        }
        freeThreadIdsList_.push_back(threadId);
    }
    uint64_t eTime = TimeUtility::CurrentTimeInMs();
    TLOG_LOG(DEBUG,"Totally consumed:[%lu]ms,Finished merge sorted files lines:[%lu] to intermediate file:[%s] in thread %u.",
             (eTime-bTime),handleLineNum,outputFile.c_str(),threadId);
    return true;
}

uint32_t LargeFileSorter::CountRunningOrWaitStartTasks() {
    uint32_t count = 0;
    for (auto& task: taskList_) {
        if (task.GetTaskState() == Task::TASK_STATE_WAIT_START
        || task.GetTaskState() == Task::TASK_STATE_RUNNING)
            ++count;
    }
    return count;
}

bool LargeFileSorter::CheckIfFinishedAllTasks() {
    bool allTaskFinished = true;
    bool hasFailedTask = false;

    lock_guard lockGuard(mutex_);
    for (auto& task: taskList_) {
        if (!hasFailedTask && task.IsFailed()) {
            hasFailedTask = true;
        }
        if (!task.IsFinished()) {
            allTaskFinished = false;
            break;
        }
    }
    if (allTaskFinished) {
        if (hasFailedTask) {
            TLOG_LOG(INFO,"All works done but failed to sort large file:[%s]!",largeFilePath_.c_str());
            return false;
        }
        else {
            TLOG_LOG(DEBUG,"Successfully finished all large file sort processing totally for input file:[%s] with result output file:[%s]."
            ,largeFilePath_.c_str(), resultFilePath_.c_str());
            return true;
        }
    }
}

COMMON_END_NAMESPACE