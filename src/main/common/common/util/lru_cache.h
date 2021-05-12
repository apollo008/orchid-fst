/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       lru_cache.h
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           4/9/21
  *Description:    file implement LRUCache:[Least recently used] for single thread used,which
  *                use double-linked list + hash table as implementation
**********************************************************************************/
#ifndef __COMMON_LRU_CACHE__H__
#define __COMMON_LRU_CACHE__H__
#include "common/common.h"
#include <unordered_map>

STD_USE_NAMESPACE;
COMMON_BEGIN_NAMESPACE

/// Double-linked list used within LRUCache and LFUCache
template <typename T,
        typename Allocator = std::allocator<uint8_t> >
class DoubleLinkedList {
public:
    template <class,class,class,class,class,class> friend class LRUCache;
    template <class,class,class,class,class,class> friend class LFUCache;
private:
    struct Node {
        template <class,class> friend class DoubleLinkedList;

        Node(const T& data) :data_(data),prev_(nullptr),next_(nullptr){}
        Node(const T& data,Node* prev,Node* next) :data_(data),prev_(prev),next_(next){}
        T             data_;
        Node*         prev_;
        Node*         next_;

        T& DataRef() { return data_; }
        Node* Prev() { return prev_; }
        Node* Next() { return next_; }
    };
public:
    typedef Node                               NodeType;
    typedef NodeType*                          NodeTypePtr;

private:
    class Iterator {
        friend class DoubleLinkedList<T>;
    public:
        bool HasNext() { return nullptr != curNode_; }
        T&  Next() { T& data = curNode_->data_; curNode_ = curNode_->next_; return data; }
    private:
        Iterator(): curNode_(nullptr){}
        Iterator(NodeTypePtr curNode): curNode_(curNode){}
    private:
        NodeTypePtr            curNode_;
    };

public:
    typedef Iterator  IteratorType;
public:
    DoubleLinkedList(): head_(nullptr), tail_(nullptr), size_(0) {}
    ~DoubleLinkedList();

    IteratorType GetIterator() { return Iterator(head_); }

    uint32_t Size() const { return size_; }
    bool Empty() const { return size_ == 0; }

    NodeTypePtr PushFront(const T& data);
    NodeTypePtr PushBack(const T& data);

    NodeTypePtr PushFront(NodeTypePtr nodePtr);
    NodeTypePtr PushBack(NodeTypePtr nodePtr);

    void   MoveToHead(NodeTypePtr nodePtr);
    void   MoveToTail(NodeTypePtr nodePtr);
    void   Erase(NodeTypePtr nodePtr);
    bool   GetFront(T& v) { if (nullptr == head_) {return false; } v = head_->data_; return true;}
    bool   GetBack(T& v) { if (nullptr == tail_) {return false; } v = tail_->data_; return true;}

    bool   GetFront(NodeTypePtr& nodePtr) { if (nullptr == head_) {return false; } nodePtr = head_; return true;}
    bool   GetBack(NodeTypePtr& nodePtr) { if (nullptr == tail_) {return false; } nodePtr = tail_; return true;}

    void  UnLink(NodeTypePtr nodePtr) { UnLinkInternal(nodePtr); }
private:
    void   UnLinkInternal(NodeTypePtr nodePtr);
    void   PushFrontInternal(NodeTypePtr nodePtr);
    void   PushBackInternal(NodeTypePtr nodePtr);
private:
    NodeTypePtr          head_;
    NodeTypePtr          tail_;
    uint32_t             size_;
};


/**
 *@brief     LRUCache : Least recently used cache, use double-linked list + hash map to reach high performance
 *@param     GetKeySizeCallBack and GetValueSizeCallBack can be implemented to record element counts or memory
 *           size.
 *@author    dingbinthu@163.com
 *@date      4/8/21, 7:43 AM
 */
template<typename K,
        typename V,
        typename GetKeySizeCallBack,
        typename GetValueSizeCallBack,
        typename HashFn = std::hash<K>,
        typename KeyEqual = std::equal_to<K> >
class LRUCache
{
public:
    typedef DoubleLinkedList<K,std::allocator<uint8_t> >           ListType;
    typedef typename ListType::NodeTypePtr                         KeyPointer;
    typedef typename ListType::IteratorType                        ListIteratorType;
private:
    class CacheValue {
    public:
        CacheValue() { keyPointer_ = nullptr;}
        CacheValue(const V& value) : value_(value) , keyPointer_(nullptr) {}
        CacheValue(const V& value,KeyPointer& kp) : value_(value) , keyPointer_(kp) {}


        V& GetValue() { return value_; }
        void SetValue(const V& v) { value_ = v; }
        KeyPointer&  GetKeyPointer() { return keyPointer_; }

        bool operator==(const CacheValue& ref) const {
            return value_ == ref.value_ && keyPointer_ == ref.keyPointer_;
        }
    private:
        V                                           value_;
        KeyPointer                                  keyPointer_;
    };
public:
    typedef std::unordered_map<K,CacheValue,HashFn,KeyEqual>       MapType;
public:
    LRUCache(uint64_t initialHashSize, uint64_t totalMemSize);
    virtual ~LRUCache();

    bool      Get(const K& key, V& value);
    bool      Put(const K& key, const V& value);
    bool      IsInCache(const K& key);

    uint64_t  GetCacheSize() { return totalMemSize_; }
    uint64_t  GetCacheSizeUsed()  { return usedMemSize_; }
    uint64_t  GetKeyCount() { return list_.Size(); }
    uint64_t  GetKeyCountInHash() { return map_.size(); }

    double    GetHitRatio();
    uint64_t   GetTotalQueryTimes() { return totalQueryTimes_; }
    uint64_t   GetHitQueryTimes() { return hitQueryTimes_; }
    void       ResetHitStatistics() { totalQueryTimes_ = 0; hitQueryTimes_ = 0; }

    ListType&  GetList() { return list_; }
    MapType&   GetMap() { return map_; }
private:
    bool Replacement(uint64_t addedMemSize);
    void IncHitQueryTimes() { ++hitQueryTimes_; }
    void IncTotalQueryTimes() { ++totalQueryTimes_; }
private:
    GetKeySizeCallBack                                      getKeySizeCallBack_;
    GetValueSizeCallBack                                    getValueSizeCallBack_;
    ListType                                                list_;
    uint64_t                                                initialHashSize_;
    MapType                                                 map_;

    uint64_t                                                 usedMemSize_;
    uint64_t                                                 totalMemSize_;
    uint64_t                                                 hitQueryTimes_;
    uint64_t                                                 totalQueryTimes_;
private:
    TLOG_DECLARE();
};
template<typename K,
        typename V,
        typename GetKeySizeCallBack,
        typename GetValueSizeCallBack,
        typename HashFn,
        typename KeyEqual>
tulip::TLogger* LRUCache<K, V, GetKeySizeCallBack, GetValueSizeCallBack,HashFn,KeyEqual>::_logger
        = tulip::TLogger::s_GetLogger("common.util.LruCache");


template<typename K,
        typename V,
        typename GetKeySizeCallBack,
        typename GetValueSizeCallBack,
        typename HashFn,
        typename KeyEqual>
LRUCache<K, V, GetKeySizeCallBack, GetValueSizeCallBack,HashFn,KeyEqual>::LRUCache(uint64_t initialHashSize, uint64_t totalMemSize)
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
LRUCache<K, V, GetKeySizeCallBack, GetValueSizeCallBack,HashFn,KeyEqual>::~LRUCache()
{
}


template<typename K,
        typename V,
        typename GetKeySizeCallBack,
        typename GetValueSizeCallBack,
        typename HashFn,
        typename KeyEqual>
double LRUCache<K, V, GetKeySizeCallBack, GetValueSizeCallBack,HashFn,KeyEqual>::GetHitRatio() {
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
bool LRUCache<K, V, GetKeySizeCallBack, GetValueSizeCallBack,HashFn,KeyEqual>::Get(const K& key, V& value) {
    bool bGot = false;
    auto it = map_.find(key);
    if (it != map_.cend()) {
        bGot = true;
        CacheValue& cacheValue = it->second;
        value = cacheValue.GetValue();
        list_.MoveToHead(cacheValue.GetKeyPointer());
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
bool LRUCache<K, V, GetKeySizeCallBack, GetValueSizeCallBack,HashFn,KeyEqual>::Replacement(uint64_t addedMemSize) {
    if (addedMemSize > totalMemSize_) {
        TLOG_LOG(WARN,"lrucache added size:[%lu] exceeds memory limit:[%lu]",addedMemSize,totalMemSize_);
        return false;
    }
    while (addedMemSize + usedMemSize_ > totalMemSize_) {
        K tmpKey;
        if (!list_.GetBack(tmpKey)) {
            TLOG_LOG(WARN,"lrucache get back from list Fail!");
            return false;
        }
        auto it = map_.find(tmpKey);
        if (it == map_.cend()) {
            TLOG_LOG(WARN,"lrucache find in hashmap to remove Fail!");
            continue;
        }
        else {
            CacheValue& cacheValue = it->second;
            if (map_.erase(tmpKey) > 0) {
//                TLOG_LOG(DEBUG,"lrucache key:[%s] with value:[%u] removed!",tmpKey.c_str(), cacheValue.GetValue());
                list_.Erase(cacheValue.GetKeyPointer());
                usedMemSize_ -= (getKeySizeCallBack_(tmpKey) + getValueSizeCallBack_(cacheValue.GetValue()));
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
bool LRUCache<K, V, GetKeySizeCallBack, GetValueSizeCallBack,HashFn,KeyEqual>::Put(const K& key, const V& value) {
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
//    TLOG_LOG(DEBUG, "lrucache will Put key:[%s] with value:[%lu], addedMemSize:[%lu],usedMemSize:[%lu],addedMemSize+usedMemSize:[%lu],totalMemSize:[%lu]",
//             key.c_str(),value, addedMemSize, usedMemSize_, (addedMemSize+usedMemSize_), totalMemSize_);
    if (addedMemSize + usedMemSize_ > totalMemSize_) {
        if (!Replacement(addedMemSize)) {
            TLOG_LOG(WARN, "lrucache Put failed, because of Replacement fail memSizeUsed_=%lu", usedMemSize_);
            return false;
        }
    }
    if (it != map_.cend()) {
        //exist
        CacheValue& oldCacheValue = it->second;
        CacheValue newCacheValue = oldCacheValue;
        newCacheValue.SetValue(value);
        auto res = map_.insert_or_assign(key, newCacheValue);
        if (res.second) {
            //statistics of memory size
            usedMemSize_ -= getValueSizeCallBack_(oldCacheValue.GetValue());
            usedMemSize_ += newValueSize;
            list_.MoveToHead(newCacheValue.GetKeyPointer());
            ret = true;
        }
        else {
            TLOG_LOG(WARN, "lrucache Put hashmap failed");
        }
    }
    else {
        //non-exist
        KeyPointer kp = list_.PushFront(key);
        CacheValue cacheValue(value,kp);
        auto res = map_.emplace(key, cacheValue);
        if (res.second) {
            usedMemSize_ += addedMemSize;
            ret = true;
        }
        else {
            list_.Erase(kp);
            TLOG_LOG(WARN, "lrucache Put hashmap failed");
        }
    }
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
bool LRUCache<K, V, GetKeySizeCallBack, GetValueSizeCallBack,HashFn,KeyEqual>::IsInCache(const K &key) {
    return map_.find(key) != map_.cend();
}


template<typename T,typename Allocator>
DoubleLinkedList<T,Allocator>::~DoubleLinkedList() {
    NodeTypePtr pNode = head_;
    while (nullptr != pNode) {
        auto pNext = pNode->next_;
        pNode->~NodeType();
        Allocator().deallocate((uint8_t*)pNode, sizeof(NodeType));
        pNode = pNext;
    }
    head_ = tail_ = nullptr;
    size_ = 0;
}

template<typename T,typename Allocator>
typename DoubleLinkedList<T,Allocator>::NodeTypePtr
DoubleLinkedList<T,Allocator>::PushFront(NodeTypePtr nodePtr) {
    if (nullptr == head_) {
        assert(nullptr == tail_);
        head_ = tail_ = nodePtr;
    }
    else {
        nodePtr->next_ = head_;
        head_->prev_ = nodePtr;
        head_ = nodePtr;
    }
    ++size_;
    return nodePtr;
}

template<typename T,typename Allocator>
typename DoubleLinkedList<T,Allocator>::NodeTypePtr
DoubleLinkedList<T,Allocator>::PushBack(NodeTypePtr nodePtr) {
    if (nullptr == tail_) {
        head_ = tail_ = nodePtr;
    }
    else {
        nodePtr->prev_ = tail_;
        tail_->next = nodePtr;
        tail_ = nodePtr;
    }
    ++size_;
    return nodePtr;
}

template<typename T,typename Allocator>
typename DoubleLinkedList<T,Allocator>::NodeTypePtr
DoubleLinkedList<T,Allocator>::PushFront(const T& data) {
    void* p = Allocator().allocate(sizeof(NodeType));
    auto node = new (p) NodeType(data);
    return PushFront(node);
}

template<typename T,typename Allocator>
typename DoubleLinkedList<T,Allocator>::NodeTypePtr
DoubleLinkedList<T,Allocator>::PushBack(const T& data) {
    void* p = Allocator().allocate(sizeof(NodeType));
    auto node = new (p) NodeType(data);
    return PushBack(node);
}

template<typename T,typename Allocator>
void DoubleLinkedList<T,Allocator>::MoveToHead(NodeTypePtr nodePtr) {
    if (nullptr == nodePtr || nodePtr == head_) return;
    UnLinkInternal(nodePtr);
    PushFrontInternal(nodePtr);
}

template<typename T,typename Allocator>
void DoubleLinkedList<T,Allocator>::MoveToTail(NodeTypePtr nodePtr) {
    if (nullptr == nodePtr || nodePtr == tail_) return;
    UnLinkInternal(nodePtr);
    PushBackInternal(nodePtr);
}


template<typename T,typename Allocator>
void DoubleLinkedList<T,Allocator>::UnLinkInternal(NodeTypePtr nodePtr) {
    if (nullptr == nodePtr) return;

    if (nodePtr->prev_ && nodePtr->next_) {
        nodePtr->prev_->next_ = nodePtr->next_;
        nodePtr->next_->prev_ = nodePtr->prev_;
        nodePtr->prev_ = nodePtr->next_ = nullptr;
    }
    else if (!nodePtr->prev_ && nodePtr->next_) {
        assert(head_ == nodePtr);
        nodePtr->next_->prev_ = nullptr;
        head_ = nodePtr->next_;
        nodePtr->prev_ = nodePtr->next_ = nullptr;
    }
    else if (nodePtr->prev_ && !nodePtr->next_) {
        assert(tail_== nodePtr);
        nodePtr->prev_->next_ = nullptr;
        tail_ = nodePtr->prev_;
        nodePtr->prev_ = nodePtr->next_ = nullptr;
    }
    else if (!nodePtr->prev_ && !nodePtr->next_) {
        assert(tail_== nodePtr);
        assert(head_ == nodePtr);
        head_ = tail_ = nullptr;
    }
    size_--;
}

template<typename T,typename Allocator>
void DoubleLinkedList<T,Allocator>::PushBackInternal(NodeTypePtr nodePtr) {
    if (nullptr == nodePtr) return;
    assert(nodePtr->next_ == nullptr);
    assert(nodePtr->prev_ == nullptr);

    if (nullptr == tail_) {
        assert(nullptr == head_);
        head_ = tail_ = nodePtr;
    }
    else {
        nodePtr->prev_ = tail_;
        tail_->next = nodePtr;
        tail_ = nodePtr;
    }
    ++size_;
}

template<typename T,typename Allocator>
void DoubleLinkedList<T,Allocator>::PushFrontInternal(NodeTypePtr nodePtr) {
    if (nullptr == nodePtr) return;
    assert(nodePtr->next_ == nullptr);
    assert(nodePtr->prev_ == nullptr);

    if (nullptr == head_) {
        assert(nullptr == tail_);
        head_ = tail_ = nodePtr;
    }
    else {
        nodePtr->next_ = head_;
        head_->prev_ = nodePtr;
        head_ = nodePtr;
    }
    ++size_;
}

template<typename T,typename Allocator>
void DoubleLinkedList<T,Allocator>::Erase(NodeTypePtr nodePtr) {
    UnLinkInternal(nodePtr);
    nodePtr->~NodeType();
    Allocator().deallocate((uint8_t*)nodePtr, sizeof(NodeType));
}

COMMON_END_NAMESPACE
#endif //__COMMON_LRU_CACHE__H__
