/*********************************************************************************
  *Copyright(C),dingbinthu@163.com
  *All rights reserved.
  *
  *FileName:       automaton.h
  *Author:         dingbinthu@163.com
  *Version:        1.0
  *Date:           3/1/21
  *Description:    file defines automaton classes to used in fst
  *                At least four virtual methods must be implemented for every Automaton
  *                are as belows:
  *                    1)  AutomatonStatePtr Start();  --- which return the first state of
  *                        this automaton
  *                    2)  bool  IsMatch(const AutomatonStatePtr& state);  --- indicate whether
  *                        is matched for current this state of automaton with the state of fst.
  *                    3)  bool  CanMatch(const AutomatonStatePtr& state);   --- indicate whether
  *                        can try to match fst if length of this string changes longer.
  *                    4)  AutomatonStatePtr Accept(const AutomatonStatePtr& state, const vector<uint8_t>& byteVec)
  *                        --- defines what is next state based on current state and specified
  *                        bytes sequences.
**********************************************************************************/
#ifndef __CPPFST_FST_CORER_COMMON_AUTOMATON__H__
#define __CPPFST_FST_CORER_COMMON_AUTOMATON__H__
#include "common/common.h"
#include "tulip/TLogDefine.h"
#include <iostream>
#include <vector>
#include <string>
#include "common/util/hash_util.h"
#include "common/util/string_util.h"
#include "common/util/utf8_util.h"
#include <unordered_map>
#include <algorithm>
#include <cassert>

STD_USE_NAMESPACE;
COMMON_BEGIN_NAMESPACE

//base automaton state class
class AutomatonState {
public:
    AutomatonState() {}
    virtual ~AutomatonState() {}
};
TYPEDEF_PTR(AutomatonState);

//always true automaton state
class TrueAutomatonState;
TYPEDEF_PTR(TrueAutomatonState);
class TrueAutomatonState : public AutomatonState {
public:
    TrueAutomatonState() {}
    virtual ~TrueAutomatonState() {}
public:
    static TrueAutomatonStatePtr s_trueAutoState;
};

//complex automaton state base class
class ComplexAutomatonState;
TYPEDEF_PTR(ComplexAutomatonState);
class ComplexAutomatonState : public AutomatonState {
public:
    ComplexAutomatonState() {}
    ~ComplexAutomatonState() {}
public:
    std::vector<AutomatonStatePtr>  m_states;
};

//Dfa kind automaton state
class DfaAutomatonState : public AutomatonState {
public:
    DfaAutomatonState() {}
    virtual ~DfaAutomatonState() {}
public:
    virtual bool operator=(const DfaAutomatonState& rhs) const  = 0;
    virtual uint64_t hash(const DfaAutomatonState& rhs) const = 0;
};
TYPEDEF_PTR(DfaAutomatonState);

//Automaton base class
class Automaton;
TYPEDEF_PTR(Automaton);
class Automaton {
public:
public:
    Automaton() {}
    virtual ~Automaton() {}
public:
    virtual AutomatonStatePtr Start() = 0;
    virtual bool  IsMatch(const AutomatonStatePtr& state) = 0;
    virtual bool  CanMatch(const AutomatonStatePtr& state) = 0;
    virtual AutomatonStatePtr Accept(const AutomatonStatePtr& state, const vector<uint8_t>& byteVec) = 0;

public:
    static string IsLastValidUtf8Str(const vector<uint8_t>& byteVec);

protected:
};

//complex automaton base class
class ComplexAutomaton;
TYPEDEF_PTR(ComplexAutomaton);
class ComplexAutomaton : public Automaton {
public:
    ComplexAutomaton() {}
    ComplexAutomaton(const AutomatonPtr& aut1,const AutomatonPtr& aut2) {
        m_automatons.push_back(aut1);
        m_automatons.push_back(aut2);
    }
    ComplexAutomaton(const std::vector<AutomatonPtr>& auts) {
        for (AutomatonPtr aut :auts) m_automatons.push_back(aut);
    }
public:
    virtual AutomatonStatePtr Start() {
        ComplexAutomatonStatePtr state = std::make_shared<ComplexAutomatonState>();
        for (AutomatonPtr aut: m_automatons) {
            state->m_states.push_back(aut->Start());
        }
        return state;
    }
    virtual AutomatonStatePtr Accept(const AutomatonStatePtr& state, const vector<uint8_t>& byteVec) {
        ComplexAutomatonStatePtr st = dynamic_pointer_cast<ComplexAutomatonState>(state);
        assert(st);
        ComplexAutomatonStatePtr newState = std::make_shared<ComplexAutomatonState>();
        for (size_t i = 0; i < m_automatons.size(); ++i) {
            newState->m_states.push_back(m_automatons[i]->Accept(st->m_states[i],byteVec));
        }
        return newState;
    }
    virtual bool  IsMatch(const AutomatonStatePtr& state) = 0;
    virtual bool  CanMatch(const AutomatonStatePtr& state) = 0;
public:
    std::vector<AutomatonPtr>     m_automatons;
};

//intersection for multiple automatons
class IntersectAutomaton : public ComplexAutomaton {
public:
    IntersectAutomaton(AutomatonPtr& aut1,AutomatonPtr& aut2)
    : ComplexAutomaton(aut1,aut2)
    {}
    IntersectAutomaton(const std::vector<AutomatonPtr>& autos)
    : ComplexAutomaton(autos)
    {}
public:
    virtual bool  IsMatch(const AutomatonStatePtr& state) {
        ComplexAutomatonStatePtr st = dynamic_pointer_cast<ComplexAutomatonState>(state);
        assert(st);
        for (size_t i = 0; i < m_automatons.size(); ++i) {
            if (!m_automatons[i]->IsMatch(st->m_states[i])) return false;
        }
        return true;
    }
    virtual bool  CanMatch(const AutomatonStatePtr& state)  {
        ComplexAutomatonStatePtr st = dynamic_pointer_cast<ComplexAutomatonState>(state);
        assert(st);
        for (size_t i = 0; i < m_automatons.size(); ++i) {
            if (!m_automatons[i]->CanMatch(st->m_states[i])) return false;
        }
        return true;
    }
};

//union for multiple automatons
class UnionAutomaton : public ComplexAutomaton {
public:
    UnionAutomaton(AutomatonPtr& aut1,AutomatonPtr& aut2)
    : ComplexAutomaton(aut1,aut2)
    {}
    UnionAutomaton(const std::vector<AutomatonPtr>& autos)
    : ComplexAutomaton(autos)
    {}
public:
    virtual bool  IsMatch(const AutomatonStatePtr& state) {
        ComplexAutomatonStatePtr st = dynamic_pointer_cast<ComplexAutomatonState>(state);
        assert(st);
        for (size_t i = 0; i < m_automatons.size(); ++i) {
            if (m_automatons[i]->IsMatch(st->m_states[i])) return true;
        }
        return false;
    }
    virtual bool  CanMatch(const AutomatonStatePtr& state)  {
        ComplexAutomatonStatePtr st = dynamic_pointer_cast<ComplexAutomatonState>(state);
        assert(st);
        for (size_t i = 0; i < m_automatons.size(); ++i) {
            if (m_automatons[i]->CanMatch(st->m_states[i])) return true;
        }
        return false;
    }
};

//wrapper for one automatons
class WrapperAutomaton;
TYPEDEF_PTR(WrapperAutomaton);
class WrapperAutomaton : public Automaton {
public:
    WrapperAutomaton(const AutomatonPtr& aut) {
        m_automaton = aut;
    }
public:
    virtual AutomatonStatePtr Start() = 0;
    virtual bool  IsMatch(const AutomatonStatePtr& state) = 0;
    virtual bool  CanMatch(const AutomatonStatePtr& state) = 0;
    virtual AutomatonStatePtr Accept(const AutomatonStatePtr&, const vector<uint8_t>& byteVec) = 0;

protected:
    AutomatonPtr    m_automaton;
};

//Not wrapper for some one automaton
class NotAutomaton : public WrapperAutomaton {
public:
    NotAutomaton(AutomatonPtr& aut)
    : WrapperAutomaton(aut)
    {
    }
public:
    virtual bool  IsMatch(const AutomatonStatePtr& state) {
        return !m_automaton->IsMatch(state);
    }
    virtual bool  CanMatch(const AutomatonStatePtr& state)  {
        return !m_automaton->CanMatch(state);
    }

    virtual AutomatonStatePtr Start() {
        return m_automaton->Start();
    }
    virtual AutomatonStatePtr Accept(const AutomatonStatePtr& state, const vector<uint8_t>& byteVec) {
        return m_automaton->Accept(state,byteVec);
    }
};

//Starts with wrapper for some one automaton
class StartsWithAutomaton : public WrapperAutomaton {
public:
    class RunningState : public AutomatonState {
    public:
        RunningState(AutomatonStatePtr st)
        : m_innerState(st)
        {}
    public:
        AutomatonStatePtr  m_innerState;
    };
    TYPEDEF_PTR(RunningState);

    class DoneState;
    TYPEDEF_PTR(DoneState);
    class DoneState : public AutomatonState {
    public:
        DoneState(){}
    public:
        static DoneStatePtr s_doneState;
    };
public:
    StartsWithAutomaton(AutomatonPtr& aut)
    : WrapperAutomaton(aut)
    {
    }
public:
    virtual AutomatonStatePtr Start() {
        AutomatonStatePtr st = m_automaton->Start();
        if (m_automaton->IsMatch(st)) {
            return  StartsWithAutomaton::DoneState::s_doneState;
        }
        else {
            return std::make_shared<StartsWithAutomaton::RunningState>(st);
        }
    }
    virtual AutomatonStatePtr Accept(const AutomatonStatePtr& state, const vector<uint8_t>& byteVec) {
        if (state == StartsWithAutomaton::DoneState::s_doneState) return state;

        StartsWithAutomaton::RunningStatePtr runningSt = dynamic_pointer_cast<StartsWithAutomaton::RunningState>(state);
        if (runningSt) {
            AutomatonStatePtr nextSt = m_automaton->Accept(runningSt->m_innerState,byteVec);
            if (m_automaton->IsMatch(nextSt)) {
                return  StartsWithAutomaton::DoneState::s_doneState;
            }
            else {
                return std::make_shared<StartsWithAutomaton::RunningState>(nextSt);
            }
        }
        else {
            return nullptr;
        }
    }
    virtual bool  IsMatch(const AutomatonStatePtr& state) {
        if (state == StartsWithAutomaton::DoneState::s_doneState) return true;
        StartsWithAutomaton::RunningStatePtr runningSt = dynamic_pointer_cast<StartsWithAutomaton::RunningState>(state);
        return m_automaton->IsMatch(runningSt);
    }
    virtual bool  CanMatch(const AutomatonStatePtr& state)  {
        if (state == StartsWithAutomaton::DoneState::s_doneState) return true;
        StartsWithAutomaton::RunningStatePtr runningSt = dynamic_pointer_cast<StartsWithAutomaton::RunningState>(state);
        return m_automaton->CanMatch(runningSt);
    }
};



AutomatonPtr Intersect(AutomatonPtr& aut1, AutomatonPtr& aut2);
AutomatonPtr Union(AutomatonPtr& aut1, AutomatonPtr& aut2);
AutomatonPtr Not(AutomatonPtr& aut);
AutomatonPtr StartsWith(AutomatonPtr& aut);


class AlwaysAutomaton : public Automaton {
public:
    AlwaysAutomaton() {}
    virtual ~AlwaysAutomaton() {}
public:
    virtual AutomatonStatePtr Start() {
        return nullptr;
    }
    virtual bool  IsMatch(const AutomatonStatePtr& state) {
        return true;
    }
    virtual bool  CanMatch(const AutomatonStatePtr& state) {
        return true;
    }
    virtual AutomatonStatePtr Accept(const AutomatonStatePtr&, const vector<uint8_t>& byteVec) {
        return nullptr;
    }
};
TYPEDEF_PTR(AlwaysAutomaton);


class StrAutomaton : public Automaton {
public:
    class StrAutomatonState : public AutomatonState {
    public:
        StrAutomatonState(size_t len)
        :m_matchedLength(len)
        {}
        ~StrAutomatonState() {}
    public:
        std::size_t    m_matchedLength;
    };
public:
    StrAutomaton(const string& str);

public:
    AutomatonStatePtr Start() override;
    bool IsMatch(const AutomatonStatePtr &state) override;
    bool CanMatch(const AutomatonStatePtr &state) override;
    AutomatonStatePtr Accept(const AutomatonStatePtr &ptr, const vector<uint8_t>& byteVec) override;

protected:
    string          m_str;
    vector<string>  m_utf8strs;
};

//automaton which greater than some state
class GreaterThanAutomaton : public Automaton {
public:
    class GreaterThanAutomatonState : public AutomatonState {
    public:
        GreaterThanAutomatonState(size_t len,bool isBeforeEqualMatch)
        :m_matchedLength(len)
        ,m_isEqualMatchBefore(isBeforeEqualMatch)
        {}
    public:
        std::size_t    m_matchedLength;
        bool           m_isEqualMatchBefore;
    };
    TYPEDEF_PTR(GreaterThanAutomatonState);
public:
    GreaterThanAutomaton(const string& str,bool inclusive)
    :  m_str(str)
    ,  m_inclusive(inclusive)
    {
        Utf8Util::String2utf8(m_str,m_utf8strs);
    }

public:
    AutomatonStatePtr Start() override;
    bool IsMatch(const AutomatonStatePtr &state) override;
    bool CanMatch(const AutomatonStatePtr &state) override;
    AutomatonStatePtr Accept(const AutomatonStatePtr &ptr, const vector<uint8_t>& byteVec) override;

protected:
    string           m_str;
    vector<string>   m_utf8strs;
    bool             m_inclusive;
};

//automaton which less than some state
class LessThanAutomaton : public Automaton {
public:
    class LessThanAutomatonState : public AutomatonState {
    public:
        LessThanAutomatonState(size_t len,bool isBeforeEqualMatch)
        :m_matchedLength(len)
        ,m_isEqualMatchBefore(isBeforeEqualMatch)
        {}
    public:
        std::size_t    m_matchedLength;
        bool           m_isEqualMatchBefore;
    };
    TYPEDEF_PTR(LessThanAutomatonState);
public:
    LessThanAutomaton(const string& str, bool inclusive)
    : m_str(str)
    , m_inclusive(inclusive)
    {
        Utf8Util::String2utf8(m_str,m_utf8strs);
    }

public:
    AutomatonStatePtr Start() override;
    bool IsMatch(const AutomatonStatePtr &state) override;
    bool CanMatch(const AutomatonStatePtr &state) override;
    AutomatonStatePtr Accept(const AutomatonStatePtr &ptr, const vector<uint8_t>& byteVec) override;

protected:
    string            m_str;
    vector<string>    m_utf8strs;
    bool              m_inclusive;
};

//prefix automaton which less than some state
class PrefixAutomaton : public Automaton {
public:
    class PrefixAutomatonState : public AutomatonState {
    public:
        PrefixAutomatonState(size_t len)
        :m_matchedLength(len)
        {}
    public:
        std::size_t    m_matchedLength;
    };
    TYPEDEF_PTR(PrefixAutomatonState);

public:
    PrefixAutomaton(const string& str)
    :  m_str(str)
    {
        Utf8Util::String2utf8(m_str,m_utf8strs);
    }

public:
    AutomatonStatePtr Start() override;
    bool IsMatch(const AutomatonStatePtr &state) override;
    bool CanMatch(const AutomatonStatePtr &state) override;
    AutomatonStatePtr Accept(const AutomatonStatePtr &ptr, const vector<uint8_t>& byteVec) override;

protected:
    string           m_str;
    vector<string>   m_utf8strs;
};

//levenshtein automaton state
class LevenshteinAutomatonState : public AutomatonState {
public:
    LevenshteinAutomatonState(const vector<size_t>& curEdits)
    : m_curEdits(curEdits)
    {}
public:
    vector<size_t>      m_curEdits;
};
TYPEDEF_PTR(LevenshteinAutomatonState);

class LevenshteinAutomatonStateHash {
public:
    size_t operator()(const LevenshteinAutomatonStatePtr& key) const {
        uint64_t seed = 0;
        for (size_t st : key->m_curEdits) {
            HashCombine(seed,st);
        }
        return seed;
    }
};

class LevenshteinAutomatonStateEqual {
public:
    bool operator()(const LevenshteinAutomatonStatePtr & key1, const LevenshteinAutomatonStatePtr & key2) const {
        if (key1->m_curEdits.size() != key2->m_curEdits.size()) return false;
        for (size_t i = 0; i < key1->m_curEdits.size(); ++i) {
            if (key1->m_curEdits[i] != key2->m_curEdits[i]) return false;
        }
        return true;
    }
};

//levenshtein automaton  which indicates Levenshtein edit distance
class LevenshteinAutomaton : public Automaton {
public:
    //NOTE THAT use empty string to indicates the trans which not in 'm_str'
    typedef unordered_map<LevenshteinAutomatonStatePtr,
    std::shared_ptr<unordered_map<string, LevenshteinAutomatonStatePtr> >,
            LevenshteinAutomatonStateHash,LevenshteinAutomatonStateEqual>  StateCacheMapType;
public:
    LevenshteinAutomaton(const string& str, uint32_t editDistance)
    :  m_str(str)
    ,  m_editDistance (editDistance)
    {
        Utf8Util::String2utf8(m_str,m_utf8strs);
        for (string s: m_utf8strs) {
            m_bStrOccursMap[s] = true;
        }
        buildDfa();
    }

private:
    void buildDfa();
public:
    AutomatonStatePtr Start() override;
    bool IsMatch(const AutomatonStatePtr &state) override;
    bool CanMatch(const AutomatonStatePtr &state) override;
    AutomatonStatePtr Accept(const AutomatonStatePtr &ptr, const vector<uint8_t>& byteVec) override;

protected:
    string                             m_str;
    uint32_t                           m_editDistance;
    vector<string>                     m_utf8strs;
    unordered_map<string, bool>        m_bStrOccursMap;
    StateCacheMapType                  m_statesCacheMap;
};
TYPEDEF_PTR(LevenshteinAutomaton);



COMMON_END_NAMESPACE
#endif //__CPPFST_FST_CORER_COMMON_AUTOMATON__H__
