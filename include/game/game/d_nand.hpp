#pragma once

// d_nand (DOL TU d_nand.cpp, .text 800D22F8..800D3590): the save file's NAND access. Saves run on a worker
// thread (one request at a time, its stack from dHeap::nandHeap_p); each NAND SDK call is wrapped to turn its
// result into a RESULT_e. No RTTI: the names are inferred (namespace dNand: see notes/game_tu_conventions.md).

#include <types.h>
#include <lib/revolution/NAND.h>

class dSceneChange_c;

namespace dNand {

// The result of a NAND operation (getResult; the wrappers' return values).
enum RESULT_e {
    RESULT_NONE,      // a request is running
    RESULT_OK,
    RESULT_CORRUPT,   // NAND_RESULT_CORRUPT (d_reset: fatal)
    RESULT_SHORT,     // fewer bytes read / sought than asked
    RESULT_NOEXISTS,  // NAND_RESULT_NOEXISTS
    RESULT_RETRY,     // NAND_RESULT_AUTHENTICATION / ECC_CRIT (isRetryResult): the safe write starts over
    RESULT_ERROR,     // anything else
    RESULT_MAXBLOCKS, // NAND_RESULT_MAXBLOCKS
    RESULT_MAXFILES,  // NAND_RESULT_MAXFILES
};

// A file request for the worker thread (sRequest).
struct Request_s {
    /* 0x00 */ const char *mPath;
    /* 0x04 */ u32 mOffset;
    /* 0x08 */ const void *mWriteData; // set: a write (isWriting)
    /* 0x0C */ void *mReadData;
    /* 0x10 */ u32 mSize;
}; // size 0x14

void updateCheckFlags();       // 800D22F8: dSaveCheck_c::mFlags of the town (d_sv_mgr before saving)
BOOL isReady();                // 800D2360: no request runs (d_reset waits for it before a reset / fade-out)
void reset();                  // 800D23A0: (d_reset soft reset)
BOOL isWriting();              // 800D2404: a write / delete request runs (d_net_mgr: don't copy the friend list)
u32 getRequestOffset();        // 800D240C
u32 getRequestSize();          // 800D241C

void *writeAllThread(void *arg);        // 800D242C
void *writeFriendListThread(void *arg); // 800D2454
void *writePlayerThread(void *arg);     // 800D247C
void *writeAnimalsThread(void *arg);    // 800D24A4
void *writeDLItemsThread(void *arg);    // 800D24CC
void *writeTownThread(void *arg);       // 800D24F4
void *readAllThread(void *arg);         // 800D251C
void *deleteAllThread(void *arg);       // 800D2544

BOOL saveFriendList();         // 800D256C: the current player's friend list
BOOL savePlayer();             // 800D25D8: the current player
BOOL saveAnimals();            // 800D2648
BOOL saveDLItems();            // 800D26B0
BOOL saveTown();               // 800D2720: the save up to its DL items
BOOL saveAll();                // 800D2780: the whole save and the banner
BOOL loadAll();                // 800D27E0: the save into dSaveData_c::getRaw2, the banner into sBannerBuffer
BOOL deleteAll();              // 800D2830
u32 getStackSize();            // 800D2840: the worker thread's stack (from dHeap::nandHeap_p)
BOOL startThread(OSThreadFunc func, const Request_s *request); // 800D284C: FALSE while one still runs
BOOL updateThread();           // 800D2968: TRUE when no request runs (frees a finished one)
BOOL isIdle();                 // 800D2A4C: updateThread
BOOL isRetryResult(s32 result); // 800D2A24
int getResult();               // 800D2A44: the last request's RESULT_e

int create(const char *path);                                                  // 800D2A50
int safeOpen(NANDFileInfo *info, const char *path, void *buffer, u32 size);    // 800D2ACC: for writing
int safeCancel(NANDFileInfo *info);                                            // 800D2B78
int safeClose(NANDFileInfo *info);                                             // 800D2BD4
int open(NANDFileInfo *info, const char *path);                                // 800D2C30: for reading
int close(NANDFileInfo *info);                                                 // 800D2CAC
int seek(NANDFileInfo *info, s32 offset);                                      // 800D2D18
int read(NANDFileInfo *info, void *buffer, s32 size);                          // 800D2DBC
int deleteFile(const char *path);                                              // 800D2E5C
int safeSeek(NANDFileInfo *info, s32 offset);                                  // 800D2EB8
int write(NANDFileInfo *info, const void *buffer, s32 size);                   // 800D2F3C
int writeFile(const char *path, u32 offset, const void *data, u32 size);       // 800D2FCC: retried up to 10 times
int writeAll();                // 800D3118
int writePlayer();             // 800D3168
int writeFriendList();         // 800D31DC
int writeAnimals();            // 800D324C
int writeDLItems();            // 800D3298
int writeTown();               // 800D32E4
int readFile(const char *path, u32 offset, void *data, u32 size);              // 800D332C
int readAll();                 // 800D340C
int deleteAllFiles();          // 800D3464
int getType(const char *path); // 800D34C4: RESULT_OK if it exists
int check(u32 blocks, u32 files, u32 *answer); // 800D3524: NANDCheck
void log(const char *func, s32 result);        // 800D3570: empty
int fn_800D3574();             // 800D3574: returns 0 (d_s_boot_static)
int fn_800D357C();             // 800D357C: returns 0 (d_s_boot_static)
dSceneChange_c *getSceneChange(); // 800D3584: &gSceneChange (d_net)

} // namespace dNand
