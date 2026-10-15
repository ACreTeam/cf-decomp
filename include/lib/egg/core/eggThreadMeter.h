#pragma once

#include <revolution/GX.h>
#include <revolution/OS/OSThread.h>

namespace EGG {

// Thread scheduling meter interface; names and the known layout prefix are inferred.
class ThreadMeter {
public:
    void setThreadColor(OSThread *thread, GXColor color); // 80441494
    GXColor getDefaultColor() const { return mDefaultColor; }

private:
    u8 _000[0xEC];
    GXColor mDefaultColor; // 0xEC
};

} // namespace EGG
