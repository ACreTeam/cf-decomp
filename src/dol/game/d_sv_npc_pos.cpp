// Packed npc spots in the town save (dSvNpcPos). .text 80150524..80150778, .sdata2 80750CD8..80750CE0.
// See include/game/game/d_sv_npc_pos.hpp.
#include <game/game/d_sv_npc_pos.hpp>
#include <game/game/d_field_info.hpp>

namespace dSvNpcPos {

// 80150524
void KindPos_c::init() {
    mKind = 0x44;
    mUnitX = 0;
    mUnitZ = 0;
    mFlag = 0;
    mValid = 0;
}

// 80150544
void KindPos_c::set(u8 kind, const mVec3_c *pos) {
    f32 x = pos->x;
    f32 z = pos->z;
    mUnitX = (int)x >> 5;
    mUnitZ = (int)z >> 5;
    mKind = kind;
    mValid = 1;
    mFlag = 0;
}

// 80150594
void KindPos_c::update(u8 kind, const mVec3_c *pos) {
    f32 x = pos->x;
    f32 z = pos->z;
    mUnitX = (int)x >> 5;
    mUnitZ = (int)z >> 5;
    mKind = kind;
}

// 801505D4
mVec3_c KindPos_c::getPos() const {
    mVec3_c pos;
    dFdBase_c::getUnitCenterPos(&pos, mUnitX, mUnitZ);
    return pos;
}

// 801505E4
void Pos_c::clear() {
    mUnitX = 0;
    mUnitZ = 0;
    mValid = 0;
    mFlag = 0;
}

// 801505FC
void Pos_c::set(u8 unitX, u8 unitZ) {
    mUnitX = unitX;
    mUnitZ = unitZ;
    mFlag = 0;
    mValid = 1;
}

// 80150618
mVec3_c Pos_c::getPos() const {
    mVec3_c pos;
    dFdBase_c::getUnitCenterPos(&pos, mUnitX, mUnitZ);
    return pos;
}

// 80150628
void SidePos_c::clear() {
    mUnitX = 0;
    mUnitZ = 0;
    mFlag = 0;
    mValid = 0;
    mSide = SIDE_CENTER;
}

// 80150648
void SidePos_c::set(const mVec3_c *pos) {
    f32 x = pos->x;
    f32 z = pos->z;
    mUnitX = (int)x >> 5;
    mUnitZ = (int)z >> 5;
    mSide = SIDE_CENTER;
    mFlag = 0;
    mValid = 1;
    if ((int)x % 32 < 6) {
        mSide = SIDE_LEFT;
    } else if ((int)x % 32 >= 25) {
        mSide = SIDE_RIGHT;
    }
}

// 801506F8
mVec3_c SidePos_c::getPos() const {
    mVec3_c pos;
    dFdBase_c::getUnitCenterPos(&pos, mUnitX, mUnitZ);
    switch (mSide) {
    case SIDE_LEFT:
        pos.x -= 14.4f;
        break;
    case SIDE_RIGHT:
        pos.x += 14.4f;
        break;
    }
    return pos;
}

} // namespace dSvNpcPos
