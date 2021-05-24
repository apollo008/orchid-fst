/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       automaton.cpp
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           3/1/21
  *Description:    file implements automaton responding class and its implementation
**********************************************************************************/
#include "fst/fst_core/automaton.h"
#include <stack>
#include <cassert>

STD_USE_NAMESPACE;
COMMON_BEGIN_NAMESPACE

StartsWithAutomaton::DoneStatePtr StartsWithAutomaton::DoneState::s_doneState = std::make_shared<StartsWithAutomaton::DoneState>();
TrueAutomatonStatePtr TrueAutomatonState::s_trueAutoState = std::make_shared<TrueAutomatonState>();

///this method is so useful for UTF8 code
string Automaton::IsLastValidUtf8Str(const vector<uint8_t>& byteVec) {
    string s;
    uint32_t nByte;
    for (vector<uint8_t>::const_reverse_iterator rit = byteVec.rbegin();
         rit != byteVec.rend(); ++rit) {
        s.push_back(*rit);
        if (Utf8Util::IsUtf8Beginning(*rit,nByte)) {
            if (s.size() == nByte) {
                std::reverse(s.begin(), s.end());
                return s;
            }
            break;
        }
    }
    s.clear();
    return s;
}

AutomatonPtr Intersect(AutomatonPtr& aut1, AutomatonPtr& aut2) {
    return std::make_shared<IntersectAutomaton>(aut1,aut2);
}
AutomatonPtr Union(AutomatonPtr& aut1, AutomatonPtr& aut2) {
    return std::make_shared<UnionAutomaton>(aut1,aut2);
}
AutomatonPtr Not(AutomatonPtr& aut) {
    return std::make_shared<NotAutomaton>(aut);
}
AutomatonPtr StartsWith(AutomatonPtr& aut) {
    return std::make_shared<StartsWithAutomaton>(aut);
}

StrAutomaton::StrAutomaton(const string& str)
: m_str(str)
{
    Utf8Util::String2utf8(m_str,m_utf8strs);
}


AutomatonStatePtr StrAutomaton::Start() {
    return std::make_shared<StrAutomatonState>(0);
}

bool StrAutomaton::IsMatch(const AutomatonStatePtr &state) {
    if (nullptr == state) return false;
    return dynamic_pointer_cast<StrAutomatonState>(state)->m_matchedLength == m_utf8strs.size();
}

bool StrAutomaton::CanMatch(const AutomatonStatePtr &state) {
    if (nullptr == state) return false;
    return dynamic_pointer_cast<StrAutomatonState>(state)->m_matchedLength < m_utf8strs.size();
}

AutomatonStatePtr StrAutomaton::Accept(const AutomatonStatePtr & state,
                                                       const vector<uint8_t>& byteVec) {
    if (nullptr == state) return nullptr;
    string s = Automaton::IsLastValidUtf8Str(byteVec);
    if (s.empty()) return state;
    size_t len = dynamic_pointer_cast<StrAutomatonState>(state)->m_matchedLength;
    return (len < m_utf8strs.size() && (s == m_utf8strs[len]))
    ? std::make_shared<StrAutomatonState>(len+1) : nullptr;
}


AutomatonStatePtr GreaterThanAutomaton::Start() {
    return std::make_shared<GreaterThanAutomatonState>(0,true);
}

bool GreaterThanAutomaton::IsMatch(const AutomatonStatePtr &state) {
    if (nullptr == state) return false;
    GreaterThanAutomatonStatePtr  st = dynamic_pointer_cast<GreaterThanAutomatonState>(state);
    return !st->m_isEqualMatchBefore
    || st->m_matchedLength > m_utf8strs.size()
    || (st->m_matchedLength == m_utf8strs.size() && m_inclusive);
}

bool GreaterThanAutomaton::CanMatch(const AutomatonStatePtr &state) {
    return nullptr != state;
}

AutomatonStatePtr GreaterThanAutomaton::Accept(const AutomatonStatePtr & state,
                                                               const vector<uint8_t>& byteVec) {
    if (nullptr == state) return nullptr;

    string s = Automaton::IsLastValidUtf8Str(byteVec);
    if (s.empty()) return state;

    GreaterThanAutomatonStatePtr st = dynamic_pointer_cast<GreaterThanAutomatonState>(state);
    if (!st->m_isEqualMatchBefore) {
        return st;
    }
    else {
        if (st->m_matchedLength >= m_utf8strs.size()) {
            return std::make_shared<GreaterThanAutomatonState>(st->m_matchedLength, false);
        }
        else {
            if (s > m_utf8strs[st->m_matchedLength]) {
                return std::make_shared<GreaterThanAutomatonState>(st->m_matchedLength, false);
            }
            else if (s < m_utf8strs[st->m_matchedLength]) {
                return nullptr;
            }
            else {
                assert(s == m_utf8strs[st->m_matchedLength]);
                return std::make_shared<GreaterThanAutomatonState>(st->m_matchedLength+1, true);
            }
        }
    }
}


AutomatonStatePtr LessThanAutomaton::Start() {
    return std::make_shared<LessThanAutomatonState>(0,true);
}

bool LessThanAutomaton::IsMatch(const AutomatonStatePtr &state) {
    if (nullptr == state) return false;
    LessThanAutomatonStatePtr  st = dynamic_pointer_cast<LessThanAutomatonState>(state);
    return !st->m_isEqualMatchBefore
           || st->m_matchedLength < m_utf8strs.size()
           || (st->m_matchedLength == m_utf8strs.size() && m_inclusive);
}

bool LessThanAutomaton::CanMatch(const AutomatonStatePtr &state) {
    if (nullptr == state) return false;
    LessThanAutomatonStatePtr  st = dynamic_pointer_cast<LessThanAutomatonState>(state);
    return !st->m_isEqualMatchBefore || st->m_matchedLength < m_utf8strs.size();
}

AutomatonStatePtr LessThanAutomaton::Accept(const AutomatonStatePtr & state,
                                                            const vector<uint8_t>& byteVec) {
    if (nullptr == state) return nullptr;

    string s = Automaton::IsLastValidUtf8Str(byteVec);
    if (s.empty()) return state;

    LessThanAutomatonStatePtr  st = dynamic_pointer_cast<LessThanAutomatonState>(state);
    if (!st->m_isEqualMatchBefore) {
        return st;
    }
    else {
        if (st->m_matchedLength > m_utf8strs.size()) {
            return st;
        }
        if (st->m_matchedLength == m_utf8strs.size()) {
            return std::make_shared<LessThanAutomatonState>(st->m_matchedLength+1, true);
        }
        else {
            if (s > m_utf8strs[st->m_matchedLength]) {
                return nullptr;
            }
            else if (s < m_utf8strs[st->m_matchedLength]) {
                return std::make_shared<LessThanAutomatonState>(st->m_matchedLength, false);
            }
            else {
                assert(s == m_utf8strs[st->m_matchedLength]);
                return std::make_shared<LessThanAutomatonState>(st->m_matchedLength+1, true);
            }
        }
    }
}



AutomatonStatePtr PrefixAutomaton::Start() {
    return std::make_shared<PrefixAutomatonState>(0);
}

bool PrefixAutomaton::IsMatch(const AutomatonStatePtr &state) {
    if (nullptr == state) return false;
    return dynamic_pointer_cast<PrefixAutomatonState>(state)->m_matchedLength >= m_utf8strs.size();
}

bool PrefixAutomaton::CanMatch(const AutomatonStatePtr &state) {
    return nullptr != state;
}

AutomatonStatePtr PrefixAutomaton::Accept(const AutomatonStatePtr & state,
                                                          const vector<uint8_t>& byteVec) {
    if (nullptr == state) return nullptr;

    string s = Automaton::IsLastValidUtf8Str(byteVec);
    if (s.empty()) return state;

    PrefixAutomatonStatePtr st = dynamic_pointer_cast<PrefixAutomatonState>(state);
    if (st->m_matchedLength >= m_utf8strs.size()) {
        return st;
    }
    else if (m_utf8strs[st->m_matchedLength] == s){
        return std::make_shared<PrefixAutomatonState>(st->m_matchedLength+1);
    }
    else return nullptr;
}


AutomatonStatePtr LevenshteinAutomaton::Start() {
    vector<size_t> edits;
    for (size_t i = 0; i <= m_utf8strs.size(); ++i) {
        edits.push_back( std::min((size_t)m_editDistance+1,i));
    }
    return std::make_shared<LevenshteinAutomatonState>(edits);
}


bool LevenshteinAutomaton::IsMatch(const AutomatonStatePtr &state) {
    if (nullptr == state) return false;
    LevenshteinAutomatonStatePtr st = dynamic_pointer_cast<LevenshteinAutomatonState>(state);
    return st->m_curEdits.back() <= m_editDistance;
}

bool LevenshteinAutomaton::CanMatch(const AutomatonStatePtr &state) {
    if (nullptr == state) return false;
    LevenshteinAutomatonStatePtr st = dynamic_pointer_cast<LevenshteinAutomatonState>(state);
    for (size_t i = 0; i < st->m_curEdits.size(); ++i) {
        if (st->m_curEdits[i] <= m_editDistance) return true;
    }
    return false;
}

AutomatonStatePtr LevenshteinAutomaton::Accept(const AutomatonStatePtr &ptr, const vector<uint8_t>& byteVec) {
    if (nullptr == ptr) return nullptr;
    string s = Automaton::IsLastValidUtf8Str(byteVec);
    if (s.empty()) return ptr;

    LevenshteinAutomatonStatePtr st = dynamic_pointer_cast<LevenshteinAutomatonState>(ptr);
    StateCacheMapType::iterator it1 = m_statesCacheMap.find(st);
    if (it1 == m_statesCacheMap.end()) {
        return nullptr;
    }

    auto it2 = it1->second->find(s);
    if (it2 != it1->second->end()) {
        return it2->second;
    }
    else {
        it2 = it1->second->find("");
        return it2 == it1->second->end() ? nullptr : it2->second;
    }
}

void LevenshteinAutomaton::buildDfa() {

    m_statesCacheMap.clear();

    stack<LevenshteinAutomatonStatePtr> statesStack;
    statesStack.push((dynamic_pointer_cast<LevenshteinAutomatonState>(Start())));

    set<string> transStrSet;

    while (!statesStack.empty()) {

        LevenshteinAutomatonStatePtr lastState = statesStack.top();
        statesStack.pop();

        transStrSet.clear();

        std::shared_ptr<unordered_map<string,LevenshteinAutomatonStatePtr> > trans2StateMap
        = std::make_shared<unordered_map<string,LevenshteinAutomatonStatePtr> >();

        vector<size_t> newState;
        for (size_t ix = 0; ix < m_utf8strs.size(); ++ix) {
            if (lastState->m_curEdits[ix] > m_editDistance) continue;

            if (transStrSet.find(m_utf8strs[ix]) != transStrSet.end()) continue;
            transStrSet.insert(m_utf8strs[ix]);

            newState.clear();
            newState.push_back(std::min(lastState->m_curEdits[0] + 1,(size_t)m_editDistance + 1));
            for (size_t k = 1; k <= m_utf8strs.size(); ++k) {
                size_t cost = ((m_utf8strs[ix] == m_utf8strs[k-1]) ? 0 : 1);
                size_t distance = std::min(std::min(lastState->m_curEdits[k-1] + cost, lastState->m_curEdits[k] + 1),newState[k-1] + 1);
                newState.push_back(std::min(distance,(size_t)m_editDistance+1));
            }
            LevenshteinAutomatonStatePtr newStPtr = std::make_shared<LevenshteinAutomatonState>(newState);
            if (!CanMatch(newStPtr)) continue;

            trans2StateMap->insert(std::make_pair(m_utf8strs[ix],newStPtr));

            if (newStPtr != lastState && m_statesCacheMap.find(newStPtr) == m_statesCacheMap.end()) {
                statesStack.push(newStPtr);
            }
        }
        do {
            //consider special char not in 'm_str', use special empty string for this case
            newState.clear();
            newState.push_back(std::min(lastState->m_curEdits[0] + 1,(size_t)m_editDistance+1));
            size_t cost = 1;
            for (size_t k = 1; k <= m_utf8strs.size(); ++k) {
                size_t distance = std::min(std::min(lastState->m_curEdits[k-1] + cost, lastState->m_curEdits[k] + 1),newState[k-1] + 1);
                newState.push_back(std::min(distance,(size_t)m_editDistance+1));
            }
            LevenshteinAutomatonStatePtr newStPtr = std::make_shared<LevenshteinAutomatonState>(newState);
            if (!CanMatch(newStPtr)) break;

            trans2StateMap->insert(std::make_pair(string(),newStPtr));

            if (newStPtr != lastState && m_statesCacheMap.find(newStPtr) == m_statesCacheMap.end()) {
                statesStack.push(newStPtr);
            }
        } while (false);

        if (!trans2StateMap->empty()) {
            m_statesCacheMap.insert(std::make_pair(lastState,trans2StateMap));
        }
    }
}

DamerauLevenshteinAutomatonState::DamerauLevenshteinAutomatonState(DISTANCE_SEQUENCE_PTR curEdits,
                                                                   DISTANCE_SEQUENCE_PTR prevEdits,
                                                                   const string&         prevStr,
                                                                   bool                  isPrevStrInQueryStr,
                                                                   UTF8_QUERY_STRS_PTR   utf8QueryStrs,
                                                                   uint32_t              editDistance )
        : curEdits_(curEdits)
        , prevEdits_(prevEdits)
        , prevStr_(prevStr)
        , isPrevStrInQueryStr_(isPrevStrInQueryStr)
        , utf8QueryStrs_(utf8QueryStrs)
        , editDistance_(editDistance)
{}

bool DamerauLevenshteinAutomatonState::IsPossibleTransposition() {
    bool bNeedTransPositions = false;

    string lastStr;
    for (size_t i = 0; i < utf8QueryStrs_->size(); ++i) {
        string str = utf8QueryStrs_->operator[](i);

        if ( (i > 0) && (str != lastStr) && (prevStr_.empty() ? false: (prevStr_==str)) ) {
            size_t prevLLastState = (prevEdits_?(prevEdits_->operator[](i-1)):0);
            size_t curLastState = curEdits_->operator[](i);
            size_t curCurState = curEdits_->operator[](i+1);
            if ( (prevLLastState < editDistance_) && (prevLLastState < curLastState) && (prevLLastState < curCurState)) {
                bNeedTransPositions = true;
                break;
            }
        }
        lastStr = str;
    }
    return bNeedTransPositions;
}

bool DamerauLevenshteinAutomatonState::IsDistanceSequencesEqual(
        const DISTANCE_SEQUENCE& seq1, const DISTANCE_SEQUENCE& seq2) {
    if (seq1.size() != seq2.size()) return false;
    for (size_t i = 0; i < seq1.size(); ++i) {
        if (seq1[i] != seq2[i]) return false;
    }
    return true;
}




void DamerauLevenshteinAutomatonState::GetPossibleTranspositionStrs( unordered_set<string>& resultStrs) {
    resultStrs.clear();
    string lastStr;
    for (size_t k = 1; k <= utf8QueryStrs_->size(); ++k) {
        string tmpstr = utf8QueryStrs_->operator[](k-1);
        if ( k>1 &&  tmpstr != lastStr && prevStr_ == tmpstr ) {
            size_t prevLLastState = (prevEdits_?(prevEdits_->operator[](k-2)):0);
            size_t curLastState = curEdits_->operator[](k-1);
            size_t curCurState = curEdits_->operator[](k);
            if (prevLLastState < editDistance_ && prevLLastState < curLastState && prevLLastState < curCurState) {
                resultStrs.insert(lastStr);
            }
        }
        lastStr = tmpstr;
    }
}



void DamerauLevenshteinAutomaton::buildDfa() {
    m_statesCacheMap.clear();

    stack<DamerauLevenshteinAutomatonStatePtr> statesStack;
    statesStack.push((dynamic_pointer_cast<DamerauLevenshteinAutomatonState>(Start())));

    set<string> transStrSet;

    while (!statesStack.empty()) {
        DamerauLevenshteinAutomatonStatePtr lastState = statesStack.top();
        statesStack.pop();
        transStrSet.clear();
        std::shared_ptr<unordered_map<string,DamerauLevenshteinAutomatonStatePtr> > trans2StateMap
                = std::make_shared<unordered_map<string,DamerauLevenshteinAutomatonStatePtr> >();
        vector<size_t> newState;
        unordered_set<string> possibleTranspositionStrs(16);
        for (size_t ix = 0; ix < m_utf8strs->size(); ++ix) {
            string curstr = m_utf8strs->operator[](ix);
            lastState->GetPossibleTranspositionStrs(possibleTranspositionStrs);
            if (!possibleTranspositionStrs.count(curstr) && lastState->curEdits_->operator[](ix) > m_editDistance) {
                continue;
            }
            if (transStrSet.find(m_utf8strs->operator[](ix)) != transStrSet.end()) continue;
            transStrSet.insert(m_utf8strs->operator[](ix));

            newState.clear();
            newState.push_back(std::min(lastState->curEdits_->operator[](0) + 1,(size_t)m_editDistance + 1));
            string lastStr;
            for (size_t k = 1; k <= m_utf8strs->size(); ++k) {
                size_t cost = ((curstr == m_utf8strs->operator[](k-1)) ? 0 : 1);
                size_t distance = std::min(std::min(lastState->curEdits_->operator[](k-1) + cost,
                                                    lastState->curEdits_->operator[](k) + 1), newState[k-1] + 1);
                if ( (k>1)
                     && !lastState->prevStr_.empty()
                     && curstr == lastStr
                     && lastState->prevStr_ == m_utf8strs->operator[](k-1) ) {
                    distance = std::min(distance, (lastState->prevEdits_?lastState->prevEdits_->operator[](k-2):m_editDistance) +1 );
                }
                newState.push_back(std::min(distance,(size_t)m_editDistance+1));
                lastStr = m_utf8strs->operator[](k-1);
            }
            DamerauLevenshteinAutomatonStatePtr newStPtr = std::make_shared<DamerauLevenshteinAutomatonState>(
                    std::make_shared<DamerauLevenshteinAutomatonState::DISTANCE_SEQUENCE>(newState),
                            lastState->curEdits_,
                            curstr,
                            m_bStrOccursMap.find(curstr) != m_bStrOccursMap.end(),
                            m_utf8strs,
                            m_editDistance );
            if (!CanMatch(newStPtr)) continue;
            trans2StateMap->insert(std::make_pair(curstr,newStPtr));
            if (newStPtr != lastState && m_statesCacheMap.find(newStPtr) == m_statesCacheMap.end()) {
                statesStack.push(newStPtr);
            }
        }
        do {
            //consider special char not in 'm_str', use special empty string for this case
            newState.clear();
            newState.push_back(std::min(lastState->curEdits_->operator[](0) + 1,(size_t)m_editDistance+1));
            size_t cost = 1;
            for (size_t k = 1; k <= m_utf8strs->size(); ++k) {
                size_t distance = std::min(std::min(lastState->curEdits_->operator[](k-1) + cost,
                                                    lastState->curEdits_->operator[](k) + 1),newState[k-1] + 1);
                newState.push_back(std::min(distance,(size_t)m_editDistance+1));
            }
            DamerauLevenshteinAutomatonStatePtr newStPtr = std::make_shared<DamerauLevenshteinAutomatonState>(
                    std::make_shared<DamerauLevenshteinAutomatonState::DISTANCE_SEQUENCE>(newState),
                    lastState->curEdits_,
                    string(),
                    false,
                    m_utf8strs,
                    m_editDistance
            );
            if (!CanMatch(newStPtr)) break;

            trans2StateMap->insert(std::make_pair(string(),newStPtr));
            if (newStPtr != lastState && m_statesCacheMap.find(newStPtr) == m_statesCacheMap.end()) {
                statesStack.push(newStPtr);
            }
        } while (false);

        if (!trans2StateMap->empty()) {
            m_statesCacheMap.insert(std::make_pair(lastState,trans2StateMap));
        }
    }
}

AutomatonStatePtr DamerauLevenshteinAutomaton::Start() {
    DamerauLevenshteinAutomatonState::DISTANCE_SEQUENCE_PTR pEdits
    = std::make_shared<DamerauLevenshteinAutomatonState::DISTANCE_SEQUENCE>();
    for (size_t i = 0; i <= m_utf8strs->size(); ++i) {
        pEdits->push_back( std::min((size_t)m_editDistance+1,i));
    }
    return std::make_shared<DamerauLevenshteinAutomatonState>(pEdits,nullptr,string(),false,m_utf8strs,m_editDistance);
}


bool DamerauLevenshteinAutomaton::IsMatch(const AutomatonStatePtr &state) {
    if (nullptr == state) return false;
    DamerauLevenshteinAutomatonStatePtr st = dynamic_pointer_cast<DamerauLevenshteinAutomatonState>(state);
    return st->curEdits_->back() <= m_editDistance;
}

bool DamerauLevenshteinAutomaton::CanMatch(const AutomatonStatePtr &state) {
    if (nullptr == state) return false;
    DamerauLevenshteinAutomatonStatePtr st = dynamic_pointer_cast<DamerauLevenshteinAutomatonState>(state);
    for (size_t i = 0; i < st->curEdits_->size(); ++i) {
        if (st->curEdits_->operator[](i) <= m_editDistance) return true;
    }
    return false;
}

AutomatonStatePtr DamerauLevenshteinAutomaton::Accept(const AutomatonStatePtr &ptr, const vector<uint8_t>& byteVec) {
    if (nullptr == ptr) return nullptr;
    string s = Automaton::IsLastValidUtf8Str(byteVec);
    if (s.empty()) return ptr;

    DamerauLevenshteinAutomatonStatePtr st = dynamic_pointer_cast<DamerauLevenshteinAutomatonState>(ptr);
    StateCacheMapType::iterator it1 = m_statesCacheMap.find(st);
    if (it1 == m_statesCacheMap.end()) {
        return nullptr;
    }

    auto it2 = it1->second->find(s);
    if (it2 != it1->second->end()) {
        return it2->second;
    }
    else {
        it2 = it1->second->find("");
        return it2 == it1->second->end() ? nullptr : it2->second;
    }
}

COMMON_END_NAMESPACE