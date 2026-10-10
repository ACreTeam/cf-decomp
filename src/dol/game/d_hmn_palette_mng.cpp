#include <game/game/d_hmn_palette_mng.hpp>
#include <lib/egg/core/eggHeap.h>
#include <revolution/OS/OSCache.h>
#include <cstdio>
#include <cstring>
#include <game/game/d_heap.hpp>

// 8074E3C0: the palette heap (d_heap, not decompiled).

// The palette buffers of one palette type (inferred).
class plttBuf_c {
public:
    plttBuf_c();
    ~plttBuf_c() {}

    void *get(int idx) { return mpBuf[idx]; }
    BOOL create(EGG::Heap *heap);
    void copy(int idx, const void *src, u32 size);

    /* 0x0 */ void *mpBuf[3];
}; // size 0xC

// The archive and the buffers of all palette types (inferred).
class plttData_c {
public:
    plttData_c();
    ~plttData_c();

    static u32 getWorkSize();
    static void *getPltt(u32 *size, int plttId);
    BOOL load();
    static plttBuf_c *getBuf(int type);

    /* 0x00 */ dHmnPaletteMng_c::arc_c mArc;
    /* 0x74 */ plttBuf_c mBuf[4];
}; // size 0xA4

static plttData_c l_data;

static const char l_arcName[] = "/Plyr/Pltt/Palette.arc";

plttData_c::plttData_c() {}

plttData_c::~plttData_c() {}

u32 plttData_c::getWorkSize() {
    return 0x6FC0;
}

void *plttData_c::getPltt(u32 *size, int plttId) {
    return l_data.mArc.getPltt(size, plttId);
}

BOOL plttData_c::load() {
    EGG::Heap *heap = dHeap::hmnPaletteHeap_p;
    if (heap == NULL) {
        return TRUE;
    }
    if (!mArc.load(l_arcName, heap, 0)) {
        return FALSE;
    }
    plttBuf_c *buf = mBuf;
    plttBuf_c *end = buf + 4;
    for (; buf != end; buf++) {
        buf->create(heap);
    }
    return TRUE;
}

plttBuf_c *plttData_c::getBuf(int type) {
    if (type < 4) {
        return &l_data.mBuf[type];
    }
    return NULL;
}

void dHmnPaletteMng_c::arc_c::onLoaded() {
    ARCInitHandle(mpData, &mHandle);
    mArcReady = 1;
}

void *dHmnPaletteMng_c::arc_c::getPltt(u32 *size, int plttId) {
    if (plttId < 0x5C) {
        u32 fileSize = 0;
        char name[12];
        sprintf(name, "%d.brplt", plttId);
        void *file = getFile(name, &fileSize);
        if (size != NULL) {
            *size = fileSize;
        }
        return file;
    }
    return NULL;
}

plttBuf_c::plttBuf_c() {
    mpBuf[0] = NULL;
    mpBuf[1] = NULL;
    mpBuf[2] = NULL;
}

BOOL plttBuf_c::create(EGG::Heap *heap) {
    for (int i = 0; i < 3; i++) {
        if (mpBuf[i] == NULL) {
            mpBuf[i] = heap->alloc(0x100, 0x20);
        }
    }
    return TRUE;
}

void plttBuf_c::copy(int idx, const void *src, u32 size) {
    if (idx < 3) {
        void *buf = mpBuf[idx];
        if (buf != NULL) {
            u32 n = size < 0x100 ? size : 0x100;
            memcpy(buf, src, n);
            DCFlushRange(buf, n);
        }
    }
}

dHmnPaletteMng_c::dHmnPaletteMng_c() : mType(4) {}

dHmnPaletteMng_c::~dHmnPaletteMng_c() {}

void dHmnPaletteMng_c::setType(int type) {
    mType = type;
}

BOOL dHmnPaletteMng_c::setPltt(int plttId, int idx) {
    plttBuf_c *buf = plttData_c::getBuf(mType);
    if (buf != NULL) {
        u32 size = 0;
        void *pltt = plttData_c::getPltt(&size, plttId);
        if (pltt != NULL) {
            buf->copy(idx, pltt, size);
            initResFile(idx);
            return TRUE;
        }
    }
    return FALSE;
}

nw4r::g3d::ResFile dHmnPaletteMng_c::getResFile(int idx) const {
    plttBuf_c *buf = plttData_c::getBuf(mType);
    if (buf != NULL) {
        return nw4r::g3d::ResFile(buf->get(idx));
    }
    return nw4r::g3d::ResFile(NULL);
}

void dHmnPaletteMng_c::initResFile(int idx) const {
    nw4r::g3d::ResFile res = getResFile(idx);
    if (res.IsValid()) {
        res.Init();
    }
}

u32 dHmnPaletteMng_c::getWorkSize() {
    return plttData_c::getWorkSize();
}

BOOL dHmnPaletteMng_c::load() {
    return l_data.load();
}
