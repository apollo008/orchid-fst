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
            else if (s == m_utf8strs[st->m_matchedLength]) {
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
            else if (s == m_utf8strs[st->m_matchedLength]) {
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
    if (it1 == m_statesCacheMap.end()) return nullptr;

    if (m_bStrOccursMap.find(s) == m_bStrOccursMap.end()) {
        s.clear();
    }
    auto it2 = it1->second->find(s);
    if (it2 == it1->second->end()) return nullptr;
    return it2->second;
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

            if (transStrSet.find(m_utf8strs[ix]) != transStrSet.end()) continue;
            transStrSet.insert(m_utf8strs[ix]);

            if (lastState->m_curEdits[ix] > m_editDistance) continue;

            newState.clear();
            newState.push_back(lastState->m_curEdits[0] + 1);
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
        {
            //consider special char not in 'm_str', use special empty string for this case
            newState.clear();
            newState.push_back(lastState->m_curEdits[0] + 1);
            size_t cost = 1;
            for (size_t k = 1; k <= m_utf8strs.size(); ++k) {
                size_t distance = std::min(std::min(lastState->m_curEdits[k-1] + cost, lastState->m_curEdits[k] + 1),newState[k-1] + 1);
                newState.push_back(std::min(distance,(size_t)m_editDistance+1));
            }
            LevenshteinAutomatonStatePtr newStPtr = std::make_shared<LevenshteinAutomatonState>(newState);
            if (!CanMatch(newStPtr)) continue;

            trans2StateMap->insert(std::make_pair(string(),newStPtr));

            if (newStPtr != lastState && m_statesCacheMap.find(newStPtr) == m_statesCacheMap.end()) {
                statesStack.push(newStPtr);
            }
        }
        if (!trans2StateMap->empty()) {
            m_statesCacheMap.insert(std::make_pair(lastState,trans2StateMap));
        }
    }
}

COMMON_END_NAMESPACE