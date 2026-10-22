#pragma once

// RegEad DFA (src: lib/RegEad/RegDfa.cpp, .text 801E4EF4..801E90B8): subset
// construction from a RegNfa, (de)serialization and whole-string matching.
//
// Serialized form: u8 header = idxKind | (cntKind << 2) (getIndexKind /
// getCountKind: 0 = u8, 1 = u16, 2 = u32); Idx stateCount; per state: u8
// accepting, Idx transCount, per transition: Idx target, u8 type, then u16 char
// (TYPE_CHAR) or the set (TYPE_SET: Cnt rangeCount, (u16 lo, u16 hi)..., Cnt
// holeCount, u16 hole...; ranges one char apart are stored merged plus the char
// between them as a hole).

#include <lib/RegEad/RegNfa.hpp>

namespace RegEad {

class RegDfaTrans {
  public:
    RegDfaTrans();                         // 801E8C14
    RegDfaTrans(const RegNfaEdge &edge);   // 801E8C54
    RegDfaTrans(const CharSet &set);       // 801E8C9C

    /* 0x0 */ int mTarget;    // DFA state index, -1 until linked
    /* 0x4 */ int mNfaTarget; // NFA state index
    /* 0x8 */ RegLabel mLabel;
}; // Size: 0x20
typedef Array<RegDfaTrans, 4> RegDfaTransArray;

class RegDfaState {
  public:
    RegDfaState();  // 801E8CDC
    ~RegDfaState(); // 801E8D1C
    // Takes other's arrays (transferTo).
    RegDfaState &operator=(RegDfaState &other); // 801E8D84

    /* 0x00 */ Array<int> mNfaStates; // sorted NFA state indices
    /* 0x10 */ RegDfaTransArray mTrans;
    /* 0x20 */ bool mAccepting;
}; // Size: 0x24

class RegDfa {
  public:
    RegDfaState *getCurrent() const {
        return mpCurrent;
    }

    RegDfa();          // 801E4EF4
    virtual ~RegDfa(); // 801E4F3C

    virtual bool build(RegNfa *nfa);     // 801E4FB8
    virtual bool load(const void *data); // 801E65F0
    virtual u32 getSerializedSize();     // 801E623C
    virtual void serialize(void *dst);   // 801E6440
    virtual void reset();                // 801E4FB0
    virtual bool match(const u16 *str);  // 801E5F14: whole-string match

    virtual void closure(Array<int> &nfaStates); // 801E50E8
    virtual void addEpsilonClosure(Array<int> &nfaStates,
                                   int state);  // 801E5178
    virtual void buildState(int idx);           // 801E5268
    virtual void collectTransitions(int idx);   // 801E5388
    virtual void splitCharTransitions(int idx); // 801E5474
    virtual void splitSetTransitions(int idx);  // 801E5718
    virtual void resolveAssertions(int idx);    // 801E5ABC
    virtual bool isDisjointSet(const CharSet &set, int idx,
                               int skipTrans);       // 801E5DF0
    virtual int findState(const RegDfaState &state); // 801E5E8C
    virtual void linkTransition(RegDfaState &state, int idx,
                                int trans); // 801E5D24
    // Follows assertion transitions whose type bit is set in mask from
    // mpCurrent.
    virtual void followAssertions(u32 mask);           // 801E6088
    virtual bool step(u16 ch);                         // 801E60EC
    virtual int getLineTerminatorLength(const u16 *p); // 801E6188: -1 if none
    virtual bool isSpace(u16 ch);                      // 801E61EC
    virtual int getIndexKind() const;                  // 801E6554
    virtual int getCountKind() const;                  // 801E657C

    // Serializer / loader for each (index, count) width; dispatched by
    // serialize / load. Instantiations: serializeImpl<u32,u16> 801E6734,
    // <u32,u8> 801E69F4, <u16,u16> 801E6C6C, <u16,u8> 801E6EF0, <u8,u16>
    // 801E7174, <u8,u8> 801E73F8; loadImpl<u32,u16> 801E767C, <u32,u8>
    // 801E78D0, <u16,u16> 801E7ADC, <u16,u8> 801E7CEC, <u8,u16> 801E7EFC,
    // <u8,u8> 801E810C.
    template <typename IDX, typename CNT> void serializeImpl(u8 *dst) const;
    template <typename IDX, typename CNT> bool loadImpl(const u8 *src);

    /* 0x00 vtable */
    /* 0x04 */ Array<RegDfaState> mStates;
    /* 0x14 */ RegNfa *mpNfa;
    /* 0x18 */ RegDfaState *mpCurrent; // set by match
}; // Size: 0x1C

// Byte cursor helpers for the serialized form.
// put<u16> 801E69AC, put<u8> 801E69C4, put<u32> 801E69DC; get<u16> 801E7888,
// get<u8> 801E78A0, get<u32> 801E78B8.
template <typename T> void put(u8 *&p, T v);
template <typename T> T get(const u8 *&p);

} // namespace RegEad
