// The request node pool (dNetReqPool). .text 80081F04..8008205C, .ctors 80465650, .bss 80583658..805839D8.
// See include/game/game/d_net_req_pool.hpp.
#include <game/game/d_net_req_pool.hpp>

namespace dNetReqPool {

// 80081F04
void Req_c::clear() {
    mState = 0;
    mSrcID = 0;
    mTargetID = 0;
    mType = 0;
    mStep = 0;
    mParam = 0;
    mCheck = 0;
}

// 80081F28
void ReqList_c::clear() {
    Req_c *node = (Req_c *)mpFirst;
    while (node != NULL) {
        Req_c *next = (Req_c *)node->getNext();
        removeLineNode(node);
        node->clear();
        node = next;
    }
}

// 80081F8C
void init() {
    for (int i = 0; i < REQ_POOL_NUM; i++) {
        sPool[i].clear();
    }
}

// 80081FDC
Req_c *alloc() {
    for (int i = 0; i < REQ_POOL_NUM; i++) {
        if (sPool[i].mState == 0) {
            return &sPool[i];
        }
    }
    return NULL;
}

Req_c sPool[REQ_POOL_NUM];

} // namespace dNetReqPool
