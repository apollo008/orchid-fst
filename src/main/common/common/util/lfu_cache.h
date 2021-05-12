/******************************************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       lfu_cache.h
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           4/10/21
  *Description:    file implement LFUCache:[Least frequently used] for single thread used,which
  *                use two double-linked lists + hash table as implementation,
  *                one double-linked list is frequency list, in which the element record frequency information,
  *                and its every element is also a doubled-linked list which indicates which element is more earlier
  *                for the last query or used. The most old element is on the back of the sub list.
**************************************************************************************************************/
#ifndef __COMMON_LFU_CACHE__H__
#define __COMMON_LFU_CACHE__H__
#include "common/common.h"
#include <unordered_map>
#include "common/util/lru_cache.h"

STD_USE_NAMESPACE;
COMMON_BEGIN_NAMESPACE

/**
 *@brief     LFUCache : Least frequently used cache, use two double-linked lists + hash map to reach high performance
 *@param     GetKeySizeCallBack and GetValueSizeCallBack can be implemented to record element counts or memory
 *           size.
 *@author    dingbinthu@163.com
 *@date      4/10/21, 9:13 AM
 */
template<typename K,
        typename V,
        typename GetKeySizeCallBack,
        typename GetValueSizeCallBack,
        typename HashFn = std::hash<K>,
        typename KeyEqual = std::equal_to<K> >
class LFUCache {
public:
    class FreqListItem;
    class TimeListItem;

    typedef DoubleLinkedList<FreqListItem,std::allocator<uint8_t> >   FreqListType;
    typedef shared_ptr<FreqListType>                                  FreqListTypePtr;
    typedef typename FreqListType::NodeTypePtr                        FreqListKeyPointer;
    typedef typename FreqListType::IteratorType                       FreqListIteratorType;

    typedef DoubleLinkedList<TimeListItem,std::allocator<uint8_t> >   TimeListType;
    typedef shared_ptr<TimeListType>                                  TimeListTypePtr;
    typedef typename TimeListType::NodeTypePtr                        TimeListKeyPointer;
    typedef typename TimeListType::IteratorType                       TimeListIteratorType;
public:
    /// time list, record who is more newer or later for queried.
    /// the one newer will be before in the list
    class TimeListItem {
    public:
        TimeListItem() { parentNode_ = nullptr; }
        TimeListItem(const K& k ) : key_(k) , parentNode_(nullptr) { }
        TimeListItem(const K& k ,FreqListKeyPointer parent) : key_(k) , parentNode_(parent) { }

        FreqListKeyPointer GetParent() { return parentNode_; }
        void ChangeParent(FreqListKeyPointer parent) { parentNode_ = parent; }
        K& KeyRef() { return key_; }
    private:
        K  key_;
        FreqListKeyPointer parentNode_;
    };

    ///frequency list, every element indicates all elements which hash the same certain frequency.
    ///front element has the larger frequency,and the back one has the smallest frequency,may be 1.
    class FreqListItem {
    public:
        FreqListItem() : freq_ (1) ,timeList_(make_shared<TimeListType >()) { }
        FreqListItem(uint64_t freq) : freq_ (freq) , timeList_(make_shared<TimeListType >()) { }

        uint64_t  GetFreq() { return freq_; }
        TimeListTypePtr GetTimeList() { return timeList_; }
    private:
        uint64_t        freq_;
        TimeListTypePtr timeList_;
    };

    class CacheValue {
    public:
        CacheValue() { keyPointer_ = nullptr;}
        CacheValue(const V& value) : value_(value) , keyPointer_(nullptr) {}
        CacheValue(const V& value,TimeListKeyPointer& kp) : value_(value) , keyPointer_(kp) {}

        V& GetValue() { return value_; }
        void SetValue(const V& v) { value_ = v; }
        TimeListKeyPointer&  GetKeyPointer() { return keyPointer_; }

        bool operator==(const CacheValue& ref) const {
            return value_ == ref.value_ && keyPointer_ == ref.keyPointer_;
        }
    private:
        V  value_;
        TimeListKeyPointer keyPointer_;
    };
public:
    typedef std::unordered_map<K,CacheValue,HashFn,KeyEqual>       MapType;

private:
    class Iterator {
        template <class,class,class,class,class,class> friend class LFUCache;
    public:
        bool HasNext() { return nullptr != curTimeKp_ && nullptr != curFreqKp_; }
        TimeListItem&  Next();
    private:
        Iterator(): curFreqKp_(nullptr), curTimeKp_(nullptr){}
        Iterator(FreqListKeyPointer freqKp, TimeListKeyPointer timeKp): curFreqKp_(freqKp), curTimeKp_(timeKp){}
    private:
        FreqListKeyPointer    curFreqKp_;
        TimeListKeyPointer    curTimeKp_;
    };

public:
    typedef Iterator  IteratorType;
public:
    LFUCache(uint64_t initialHashSize, uint64_t totalMemSize);
    virtual ~LFUCache();

    bool Get(const K& key, V& value);
    bool Put(const K& key, const V& value);
    bool IsInCache(const K& key);

    uint64_t  GetCacheSize() { return totalMemSize_; }
    uint64_t  GetCacheSizeUsed()  { return usedMemSize_; }
    uint64_t  GetKeyCount();
    uint64_t  GetKeyCountInHash() { return map_.size(); }

    double    GetHitRatio();
    uint64_t   GetTotalQueryTimes() { return totalQueryTimes_; }
    uint64_t   GetHitQueryTimes() { return hitQueryTimes_; }
    void       ResetHitStatistics() { totalQueryTimes_ = 0; hitQueryTimes_ = 0; }

    FreqListType&  GetList() { return list_; }
    MapType&   GetMap() { return map_; }

    bool IsEqual(LRUCache<K,V,GetKeySizeCallBack,GetValueSizeCallBack,HashFn,KeyEqual>& lruCache);
    IteratorType GetIterator() {
       FreqListKeyPointer firstFreqKp;
       if (!list_.GetFront(firstFreqKp)) {
           return Iterator();
       }
       auto freqKp = firstFreqKp;
       TimeListKeyPointer  firstTimeKp;
       while (freqKp) {
           auto timeListPtr = freqKp->DataRef().GetTimeList();
           if (!timeListPtr->GetFront(firstTimeKp)){
               freqKp = freqKp->Next();
           }
           else {
               return Iterator(freqKp,firstTimeKp);
           }
       }
        return Iterator();
    }
private:
    bool Replacement(uint64_t addedMemSize);
    void IncHitQueryTimes() { ++hitQueryTimes_; }
    void IncTotalQueryTimes() { ++totalQueryTimes_; }
private:
    GetKeySizeCallBack      getKeySizeCallBack_;
    GetValueSizeCallBack    getValueSizeCallBack_;

    FreqListType            list_;
    uint64_t                initialHashSize_;
    MapType                 map_;

    uint64_t                usedMemSize_;
    uint64_t                totalMemSize_;
    uint64_t                hitQueryTimes_;
    uint64_t                totalQueryTimes_;
private:
    TLOG_DECLARE();
};

template<typename K,
        typename V,
        typename GetKeySizeCallBack,
        typename GetValueSizeCallBack,
        typename HashFn,
        typename KeyEqual>
tulip::TLogger* LFUCache<K, V, GetKeySizeCallBack, GetValueSizeCallBack,HashFn,KeyEqual>::_logger
        = tulip::TLogger::s_GetLogger("common.util.LfuCache");

template<typename K,
        typename V,
        typename GetKeySizeCallBack,
        typename GetValueSizeCallBack,
        typename HashFn,
        typename KeyEqual>
LFUCache<K, V, GetKeySizeCallBack, GetValueSizeCallBack,HashFn,KeyEqual>::LFUCache(uint64_t initialHashSize, uint64_t totalMemSize)
        : getKeySizeCallBack_(GetKeySizeCallBack())
        , getValueSizeCallBack_(GetValueSizeCallBack())
        , initialHashSize_(initialHashSize)
        , map_(initialHashSize_)
        , usedMemSize_(0)
        , totalMemSize_(totalMemSize)
        , hitQueryTimes_(0)
        , totalQueryTimes_(0)
{
}

template<typename K,
        typename V,
        typename GetKeySizeCallBack,
        typename GetValueSizeCallBack,
        typename HashFn,
        typename KeyEqual>
LFUCache<K, V, GetKeySizeCallBack, GetValueSizeCallBack,HashFn,KeyEqual>::~LFUCache()
{
}

template<typename K,
        typename V,
        typename GetKeySizeCallBack,
        typename GetValueSizeCallBack,
        typename HashFn,
        typename KeyEqual>
bool LFUCache<K, V, GetKeySizeCallBack, GetValueSizeCallBack,HashFn,KeyEqual>::IsInCache(const K &key) {
    return map_.find(key) != map_.cend();
}

template<typename K,
        typename V,
        typename GetKeySizeCallBack,
        typename GetValueSizeCallBack,
        typename HashFn,
        typename KeyEqual>
bool LFUCache<K, V, GetKeySizeCallBack, GetValueSizeCallBack,HashFn,KeyEqual>::Get(const K& key, V& value) {
    bool bGot = false;
    auto it = map_.find(key);
    if (it != map_.cend()) {
        bGot = true;
        CacheValue& cacheValue = it->second;
        value = cacheValue.GetValue();

        auto kp = cacheValue.GetKeyPointer();
        auto parentFreqListNodePtr = kp->DataRef().GetParent();
        auto timeListPtr = parentFreqListNodePtr->DataRef().GetTimeList();
        auto freq = parentFreqListNodePtr->DataRef().GetFreq();
        timeListPtr->UnLink(kp);
        if (!parentFreqListNodePtr->Prev()) { list_.PushFront(FreqListItem(freq+1)); }
        auto prevTimeListPtr = parentFreqListNodePtr->Prev()->DataRef().GetTimeList();
        kp->DataRef().ChangeParent(parentFreqListNodePtr->Prev());
        prevTimeListPtr->PushFront(kp);

        IncHitQueryTimes();
    }
    IncTotalQueryTimes();
    return bGot;
}

template<typename K,
        typename V,
        typename GetKeySizeCallBack,
        typename GetValueSizeCallBack,
        typename HashFn,
        typename KeyEqual>
bool LFUCache<K, V, GetKeySizeCallBack, GetValueSizeCallBack,HashFn,KeyEqual>::Replacement(uint64_t addedMemSize) {
    if (addedMemSize > totalMemSize_) {
        TLOG_LOG(WARN,"lfucache added size:[%lu] exceeds memory limit:[%lu]",addedMemSize,totalMemSize_);
        return false;
    }
    while (addedMemSize + usedMemSize_ > totalMemSize_) {
        FreqListKeyPointer  lastFreqKp;
        if (!list_.GetBack(lastFreqKp)) {
            TLOG_LOG(WARN,"lfucache Get back from list Fail!");
            return false;
        }
        auto freqKp = lastFreqKp;
        while (freqKp) {
            auto prevKp = freqKp->Prev();
            if (freqKp->DataRef().GetTimeList()->Empty()) {
                list_.Erase(freqKp);
            } else { break; }
            freqKp = prevKp;
        }
        TimeListKeyPointer lastTimeKp;
        if (!freqKp->DataRef().GetTimeList()->GetBack(lastTimeKp)) {
            TLOG_LOG(WARN,"lfucache Get back time node from list Fail!");
            return false;
        }
        K& lastKey = lastTimeKp->DataRef().KeyRef();
        auto it = map_.find(lastKey);
        if (it == map_.cend()) {
            TLOG_LOG(WARN,"lfucache find in hashmap to remove Fail!");
            continue;
        }
        else {
            CacheValue& cacheValue = it->second;
            if (map_.erase(lastKey) > 0) {
                usedMemSize_ -= (getKeySizeCallBack_(lastKey) + getValueSizeCallBack_(cacheValue.GetValue()));
                freqKp->DataRef().GetTimeList()->Erase(lastTimeKp);
            }
            else {
                TLOG_LOG(WARN,"lfuCache find in hashmap to remove Fail!");
                continue;
            }
        }
    }
    return true;
}

template<typename K,
        typename V,
        typename GetKeySizeCallBack,
        typename GetValueSizeCallBack,
        typename HashFn,
        typename KeyEqual>
bool LFUCache<K, V, GetKeySizeCallBack, GetValueSizeCallBack,HashFn,KeyEqual>::Put(const K& key, const V& value) {
    bool ret = false;

    uint64_t newKeySize = getKeySizeCallBack_(key);
    uint64_t newValueSize = getValueSizeCallBack_(value);
    uint64_t addedMemSize = 0;
    auto it = map_.find(key);
    if (it != map_.cend()) {
        //exist
        uint64_t oldValueSize = getValueSizeCallBack_(it->second.GetValue());
        if (newValueSize > oldValueSize) {
            addedMemSize = newValueSize - oldValueSize;
        }
    }
    else {
        //non-exist
        addedMemSize = newKeySize + newValueSize;
    }
//    TLOG_LOG(DEBUG, "lfucache will Put key:[%s] with value:[%lu], addedMemSize:[%lu],usedMemSize:[%lu],addedMemSize+usedMemSize:[%lu],totalMemSize:[%lu]",
//             key.c_str(),value, addedMemSize, usedMemSize_, (addedMemSize+usedMemSize_), totalMemSize_);
    if (addedMemSize + usedMemSize_ > totalMemSize_) {
        if (!Replacement(addedMemSize)) {
            TLOG_LOG(WARN, "lfucache Put failed, because of Replacement fail memSizeUsed_=%lu", usedMemSize_);
            return false;
        }
    }
    ///////////////////////////////////////
    if (it != map_.cend()) {
        //exist
        CacheValue& oldCacheValue = it->second;
        CacheValue newCacheValue = oldCacheValue;
        newCacheValue.SetValue(value);
        auto res = map_.insert_or_assign(key, newCacheValue);
        if (res.second) {
            usedMemSize_ -= getValueSizeCallBack_(oldCacheValue.GetValue());
            usedMemSize_ += newValueSize;
            ////////////////////////////////
            auto timeKp = newCacheValue.GetKeyPointer();
            auto freqKp = timeKp->DataRef().GetParent();
            auto timeListPtr = freqKp->DataRef().GetTimeList();
            auto freq = freqKp->DataRef().GetFreq();
            timeListPtr->UnLink(timeKp);
            if (!freqKp->Prev()) {
                list_.PushFront(FreqListItem(freq+1));
            }
            auto prevTimeListPtr = freqKp->Prev()->DataRef().GetTimeList();
            timeKp->DataRef().ChangeParent(freqKp->Prev());
            prevTimeListPtr->PushFront(timeKp);
            ////////////////////////////////
            ret = true;
        }
        else {
            TLOG_LOG(WARN, "lfucache Put hashmap failed");
        }
    }
    else {
        //non-exist
        ///////////////////////////////////////////////////
        FreqListKeyPointer firstFreqKp;
        if (!list_.GetFront(firstFreqKp)) {
            firstFreqKp = nullptr;
        }
        if (firstFreqKp && firstFreqKp->DataRef().GetFreq() > 1) {
            firstFreqKp = nullptr;
        }
        if (!firstFreqKp) {
            list_.PushFront(FreqListItem(1));
            list_.GetFront(firstFreqKp);
        }
        auto firstTimeListPtr = firstFreqKp->DataRef().GetTimeList();
        auto kp = firstTimeListPtr->PushFront(TimeListItem(key,firstFreqKp));
        CacheValue cacheValue(value,kp);
        auto res = map_.emplace(key, cacheValue);
        if (res.second) {
            usedMemSize_ += addedMemSize;
            ret = true;
        }
        else {
            firstTimeListPtr->Erase(kp);
            TLOG_LOG(WARN, "lrucache Put hashmap failed");
        }
    }
    ///////////////////////////////////////////
    if (ret) {
        Replacement(0);
    }
    return ret;
}




template<typename K,
        typename V,
        typename GetKeySizeCallBack,
        typename GetValueSizeCallBack,
        typename HashFn,
        typename KeyEqual>
uint64_t  LFUCache<K, V, GetKeySizeCallBack, GetValueSizeCallBack,HashFn,KeyEqual>::GetKeyCount() {
    uint64_t count = 0;
    auto it = list_.GetIterator();
    while (it.HasNext()) {
        auto& freqItem = it.Next();
        count += freqItem.GetTimeList()->Size();
    }
    return count;
}


template<typename K,
        typename V,
        typename GetKeySizeCallBack,
        typename GetValueSizeCallBack,
        typename HashFn,
        typename KeyEqual>
double LFUCache<K, V, GetKeySizeCallBack, GetValueSizeCallBack,HashFn,KeyEqual>::GetHitRatio() {
    if (totalQueryTimes_ != 0) {
        return (double)hitQueryTimes_ / totalQueryTimes_;
    } else {
        return 0.0f;
    }
}

template<typename K,
        typename V,
        typename GetKeySizeCallBack,
        typename GetValueSizeCallBack,
        typename HashFn,
        typename KeyEqual>
typename LFUCache<K, V, GetKeySizeCallBack, GetValueSizeCallBack,HashFn,KeyEqual>::TimeListItem&
LFUCache<K, V, GetKeySizeCallBack, GetValueSizeCallBack,HashFn,KeyEqual>::IteratorType::Next() {
    TimeListItem& ref = curTimeKp_->DataRef();
    if (curTimeKp_->Next()) {
        curTimeKp_ = curTimeKp_->Next();
    }
    else {
        auto freqKp = curFreqKp_->Next();
        while (freqKp ) {
            auto timeListPtr = freqKp->DataRef().GetTimeList();
            if (timeListPtr->GetFront(curTimeKp_)) {
                return ref;
            }
            freqKp = freqKp->Next();
        }
        curFreqKp_ = nullptr;
        curTimeKp_ = nullptr;
    }
    return ref;
}


template<typename K,
        typename V,
        typename GetKeySizeCallBack,
        typename GetValueSizeCallBack,
        typename HashFn,
        typename KeyEqual>
bool LFUCache<K,V,GetKeySizeCallBack,GetValueSizeCallBack,HashFn,KeyEqual>::IsEqual(LRUCache<K,V,GetKeySizeCallBack,GetValueSizeCallBack,HashFn,KeyEqual>& lruCache) {
    if (GetKeyCount() != lruCache.GetKeyCount()) {
        TLOG_LOG(ERROR,"key count differs with concurrentLru:[%lu] vs lru:[%lu]!", GetKeyCount() , lruCache.GetKeyCount());
        return false;
    }
    if (GetKeyCountInHash() != lruCache.GetKeyCountInHash()) {
        TLOG_LOG(ERROR,"key count differs with concurrentLru:[%lu] vs lru:[%lu]!", GetKeyCountInHash() , lruCache.GetKeyCountInHash());
        return false;
    }

    typedef LRUCache<K,V,GetKeySizeCallBack,GetValueSizeCallBack,HashFn,KeyEqual> LRUCacheType;
    typename LRUCacheType::ListIteratorType  it = lruCache.GetList().GetIterator();
    typename LRUCacheType::MapType&  lruMap = lruCache.GetMap();
    while (it.HasNext()) {
        auto key = it.Next();
        auto selfIt = map_.find(key);
        auto lruIt = lruMap.find(key);

        bool inSelf = (selfIt != map_.cend());
        bool inOther = (lruIt != lruMap.cend());
        if (inSelf != inOther) {
            TLOG_LOG(ERROR,"Found value existence differs for key:[%s]",key.c_str());
            return false;
        }
        if (selfIt->second.GetValue() != lruIt->second.GetValue()) {
            TLOG_LOG(ERROR,"Found value not equal:[%u] vs [%u] for key:[%s]", selfIt->second.GetValue(), lruIt->second.GetValue(),key.c_str());
            return false;
        }
    }
    return true;
}


COMMON_END_NAMESPACE
#endif //__COMMON_LFU_CACHE__H__
