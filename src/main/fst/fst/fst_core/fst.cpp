/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       fst.cpp
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           2/3/21
  *Description:    file implements class to implements fst data structure: Finite state transducer
**********************************************************************************/
#include <cassert>
#include "fst/fst_core/fst.h"
#include "common/util/utf8_util.h"

STD_USE_NAMESPACE;

COMMON_BEGIN_NAMESPACE


TLOG_SETUP(COMMON_NS,FstBuilder);

FstWriteNodePtr FstBuilder::s_FinalTerminateNode = std::make_shared<FstWriteNode>(true);


void FstWriteNode::ResetTargetNode() {
    size_t transCnt = m_trans.size();
    for (size_t i = 0; i < transCnt; ++i) {
        m_trans[i]->m_targetNode = nullptr;
    }
}

void FstWriteNode::Dump(OutputStreamBase* outputStream, bool hasOutput) {
    /**
     *@brief        type of build node which be stored in only 1 byte to save bytes space serialization.
     *
     *              0  0  0  0  0  0  0   0
     *                          |  |  |   |----> isFinal for 1 bit: '1' indicates here finished for some term ends;
     *                          |  |  |                             '0' indicates is not.
     *                          |  |  |--------> transCount for 2 bits: '00' indicates no transitions;
     *                          |                                       '01' indicates only one transition;
     *                          |                                       '10' indicates more than one transitions.
     *                          |                                        NOTE THAT it will not serialize 'transCount' field this case for saving space during '00' and '10' cases.
     *                          |--------------->hasFinalOutput for 1 bit: '1' indicate has finalOutput, '0' indicates not.
     *                                                                    NOTE THAT it is '0' when isFinal is false.
     */
     uint8_t type = m_isFinal;
     size_t transCnt = m_trans.size();
     if (transCnt == 0) {
         if (hasOutput && m_finalOutput > 0) {
             type |= (0x1 << 3);
             outputStream->Write(&type,1);
             outputStream->Write((uint8_t*)&m_finalOutput,8);
         }
         else {
             outputStream->Write(&type,1);
         }
     }
     else if (transCnt == 1) {
         type |= (0x1 << 1);
         if (hasOutput && m_finalOutput > 0) {
             type |= (0x1 << 3);
             outputStream->Write(&type,1);
             outputStream->Write((uint8_t*)&m_finalOutput,8);
         }
         else {
             outputStream->Write(&type,1);
         }
         outputStream->Write((uint8_t*)&(m_trans[0]->m_input),1);
         if (hasOutput) {
             outputStream->Write((uint8_t*)&(m_trans[0]->m_output),8);
         }
         outputStream->Write((uint8_t*)&(m_trans[0]->m_targetAddrOffset),8);
     }
     else {
         type |= (0x1 << 2);
         if (hasOutput && m_finalOutput > 0) {
             type |= (0x1 << 3);
             outputStream->Write(&type,1);
             outputStream->Write((uint8_t*)&m_finalOutput,8);
         }
         else {
             outputStream->Write(&type,1);
         }
         uint8_t tmpTransCnt = transCnt;
         outputStream->Write((uint8_t*)&tmpTransCnt,1);
         for (size_t i = 0; i < transCnt; ++i) {
             outputStream->Write((uint8_t*)&(m_trans[i]->m_input),1);
             if (hasOutput) {
                 outputStream->Write((uint8_t*)&(m_trans[i]->m_output),8);
             }
             outputStream->Write((uint8_t*)&(m_trans[i]->m_targetAddrOffset),8);
         }
     }
}

uint64_t FstBuilder::FreezeNodes(FstWriteNodePtr node) {
    if (node->m_trans.empty()) {
        return FreezeNode(node);
    }
    node->m_trans.back()->m_targetAddrOffset = FreezeNodes(node->m_trans.back()->m_targetNode);
    node->ResetTargetNode();
    return FreezeNode(node);
}

uint64_t FstBuilder::FreezeNode(FstWriteNodePtr node) {
    if (nullptr == node) return 0;
    uint64_t addr;
    if (m_node2AddrOffsetMap.Get(node,addr)) {
        return addr;
    }
    uint64_t addrOffset = m_outputStream->GetTotalBytesCnt();
    bool ret = m_node2AddrOffsetMap.Put(node,addrOffset);
    assert(ret);
    node->Dump(m_outputStream,m_hasOutput);
    return addrOffset;
}

void FstBuilder::Finish() {
    uint64_t rootAddrOffset = FreezeNodes(m_rootNode);
    m_outputStream->WriteAt(0,(uint8_t*)&rootAddrOffset,8);
    m_outputStream->Flush();
}

static string bytes2str(const uint8_t* key, uint32_t len) {
    if (nullptr == key || len < 1) return "";
    string s;
    for (uint32_t i = 0; i < len; ++i) {
        s.push_back(key[i]);
    }
    return s;
}

bool FstBuilder::Insert(const uint8_t* key, uint32_t len, uint64_t value) {
    //when len == 0, consider it to be empty string , it is valid.
    if (nullptr == key) {
        TLOG_LOG(ERROR,"invalid input key to insert into fst builder!");
        return false;
    }
    FstWriteNodePtr pNode = m_rootNode;
    uint32_t keyPos = 0;
    FstBuildTransPtr lastTrans;
    while (!pNode->m_trans.empty() &&  keyPos < len) {
        lastTrans = pNode->m_trans.back();
        if (key[keyPos] == lastTrans->m_input) {
            pNode = lastTrans->m_targetNode;

            if (m_hasOutput) {
                uint64_t prefixValue = std::min(value,lastTrans->m_output);
                value -= prefixValue;
                uint64_t addPrefixValue = lastTrans->m_output - prefixValue;
                lastTrans->m_output = prefixValue;

                if (addPrefixValue > 0) {
                    if (pNode->m_isFinal) {
                        pNode->m_finalOutput += addPrefixValue;
                    }
                    for(FstBuildTransPtr trans : pNode->m_trans) {
                        trans->m_output += addPrefixValue;
                    }
                }
            }
            ++keyPos;
            continue;
        }
        else if (key[keyPos] < lastTrans->m_input) {
            TLOG_LOG(ERROR,"invalid input key:[%s] to insert into fst builder,which is not larger than last key!",
                     bytes2str(key,len).c_str());
            return false;
        }
        else if (key[keyPos] > lastTrans->m_input) {
            break;
        }
    }
    if (keyPos == len) {
        //empty string
        if (len == 0 && !pNode->m_trans.empty()) {
            TLOG_LOG(ERROR,"invalid input empty string key to insert into fst builder,which is not larger than last key!");
            return false;
        }
        else {
            if (pNode->m_isFinal) {
                TLOG_LOG(INFO,"Found input key:[%s] is already inserted into fst builder,update its value.", bytes2str(key,len).c_str());
            }
            else {
                pNode->SetIsFinal(true);
            }
            if (m_hasOutput) {
                pNode->m_finalOutput = value;
            }
            return true;
        }
    }
    else {
        if (!pNode->m_trans.empty()) {
            pNode->m_trans.back()->m_targetAddrOffset = FreezeNodes(
                    pNode->m_trans.back()->m_targetNode );
        }
        FstWriteNodePtr tmpNode = pNode;
        bool bForFirst = true;
        while (keyPos < len) {
            FstBuildTransPtr trans = std::make_shared<FstBuildTrans>(key[keyPos]);
            if (m_hasOutput && bForFirst) {
                trans->m_output = value;
                bForFirst = false;
            }
            FstWriteNodePtr nextNode;
            if (keyPos == len-1) {
                nextNode = std::make_shared<FstWriteNode>(true);
            }
            else {
                nextNode = std::make_shared<FstWriteNode>(false);
            }
            trans->m_targetNode = nextNode;
            tmpNode->m_trans.push_back(trans);
            tmpNode = nextNode;
            ++keyPos;
        }
        return true;
    }
}

std::shared_ptr<FstReaderNode> FstReaderNode::Mount(uint8_t* startPtr, uint64_t addrOffset, bool hasOutput) {
    if (nullptr == startPtr) return nullptr;
    uint8_t* ptr = startPtr + addrOffset;

    FstReaderNodePtr node = std::make_shared<FstReaderNode>();
    node->m_startPtr = startPtr;
    node->m_addrOffset = addrOffset;
    node->m_hasOutput = hasOutput;


    uint8_t type = *ptr;
    ++ptr;
    node->m_isFinal = (type & 0x1);
    uint32_t transCnt = ((type & 6) >> 1);
    bool hasFinalOutput = hasOutput && ((type & (0x1<<3)) >> 3);

    if (transCnt == 0) {
        if (hasFinalOutput) {
            node->m_finalOutput = *(uint64_t*)ptr;
            ptr += 8;
        }
    }
    else if (transCnt == 1) {
        if (hasFinalOutput) {
            node->m_finalOutput = *(uint64_t*)ptr;
            ptr += 8;
        }
        uint8_t input = *ptr;
        ++ptr;
        uint64_t output = 0;
        if (hasOutput) {
            output = *(uint64_t*)ptr;
            ptr += 8;
        }
        uint64_t addrOffset = *(uint64_t*)ptr;
        ptr += 8;
        FstReaderTransPtr trans = std::make_shared<FstReaderTrans>(input,output,addrOffset);
        node->m_trans.push_back(trans);
    }
    else {
        if (hasFinalOutput) {
            node->m_finalOutput = *(uint64_t*)ptr;
            ptr += 8;
        }
        transCnt = *ptr;
        ++ptr;
        for (uint32_t i = 0; i < transCnt; ++i) {
            uint8_t input = *ptr;
            ++ptr;

            uint64_t output = 0;
            if (hasOutput) {
                output = *(uint64_t*)ptr;
                ptr += 8;
            }

            uint64_t addrOffset = *(uint64_t*)ptr;
            ptr += 8;
            FstReaderTransPtr trans = std::make_shared<FstReaderTrans>(input,output,addrOffset);
            node->m_trans.push_back(trans);
        }
    }
    return node;
}

std::shared_ptr<FstReaderNode> FstReaderNode::GetTransNode(size_t idx) {
    if (idx >= m_trans.size()) return nullptr;
    return FstReaderNode::Mount(m_startPtr, m_trans[idx]->m_targetAddrOffset,m_hasOutput);
}

bool FstReaderNode::FindInput(uint8_t input, uint32_t* result) {
    assert(nullptr != result);
    uint32_t sz = m_trans.size();
    if (sz < 8) {
        for (uint32_t i = 0; i < sz; ++i) {
            if (m_trans[i]->m_input == input) {
                *result = i;
                return true;
            }
            else if (m_trans[i]->m_input > input) {
                *result = i;
                return false;
            }
        }
        *result = sz;
        return false;
    }
    else {
        //binary search for fast
        int st = 0, ed = sz -1, mid;
        while (st <= ed) {
            mid = st + (ed-st)/2;
            if (m_trans[mid]->m_input == input) {
                *result = mid;
                return true;
            }
            else if (m_trans[mid]->m_input > input) {
                if (st == ed) {
                    *result = mid;
                    return false;
                }
                ed = mid -1;
            }
            else {
                if (st == ed) {
                    *result = mid + 1;
                    return false;
                }
                st = mid + 1;
            }
        }
        return false;
    }
}

void FstReader::DotDraw( std::ostream& os) {
    FstReaderNodePtr rootNode = FstReaderNode::Mount(m_pData,*(uint64_t*)m_pData,m_hasOutput);
    os << "digraph fst {" << endl;
    os << "\t\tlabelloc=\"l\";" << endl;
    os << "\t\tlabeljust=\"l\";" << endl;
    os << "\t\trankdir=\"UD\";" << endl << endl;
    vector<pair<uint8_t,string> > inputs;
    std::unordered_map<uint64_t,std::pair<uint32_t,bool> > offset2idxMap;
    uint32_t idx = 0;
    DotDrawRecur(rootNode,idx,inputs,offset2idxMap, os);
    os << "}" << endl;
}


void FstReader::DotDrawRecur(FstReaderNodePtr node,uint32_t& idx,vector<pair<uint8_t,string> >& inputs, std::unordered_map<uint64_t,std::pair<uint32_t,bool> >& offset2idxMap, std::ostream& os) {
    Utf8Util::UTF8Visible(inputs);

    std::unordered_map<uint64_t,std::pair<uint32_t,bool> >::iterator it = offset2idxMap.find(node->m_addrOffset);
    if (it == offset2idxMap.end()) {
        offset2idxMap.insert(std::make_pair(node->m_addrOffset,std::make_pair(idx++,true)));
    }
    else {
        //already print
        if (it->second.second) {
            return;
        }
        else {
            it->second.second = true;
        }
    }
    it = offset2idxMap.find(node->m_addrOffset);
    uint32_t nodeIdx = it->second.first;
    string nodeLabelStr;
    {
        ostringstream oss;
        oss << "[" << "label=\"" << nodeIdx;
        if (node->m_finalOutput > 0) {
            oss << "/" << node->m_finalOutput;
        }
        oss <<"\"";
        if (node->m_isFinal) {
            oss << ",peripheries=2";
        }
        oss << "]" << std::flush;
        nodeLabelStr = oss.str();
    }
    os << "\t\t" << nodeIdx << nodeLabelStr << endl;

    for (size_t i = 0; i < node->GetTransCount(); ++i) {
        FstReaderTransPtr trans = node->m_trans[i];
        FstReaderNodePtr subNode = node->GetTransNode(i);

        inputs.push_back(make_pair(trans->m_input,""));
        DotDrawRecur(subNode,idx,inputs,offset2idxMap,os);

        if (offset2idxMap.find(subNode->m_addrOffset) == offset2idxMap.end()) {
            offset2idxMap.insert(std::make_pair(subNode->m_addrOffset,std::make_pair(idx++,false)));
        }
        uint32_t subNodeIdx = offset2idxMap.find(subNode->m_addrOffset)->second.first;
        os << "\t\t" << nodeIdx << " -> " << subNodeIdx
        << " [label=\"";
        if (Utf8Util::IsAscii(trans->m_input)) {
            //ascii
            os << trans->m_input;
        }
        else {
            os << "0x" << std::hex<< static_cast<unsigned short>(trans->m_input) << "[" << inputs.back().second<<"]" << std::dec;
        }
        if (trans->m_output > 0) {
            os << "/" << trans->m_output;
        }
        os << "\"]" << endl;

        inputs.pop_back();
    }
}

FstReader::Iterator::Iterator(uint8_t* startPtr,
                           uint64_t addrOffset,
                           const FstIterBound& min,
                           const FstIterBound& max,
                           AutomatonPtr aut /*= std::make_shared<AlwaysAutomaton>()*/)
: m_startPtr (startPtr)
, m_addrOffset(addrOffset)
, m_min(min)
, m_max(max)
, m_automaton(aut)
{
    m_hasOutput = *(m_startPtr+8);
    SeekMin();
}

void FstReader::Iterator::SeekMin() {
    FstReaderNodePtr rootNode = FstReaderNode::Mount(m_startPtr,m_addrOffset,m_hasOutput);
    if (m_min.IsEmpty()) {
        if (m_min.IsInclusive()) {
            if (rootNode->m_isFinal) {
                m_emptyOutput.push_back(rootNode->m_finalOutput);
            }
        }
        m_iterStack.push(IteratorNode(rootNode, m_automaton->Start(),0,0));
        return;
    }
    FstReaderNodePtr lastFstNode = rootNode;
    uint64_t sumOutput = 0;
    AutomatonStatePtr lastAutState = m_automaton->Start();
    for (uint8_t b : m_min.m_bound) {
        uint32_t idx = 0;
        if (lastFstNode->FindInput(b,&idx)) {
            m_iterStack.push(IteratorNode(lastFstNode, lastAutState,idx+1,sumOutput));

            m_sumInputs.push_back(b);

            sumOutput += lastFstNode->m_trans[idx]->m_output;
            lastAutState = m_automaton->Accept(lastAutState,m_sumInputs);
            lastFstNode =  lastFstNode->GetTransNode(idx);

        }
        else {
            m_iterStack.push(IteratorNode(lastFstNode, lastAutState,idx,sumOutput));
            return;
        }
    }
    if (!m_iterStack.empty()) {
        if (m_min.IsInclusive()) {
            m_iterStack.top().m_curTransIndex -= 1;
            m_sumInputs.pop_back();
        }
        else {
            m_iterStack.push(IteratorNode(lastFstNode, lastAutState,0,sumOutput));
        }
    }
}


FstReader::IteratorResultPtr FstReader::Iterator::Next() {
    if (m_emptyOutput.size()) {
        uint64_t emptyOut = m_emptyOutput.back();
        m_emptyOutput.clear();
        if (m_max.ExceededBy(vector<uint8_t>())) {
            m_iterStack = stack<IteratorNode>();
            return nullptr;
        }
        AutomatonStatePtr startAutState = m_automaton->Start();
        if (m_automaton->IsMatch(startAutState)) {
            IteratorResultPtr result = std::make_shared<IteratorResult>();
            result->m_output = emptyOut;
            return result;
        }
    }

    while (!m_iterStack.empty()) {
        IteratorNode curNode = m_iterStack.top();
        m_iterStack.pop();
        if (curNode.m_curTransIndex >= curNode.m_lastNode->GetTransCount()
           || !m_automaton->CanMatch(curNode.m_lastAutState)) {
            if (curNode.m_lastNode->m_addrOffset != m_addrOffset) {
                m_sumInputs.pop_back();
            }
            continue;
        }

        IteratorNode nextNode = curNode;
        nextNode.m_curTransIndex++;
        m_iterStack.push(nextNode);

        FstReaderTransPtr curTrans = curNode.m_lastNode->m_trans[curNode.m_curTransIndex];

        m_sumInputs.push_back(curTrans->m_input);

        uint64_t sumOutput = curNode.m_sumOutput + curTrans->m_output;
        FstReaderNodePtr  subNode = curNode.m_lastNode->GetTransNode(curNode.m_curTransIndex);
        AutomatonStatePtr nextAutState = m_automaton->Accept(curNode.m_lastAutState,m_sumInputs);

        m_iterStack.push(IteratorNode(subNode, nextAutState,0,sumOutput));
        if (m_max.ExceededBy(m_sumInputs)) {
            m_iterStack = stack<IteratorNode>();
            return nullptr;
        }
        if (subNode->m_isFinal && m_automaton->IsMatch(nextAutState)) {
            IteratorResultPtr result = std::make_shared<IteratorResult>();
            result->m_output += (sumOutput + subNode->m_finalOutput);
            result->m_inputs = m_sumInputs;
            return result;
        }
    }

    return nullptr;
}

FstReader::Iterator FstReader::GetIterator(const FstIterBound& min,const FstIterBound& max,AutomatonPtr aut /*= std::make_shared<AlwaysAutomaton>()*/) {
    return FstReader::Iterator(m_pData,*(uint64_t*)m_pData, min,max,aut);
}

FstReader::Iterator FstReader::GetFuzzyIterator(string str, uint32_t editDistance, uint32_t samePrefixLen, bool isUseDamerauLevenshtein) {
    vector<string> utf8strs;
    Utf8Util::String2utf8(str,utf8strs);
    if (samePrefixLen > utf8strs.size()) {
        samePrefixLen = utf8strs.size();
    }
    if (samePrefixLen==0) {
        return isUseDamerauLevenshtein ?
          GetIterator(FstIterBound(),FstIterBound(),std::make_shared<DamerauLevenshteinAutomaton>(str,editDistance))
        : GetIterator(FstIterBound(),FstIterBound(),std::make_shared<LevenshteinAutomaton>(str,editDistance));
    }
    else {
        string prefix;
        for (size_t i = 0; i < samePrefixLen; ++i) {
            prefix += utf8strs[i];
        }
        AutomatonPtr prefixAut = std::make_shared<PrefixAutomaton>(prefix);
        AutomatonPtr levAut = ( isUseDamerauLevenshtein ?
        (AutomatonPtr)std::make_shared<DamerauLevenshteinAutomaton>(str,editDistance)
        : (AutomatonPtr)std::make_shared<LevenshteinAutomaton>(str,editDistance) );
        AutomatonPtr aut = Intersect(prefixAut,levAut);
        return GetIterator(FstIterBound(),FstIterBound(),aut);
    }
}

FstReader::Iterator FstReader::GetMatchIterator(const FstIterBound& min,const FstIterBound& max,string str) {
    return GetIterator(min,max,std::make_shared<StrAutomaton>(str));
}

FstReader::Iterator FstReader::GetPrefixIterator(const FstIterBound& min,const FstIterBound& max,string prefixstr) {
    return GetIterator(min,max,std::make_shared<PrefixAutomaton>(prefixstr));
}

FstReader::Iterator FstReader::GetRangeIterator(const FstIterBound& min,const FstIterBound& max ) {
    return GetIterator(min,max,std::make_shared<AlwaysAutomaton>());
}

COMMON_END_NAMESPACE
