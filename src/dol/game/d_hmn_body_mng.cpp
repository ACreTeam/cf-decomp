// d_hmn_body_mng.cpp: the player body models. .text 800B6750..800B6CF0.
// See include/game/game/d_hmn_body_mng.hpp. The data classes and all member names are inferred.
#include <game/game/d_hmn_body_mng.hpp>
#include <lib/egg/core/eggHeap.h>
#include <nw4r/g3d/res/g3d_resfile.h>
#include <revolution/OS.h>
#include <cstdio>
#include <cstring>

// 8074E3C8: dHeap::hmnBodyHeap_p (d_heap, not decompiled; name from its debug string).
extern EGG::Heap *lbl_8074E3C8;

// One body model buffer (inferred).
class bodyBuf_c {
public:
    bodyBuf_c() : mpData(NULL) {}
    ~bodyBuf_c() {}

    BOOL create(EGG::Heap *heap);
    void copy(const void *src, u32 size);

    /* 0x0 */ void *mpData;
}; // size 0x4

// The body archive and the four body model buffers (one file-local instance; inferred).
class bodyData_c {
public:
    bodyData_c();
    ~bodyData_c();

    static u32 getWorkSize();
    static void *getBody(u32 *size, int idx);
    BOOL load();
    static bodyBuf_c *getBuf(int idx);

    /* 0x00 */ dHmnBodyMng_c::arc_c mArc;
    /* 0x74 */ bodyBuf_c mBuf[4];
}; // size 0x84

static const char l_arcName[] = "/Plyr/Body/Body.arc";

static bodyData_c l_data;

bodyData_c::bodyData_c() {}

bodyData_c::~bodyData_c() {}

u32 bodyData_c::getWorkSize() {
    return 0x1EFE0;
}

void *bodyData_c::getBody(u32 *size, int idx) {
    return l_data.mArc.getBody(size, idx);
}

BOOL bodyData_c::load() {
    EGG::Heap *heap = lbl_8074E3C8;
    if (heap == NULL) {
        return TRUE;
    }
    if (!mArc.load(l_arcName, heap, 0)) {
        return FALSE;
    }
    bodyBuf_c *buf = mBuf;
    bodyBuf_c *end = buf + 4;
    for (; buf != end; buf++) {
        buf->create(heap);
    }
    return TRUE;
}

bodyBuf_c *bodyData_c::getBuf(int idx) {
    if (idx < 4) {
        return &l_data.mBuf[idx];
    }
    return NULL;
}

void dHmnBodyMng_c::arc_c::onLoaded() {
    arcBank_c::onLoaded();
}

void *dHmnBodyMng_c::arc_c::getBody(u32 *size, int idx) {
    if (idx < 2) {
        u32 fileSize = 0;
        char name[8];
        sprintf(name, "%d.brres", idx);
        void *data = arcBank_c::getFile(name, &fileSize);
        if (size != NULL) {
            *size = fileSize;
        }
        return data;
    }
    return NULL;
}

BOOL bodyBuf_c::create(EGG::Heap *heap) {
    if (mpData == NULL) {
        mpData = heap->alloc(0x5380, 0x20);
    }
    return TRUE;
}

void bodyBuf_c::copy(const void *src, u32 size) {
    u32 n = size < 0x5380 ? size : 0x5380;
    memcpy(mpData, src, n);
    DCFlushRange(mpData, n);
}

dHmnBodyMng_c::dHmnBodyMng_c() : mBufIdx(4) {}

dHmnBodyMng_c::~dHmnBodyMng_c() {}

void dHmnBodyMng_c::setBufIdx(u8 idx) {
    mBufIdx = idx;
}

BOOL dHmnBodyMng_c::setBody(int no) {
    bodyBuf_c *buf = bodyData_c::getBuf(mBufIdx);
    if (buf != NULL) {
        u32 size = 0;
        void *data = bodyData_c::getBody(&size, no);
        if (data != NULL) {
            buf->copy(data, size);
            bind();
            return TRUE;
        }
    }
    return FALSE;
}

void *dHmnBodyMng_c::getData() const {
    bodyBuf_c *buf = bodyData_c::getBuf(mBufIdx);
    if (buf != NULL) {
        return buf->mpData;
    }
    return NULL;
}

void dHmnBodyMng_c::bind() {
    nw4r::g3d::ResFile res(getData());
    if (res.IsValid()) {
        res.Init();
        res.Bind(res);
    }
}

u32 dHmnBodyMng_c::getWorkSize() {
    return bodyData_c::getWorkSize();
}

BOOL dHmnBodyMng_c::load() {
    return l_data.load();
}
