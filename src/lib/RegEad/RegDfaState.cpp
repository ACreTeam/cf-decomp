#include <lib/RegEad/RegDfa.hpp>
#include <lib/RegEad/RegEadArray.inl>

namespace RegEad {

// 801E8C14
RegDfaTrans::RegDfaTrans() : mTarget(-1), mNfaTarget(-1) {
}

// 801E8C54
RegDfaTrans::RegDfaTrans(const RegNfaEdge &edge)
    : mTarget(-1), mNfaTarget(edge.mTarget), mLabel(edge.mLabel) {
}

// 801E8C9C
RegDfaTrans::RegDfaTrans(const CharSet &set)
    : mTarget(-1), mNfaTarget(-1), mLabel(set) {
}

// 801E8CDC
RegDfaState::RegDfaState() : mAccepting(false) {
}

// 801E8D1C
RegDfaState::~RegDfaState() {
}

// 801E8D84
RegDfaState &RegDfaState::operator=(RegDfaState &other) {
    other.mNfaStates.transferTo(mNfaStates);
    other.mTrans.transferTo(mTrans);
    mAccepting = other.mAccepting;
    return *this;
}

// These queries are instantiated here for Array<int>. Keeping their template
// definitions in this unit prevents earlier users from emitting weak copies.
template <typename T, int BLOCK> bool Array<T, BLOCK>::contains(T v) const {
    for (u32 i = 0; i < mSize; i++) {
        if (v == mData[i]) {
            return true;
        }
    }
    return false;
}

template <typename T, int BLOCK>
bool Array<T, BLOCK>::operator==(const Array &other) const {
    if (mSize != other.mSize) {
        return false;
    }
    for (u32 i = 0; i < mSize; i++) {
        if (mData[i] != other.mData[i]) {
            return false;
        }
    }
    return true;
}

template bool Array<int, 16>::contains(int) const;
template bool Array<int, 16>::operator==(const Array<int, 16> &) const;

} // namespace RegEad
