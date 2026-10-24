#pragma once

#include <revolution/gx.h>
#include <revolution/os.h>

namespace EGG {

// Thread scheduling meter (Nintendo-internal extension of EGG); names are inferred.
class ThreadMeter {
public:
    // One recorded thread slice in a frame record.
    struct Slice {
        OSThread *mpThread; // 0x00
        u32 mStartTick;     // 0x04
        u32 mEndTick;       // 0x08
    };

    struct FrameRecord {
        Slice *mpSlices; // 0x00
        u32 _04;
        u32 _08;
        int mState;      // 0x0C
        int mSliceCount; // 0x10
    };

    // One tracked thread.
    struct ThreadEntry {
        OSThread *mpThread; // 0x00
        GXColor mColor;     // 0x04
        u8 mVisible;        // 0x08
        char mName[0x20];
        u32 _2C;
    };

    void setThreadColor(OSThread *thread, GXColor color); // 80441494
    GXColor getDefaultColor() const { return mDefaultColor; }

    // Records a thread switch into the current frame record.
    static void onSwitchThread(OSThread *from, OSThread *to); // 80440F2C

    int findIndex(OSThread *thread) const; // 80440FFC
    int add(OSThread *thread);             // 80441040
    void sortByPriority();                 // 80441118

private:
    u32 _00;
    int mMaxSlices;              // 0x04
    int mMaxThreads;             // 0x08
    u32 _0C;
    FrameRecord mFrames[2];      // 0x10
    u8 mCurrentFrame;            // 0x38
    u8 _39[3];
    ThreadEntry *mpThreads;      // 0x3C
    u32 _40;
    int mThreadCount;            // 0x44
    u8 _48[0xEC - 0x48];
    GXColor mDefaultColor;       // 0xEC
    u32 _F0;
    u32 mDefaultParam;           // 0xF4
    u8 mSortEnabled;             // 0xF8
};

} // namespace EGG
