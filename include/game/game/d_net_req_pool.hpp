#pragma once

// d_net_req_pool (DOL TU d_net_req_pool.cpp, .text 80081F04..8008205C, .bss 80583658): a pool of 32 prioritised
// request nodes. Only d_demo uses it: it allocates nodes (alloc), fills them (building code at _0C, kinds at _14..)
// and queues them on its own lists (ReqList_c, e.g. 8074E9D8). No RTTI: names inferred.

#include <types.h>
#include <game/game/d_line.hpp>

namespace dNetReqPool {

enum {
    REQ_POOL_NUM = 32,
};

class Req_c : public dPriLineNd_c {
public:
    Req_c() { mPriority = 0xFF; } // 80082044 (weak)
    void clear();                 // 80081F04: frees the node (mState 0)

    /* 0x0C */ u32 _0C;
    /* 0x10 */ u32 _10;
    /* 0x14 */ u8 mState; // 0: free
    /* 0x15 */ u8 _15;
    /* 0x16 */ u8 _16;
    /* 0x17 */ u8 _17;
    /* 0x18 */ u8 _18;
}; // size 0x1C

class ReqList_c : public cLineMg_c {
public:
    void clear(); // 80081F28: removes and frees every node
};

void init();    // 80081F8C: frees every pool node
Req_c *alloc(); // 80081FDC: the first free node, or NULL

extern Req_c sPool[REQ_POOL_NUM]; // 80583658

} // namespace dNetReqPool
