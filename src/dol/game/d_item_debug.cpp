// d_item_debug.cpp: item debugging. .text 800C578C..800C5C30, .ctors 804656B8, .data 804EA8E0..804EA970,
// .bss 8059DD30..8059FF80, .sdata 8074A9A0..8074A9B8. See include/game/game/d_item_debug.hpp.
// Class names from RTTI ("hostIO_c" is global); all other names are inferred.
#include <game/game/d_item_debug.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_save_dl_item.hpp>
#include <lib/MSL/arith.h>

#define ADD_PACK_NUM_NONE -10000
#define ADD_PACK_NUM_MAX 0x100U // unsigned: labs(num) is compared unsigned
#define HOST_IO_ID_NUM 87

namespace dItemDebug {

// Loads the test item archive (into dHeap::localHeap_p) for the host IO.
class addPackLoader_c : public dDvd::arcBank_c {
public:
    enum MODE_e {
        MODE_NUM, // loadNum: add mNum items
        MODE_IDX, // loadIdx: add item mIdx
    };

    addPackLoader_c();
    virtual ~addPackLoader_c() {}
    virtual void onLoaded();

    BOOL loadNum(int num);
    BOOL loadIdx(int idx);
    BOOL close();

    /* 0x74 */ int mNum;
    /* 0x78 */ int mMode;
    /* 0x7C */ int mIdx;
    /* 0x80 */ u8 mReset;
}; // size 0x84

} // namespace dItemDebug

// The host IO block of the item debugger.
class hostIO_c {
public:
    enum STATE_e {
        STATE_IDLE = -1,
        STATE_LOAD,
        STATE_CLOSE,
    };

    hostIO_c() {
        for (int i = 0; i < HOST_IO_ID_NUM; i++) {
            mIds[i] = 0;
        }
        mNum = 0;
        mIdx = 0;
        mState = STATE_IDLE;
        mDebug = s_debugFlag.mEnabled;
    }
    virtual ~hostIO_c() {}

    /* 0x04 */ u16 mIds[HOST_IO_ID_NUM];
    /* 0xB2 */ s16 mNum;
    /* 0xB4 */ s16 mIdx;
    /* 0xB6 */ s16 mState;
    /* 0xB8 */ u8 mDebug;
    /* 0xBC */ dItemDebug::addPackLoader_c mLoader;
}; // size 0x140

static const char *l_arcPath = "/TestAddItem/testAddItem.arc";

static dSaveDLItem_c l_dlItem;
static hostIO_c l_hostIO;
static dItemDebug::dlList_c l_dlList;

namespace dItemDebug {

// 800C578C
BOOL create() {
    return TRUE;
}

// 800C5794
void execute() {
    hostIO_c *io = &l_hostIO;
    switch (io->mState) {
        case hostIO_c::STATE_LOAD:
            if (abs(io->mNum) > 0) {
                if (io->mLoader.loadNum(io->mNum)) {
                    io->mState++;
                }
            } else {
                if (io->mLoader.loadIdx(io->mIdx)) {
                    io->mState++;
                }
            }
            break;
        case hostIO_c::STATE_CLOSE:
            if (io->mLoader.close()) {
                io->mState = hostIO_c::STATE_IDLE;
                io->mNum = 0;
            }
            break;
        case hostIO_c::STATE_IDLE:
            break;
        default:
            l_hostIO.mState = hostIO_c::STATE_IDLE;
            break;
    }
}

// 800C5874
BOOL draw() {
    return TRUE;
}

// 800C587C
addPackLoader_c::addPackLoader_c() : mNum(ADD_PACK_NUM_NONE), mMode(MODE_NUM), mReset(1) {}

// 800C58D4
BOOL addPackLoader_c::loadNum(int num) {
    if (mReset) {
        mNum = ADD_PACK_NUM_MAX;
        if (labs(num) <= ADD_PACK_NUM_MAX) {
            mNum = num;
        }
        mMode = MODE_NUM;
    }
    if (mReset) {
        mReset = 0;
    }
    return load(l_arcPath, dHeap::localHeap_p, 2);
}

// 800C5960
BOOL addPackLoader_c::loadIdx(int idx) {
    mIdx = idx;
    mMode = MODE_IDX;
    return load(l_arcPath, dHeap::localHeap_p, 2);
}

// 800C597C
void addPackLoader_c::onLoaded() {
    mNum = ADD_PACK_NUM_NONE;
}

// 800C5988
BOOL addPackLoader_c::close() {
    if (closeArc()) {
        mReset = 1;
        return TRUE;
    }
    return FALSE;
}

// 800C59D0
BOOL loadRes() {
    return FALSE;
}

// 800C59D8
BOOL checkRes() {
    return FALSE;
}

// 800C59E0
void dlList_c::clear() {}

// 800C59E4
void dlList_c::update() {}

// 800C59E8
void dlList_c::add(dItem::Item item) {}

// 800C59EC
dlList_c *dlList_c::get() {
    return &l_dlList;
}

} // namespace dItemDebug
