#include <lib/RegEad/RegNfa.hpp>
#include <lib/RegEad/RegEadArray.inl>

namespace RegEad {

// 801EC588
RegNfaEdge::RegNfaEdge() : mTarget(-1) {
}

// 801EC5C4
RegNfaEdge::RegNfaEdge(int target) : mTarget(target) {
}

// 801EC5FC
RegNfaEdge::RegNfaEdge(int target, u16 ch) : mTarget(target), mLabel(ch) {
}

// 801EC638
RegNfaEdge::RegNfaEdge(int target, int type) : mTarget(target), mLabel(type) {
}

// 801EC674
RegNfaEdge::RegNfaEdge(int target, const RegLabel &label) : mTarget(target), mLabel(label) {
}

// 801EC6B0
RegNfaEdge &RegNfaEdge::operator=(const RegNfaEdge &other) {
    mTarget = other.mTarget;
    mLabel = other.mLabel;
    return *this;
}

// 801EC6F0
RegNfaState::RegNfaState() {
}

// 801EC730
RegNfaState::~RegNfaState() {
}

// 801EC78C
RegNfaState &RegNfaState::operator=(const RegNfaState &other) {
    mEdges = other.mEdges;
    return *this;
}


} // namespace RegEad
