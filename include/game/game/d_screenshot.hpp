#pragma once

// Start a request, poll getResult() until it returns nonzero, then call close().

#include <types.h>

namespace dScreenshot {

// The meanings of result values 4, 6, 7, 8 and 10 come from the messages the
// photo menu (d_menu_photoNP) shows for them.
enum Result_e {
    RESULT_BUSY = 0,
    RESULT_OK = 1,
    RESULT_ERROR = 2,
    RESULT_BAD_CARD = 3,     // mounted, but the drive flags lack 0x10
    RESULT_DEVICE_ERROR = 4, // errno 5
    RESULT_NO_CARD = 5,
    RESULT_DEVICE_ERROR_2 = 6, // errno 8 or 88
    RESULT_WRITE_PROTECTED = 7, // errno 201
    RESULT_NO_SPACE = 8,        // errno 28
    RESULT_FULL = 9,         // also: no higher directory number is available
    RESULT_TOO_MANY_FILES = 10,
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
