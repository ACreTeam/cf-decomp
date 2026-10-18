#pragma once

// d_net_req_pool (DOL TU d_net_req_pool.cpp, .text 80081F04..8008205C, .bss 80583658): a pool of 32 prioritised
// request nodes. Only d_demo uses it: its demo requests are pool nodes, queued on dDemo's request list (ReqList_c)
// until the demo process picks one to run (see d_demo.hpp). No RTTI: names inferred.

#include <types.h>
#include <game/game/d_line.hpp>

namespace dNetReqPool {

enum {
    REQ_POOL_NUM = 32,
};

class Req_c : public dLineNdPr_c {
public:
    Req_c() { mPriority = 0xFF; } // 80082044 (weak)
    void clear();                 // 80081F04: frees the node (mState 0)
    u8 getState() const { return mState; }

    /* 0x0C */ u32 mSrcID;    // demo ID of the actor that asked (usually the player), or the building code
    /* 0x10 */ u32 mTargetID; // demo ID of the actor the demo is with
    /* 0x14 */ u8 mState;     // dDemo::ReqState_e (0: free)
    /* 0x15 */ u8 mType;      // dDemo::Type_e
    /* 0x16 */ u8 mStep;      // dDemo::Step_e: progress of the running demo
    /* 0x17 */ u8 mParam;     // a type's argument (menu number, exit index)
    /* 0x18 */ u8 mCheck;     // dDemoCmd_e: the talk kind, i.e. the start check the target runs
}; // size 0x1C

class ReqList_c : public dLineMg_c {
public:
    void clear(); // 80081F28: removes and frees every node
};

void init();    // 80081F8C: frees every pool node
Req_c *alloc(); // 80081FDC: the first free node, or NULL

extern Req_c sPool[REQ_POOL_NUM]; // 80583658

} // namespace dNetReqPool
