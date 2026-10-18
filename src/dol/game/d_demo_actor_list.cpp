// The priority line list (dLineMg_c) and the demo actor list (dDemoActorList_c). .text 800CD590..800CD670.
// See include/game/game/d_line.hpp and d_demo_actor.hpp.
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_line.hpp>

// 800CD590
void dLineMg_c::init() {
    mpFirst = NULL;
    mpLast = NULL;
}

// 800CD5A0
bool dLineMg_c::insertByPriority(dLineNdPr_c *node) {
    if (mpFirst == NULL) {
        mpFirst = node;
        mpLast = node;
        return true;
    }
    cLineNd_c *prev = mpFirst;
    for (dLineNdPr_c *cur = (dLineNdPr_c *)prev->getNext(); cur != NULL; cur = (dLineNdPr_c *)cur->getNext()) {
        if (cur->mPriority > node->mPriority) {
            insertLineNode(node, prev);
            return true;
        }
        prev = cur;
    }
    insertLineNode(node, prev);
    return true;
}

// 800CD614
bool dDemoActorList_c::add(dDemoActorNode_c *node) {
    addLastLineNode(node);
    return true;
}

// 800CD638
dDemoActorNode_c *dDemoActorList_c::find(u32 id) {
    if (id == 0) {
        return NULL;
    }
    for (dDemoActorNode_c *node = (dDemoActorNode_c *)mpFirst; node != NULL; node = (dDemoActorNode_c *)node->getNext()) {
        if (id == node->mID) {
            return node;
        }
    }
    return NULL;
}
