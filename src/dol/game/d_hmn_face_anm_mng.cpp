#include <game/game/d_hmn_face_anm_mng.hpp>
#include <lib/egg/core/eggHeap.h>
#include <revolution/OS/OSCache.h>
#include <cstdio>
#include <cstring>
#include <game/cLib/c_math.hpp>
#include <game/game/d_heap.hpp>

// 8074E3CC: the face animation heap (d_heap, not decompiled).

// The texture animation buffers of one face type (inferred).
class faceAnmBuf_c {
public:
    faceAnmBuf_c() { memset(this, 0, sizeof(*this)); }
    ~faceAnmBuf_c() {}

    void *get(int idx);
    void copy(const void *src, u32 size, int idx);
    BOOL create(EGG::Heap *heap);

    /* 0x0 */ void *mpBuf[2];
}; // size 0x8

// The archive and the buffers of all face types (inferred).
class faceAnmData_c {
public:
    faceAnmData_c();
    ~faceAnmData_c();

    BOOL load();
    static u32 getWorkSize();
    static void *getAnm(u32 *size, int texId);
    static int getMouthKind(u32 texId);
    static int getPlayMode(u32 texId);

    faceAnmBuf_c *getBuf(int type) { return &mBuf[type]; }

    /* 0x00 */ dHmnFaceAnmMng_c::arc_c mArc;
    /* 0x74 */ faceAnmBuf_c mBuf[13];
}; // size 0xDC

static faceAnmData_c l_data;

static const char l_arcName[] = "/FaceAnm/FaceAnm.arc";

// The eye texture animation of each body animation.
static const int l_eyeTexId[0x1BC] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 467, 0, 0, 0,
    6, 8, 10, 12, 467, 0, 0, 35, 0, 467, 0, 14, 0, 0, 17, 0,
    0, 0, 0, 19, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 21, 21, 22, 23, 23, 24, 24, 25, 25, 27, 27,
    467, 0, 0, 0, 29, 31, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 39, 41,
    467, 0, 0, 0, 0, 0, 0, 0, 0, 43, 45, 467, 0, 0, 0, 46,
    0, 467, 0, 0, 0, 0, 0, 0, 0, 0, 47, 49, 51, 53, 55, 57,
    59, 61, 63, 65, 67, 67, 69, 71, 73, 75, 77, 79, 81, 83, 0, 0,
    0, 0, 0, 0, 0, 85, 0, 87, 0, 0, 89, 0, 91, 467, 93, 467,
    94, 96, 98, 100, 102, 104, 106, 108, 110, 0, 0, 0, 0, 0, 0, 112,
    114, 0, 116, 118, 0, 0, 0, 0, 0, 120, 122, 124, 0, 0, 0, 0,
    126, 0, 0, 0, 134, 136, 138, 140, 142, 144, 146, 148, 150, 152, 154, 156,
    158, 160, 162, 164, 166, 168, 170, 172, 467, 0, 0, 174, 175, 176, 467, 178,
    179, 180, 181, 0, 0, 0, 0, 0, 0, 0, 182, 184, 186, 188, 0, 190,
    192, 194, 0, 0, 0, 0, 0, 0, 0, 0, 128, 0, 0, 0, 0, 0,
    0, 0, 196, 198, 200, 201, 0, 202, 204, 206, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 208, 0, 0, 209, 211, 213, 215, 217, 219, 221, 223,
    225, 227, 229, 231, 233, 235, 237, 239, 241, 0, 244, 246, 248, 0, 251, 253,
    255, 257, 259, 0, 262, 264, 266, 268, 270, 272, 274, 276, 278, 280, 282, 284,
    286, 288, 290, 292, 294, 296, 298, 300, 302, 304, 306, 308, 310, 312, 314, 316,
    318, 320, 322, 324, 0, 327, 328, 330, 332, 333, 334, 335, 336, 337, 338, 340,
    342, 344, 346, 348, 350, 352, 354, 356, 358, 360, 362, 364, 366, 368, 370, 372,
    374, 376, 378, 380, 382, 384, 386, 388, 390, 392, 394, 396, 398, 399, 400, 401,
    402, 403, 404, 406, 408, 410, 412, 414, 416, 418, 0, 421, 423, 425, 427, 429,
    431, 433, 435, 437, 439, 441, 443, 445, 447, 449, 451, 453, 455, 457, 0, 459,
    460, 0, 461, 462, 467, 467, 467, 467, 467, 467, 467, 467, 467, 467, 467, 467,
    467, 467, 467, 467, 467, 467, 467, 467, 467, 467, 467, 467,
};

// The mouth texture animation of each body animation.
static const int l_mouthTexId[0x1BC] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 467, 1, 1, 5,
    7, 9, 11, 13, 467, 1, 1, 36, 1, 467, 1, 15, 1, 16, 18, 1,
    1, 1, 1, 20, 2, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 26, 26, 1, 1,
    467, 1, 1, 28, 30, 32, 33, 34, 1, 1, 1, 37, 38, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 40, 42,
    467, 1, 1, 1, 1, 1, 1, 1, 1, 44, 1, 467, 1, 1, 1, 1,
    1, 467, 1, 1, 1, 1, 1, 1, 1, 1, 48, 50, 52, 54, 56, 58,
    60, 62, 64, 66, 68, 68, 70, 72, 74, 76, 78, 80, 82, 84, 1, 1,
    1, 1, 1, 1, 1, 86, 1, 88, 1, 1, 90, 1, 92, 467, 1, 467,
    95, 97, 99, 101, 103, 105, 107, 109, 111, 1, 1, 1, 1, 1, 1, 113,
    115, 1, 117, 119, 1, 1, 1, 1, 1, 121, 123, 125, 1, 1, 1, 1,
    127, 467, 467, 1, 135, 137, 139, 141, 143, 145, 147, 149, 151, 153, 155, 157,
    159, 161, 163, 165, 167, 169, 171, 173, 467, 1, 1, 464, 464, 177, 467, 464,
    464, 464, 464, 464, 1, 1, 1, 467, 467, 1, 183, 185, 187, 189, 1, 191,
    193, 195, 465, 464, 464, 465, 464, 464, 464, 464, 129, 1, 1, 1, 1, 1,
    1, 1, 197, 199, 464, 464, 1, 203, 205, 207, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 210, 212, 214, 216, 218, 220, 222, 224,
    226, 228, 230, 232, 234, 236, 238, 240, 242, 243, 245, 247, 249, 250, 252, 254,
    256, 258, 260, 261, 263, 265, 267, 269, 271, 273, 275, 277, 279, 281, 283, 285,
    287, 289, 291, 293, 295, 297, 299, 301, 303, 305, 307, 309, 311, 313, 315, 317,
    319, 321, 323, 325, 326, 1, 329, 331, 464, 464, 464, 464, 464, 464, 339, 341,
    343, 345, 347, 349, 351, 353, 355, 357, 359, 361, 363, 365, 367, 369, 371, 373,
    375, 377, 379, 381, 383, 385, 387, 389, 391, 393, 395, 397, 464, 464, 464, 464,
    464, 465, 405, 407, 409, 411, 413, 415, 417, 419, 420, 422, 424, 426, 428, 430,
    432, 434, 436, 438, 440, 442, 444, 446, 448, 450, 452, 454, 456, 458, 463, 465,
    464, 466, 464, 466, 467, 467, 467, 467, 467, 467, 467, 467, 467, 467, 467, 467,
    467, 467, 467, 467, 467, 467, 467, 467, 467, 467, 467, 467,
};

static const int l_mouthKind[0x1D3] = {
    2, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 0, 2, 2, 2, 1, 2, 2, 2, 1,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 1, 2, 2, 2, 1, 2, 2, 2, 0, 2, 2, 2,
    1, 2, 2, 2, 1, 2, 2, 2, 1, 2, 2, 2, 1, 2, 2, 2,
    1, 2, 2, 1, 2, 2, 2, 0, 2, 2, 1, 2, 2, 2, 0, 2,
    2, 2, 0, 2, 2, 1, 2, 2, 2, 1, 2, 2, 2, 1, 2, 2,
    2, 0, 2, 2, 2, 1, 2, 2, 2, 0, 2, 0, 2, 1, 2, 1,
    2, 2, 2, 0, 2, 2, 2, 1, 2, 2, 2, 0, 2, 0, 2, 2,
    2, 1, 2, 2, 2, 1, 2, 2, 2, 0, 2, 2, 2, 0, 2, 1,
    2, 2, 2, 1, 2, 2, 1, 2, 2, 2, 2, 1, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 2, 2, 1,
    2, 2, 2, 1, 2, 2, 2, 1, 2, 2, 2, 0, 2, 2, 2, 1,
    2, 2, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0, 2, 2, 2, 2,
    2, 0, 2, 1, 2, 2, 2, 2, 2, 2, 2, 0, 2, 1, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 2, 2, 2, 0, 2, 2,
    2, 1, 2, 2, 1, 2, 2, 2, 0, 2, 2, 2, 0, 2, 0, 2,
    1, 2, 2, 2, 0, 2, 1, 2, 2, 2, 1, 2, 2, 2, 1, 2,
    2, 2, 0, 2, 2, 2, 1, 2, 2, 2, 1, 2, 2, 2, 2, 1,
    2, 0, 1,
};

static const u8 l_playMode[0x1D3] = {
    1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1,
    0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1,
    1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0,
    0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1,
    1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
    0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1,
    0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0,
    1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0,
    1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0,
    1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0,
    1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0,
    0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0,
    1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1,
    0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0,
    0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1,
    1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1,
    1, 1, 1,
};

dHmnFaceAnmMng_c::dHmnFaceAnmMng_c() : mType(13) {}

dHmnFaceAnmMng_c::~dHmnFaceAnmMng_c() {}

void dHmnFaceAnmMng_c::setType(int type) {
    mType = type;
}

BOOL dHmnFaceAnmMng_c::setTex(int idx, int texId) {
    if (texId < 0 || texId >= 0x1D3) {
        return TRUE;
    }
    faceAnmBuf_c *buf = l_data.getBuf(mType);
    if (buf == NULL) {
        return TRUE;
    }
    u32 size = 0;
    void *anm = faceAnmData_c::getAnm(&size, texId);
    if (anm == NULL) {
        return TRUE;
    }
    buf->copy(anm, size, idx);
    nw4r::g3d::ResFile res(buf->get(idx));
    if (res.IsValid()) {
        res.Init();
    }
    return TRUE;
}

BOOL dHmnFaceAnmMng_c::setEyeTex(int texId) {
    return setTex(0, texId);
}

BOOL dHmnFaceAnmMng_c::setMouthTex(int texId) {
    return setTex(1, texId);
}

BOOL dHmnFaceAnmMng_c::setEyeTexByAnm(int anmId) {
    int texId = getEyeTexId(anmId);
    if (texId == 0x1D3) {
        return TRUE;
    }
    return setTex(0, texId);
}

BOOL dHmnFaceAnmMng_c::setMouthTexByAnm(int anmId) {
    int texId = getMouthTexId(anmId);
    if (texId == 0x1D3) {
        return TRUE;
    }
    return setTex(1, texId);
}

int dHmnFaceAnmMng_c::getEyeTexId(int anmId) {
    if (anmId >= 0x1BC) {
        return 0x1D3;
    }
    return l_eyeTexId[anmId];
}

int dHmnFaceAnmMng_c::getMouthTexId(int anmId) {
    if (anmId >= 0x1BC) {
        return 0x1D3;
    }
    return l_mouthTexId[anmId];
}

nw4r::g3d::ResAnmTexPat dHmnFaceAnmMng_c::getResAnmTexPat(int idx) const {
    faceAnmBuf_c *buf = l_data.getBuf(mType);
    if (buf == NULL) {
        return nw4r::g3d::ResAnmTexPat(NULL);
    }
    nw4r::g3d::ResFile res(buf->get(idx));
    if (res.IsValid()) {
        return res.GetResAnmTexPat(0);
    }
    return nw4r::g3d::ResAnmTexPat(NULL);
}

nw4r::g3d::ResAnmTexPat dHmnFaceAnmMng_c::getEyeResAnmTexPat() const {
    return getResAnmTexPat(0);
}

nw4r::g3d::ResAnmTexPat dHmnFaceAnmMng_c::getMouthResAnmTexPat() const {
    return getResAnmTexPat(1);
}

faceAnmData_c::faceAnmData_c() {}

faceAnmData_c::~faceAnmData_c() {}

BOOL faceAnmData_c::load() {
    EGG::Heap *heap = dHeap::hmnFaceAnmHeap_p;
    if (heap == NULL) {
        return TRUE;
    }
    if (!mArc.load(l_arcName, heap, 0)) {
        return FALSE;
    }
    faceAnmBuf_c *buf = mBuf;
    faceAnmBuf_c *end = buf + 13;
    for (; buf != end; buf++) {
        buf->create(heap);
    }
    return TRUE;
}

u32 faceAnmData_c::getWorkSize() {
    return 0x35580;
}

void *faceAnmData_c::getAnm(u32 *size, int texId) {
    return l_data.mArc.getAnm(size, texId);
}

int faceAnmData_c::getMouthKind(u32 texId) {
    if (texId < 0x1D3) {
        return l_mouthKind[texId];
    }
    return 2;
}

int faceAnmData_c::getPlayMode(u32 texId) {
    if (texId < 0x1D3) {
        return l_playMode[texId];
    }
    return 4;
}

void dHmnFaceAnmMng_c::arc_c::onLoaded() {
    ARCInitHandle(mpData, &mHandle);
    mArcReady = 1;
}

void *dHmnFaceAnmMng_c::arc_c::getAnm(u32 *size, int texId) {
    if (texId < 0 || texId >= 0x1D3) {
        return NULL;
    }
    char name[12];
    sprintf(name, "%d.brres", texId);
    u32 fileSize = 0;
    void *file = getFile(name, &fileSize);
    if (size != NULL) {
        *size = fileSize;
    }
    return file;
}

void *faceAnmBuf_c::get(int idx) {
    return mpBuf[idx];
}

void faceAnmBuf_c::copy(const void *src, u32 size, int idx) {
    u32 n = 0x280;
    if (size < 0x280) {
        n = size;
    }
    memcpy(mpBuf[idx], src, n);
    DCFlushRange(mpBuf[idx], n);
}

BOOL faceAnmBuf_c::create(EGG::Heap *heap) {
    for (int i = 0; i < 2; i++) {
        if (mpBuf[i] == NULL) {
            mpBuf[i] = heap->alloc(0x280, 0x20);
        }
    }
    return TRUE;
}

void dHmnFaceAnmMng_c::setBlinkCount() {
    mBlinkCount = 1;
    if ((u32)cM::rndF(100.0f) <= 10) {
        mBlinkCount = 2;
    }
}

BOOL dHmnFaceAnmMng_c::calcBlink() {
    if (mBlinkTimer == 0) {
        if (mBlinkCount == 0) {
            setBlinkCount();
            mBlinkTimer = 90.0f + cM::rndF(270.0f);
        } else {
            mBlinkCount--;
            return TRUE;
        }
    } else {
        mBlinkTimer--;
    }
    return FALSE;
}

void dHmnFaceAnmMng_c::resetBlink() {
    mBlinkCount = 0;
    mBlinkTimer = 0;
}

u32 dHmnFaceAnmMng_c::getWorkSize() {
    return faceAnmData_c::getWorkSize();
}

BOOL dHmnFaceAnmMng_c::load() {
    return l_data.load();
}

int dHmnFaceAnmMng_c::getMouthKind(u32 texId) {
    return faceAnmData_c::getMouthKind(texId);
}

int dHmnFaceAnmMng_c::getPlayMode(u32 texId) {
    return faceAnmData_c::getPlayMode(texId);
}
