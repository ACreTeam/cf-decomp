// d_hmn_head_mng.cpp: the player head models. .text 800B8EE0..800B953C.
// See include/game/game/d_hmn_head_mng.hpp. The data classes and all member names are inferred.
#include <game/game/d_hmn_head_mng.hpp>
#include <lib/egg/core/eggHeap.h>
#include <nw4r/g3d/res/g3d_resfile.h>
#include <revolution/OS.h>
#include <cstdio>
#include <cstring>

// 8074E3BC: dHeap::hmnHeadHeap_p (d_heap, not decompiled; name from its debug string).
extern EGG::Heap *lbl_8074E3BC;

// One double-buffered head model buffer (inferred).
class headBuf_c {
public:
    headBuf_c() {
        memset(mpBuf, 0, sizeof(mpBuf));
        mCur = 0;
    }
    ~headBuf_c() {}

    BOOL create(EGG::Heap *heap);
    void copy(const void *src, u32 size);
    void flip();
    void *get() const { return mpBuf[mCur]; }

    /* 0x0 */ void *mpBuf[2];
    /* 0x8 */ int mCur;
}; // size 0xC

// The head archive and the four head model buffers (one file-local instance; inferred).
class headData_c {
public:
    headData_c();
    ~headData_c();

    static u32 getWorkSize();
    static void *getHead(u32 *size, int no, int kind);
    BOOL load();
    static headBuf_c *getBuf(int idx);

    /* 0x00 */ dHmnHeadMng_c::arc_c mArc;
    /* 0x74 */ headBuf_c mBuf[4];
}; // size 0xA4

static const char l_arcName[] = "/Plyr/Head/Head.arc";

static headData_c l_data;

headData_c::headData_c() {}

headData_c::~headData_c() {}

u32 headData_c::getWorkSize() {
    return 0x1100A0;
}

void *headData_c::getHead(u32 *size, int no, int kind) {
    return l_data.mArc.getHead(size, no, kind);
}

BOOL headData_c::load() {
    EGG::Heap *heap = lbl_8074E3BC;
    if (heap == NULL) {
        return TRUE;
    }
    if (!mArc.load(l_arcName, heap, 0)) {
        return FALSE;
    }
    headBuf_c *buf = mBuf;
    headBuf_c *end = buf + 4;
    for (; buf != end; buf++) {
        buf->create(heap);
    }
    return TRUE;
}

headBuf_c *headData_c::getBuf(int idx) {
    if (idx < 4) {
        return &l_data.mBuf[idx];
    }
    return NULL;
}

void dHmnHeadMng_c::arc_c::onLoaded() {
    arcBank_c::onLoaded();
}

void *dHmnHeadMng_c::arc_c::getHead(u32 *size, int no, int kind) {
    if (no < 27 && kind < 2) {
        u32 fileSize = 0;
        char name[12];
        sprintf(name, "%d.brres", no + kind * 27);
        void *data = arcBank_c::getFile(name, &fileSize);
        if (size != NULL) {
            *size = fileSize;
        }
        return data;
    }
    return NULL;
}

BOOL headBuf_c::create(EGG::Heap *heap) {
    for (int i = 0; i < 2; i++) {
        if (mpBuf[i] == NULL) {
            mpBuf[i] = heap->alloc(0x5880, 0x20);
        }
    }
    return TRUE;
}

void headBuf_c::copy(const void *src, u32 size) {
    u32 n = size < 0x5880 ? size : 0x5880;
    memcpy(mpBuf[mCur], src, n);
    DCFlushRange(mpBuf[mCur], n);
}

void headBuf_c::flip() {
    mCur = mCur == 0;
}

dHmnHeadMng_c::dHmnHeadMng_c() : mBufIdx(4) {}

dHmnHeadMng_c::~dHmnHeadMng_c() {}

void dHmnHeadMng_c::setBufIdx(u8 idx) {
    mBufIdx = idx;
}

BOOL dHmnHeadMng_c::setHead(int no, int kind) {
    headBuf_c *buf = headData_c::getBuf(mBufIdx);
    if (buf != NULL) {
        u32 size = 0;
        void *data = headData_c::getHead(&size, no, kind);
        if (data != NULL) {
            buf->flip();
            buf->copy(data, size);
            bind();
            return TRUE;
        }
    }
    return FALSE;
}

void *dHmnHeadMng_c::getData() const {
    headBuf_c *buf = headData_c::getBuf(mBufIdx);
    if (buf != NULL) {
        return buf->get();
    }
    return NULL;
}

void dHmnHeadMng_c::bind() {
    nw4r::g3d::ResFile res(getData());
    if (res.IsValid()) {
        res.Init();
        res.Bind(res);
    }
}

u32 dHmnHeadMng_c::getWorkSize() {
    return headData_c::getWorkSize();
}

BOOL dHmnHeadMng_c::load() {
    return l_data.load();
}
