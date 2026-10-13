#pragma once
#include <types.h>
// Names inferred from download application and connection operations.
namespace dNetDsDownload {
typedef void (*RecvFunc)(u16, const void *, u32);
typedef void (*SendFunc)(int);
class Application_c {
  public:
    Application_c();  // 800E42AC
    ~Application_c(); // 800E42E8
    bool isError() const {
        return mState == 9 || mState == 10;
    }
    u32 execute();                                    // 800E4344
    void start(u32 id, const char *path, int banner); // 800E4550
    void stop();                                      // 800E46AC
    /* 0x000 */ u8 _0000[0x5C];
    /* 0x05C */ u32 mState;
    /* 0x060 */ u8 _0060[0x13C];
}; // 0x19C
class Connection_c {
  public:
    Connection_c();                                       // 800E3E78
    ~Connection_c();                                      // 800E3E9C
    void execute();                                       // 800E3EE4
    void start(u32 id, u8 listen, RecvFunc recv);         // 800E4018
    void stop();                                          // 800E40A4
    BOOL send(const void *data, u32 size, SendFunc done); // 800E40C4
    /* 0x00 */ u8 _0000[8];
    /* 0x08 */ u32 mState;
    /* 0x0C */ u32 _000C;
    /* 0x10 */ u16 _0010;
    /* 0x12 */ u16 mConnected;
    /* 0x14 */ u8 _0014[8];
}; // 0x1C
} // namespace dNetDsDownload
extern "C" {
u32 fn_800E4888(); // socket heap size
u32 fn_800E4894(); // DWC heap size
}
