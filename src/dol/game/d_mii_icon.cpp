// .text 8008AEF0..8008B91C, .bss 805853E0..80587818.
#include <game/game/d_mii_icon.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_thunder.hpp>
#include <game/mLib/m_perf.hpp>
#include <lib/egg/core/eggHeap.h>
#include <lib/revolution/OS/OSThread.h>

// RVLFaceLib functions not named yet.
extern "C" {
RFLErrcode fn_802B96B0(); // RFLGetAsyncStatus
RFLErrcode fn_802C4BE8(RFLStoreData *data, RFLDataSource source, u16 index);
u32 RFLGetMiddleDBBufferSize(u16 num); // RFLGetMiddleDBBufferSize
void fn_802C4F80(RFLMiddleDB *db, int type, void *buf, u16 num); // RFLInitMiddleDB
RFLErrcode RFLAddMiddleDBStoreData(RFLMiddleDB *db, const RFLStoreData *data); // RFLAddMiddleDBStoreData
u32 fn_802C6010(u16 num);
void fn_802C601C(void *buf, u16 num);
RFLErrcode fn_802C60B4(void *dst, int, int, u16 index, u16 num);
u32 fn_802C61EC(u16 memberNum, u16 num);
void fn_802C6200(u32 *converter, void *buf, u16 myNo, u16 memberNum, u16 num);
RFLErrcode fn_802C63E8(u32 *converter, const void *data, u16 member);
void fn_802C64E4(u32 *converter, u16 member);
RFLErrcode fn_802C6574(RFLMiddleDB *db, u32 *converter, u16 member);
}

#define MII_ICON_DB_TYPE_USER 4
#define MII_ICON_DB_TYPE_NET 5

namespace dMiiIcon {

static OSThread l_thread;
static u8 l_stack[0x2000] ALIGN(32);
static mng_c l_mng;

void net_c::init(int myNo) {
    fn_802C6200(mConverter, mConverterBuf, myNo, MII_ICON_MEMBER_MAX, 1);
    for (u32 i = 0; i < MII_ICON_MEMBER_MAX; i++) {
        if (i < myNo) {
            fn_802C4F80(&mDB[i], MII_ICON_DB_TYPE_NET, mDBBuf[i], 1);
        } else if (i > myNo) {
            u32 slot = i - 1;
            fn_802C4F80(&mDB[slot], MII_ICON_DB_TYPE_NET, mDBBuf[slot], 1);
        }
    }
    u16 index = 0;
    if (RFLSearchOfficialData(&dPlayerMgr::Mgr_c::getCurrentPlayer()->mMiiId, &index)) {
        if (fn_802C60B4(mMyData, 0, 0, index, 1) != RFLErrcode_Success) {
            mError[myNo] |= 1;
        } else {
            mHasMyData = true;
        }
    }
}

void net_c::clear() {
    mHasMyData = false;
    for (u32 i = 0; i < MII_ICON_MEMBER_MAX; i++) {
        reset(i);
    }
}

void net_c::reset(int member) {
    mState[member] = STATE_NONE;
    if (mConverting[member]) {
        fn_802C64E4(mConverter, member);
        mConverting[member] = false;
    }
    mError[member] = 0;
}

bool net_c::hasMyData() {
    return mHasMyData;
}

void *net_c::getMyData() {
    return mMyData;
}

void *net_c::getData(int member) {
    int myNo = dNet::getMyNo();
    if (member < myNo) {
        return mData[member];
    } else if (member > myNo) {
        return mData[member - 1];
    }
    return NULL;
}

void net_c::setData(const void *data, int member) {
    if (data != NULL) {
        cLib::memCpy(getData(member), data, MII_ICON_NET_DATA_SIZE);
        mState[member] = STATE_RECEIVED;
    } else {
        mState[member] = STATE_FAILED;
    }
}

RFLMiddleDB *net_c::getDB(int member) {
    int myNo = dNet::getMyNo();
    if (member < myNo) {
        return &mDB[member];
    } else if (member > myNo) {
        return &mDB[member - 1];
    }
    return NULL;
}

RFLMiddleDB *net_c::getReadyDB(int member) {
    if (member < MII_ICON_MEMBER_MAX && member != dNet::getMyNo() && mState[member] == STATE_READY) {
        return getDB(member);
    }
    return NULL;
}

BOOL net_c::isDone(int member) {
    if (member < MII_ICON_MEMBER_MAX && (mState[member] == STATE_READY || mState[member] == STATE_FAILED)) {
        return TRUE;
    }
    return FALSE;
}

void *travel_c::get() {
    if (!dPlayerMgr::Mgr_c::getCurrentPlayer()->isFlag0(PRIVATE_FLAG0_TRAVELLING)) {
        u16 index = 0;
        if (RFLSearchOfficialData(&dPlayerMgr::Mgr_c::getCurrentPlayer()->mMiiId, &index)) {
            if (fn_802C4BE8(&mData, RFLDataSource_Official, index) != RFLErrcode_Success) {
                mError |= 1;
            } else {
                return &mData;
            }
        }
        cLib::memSet(&mData, 0, sizeof(mData));
        return NULL;
    }
    if (getDB() != NULL) {
        return &mData;
    }
    return NULL;
}

BOOL travel_c::set(const void *data) {
    fn_802C4F80(&mDB, MII_ICON_DB_TYPE_USER, mDBBuf, 1);
    if (data != NULL) {
        cLib::memCpy(&mData, data, sizeof(mData));
        if (RFLAddMiddleDBStoreData(&mDB, &mData) != RFLErrcode_Success) {
            mError |= 1;
        } else {
            mState = STATE_READY;
            return TRUE;
        }
    }
    cLib::memSet(&mData, 0, sizeof(mData));
    mState = STATE_FAILED;
    return FALSE;
}

RFLMiddleDB *travel_c::getDB() {
    if (mState == STATE_READY) {
        return &mDB;
    }
    return NULL;
}

static void *threadMain(void *);

void mng_c::create(EGG::Heap *heap) {
    u32 dataSize, converterSize, dbSize;
    u32 size;
    dataSize = ROUND_UP(fn_802C6010(1), 4);
    size = dataSize + dataSize * (MII_ICON_MEMBER_MAX - 1);
    converterSize = ROUND_UP(fn_802C61EC(MII_ICON_MEMBER_MAX, 1), 4);
    size += converterSize;
    dbSize = ROUND_UP(RFLGetMiddleDBBufferSize(1), 4);
    size += dbSize * MII_ICON_MEMBER_MAX;
    mWork = (u8 *)heap->alloc(size, 4);
    mNet.mMyData = mWork;
    for (int i = 0; i < MII_ICON_MEMBER_MAX - 1; i++) {
        mNet.mData[i] = mWork + dataSize + dataSize * i;
    }
    mNet.mConverterBuf = mWork + dataSize + dataSize * (MII_ICON_MEMBER_MAX - 1);
    for (int i = 0; i < MII_ICON_MEMBER_MAX - 1; i++) {
        mNet.mDBBuf[i] = mWork + dataSize + dataSize * (MII_ICON_MEMBER_MAX - 1) + converterSize + dbSize * i;
    }
    mTravel.mDBBuf = mWork + dataSize + dataSize * (MII_ICON_MEMBER_MAX - 1) + converterSize + dbSize * (MII_ICON_MEMBER_MAX - 1);
    fn_802C601C(mWork, 1);
    fn_802C6200(mNet.mConverter, mNet.mConverterBuf, 0, MII_ICON_MEMBER_MAX, 1);
    OSCreateThread(&l_thread, threadMain, NULL, l_stack + sizeof(l_stack), sizeof(l_stack), 17, OS_THREAD_DETACHED);
    mPerf::setThreadColor(&l_thread, dPerf::getThreadColor(7));
    mCreated = true;
}

void mng_c::execute() {
    if (mCreated) {
        bool pending = false;
        for (int i = 0; i < MII_ICON_MEMBER_MAX; i++) {
            if (mNet.mState[i] == STATE_RECEIVED && !pending) {
                pending = true;
            }
        }
        if (pending && OSIsThreadSuspended(&l_thread)) {
            OSResumeThread(&l_thread);
        }
    }
}

bool mng_c::isIdle() {
    for (int i = 0; i < MII_ICON_MEMBER_MAX; i++) {
        if (mNet.mState[i] == STATE_RECEIVED) {
            mNet.mState[i] = STATE_NONE;
        }
    }
    return OSIsThreadSuspended(&l_thread);
}

static void *threadMain(void *) {
    while (true) {
        for (u32 i = 0; i < MII_ICON_MEMBER_MAX; i++) {
            if (l_mng.mNet.mState[i] == STATE_RECEIVED) {
                void *data = l_mng.mNet.getData(i);
                l_mng.mNet.mConverting[i] = true;
                RFLErrcode err = fn_802C63E8(l_mng.mNet.mConverter, data, i);
                while (err == RFLErrcode_Busy) {
                    err = fn_802B96B0();
                }
                if (err == RFLErrcode_Success) {
                    if (fn_802C6574(l_mng.mNet.getDB(i), l_mng.mNet.mConverter, i) == RFLErrcode_Success) {
                        l_mng.mNet.mState[i] = STATE_READY;
                    } else {
                        l_mng.mNet.mError[i] |= 1;
                        l_mng.mNet.mState[i] = STATE_FAILED;
                    }
                } else {
                    l_mng.mNet.mError[i] |= 1;
                    l_mng.mNet.mState[i] = STATE_FAILED;
                }
            }
        }
        OSSuspendThread(&l_thread);
    }
}

void create(EGG::Heap *heap) {
    l_mng.create(heap);
}

void startNet(int myNo) {
    l_mng.mNet.init(myNo);
}

void endNet() {
    l_mng.mNet.clear();
}

void removeMember(int member) {
    l_mng.mNet.reset(member);
}

void execute() {
    l_mng.execute();
}

bool hasMyMii() {
    return l_mng.mNet.hasMyData();
}

void *getMyMii() {
    return l_mng.mNet.getMyData();
}

void setMemberMii(const void *data, int member) {
    l_mng.mNet.setData(data, member);
}

RFLMiddleDB *getMemberDB(int member) {
    return l_mng.mNet.getReadyDB(member);
}

BOOL isMemberDone(int member) {
    return l_mng.mNet.isDone(member);
}

bool isIdle() {
    return l_mng.isIdle();
}

void *getTravelMii() {
    return l_mng.mTravel.get();
}

BOOL setTravelMii(const void *data) {
    return l_mng.mTravel.set(data);
}

RFLMiddleDB *getTravelDB() {
    return l_mng.mTravel.getDB();
}

} // namespace dMiiIcon
