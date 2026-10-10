// d_hmn_face_tex_mng.cpp: the player face textures. .text 800B7EF8..800B81F0.
// See include/game/game/d_hmn_face_tex_mng.hpp. The data class and all member names are inferred.
#include <game/game/d_hmn_face_tex_mng.hpp>
#include <lib/egg/core/eggHeap.h>
#include <cstdio>
#include <game/game/d_heap.hpp>

// 8074E3C4: dHeap::hmnFaceTexHeap_p (d_heap, not decompiled; name from its debug string).

// The face texture archive (one file-local instance; inferred).
class faceTexData_c {
public:
    faceTexData_c();
    ~faceTexData_c();

    static u32 getWorkSize();
    void *getTex(u32 *size, int no);
    BOOL load();

    /* 0x00 */ dHmnFaceTexMng_c::arc_c mArc;
}; // size 0x74

static faceTexData_c l_data;

static const char l_arcName[] = "/Plyr/FcTx/FaceTex.arc";

faceTexData_c::faceTexData_c() {}

faceTexData_c::~faceTexData_c() {}

u32 faceTexData_c::getWorkSize() {
    return 0x4E6A0;
}

void *faceTexData_c::getTex(u32 *size, int no) {
    return mArc.getTex(size, no);
}

BOOL faceTexData_c::load() {
    EGG::Heap *heap = dHeap::hmnFaceTexHeap_p;
    if (heap == NULL) {
        return TRUE;
    }
    return mArc.load(l_arcName, heap, 0) != FALSE;
}

void dHmnFaceTexMng_c::arc_c::onLoaded() {
    arcBank_c::onLoaded();
    for (int i = 0; i < 0x36; i++) {
        void *data = getTex(NULL, i);
        if (data != NULL) {
            nw4r::g3d::ResFile res(data);
            if (res.IsValid()) {
                res.Init();
            }
        }
    }
}

void *dHmnFaceTexMng_c::arc_c::getTex(u32 *size, int no) {
    if (no < 0x36) {
        u32 fileSize = 0;
        char name[12];
        sprintf(name, "%d.brtex", no);
        void *data = arcBank_c::getFile(name, &fileSize);
        if (size != NULL) {
            *size = fileSize;
        }
        return data;
    }
    return NULL;
}

u32 dHmnFaceTexMng_c::getWorkSize() {
    return faceTexData_c::getWorkSize();
}

BOOL dHmnFaceTexMng_c::load() {
    return l_data.load();
}

nw4r::g3d::ResFile dHmnFaceTexMng_c::getResFile(int no) {
    void *data = l_data.getTex(NULL, no);
    if (data != NULL) {
        return nw4r::g3d::ResFile(data);
    }
    return nw4r::g3d::ResFile(NULL);
}
