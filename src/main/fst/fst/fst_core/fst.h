/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:    fst.h
  *Author:      dingbinthu@163.com
  *Version:     1.0
  *Date:        2/3/21
  *Description: file defines class to implements fst data structure: Finite state transducer,
  *             short for fst,which is widely used in open source search engine: Elasticsearch
  *             or lucene. It is so fast for search text string, it can be understand as a key-value
  *             map or pure set data structure. It has O(n) complexity to search a text string in fst,
  *             here 'n' means length of text string searched.
  *
  *             This Fst implementation has 7 advantages as below:
  *               1) The FST construction process realized in this paper can realize the construction
  *                   of the FST while the constructed FST data to the external file to reduce the memory
  *                   occupation. This point is especially important for dictionary data with large
  *                   regularity. Because dict cannot be loaded into memory all at once to construct FST.
  *                   The FST data file is then used as a memory map. Many current open source FST
  *                   implementations do not have this feature. This is one of the important
  *                   highlights of the project.
  *
  *               2) It is implements in C++ language and may be so useful if your project is just C++
  *                   development environment. There are some versions of fst open source implements, but it
  *                   is much rarely implemented well in C/C++ language as a high quality code,so  You can
  *                   easily integrate it into your C++ development projects. As we know it has been
  *                   implemented in Rust and Java.
  *
  *               3) It has successfully resolved UTF8-code text string,and resolve more sever bugs when text
  *                   string is not english characters, such as Chinese characters and so on. It used a much
  *                   more clever approach to resolves UTF8-coding text string.
  *
  *               4) This project presents a large number of examples of automata implementations, from which
  *                   you can learn more about automata implementations for many different purposes, whether
  *                   they are performed on FST data structures or used in other scenarios. The many automata
  *                   data structures in this project Demo include, but are not limited to: GreaterThan,LessThan,
  *                   Prefix,Str,StartsWith,Intersection,Union,Not,Levenshtein Automaton and so on.
  *
  *
  *               5) Not only does it implement a code interface, but it also carefully implements an easy-to-use
  *                   command-line tool, FST, with detailed instructions on how to use parameters. The FST
  *                   command-line tool has these following functions:
  *                     A smart Fst command line tool
  *                      Usage: fst [OPTIONS] SUBCOMMAND
  *
  *                      Options:
  *                        -h,--help       Print this help message and exit
  *
  *                      Subcommands:
  *                        map             construct fst data file from a key-value(separated with comma every line)
  *                                        dictionary file.
  *                        set             construct fst data file from a only key(no value) dictionary file.
  *                        dot             generate dot file from fst data file, which can be converted to png file using dot
  *                                        command like: dot -Tpng < a.dot > a.png, then you can view the picture generated.
  *                        match           execute accurate match query for a term text in the fst.
  *                        prefix          execute prefix query starts with a term text in the fst.
  *                        range           execute range query in the fst.
  *                        fuzzy           execute fuzzy query in the fst,it works by building a Levenshtein automaton
  *                                        within a edit distance.
  *
  *                 fst map/set can construct a fst file from your input dictionary file;
  *                 fst dot can generate a dot file from fst file, which can be generate picture file
  *                 such as png file to see the fst structure by dot command,such as:
  *                 'dot -Tpng < xx.dot > xx.png'
  *
  *                 Another importance point is: You can see details in fst for every Chinese character!
  *                 This is exhilarating.
  *                 match/prefix/range/fully sub-commands executes different search in fst.
  *                 You can obtain more details usage instructions by use 'fst --help' or 'fst map --help'
  *                 or 'fst fuzzy --help', You can try it!
  *
  *               6)  This fst project implements LRUcache and LFUcache, which used in constructing fst
  *                   data structure. In order to prevent excessive memory usage, lruCache plays a good
  *                   trade-off role in the process of constructing FST. LruCache can build an approximate
  *                   minimum FST tree for you, to achieve the approximate minimum space effect. If the
  *                   memory space is sufficient,it is recommended that the available cache capacity of
  *                   the lruCache be set as large as possible when building the FST. In this way,the FST
  *                   is built as small as possible because space is saved as much as possible.
  *
  *               7)  Considering that the dictionary file construction of FST always requires the input to be
  *                   lexicographically ordered, this project implements a good module of large file external
  *                   sorting function, and also implements an easy-to-use command-line tool, lfsort,
  *                   to sort the large text file separately.  Of course, the FST command-line tool already
  *                   has large file sorting integrated into it, and by default it inputs large text files
  *                   that are unsorted.  Large file sorting is also time-consuming.  If your input dictionary
  *                   files are already sorted, you can display the specified '-s' or '--sorted' arguments
  *                   to avoid sorting large files before constructing FST. You can enter 'lfsort --help' to
  *                   see detailed instructions for large file sort command-line tool lfsort.
  *
**********************************************************************************/
#ifndef __CPPFST_FST_CORE_FST__H__
#define __CPPFST_FST_CORE_FST__H__
#include "common/common.h"
#include "tulip/TLogDefine.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <functional>
#include <fstream>
#include <stack>
#include <list>
#include <string>
#include "common/util/hash_util.h"
#include "common/util/lru_cache.h"
#include "common/util/output_stream_util.h"
#include <fst/fst_core/automaton.h>

STD_USE_NAMESPACE;

COMMON_BEGIN_NAMESPACE


class FstWriteNode;

/// class  for fst builder transition
class FstBuildTrans {
public:
    const static uint64_t  FST_BUILD_EMPTY_WRITE_NODE_ADDR_OFFSET = 0ul;
public:
    FstBuildTrans(uint8_t input)
    : m_input(input)
    , m_output(0)
    , m_targetAddrOffset(FstBuildTrans::FST_BUILD_EMPTY_WRITE_NODE_ADDR_OFFSET)
    {
    }
public:
    ///key from input, only 1 byte
    uint8_t                           m_input;
    ///value from input, must be uint64 type
    uint64_t                          m_output;

    ///target address memory offset stored in the output stream
    uint64_t                          m_targetAddrOffset;

    ///the data structure node loaded in memory for one node
    std::shared_ptr<FstWriteNode>     m_targetNode;
};
TYPEDEF_PTR(FstBuildTrans);

///class for fst write node type
class FstWriteNode {
public:
    FstWriteNode(bool isFinal)
    : m_isFinal(isFinal)
    , m_finalOutput(0)
    {}
    ~FstWriteNode(){}
public:
    void SetIsFinal(bool isFinal) { m_isFinal = isFinal; }
    void Dump(OutputStreamBase*  outputStream, bool hasOutput);
    void ResetTargetNode();
public:
    ///indicate whether reach end from input for current node
    bool                              m_isFinal;

    ///value for input, must be uint64_t type
    uint64_t                          m_finalOutput;

    ///many transitions for current node
    std::vector<FstBuildTransPtr>     m_trans;
};
TYPEDEF_PTR(FstWriteNode);


///class who implements Fst builder
class FstBuilder {
public:
    ///hash function for fst node so can store it in lru cache
    class FstWriteNodeHash {
    public:
        size_t operator()(const FstWriteNodePtr& node) const;
    };

    ///equal function for fst node so can store it in lru cache
    class FstWriteNodeEqual {
    public:
        bool operator()(const FstWriteNodePtr& node1, const FstWriteNodePtr& node2) const;
    };

    ///compute size of key(that is FstWriteNodePtr),may count or memory size, so can store it in lru cache
    class GetFstWriteNodePtrSize {
    public:
        uint64_t operator()(const FstWriteNodePtr& node) const {
            return sizeof(node->m_isFinal) + sizeof(node->m_finalOutput) + node->m_trans.size() * 33;
        }
    };

    ///compute size of value(here must be uint64_t type),may count or memory size, so can store it in lru cache
    class GetAddrOffsetSize {
    public:
        uint64_t operator()(const uint64_t & addr) const {
            return sizeof(addr);
        }
    };

    ///LRUCache used for fast search all nodes who has been output into external stream,which is often a file stream.
    typedef LRUCache<FstWriteNodePtr,uint64_t,GetFstWriteNodePtrSize,
    GetAddrOffsetSize,FstWriteNodeHash,FstWriteNodeEqual> FstBuildNodeMapType;
public:
    FstBuilder(OutputStreamBase* outputStream,bool hasOutput, uint64_t totalNodeHashCashMemSize)
    : m_outputStream(outputStream)
    , m_hasOutput (hasOutput)
    , m_rootNode(std::make_shared<FstWriteNode>(false))
    , m_node2AddrOffsetMap(std::max((uint64_t)1e8,(uint64_t)(totalNodeHashCashMemSize/20)),
                           totalNodeHashCashMemSize)
    {
        //preserve 8 bytes to store rootNodeAddrOffset
        uint64_t rootNodeAddrOffset = 0;
        m_outputStream->Write((uint8_t*)&rootNodeAddrOffset,8);

        //next 1 byte to store whether is hasOutput
        m_outputStream->Write((uint8_t*)&m_hasOutput,1);

        uint64_t addrOffset = m_outputStream->GetTotalBytesCnt();
        s_FinalTerminateNode->Dump(m_outputStream,m_hasOutput);
        bool ret = m_node2AddrOffsetMap.Put(s_FinalTerminateNode,addrOffset);
        assert(ret);
    }
    ~FstBuilder() {}
public:
    bool Insert(const uint8_t* key, uint32_t len, uint64_t value);
    uint64_t FreezeNode(FstWriteNodePtr node);
    uint64_t FreezeNodes(FstWriteNodePtr header);
    void Finish();
public:
    ///static terminated final node,used this global one node for save memory
    static FstWriteNodePtr  s_FinalTerminateNode;
private:
    ///output stream used for building the FST while you dump it to save memory
    OutputStreamBase*           m_outputStream;
    ///indicate whether hash output, yes for map, not for set
    bool                    m_hasOutput;

    ///root node for this building
    FstWriteNodePtr         m_rootNode;
    ///an hash data structure which stores FstWriteNode mapped its address memory offset dump out to stream
    ///so you can use memory map technology to use the fst future to handle memory limit problem
    FstBuildNodeMapType     m_node2AddrOffsetMap;
private:
    TLOG_DECLARE();
};
TYPEDEF_PTR(FstBuilder);

size_t FstBuilder::FstWriteNodeHash::operator()(const FstWriteNodePtr& node) const {
    size_t seed = 0;
    HashCombine(seed,node->m_isFinal);
    HashCombine(seed,node->m_finalOutput);
    HashCombine<uint64_t>(seed,(uint64_t)node->m_trans.size());
    for (size_t i = 0; i < node->m_trans.size(); ++i) {
        HashCombine(seed,node->m_trans[i]->m_input);
        HashCombine(seed,node->m_trans[i]->m_output);
        HashCombine(seed,node->m_trans[i]->m_targetAddrOffset);
    }
    return seed;
}


bool FstBuilder::FstWriteNodeEqual::operator()(const FstWriteNodePtr& node1, const FstWriteNodePtr& node2) const {
    if (node1.get() == node2.get()) return true;
    bool bEqual = (
            (node1->m_isFinal == node2->m_isFinal)
            && (node1->m_trans.size() == node2->m_trans.size())
            && (node1->m_finalOutput == node2->m_finalOutput)
    );
    for (size_t i = 0; bEqual && i < node1->m_trans.size(); ++i) {
        bEqual = bEqual && (node1->m_trans[i]->m_input == node2->m_trans[i]->m_input);
        bEqual = bEqual && (node1->m_trans[i]->m_output == node2->m_trans[i]->m_output);
        bEqual = bEqual && (node1->m_trans[i]->m_targetAddrOffset == node2->m_trans[i]->m_targetAddrOffset);
    }
    return bEqual;
}


///class to indicate Fst reader node transition, which is different from transition for the write node
class FstReaderTrans {
public:
    FstReaderTrans(uint8_t input, uint64_t output, uint64_t targetAddrOffset)
            : m_input (input)
    , m_output (output)
    , m_targetAddrOffset (targetAddrOffset)
    {
    }
    ~FstReaderTrans(){}
public:
    ///key
    uint8_t                           m_input;
    ///value ,must be uint64_t type
    uint64_t                          m_output;

    ///address memory offset in the memory map
    uint64_t                          m_targetAddrOffset;
};
TYPEDEF_PTR(FstReaderTrans);

///class defines Fst builder node
class FstReaderNode {
public:
    FstReaderNode()
    : m_startPtr(nullptr)
    , m_hasOutput(false)
    , m_isFinal (false)
    , m_finalOutput(0)
    {
    }
    ~FstReaderNode() {}
private:
    FstReaderNode(const FstReaderNode& rhs);
    FstReaderNode& operator=(const FstReaderNode& rhs);
public:
    size_t  GetTransCount() { return m_trans.size(); }
    std::shared_ptr<FstReaderNode>  GetTransNode(size_t idx);
    bool FindInput(uint8_t input, uint32_t* result);
public:
    static std::shared_ptr<FstReaderNode> Mount(uint8_t* startPtr, uint64_t addrOffset, bool hasOutput);
public:
    uint8_t*                          m_startPtr;
    bool                              m_hasOutput;
    uint64_t                          m_addrOffset;

    bool                              m_isFinal;
    uint64_t                          m_finalOutput;
    std::vector<FstReaderTransPtr>    m_trans;
};
TYPEDEF_PTR(FstReaderNode);

///Fst reader class which is charge of read fst data
class FstReader {
public:
    class FstIterBound {
    public:
        enum FST_ITER_BOUND_TYPE_ENUM {
            FST_ITER_BOUND_TYPE_INCLUDED = 0,
            FST_ITER_BOUND_TYPE_EXCLUDED,
            FST_ITER_BOUND_TYPE_UNBOUNDED,
        };
    public:
        FstIterBound()
        : m_type(FST_ITER_BOUND_TYPE_UNBOUNDED)
        {}

        FstIterBound(FST_ITER_BOUND_TYPE_ENUM type, const string& bound)
        : m_type(type)
        {
            for (char ch: bound) {
                m_bound.push_back(ch);
            }
        }

        bool ExceededBy(const vector<uint8_t>& input) {
            switch (m_type) {
                case FST_ITER_BOUND_TYPE_UNBOUNDED:
                    return false;
                    break;

                case FST_ITER_BOUND_TYPE_INCLUDED:
                    for (size_t i = 0 ; i < input.size() && m_bound.size(); ++i) {
                        if (input[i] == m_bound[i]) continue;
                        return input[i] > m_bound[i];
                    }
                    return input.size() > m_bound.size();
                    break;

                case FST_ITER_BOUND_TYPE_EXCLUDED:
                    for (size_t i = 0 ; i < input.size() && m_bound.size(); ++i) {
                        if (input[i] == m_bound[i]) continue;
                        return input[i] > m_bound[i];
                    }
                    return input.size() >= m_bound.size();
                    break;
                default:
                    return false;
                    break;
            }
        }
        bool IsEmpty() {
            if (m_type == FST_ITER_BOUND_TYPE_UNBOUNDED) return true;
            else return m_bound.empty();
        }

        bool IsInclusive() {
            if (m_type == FST_ITER_BOUND_TYPE_EXCLUDED) return false;
            return true;
        }

    public:
        FST_ITER_BOUND_TYPE_ENUM    m_type;
        vector<uint8_t>             m_bound;
    };

    class IteratorResult;
    TYPEDEF_PTR(IteratorResult);
    class IteratorResult {
    public:
        IteratorResult()
        : m_output(0)
        {}

        IteratorResultPtr  Clone() {
            IteratorResultPtr r = std::make_shared<IteratorResult>();
            r->m_inputs = m_inputs;
            r->m_output = m_output;
            return r;
        }

        string GetInputStr() {
            string s;
            for (uint8_t c : m_inputs) {
                s.push_back(c);
            }
            return s;
        }
        void Print(std::ostream& os) {
            string s = GetInputStr();
            os << "Key:[" << s.c_str() << "],value:[" << m_output <<"]" << std::endl;
        }
    public:
        vector<uint8_t>     m_inputs;
        uint64_t            m_output;
    };

    class IteratorNode {
    public:
        IteratorNode(const FstReaderNodePtr& lastNode, AutomatonStatePtr lastAutState, uint32_t curTranIndex, uint64_t sumOutput)
        : m_lastNode(lastNode)
        , m_lastAutState(lastAutState)
        , m_curTransIndex(curTranIndex)
        , m_sumOutput(sumOutput)
        {}
    public:
        FstReaderNodePtr         m_lastNode;
        AutomatonStatePtr        m_lastAutState;
        uint32_t                 m_curTransIndex;
        uint64_t                 m_sumOutput;
    };

    class Iterator {
    public:
        Iterator() {m_startPtr = nullptr; }
        Iterator(uint8_t* startPtr, uint64_t addrOffset, const FstIterBound& min,
                 const FstIterBound& max, AutomatonPtr aut = std::make_shared<AlwaysAutomaton>());
        IteratorResultPtr Next();

    private:
        void SeekMin();
    private:
        uint8_t*                 m_startPtr;
        uint64_t                 m_addrOffset;
        bool                     m_hasOutput;
        stack<IteratorNode>      m_iterStack;
        FstIterBound             m_min;
        FstIterBound             m_max;
        AutomatonPtr             m_automaton;
        vector<uint8_t>          m_sumInputs;
        vector<uint64_t>         m_emptyOutput;
    };
public:
    FstReader(uint8_t* pData)
    : m_pData (pData)
    {
        m_hasOutput = (bool)*(m_pData+8);
    }
    ~FstReader() {}
public:
    Iterator GetIterator(const FstIterBound& min,const FstIterBound& max,AutomatonPtr aut = std::make_shared<AlwaysAutomaton>());

    ///accurate text string query
    Iterator GetMatchIterator(const FstIterBound& min,const FstIterBound& max,string str);

    ///range query
    Iterator GetRangeIterator(const FstIterBound& min,const FstIterBound& max);

    ///prefix query implements prefix automaton match
    Iterator GetPrefixIterator(const FstIterBound& min,const FstIterBound& max,string prefixstr);

    ///fuzzy query implements levenshtein automaton match
    Iterator GetFuzzyIterator(string str, uint32_t editDistance, uint32_t samePrefixLen);

    ///draw fst in dot file format
    void DotDraw( std::ostream& os);

    ///whether is a map or set
    bool HasOutput() { return m_hasOutput; }
private:
    ///recursively draw fst node in dot file format
    void DotDrawRecur(FstReaderNodePtr node,uint32_t& idx,vector<pair<uint8_t,string> >& inputs,std::unordered_map<uint64_t,std::pair<uint32_t,bool> >& offset2idxMap, std::ostream& os);
private:
    uint8_t*            m_pData;
    bool                m_hasOutput;
};
TYPEDEF_PTR(FstReader);


COMMON_END_NAMESPACE

#endif //__CPPFST_FST_CORE_FST__H__
