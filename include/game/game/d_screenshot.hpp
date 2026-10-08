#pragma once

// Start a request, poll getResult() until it returns nonzero, then call close().

#include <types.h>

namespace dScreenshot {

// The meanings of result values 4, 6, 7 and 8 are not yet identified.
enum Result_e {
    RESULT_BUSY = 0,
    RESULT_OK = 1,
    RESULT_ERROR = 2,
    RESULT_BAD_CARD = 3,     // mounted, but the drive flags lack 0x10
    RESULT_NO_CARD = 5,
    RESULT_FULL = 9,         // also: no higher directory number is available
    RESULT_REMOVED = 11,
    RESULT_NO_PHOTO = 12,
};

enum {
    FIRST_DIR_NUMBER = 100,
    LAST_DIR_NUMBER = 999,
    FILE_NUMBER_LIMIT = 10000,
    SD_ERROR_NOT_FOUND = 2,
    SD_ERROR_ALREADY_EXISTS = 17,
};

// The SD card library's drive work buffers.
struct work_c {
    /* 0x00 */ u8 *mBuf0;
    /* 0x04 */ u8 *mBuf1;
    /* 0x08 */ u16 mNum0;
    /* 0x0A */ u16 mNum1;
    /* 0x0C */ u32 m0C;
    /* 0x10 */ u32 m10;
}; // size 0x14

struct drive_c {
    /* 0x0 */ u32 m0;
    /* 0x4 */ work_c *mWork;
    /* 0x8 */ char mLetter;
    /* 0x9 */ u8 mFlags;
    /* 0xA */ u8 _A[0x10 - 0xA];
}; // size 0x10

// A find (opendir) handle.
struct find_c {
    /* 0x000 */ u8 _000[0x22D];
    /* 0x22D */ char mName[13]; // 8.3
    /* 0x23A */ u8 _23A[0x448 - 0x23A];
}; // size 0x448

void init(); // 8008EDF8
BOOL close(); // 8008F220: TRUE once the thread is gone; unlocks and unmounts
int getResult(); // 8008F278: RESULT_BUSY while the thread runs
void startMount(); // 8008F320
void startMountLock(); // 8008F388
void startMakeRoot(); // 8008F430
void startMakePath(); // 8008F844
void startWrite(); // 8008F940
void startCheck(); // 8008FB38
void startDelete(); // 8008FDB8
BOOL isIdle(); // 8008FDC8
void setBusy(); // 8008FDEC
void clearBusy(); // 8008FDF8
BOOL tryClose(); // 8008FE04

} // namespace dScreenshot
