#include <lib/RegEad/RegNfa.hpp>
#include <lib/RegEad/RegEadArray.inl>

namespace RegEad {

inline RegNfaStateCopier::~RegNfaStateCopier() {
}

// 801EC998
RegNfaStateCopier::RegNfaStateCopier(RegNfaStateArray *states) : mStates(states) {
}

// 801EC9E8
int RegNfaStateCopier::copy(int start, int end) {
    {
        RegNfaState state;
        mStates->push_back(state);
    }
    if (gAllocFailed) {
        return -1;
    }
    int copied = mStates->size() - 1;
    mMap.push_back(RegIntPair(start, copied));
    if (gAllocFailed) {
        return -1;
    }
    if (start != end) {
        for (u32 i = 0; i < (*mStates)[start].mEdges.size(); i++) {
            int mapped = lookup((*mStates)[start].mEdges[i].mTarget);
            if (mapped != -1) {
                (*mStates)[copied].mEdges.push_back(
                    RegNfaEdge(mapped, (*mStates)[start].mEdges[i].mLabel));
            } else {
                RegNfaEdge &edge = (*mStates)[start].mEdges[i];
                int next = copy(edge.mTarget, end);
                (*mStates)[copied].mEdges.push_back(RegNfaEdge(next, edge.mLabel));
            }
            if (gAllocFailed) {
                return -1;
            }
        }
    }
    return copied;
}

// 801ECBA0
int RegNfaStateCopier::lookup(int from) {
    for (u32 i = 0; i < mMap.size(); i++) {
        if (mMap[i].mFirst == from) {
            return mMap[i].mSecond;
        }
    }
    return -1;
}

// 801ECBEC
RegIntPair::RegIntPair() : mFirst(-1), mSecond(-1) {
}

// 801ECBFC
RegIntPair::RegIntPair(int first, int second) : mFirst(first), mSecond(second) {
}

// 801ECC08
RegIntPair &RegIntPair::operator=(const RegIntPair &other) {
    mFirst = other.mFirst;
    mSecond = other.mSecond;
    return *this;
}

} // namespace RegEad
