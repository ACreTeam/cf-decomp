#include <lib/RegEad/RegDfa.hpp>
#include <lib/RegEad/RegEadArray.inl>

namespace RegEad {

// Bit in a followAssertions mask for an assertion label type.
#define ASSERT_BIT(type) (1 << RegLabel::type)

// 801E4EF4
RegDfa::RegDfa() : mpNfa(NULL) {
}

// 801E4F3C
RegDfa::~RegDfa() {
    reset();
}

// 801E4FB0
void RegDfa::reset() {
    mStates.clear();
}

// 801E4FB8
bool RegDfa::build(RegNfa *nfa) {
    reset();
    gAllocFailed = false;
    mpNfa = nfa;

    RegDfaState state;
    mStates.push_back(state);
    if (gAllocFailed) {
        return false;
    }

    mStates[0].mNfaStates.push_back(0);
    if (gAllocFailed) {
        return false;
    }

    closure(mStates[0].mNfaStates);
    if (gAllocFailed) {
        return false;
    }

    buildState(0);
    if (gAllocFailed) {
        return false;
    }
    return true;
}

// 801E50E8
void RegDfa::closure(Array<int> &nfaStates) {
    for (u32 i = 0; i < nfaStates.size(); i++) {
        addEpsilonClosure(nfaStates, nfaStates[i]);
        if (gAllocFailed) {
            return;
        }
    }
}

// 801E5178
void RegDfa::addEpsilonClosure(Array<int> &nfaStates, int state) {
    const RegNfaState &nfaState = mpNfa->getStates()[state];
    for (u32 i = 0; i < nfaState.mEdges.size(); i++) {
        if (nfaState.mEdges[i].mLabel.mType == RegLabel::TYPE_EPSILON &&
            !nfaStates.contains(nfaState.mEdges[i].mTarget)) {
            nfaStates.insertSorted(int(nfaState.mEdges[i].mTarget));
            if (gAllocFailed) {
                return;
            }
            addEpsilonClosure(nfaStates, nfaState.mEdges[i].mTarget);
            if (gAllocFailed) {
                return;
            }
        }
    }
}

// 801E5268
void RegDfa::buildState(int idx) {
    collectTransitions(idx);
    if (gAllocFailed) {
        return;
    }
    splitCharTransitions(idx);
    if (gAllocFailed) {
        return;
    }
    splitSetTransitions(idx);
    if (gAllocFailed) {
        return;
    }
    resolveAssertions(idx);
    if (gAllocFailed) {
        return;
    }
    for (u32 i = 0; i < mStates[idx].mNfaStates.size(); i++) {
        if (mpNfa->getAcceptState() == mStates[idx].mNfaStates[i]) {
            mStates[idx].mAccepting = true;
            break;
        }
    }
}

// 801E5388
void RegDfa::collectTransitions(int idx) {
    RegDfaState &state = mStates[idx];
    for (u32 i = 0; i < state.mNfaStates.size(); i++) {
        int nfaIdx = mStates[idx].mNfaStates[i];
        const RegNfaState &nfaState = mpNfa->getStates()[nfaIdx];
        for (u32 j = 0; j < nfaState.mEdges.size(); j++) {
            if (nfaState.mEdges[j].mLabel.mType != RegLabel::TYPE_EPSILON) {
                state.mTrans.push_back(RegDfaTrans(nfaState.mEdges[j]));
                if (gAllocFailed) {
                    return;
                }
            }
        }
    }
}

// 801E5474
void RegDfa::splitCharTransitions(int idx) {
    for (u32 i = 0; i < mStates[idx].mTrans.size(); i++) {
        RegDfaTrans &trans = mStates[idx].mTrans[i];
        if (trans.mLabel.mType != RegLabel::TYPE_CHAR) {
            continue;
        }

        RegDfaState newState;
        const u16 ch = trans.mLabel.mChar;
        newState.mNfaStates.insertSorted(int(trans.mNfaTarget));
        if (gAllocFailed) {
            return;
        }

        u32 j = 0;
        while (j < mStates[idx].mTrans.size()) {
            RegDfaTrans &other = mStates[idx].mTrans[j];
            if (i == j || other.mLabel.isAssertion()) {
                j++;
                continue;
            }
            switch (other.mLabel.mType) {
            case RegLabel::TYPE_CHAR:
                if (ch == other.mLabel.mChar) {
                    newState.mNfaStates.insertSorted(int(other.mNfaTarget));
                    if (gAllocFailed) {
                        return;
                    }
                    mStates[idx].mTrans.erase(j);
                    if (j < i) {
                        i--;
                    }
                } else {
                    j++;
                }
                break;
            case RegLabel::TYPE_SET: {
                CharSet &set = other.mLabel.mSet;
                if (set.contains(ch)) {
                    newState.mNfaStates.insertSorted(int(other.mNfaTarget));
                    if (gAllocFailed) {
                        return;
                    }
                    set.remove(ch);
                    if (gAllocFailed) {
                        return;
                    }
                    if (set.mRanges.size() == 0) {
                        mStates[idx].mTrans.erase(j);
                        if (j < i) {
                            i--;
                        }
                        break;
                    }
                }
                j++;
                break;
            }
            }
        }

        closure(newState.mNfaStates);
        if (gAllocFailed) {
            return;
        }
        linkTransition(newState, idx, i);
        if (gAllocFailed) {
            return;
        }
    }
}

// 801E5718
void RegDfa::splitSetTransitions(int idx) {
    u32 num = mStates[idx].mTrans.size();
    for (u32 i = 0; i < num;) {
        RegDfaTrans &trans = mStates[idx].mTrans[i];
        if (trans.mLabel.mType == RegLabel::TYPE_SET) {
            RegDfaState newState;
            newState.mNfaStates.insertSorted(int(trans.mNfaTarget));
            if (gAllocFailed) {
                return;
            }

            CharSet &set = trans.mLabel.mSet;
            if (isDisjointSet(set, idx, i)) {
                closure(newState.mNfaStates);
                if (gAllocFailed) {
                    return;
                }
                linkTransition(newState, idx, i);
                if (gAllocFailed) {
                    return;
                }
                i++;
            } else {
                CharSet common(set);
                for (u32 j = 0; j < num; j++) {
                    RegDfaTrans &other = mStates[idx].mTrans[j];
                    if (i != j && other.mLabel.mType == RegLabel::TYPE_SET &&
                        other.mLabel.mSet.intersects(set)) {
                        newState.mNfaStates.insertSorted(int(other.mNfaTarget));
                        if (gAllocFailed) {
                            return;
                        }
                        common.intersect(other.mLabel.mSet);
                    }
                }

                for (u32 j = 0; j < num;) {
                    RegDfaTrans &other = mStates[idx].mTrans[j];
                    if (other.mLabel.mType == RegLabel::TYPE_SET) {
                        other.mLabel.mSet.subtract(common);
                        if (gAllocFailed) {
                            return;
                        }
                        if (other.mLabel.mSet.mRanges.size() == 0) {
                            mStates[idx].mTrans.erase(j);
                            if (j < i) {
                                i--;
                            }
                            num--;
                            continue;
                        }
                    }
                    j++;
                }

                mStates[idx].mTrans.push_back(RegDfaTrans(common));
                if (gAllocFailed) {
                    return;
                }
                int newTrans = mStates[idx].mTrans.size() - 1;
                closure(newState.mNfaStates);
                if (gAllocFailed) {
                    return;
                }
                linkTransition(newState, idx, newTrans);
                if (gAllocFailed) {
                    return;
                }
            }
        } else {
            i++;
        }
    }
}

// 801E5ABC
void RegDfa::resolveAssertions(int idx) {
    u32 i = 0;
    while (i < mStates[idx].mTrans.size()) {
        RegDfaTrans &trans = mStates[idx].mTrans[i];
        if (trans.mLabel.isAssertion()) {
            RegDfaState newState;
            newState.mNfaStates.insertSorted(int(trans.mNfaTarget));
            if (gAllocFailed) {
                return;
            }

            u32 j = 0;
            while (j < mStates[idx].mTrans.size()) {
                RegDfaTrans &other = mStates[idx].mTrans[j];
                RegLabel::Type otherType;
                RegLabel::Type transType;
                if (i == j || (otherType = (RegLabel::Type)other.mLabel.mType,
                               transType = (RegLabel::Type)trans.mLabel.mType,
                               transType != otherType)) {
                    j++;
                    continue;
                }
                newState.mNfaStates.insertSorted(int(other.mNfaTarget));
                if (gAllocFailed) {
                    return;
                }
                mStates[idx].mTrans.erase(j);
                if (j < i) {
                    i--;
                }
            }

            for (u32 k = 0; k < mStates[idx].mNfaStates.size(); k++) {
                int state = mStates[idx].mNfaStates[k];
                if (!newState.mNfaStates.contains(state)) {
                    newState.mNfaStates.insertSorted(state);
                    if (gAllocFailed) {
                        return;
                    }
                }
            }

            closure(newState.mNfaStates);
            if (gAllocFailed) {
                return;
            }
            if (newState.mNfaStates == mStates[idx].mNfaStates) {
                mStates[idx].mTrans.erase(i);
            } else {
                linkTransition(newState, idx, i);
                if (gAllocFailed) {
                    return;
                }
                i++;
            }
        } else {
            i++;
        }
    }
}

// 801E5D24
void RegDfa::linkTransition(RegDfaState &state, int idx, int trans) {
    int found = findState(state);
    if (found == -1) {
        mStates.push_back(state);
        if (gAllocFailed) {
            return;
        }
        int newIdx = mStates.size() - 1;
        mStates[idx].mTrans[trans].mTarget = newIdx;
        buildState(newIdx);
    } else {
        mStates[idx].mTrans[trans].mTarget = found;
    }
}

// 801E5DF0
bool RegDfa::isDisjointSet(const CharSet &set, int idx, int skipTrans) {
    RegDfaState *const &state = &mStates[idx];
    for (u32 i = 0; i < state->mTrans.size(); i++) {
        if (i != skipTrans &&
            state->mTrans[i].mLabel.mType == RegLabel::TYPE_SET &&
            set.intersects(state->mTrans[i].mLabel.mSet)) {
            return false;
        }
    }
    return true;
}

// 801E5E8C
int RegDfa::findState(const RegDfaState &state) {
    for (u32 i = 0; i < mStates.size(); i++) {
        if (mStates[i].mNfaStates == state.mNfaStates) {
            return i;
        }
    }
    return -1;
}

// 801E5F14
bool RegDfa::match(const u16 *str) {
    int remaining = -1;
    bool prevSpace = true;
    mpCurrent = &mStates[0];
    followAssertions(ASSERT_BIT(TYPE_LINE_START) |
                     ASSERT_BIT(TYPE_INPUT_START));
    while (*str != 0) {
        if (remaining <= 0) {
            remaining = getLineTerminatorLength(str);
            if (remaining != -1) {
                followAssertions(ASSERT_BIT(TYPE_LINE_END));
            }
        }
        bool space = isSpace(*str);
        followAssertions((space ^ prevSpace)
                             ? ASSERT_BIT(TYPE_WORD_BOUNDARY)
                             : ASSERT_BIT(TYPE_NOT_WORD_BOUNDARY));
        prevSpace = space;
        if (!step(*str)) {
            return false;
        }
        if (remaining != -1 && --remaining == 0) {
            followAssertions(ASSERT_BIT(TYPE_LINE_START));
        }
        str++;
    }
    followAssertions((prevSpace ? ASSERT_BIT(TYPE_NOT_WORD_BOUNDARY)
                                : ASSERT_BIT(TYPE_WORD_BOUNDARY)) |
                     ASSERT_BIT(TYPE_LINE_END) | ASSERT_BIT(TYPE_INPUT_END) |
                     ASSERT_BIT(TYPE_INPUT_END_TERM));
    return mpCurrent->mAccepting;
}

// 801E6088
void RegDfa::followAssertions(u32 mask) {
restart:
    const RegDfaState *state = getCurrent();
    for (u32 i = 0; i < state->mTrans.size(); i++) {
        if (mask & (1 << state->mTrans[i].mLabel.mType)) {
            mpCurrent = &mStates[state->mTrans[i].mTarget];
            goto restart;
        }
    }
}

// 801E60EC
bool RegDfa::step(u16 ch) {
    const RegDfaState *state = getCurrent();
    u32 i = 0;
    while (i < state->mTrans.size()) {
        if (state->mTrans[i].mLabel.matches(ch)) {
            mpCurrent = &mStates[state->mTrans[i].mTarget];
            return true;
        }
        i++;
    }
    return false;
}

// 801E6188
int RegDfa::getLineTerminatorLength(const u16 *p) {
    switch (p[0]) {
    case 0x0D:
        return p[1] == 0x0A ? 2 : 1;
    case 0x0A:
    case 0x85:
    case 0x2028:
    case 0x2029:
        return 1;
    }
    return -1;
}

// 801E61EC
bool RegDfa::isSpace(u16 ch) {
    switch (ch) {
    case 0x09:
    case 0x0A:
    case 0x0B:
    case 0x0C:
    case 0x0D:
    case 0x20:
    case 0x85:
    case 0x2028:
    case 0x2029:
        return true;
    }
    return false;
}

// 801E623C
u32 RegDfa::getSerializedSize() {
    int idx = getIndexKind();
    int cnt = getCountKind();
    u32 size = 1 + (idx == 0 ? 1 : idx == 1 ? 2 : 4);
    for (u32 i = 0; i < mStates.size(); i++) {
        size += 1;
        size += idx == 0 ? 1 : idx == 1 ? 2 : 4;

        for (u32 j = 0; j < mStates[i].mTrans.size(); j++) {
            RegDfaTrans &trans = mStates[i].mTrans[j];
            size += idx == 0 ? 1 : idx == 1 ? 2 : 4;
            size += 1;
            switch (trans.mLabel.mType) {
            case RegLabel::TYPE_CHAR:
                size += 2;
                break;
            case RegLabel::TYPE_SET: {
                const CharSet &set = trans.mLabel.mSet;
                size += cnt == 0 ? 1 : idx == 1 ? 2 : 4;
                u32 num = set.mRanges.size();
                u32 holes = 0;
                if (num != 0) {
                    for (u32 k = 0; k < num - 1; k++) {
                        if ((u16)(set.mRanges[k].mHi + 2) ==
                            set.mRanges[k + 1].mLo) {
                            holes++;
                        }
                    }
                    size += (set.getRangeCount() - holes) * 4;
                    size += cnt == 0 ? 1 : idx == 1 ? 2 : 4;
                    size += holes * 2;
                }
                break;
            }
            }
        }
    }
    return size;
}

// 801E6440
void RegDfa::serialize(void *dst) {
    u8 *p = (u8 *)dst;
    int idx = getIndexKind();
    int cnt = getCountKind();
    put<u8>(p, (u8)idx | (u8)(cnt << 2));
    switch (idx) {
    case 0:
        if (cnt == 0) {
            serializeImpl<u8, u8>(p);
        } else {
            serializeImpl<u8, u16>(p);
        }
        break;
    case 1:
        if (cnt == 0) {
            serializeImpl<u16, u8>(p);
        } else {
            serializeImpl<u16, u16>(p);
        }
        break;
    case 2:
        if (cnt == 0) {
            serializeImpl<u32, u8>(p);
        } else {
            serializeImpl<u32, u16>(p);
        }
        break;
    }
}

// 801E6554
int RegDfa::getIndexKind() const {
    u32 num = mStates.size();
    int r;
    if (num > 0xFF) {
        r = 1;
        if (num > 0xFFFF) {
            r = 2;
        }
    } else {
        r = 0;
    }
    return r;
}

// 801E657C
int RegDfa::getCountKind() const {
    for (u32 i = 0; i < mStates.size(); i++) {

        for (u32 j = 0; j < mStates[i].mTrans.size(); j++) {
            const RegDfaTrans &trans = mStates[i].mTrans[j];
            if (trans.mLabel.mType == RegLabel::TYPE_SET &&
                trans.mLabel.mSet.mRanges.size() > 0xFF) {
                return 1;
            }
        }
    }
    return 0;
}

// 801E65F0
bool RegDfa::load(const void *data) {
    const u8 *p = (const u8 *)data;
    reset();
    u8 header = get<u8>(p);
    int idx = header & 3;
    int cnt = (header >> 2) & 3;
    switch (idx) {
    case 0:
        if (cnt == 0) {
            if (!loadImpl<u8, u8>(p)) {
                return false;
            }
        } else {
            if (!loadImpl<u8, u16>(p)) {
                return false;
            }
        }
        break;
    case 1:
        if (cnt == 0) {
            if (!loadImpl<u16, u8>(p)) {
                return false;
            }
        } else {
            if (!loadImpl<u16, u16>(p)) {
                return false;
            }
        }
        break;
    case 2:
        if (cnt == 0) {
            if (!loadImpl<u32, u8>(p)) {
                return false;
            }
        } else {
            if (!loadImpl<u32, u16>(p)) {
                return false;
            }
        }
        break;
    }
    return true;
}

// RegNfa's getters are only used here; defined after their callers, they are
// emitted at this point instead of next to them.
// 801E6724
const RegNfaStateArray &RegNfa::getStates() const {
    return mStates;
}

// 801E672C
int RegNfa::getAcceptState() const {
    return mAccept;
}

// Serializer / loader templates, explicitly instantiated below (in the
// target's order: the reverse of their use in serialize / load).

template <typename IDX, typename CNT>
void RegDfa::serializeImpl(u8 *dst) const {
    put<IDX>(dst, mStates.size());
    for (u32 i = 0; i < mStates.size(); i++) {
        put<u8>(dst, mStates[i].mAccepting);
        put<IDX>(dst, mStates[i].mTrans.size());
        for (u32 j = 0; j < mStates[i].mTrans.size(); j++) {
            const RegDfaTrans &trans = mStates[i].mTrans[j];
            put<IDX>(dst, trans.mTarget);
            put<u8>(dst, trans.mLabel.mType);
            switch (trans.mLabel.mType) {
            case RegLabel::TYPE_CHAR:
                put<u16>(dst, trans.mLabel.mChar);
                break;
            case RegLabel::TYPE_SET: {
                const Array<CharRange> &ranges = trans.mLabel.mSet.mRanges;
                const u32 &num = ranges.mSize;
                if (num == 0) {
                    put<CNT>(dst, 0);
                    break;
                }
                u32 holes = 0;
                for (u32 k = 0; k < num - 1; k++) {
                    if ((u16)(ranges[k].mHi + 2) == ranges[k + 1].mLo) {
                        holes++;
                    }
                }
                put<CNT>(dst, num - holes);
                for (u32 k = 0; k < ranges.size(); k++) {
                    put<u16>(dst, ranges[k].mLo);
                    for (; k < ranges.size() - 1; k++) {
                        if ((u16)(ranges[k].mHi + 2) != ranges[k + 1].mLo) {
                            break;
                        }
                    }
                    put<u16>(dst, ranges[k].mHi);
                }
                put<CNT>(dst, holes);
                for (u32 k = 0; k < ranges.size() - 1; k++) {
                    if ((u16)(ranges[k].mHi + 2) == ranges[k + 1].mLo) {
                        put<u16>(dst, ranges[k].mHi + 1);
                    }
                }
                break;
            }
            }
        }
    }
}

template <typename T> void put(u8 *&p, T v) {
    *(T *)p = v;
    p += sizeof(T);
}

template <typename IDX, typename CNT> bool RegDfa::loadImpl(const u8 *src) {
    u32 numStates = get<IDX>(src);
    mStates.resize(numStates);
    if (gAllocFailed) {
        return false;
    }
    for (u32 i = 0; i < numStates; i++) {
        bool accepting = get<u8>(src) == 1;
        mStates[i].mAccepting = accepting;
        u32 numTrans = get<IDX>(src);
        mStates[i].mTrans.resize(numTrans);
        if (gAllocFailed) {
            return false;
        }
        for (u32 j = 0; j < numTrans; j++) {
            RegDfaTrans &trans = mStates[i].mTrans[j];
            trans.mTarget = get<IDX>(src);
            trans.mLabel.mType = get<u8>(src);
            switch (trans.mLabel.mType) {
            case RegLabel::TYPE_CHAR:
                trans.mLabel.mChar = get<u16>(src);
                break;
            case RegLabel::TYPE_SET: {
                Array<CharRange> &ranges = trans.mLabel.mSet.getRanges();
                u32 numRanges = get<CNT>(src);
                if (numRanges != 0) {
                    ranges.resize(numRanges);
                    if (gAllocFailed) {
                        return false;
                    }
                    for (u32 k = 0; k < numRanges; k++) {
                        u16 lo = get<u16>(src);
                        u16 hi = get<u16>(src);
                        ranges[k].mLo = lo;
                        ranges[k].mHi = hi;
                    }
                    u32 numHoles = get<CNT>(src);
                    for (u32 k = 0; k < numHoles; k++) {
                        trans.mLabel.mSet.remove(get<u16>(src));
                        if (gAllocFailed) {
                            return false;
                        }
                    }
                }
                break;
            }
            }
        }
    }
    return true;
}

template <typename T> T get(const u8 *&p) {
    T v = *(const T *)p;
    p += sizeof(T);
    return v;
}

template void RegDfa::serializeImpl<u32, u16>(u8 *dst) const;
template void RegDfa::serializeImpl<u32, u8>(u8 *dst) const;
template void RegDfa::serializeImpl<u16, u16>(u8 *dst) const;
template void RegDfa::serializeImpl<u16, u8>(u8 *dst) const;
template void RegDfa::serializeImpl<u8, u16>(u8 *dst) const;
template void RegDfa::serializeImpl<u8, u8>(u8 *dst) const;
template bool RegDfa::loadImpl<u32, u16>(const u8 *src);
template bool RegDfa::loadImpl<u32, u8>(const u8 *src);
template bool RegDfa::loadImpl<u16, u16>(const u8 *src);
template bool RegDfa::loadImpl<u16, u8>(const u8 *src);
template bool RegDfa::loadImpl<u8, u16>(const u8 *src);
template bool RegDfa::loadImpl<u8, u8>(const u8 *src);

} // namespace RegEad
