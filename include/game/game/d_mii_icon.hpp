#pragma once

// The Miis drawn as icons: the other net members' (converted on a worker thread) and the travel Mii.

#include <types.h>
#include <revolution/RFL.h>

namespace EGG {
class Heap;
}

#define MII_ICON_MEMBER_MAX 4
#define MII_ICON_NET_DATA_SIZE 0x54

namespace dMiiIcon {

enum State_e {
    STATE_NONE,
    STATE_RECEIVED, // waits for the thread
    STATE_READY,
    STATE_FAILED,
};

class net_c {
public:
    void init(int myNo);
    void clear();
    void reset(int member);
    bool hasMyData();
    void *getMyData();
    void *getData(int member);
    void setData(const void *data, int member);
    RFLMiddleDB *getDB(int member);
    RFLMiddleDB *getReadyDB(int member);
    BOOL isDone(int member);

    /* 0x00 */ void *mMyData;
    /* 0x04 */ bool mHasMyData;
    /* 0x08 */ void *mData[MII_ICON_MEMBER_MAX - 1];
    /* 0x14 */ u32 mConverter[4];
    /* 0x24 */ void *mConverterBuf;
    /* 0x28 */ bool mConverting[MII_ICON_MEMBER_MAX];
    /* 0x2C */ RFLMiddleDB mDB[MII_ICON_MEMBER_MAX - 1];
    /* 0x74 */ void *mDBBuf[MII_ICON_MEMBER_MAX - 1];
    /* 0x80 */ int mState[MII_ICON_MEMBER_MAX];
    /* 0x90 */ u32 mError[MII_ICON_MEMBER_MAX];
};

class travel_c {
public:
    void *get();
    BOOL set(const void *data);
    RFLMiddleDB *getDB();

    /* 0x00 */ RFLStoreData mData;
    /* 0x4C */ RFLMiddleDB mDB;
    /* 0x64 */ void *mDBBuf;
    /* 0x68 */ int mState;
    /* 0x6C */ u32 mError;
};

class mng_c {
public:
    void create(EGG::Heap *heap);
    void execute();
    bool isIdle();

    /* 0x00 */ u8 *mWork;
    /* 0x04 */ bool mCreated;
    /* 0x08 */ net_c mNet;
    /* 0xA8 */ travel_c mTravel;
};

void create(EGG::Heap *heap);
void startNet(int myNo);
void endNet();
void removeMember(int member);
void execute();
bool hasMyMii();
void *getMyMii();
void setMemberMii(const void *data, int member); // NULL = none
RFLMiddleDB *getMemberDB(int member);
BOOL isMemberDone(int member);
bool isIdle(); // the worker thread is suspended
void *getTravelMii(); // NULL when no travel Mii is available
BOOL setTravelMii(const void *data); // NULL clears the travel Mii
RFLMiddleDB *getTravelDB();

} // namespace dMiiIcon
