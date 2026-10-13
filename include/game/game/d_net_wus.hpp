#ifndef GAME_D_NET_WUS_HPP
#define GAME_D_NET_WUS_HPP

#include <types.h>

// Wii user service presence notifications and activity inquiries.
// Names are inferred from the request fields and dWifi callers.
namespace dNetWus {
enum InitState_e {
    INIT_NONE,
    INIT_SOCKET,
    INIT_SOCKET_STARTED,
    INIT_HTTP
};
enum {
    HTTP_STATUS_OK = 200,
    RESPONSE_BUFFER_SIZE = 0x200,
    WII_ID_LENGTH = 16,
    MAX_INQUIRY_IDS = 255,
    REQUEST_TIMEOUT_MSEC = 60000,
    POLL_INTERVAL_MSEC = 8,
    HTTP_POST = 1,
    HTTP_REQUEST_PENDING = 15,
    HTTP_THREAD_PRIORITY = 21,
    RESULT_CODE_LENGTH = 3,
    SOCKET_ALREADY_INITIALIZED = -7
};
class dNetWus_c {
public:
    dNetWus_c(); // 800EB158
    ~dNetWus_c(); // 800EB170
    BOOL open(); // 800EB1B0
    BOOL close(); // 800EB2D4
    BOOL createRequest(const char *url); // 800EB398
    BOOL destroyRequest(); // 800EB43C
    BOOL sendRequest(); // 800EB4A0
    BOOL notify(u64 userId, BOOL remove); // 800EB584
    BOOL inquire(const u64 *keys, int num, int validTerm, u8 *results); // 800EB700

    /* 0x0 */ void *mRequest;
    /* 0x4 */ int mError;
    /* 0x8 */ int mStatus;
}; // size 0xC
} // namespace dNetWus

#endif
