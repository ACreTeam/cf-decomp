// The save file's NAND access. .text 800D22F8..800D3590, .data 804EC390..804EC440, .bss 805A2D80..805B2160,
// .sdata 8074AD90..8074AD98, .sbss 8074E500..8074E510. See include/game/game/d_nand.hpp.
#include <game/game/d_nand.hpp>
#include <game/game/d_banner.hpp>
#include <game/game/d_friend.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_s_boot_static.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_dl_item.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_sv_mgr.hpp>
#include <game/game/d_thunder.hpp>
#include <game/mLib/m_perf.hpp>
#include <lib/revolution/OS.h>
#include <string.h>

#define NAND_STACK_SIZE 0xA000
#define NAND_SAFE_BUFFER_SIZE 0x4000
#define NAND_WRITE_TRIES 10

#define FRIEND_LIST_SIZE 0x1120 // sizeof(dFriendList_c)
#define PLAYER_SIZE 0x86C0      // sizeof(dPrivateData_c)
#define ANIMALS_SIZE 0x3C740    // sizeof(dAnimalSave_c)
#define DL_ITEMS_SIZE 0x200004

namespace dNand {

static OSThread sThread;                   // 805A2D80
static Request_s sRequest;                 // 805A3098
static u8 sBannerBuffer[0xF0A0] ALIGN(32); // 805A30C0: loadAll reads the banner here
static int sResult = RESULT_OK;            // 8074AD90
static void *sStack;                       // 8074E500
static u8 sThreadActive;                   // 8074E504
static OSThreadFunc sThreadFunc;           // 8074E508
static u8 sWriting;                        // 8074E50C
static u8 sCheckFlagsChanged;              // 8074E50D

// The save slot of this console's player: its own town's when hosting, else its network player number.
static inline int getPlayerNo() {
    return !dNet::isHost() ? dNet::getLocalPlayerNo() : dPlayerMgr::getCurrentPlayerSlot();
}

// 800D22F8
void updateCheckFlags() {
    u8 flags = 1;
    if (dBanner_c::sFlags == 1) {
        flags = 0;
    }
    bool set = dSaveData_c::getRaw()->mHeader.mFlags != 0;
    sCheckFlagsChanged = set != flags;
    dSaveData_c::getTown()->mHeader.mFlags = flags;
}

// 800D2360
BOOL isReady() {
    if (isIdle() || fn_801086A4()) {
        return TRUE;
    }
    return FALSE;
}

// 800D23A0
void reset() {
    if (!fn_801086A4()) {
        sResult = RESULT_OK;
        sWriting = FALSE;
        sThreadActive = FALSE;
        sThreadFunc = NULL;
        sRequest.mPath = NULL;
        sRequest.mOffset = 0;
        sRequest.mWriteData = NULL;
        sRequest.mReadData = NULL;
        sRequest.mSize = 0;
        sStack = NULL;
        sCheckFlagsChanged = FALSE;
    }
}

// 800D2404
BOOL isWriting() {
    return sWriting;
}

// 800D240C
u32 getRequestOffset() {
    return sRequest.mOffset;
}

// 800D241C
u32 getRequestSize() {
    return sRequest.mSize;
}

// 800D242C
void *writeAllThread(void *arg) {
    sResult = writeAll();
    return NULL;
}

// 800D2454
void *writeFriendListThread(void *arg) {
    sResult = writeFriendList();
    return NULL;
}

// 800D247C
void *writePlayerThread(void *arg) {
    sResult = writePlayer();
    return NULL;
}

// 800D24A4
void *writeAnimalsThread(void *arg) {
    sResult = writeAnimals();
    return NULL;
}

// 800D24CC
void *writeDLItemsThread(void *arg) {
    sResult = writeDLItems();
    return NULL;
}

// 800D24F4
void *writeTownThread(void *arg) {
    sResult = writeTown();
    return NULL;
}

// 800D251C
void *readAllThread(void *arg) {
    sResult = readAll();
    return NULL;
}

// 800D2544
void *deleteAllThread(void *arg) {
    sResult = deleteAllFiles();
    return NULL;
}

// 800D256C
BOOL saveFriendList() {
    int player = getPlayerNo();
    Request_s request;
    request.mPath = lbl_8074B010;
    request.mOffset = dSvMgr_c::getPlayerDataOffset(player);
    request.mWriteData = dPlayerMgr::Mgr_c::getCurrentPlayer();
    request.mSize = FRIEND_LIST_SIZE;
    dPlayerMgr::Mgr_c::getCurrentPlayer()->mFriends.updateChecksum();
    return startThread(writeFriendListThread, &request);
}

// 800D25D8
BOOL savePlayer() {
    int player = getPlayerNo();
    Request_s request;
    request.mPath = lbl_8074B010;
    request.mOffset = dSvMgr_c::getPlayerDataOffset(player);
    request.mWriteData = dPlayerMgr::Mgr_c::getCurrentPlayerRaw();
    request.mSize = PLAYER_SIZE;
    dPlayerMgr::Mgr_c::getCurrentPlayer()->updateChecksum();
    return startThread(writePlayerThread, &request);
}

// 800D2648
BOOL saveAnimals() {
    Request_s request;
    request.mPath = lbl_8074B010;
    request.mOffset = dSvMgr_c::getVisitorDataOffset();
    request.mWriteData = &dSaveData_c::getRaw()->mAnimals;
    request.mSize = ANIMALS_SIZE;
    dSaveData_c::getTown()->mAnimals.updateChecksum();
    return startThread(writeAnimalsThread, &request);
}

// 800D26B0
BOOL saveDLItems() {
    Request_s request;
    request.mPath = lbl_8074B010;
    request.mOffset = dSvMgr_c::getDLDataOffset();
    request.mWriteData = &dSaveData_c::getRaw()->mDLItems;
    request.mSize = DL_ITEMS_SIZE;
    dSaveDLItemList_c *items = dSaveDLItemList_c::get();
    items->mChecksum = items->calcChecksum();
    return startThread(writeDLItemsThread, &request);
}

// 800D2720
BOOL saveTown() {
    Request_s request;
    request.mPath = lbl_8074B010;
    request.mOffset = 0;
    request.mWriteData = dSaveData_c::getRaw();
    dSaveData_c::getRaw();
    request.mSize = dSaveData_c::getDLDataOffset();
    dSaveData_c::getTown()->updateTownChecksum();
    dSaveData_c::getExtra()->updateChecksum();
    return startThread(writeTownThread, &request);
}

// 800D2780
BOOL saveAll() {
    Request_s request;
    request.mPath = lbl_8074B010;
    request.mOffset = 0;
    request.mWriteData = dSaveData_c::getRaw();
    request.mReadData = NULL;
    request.mSize = lbl_80750AD0;
    dSaveData_c::get()->updateChecksum();
    return startThread(writeAllThread, &request);
}

// 800D27E0
BOOL loadAll() {
    Request_s request;
    request.mPath = lbl_8074B010;
    request.mOffset = 0;
    request.mWriteData = NULL;
    request.mReadData = dSaveData_c::getRaw2();
    request.mSize = lbl_80750AD0;
    return startThread(readAllThread, &request);
}

// 800D2830
BOOL deleteAll() {
    return startThread(deleteAllThread, NULL);
}

// 800D2840
u32 getStackSize() {
    return NAND_STACK_SIZE;
}

// 800D284C
BOOL startThread(OSThreadFunc func, const Request_s *request) {
    if (!updateThread()) {
        return FALSE;
    }
    sStack = dHeap::nandHeap_p->alloc(NAND_STACK_SIZE, 4);
    if (request != NULL) {
        memcpy(&sRequest, request, sizeof(Request_s));
    }
    sThreadFunc = func;
    sThreadActive = TRUE;
    sResult = RESULT_NONE;
    if (sRequest.mWriteData != NULL || func == deleteAllThread) {
        sWriting = TRUE;
    }
    OSCreateThread(&sThread, func, &sRequest, (u8 *)sStack + NAND_STACK_SIZE, NAND_STACK_SIZE, 13, 1);
    fn_802B8D30(&sThread, fn_8016CF08(3));
    OSResumeThread(&sThread);
    return TRUE;
}

// 800D2968
BOOL updateThread() {
    if (!sThreadActive) {
        return TRUE;
    }
    if (OSIsThreadTerminated(&sThread)) {
        if (sStack != NULL) {
            dHeap::nandHeap_p->free(3);
            sStack = NULL;
            fn_802B8D90(&sThread);
        }
        if (sWriting) {
            sWriting = FALSE;
        }
        sRequest.mPath = NULL;
        sRequest.mOffset = 0;
        sRequest.mWriteData = NULL;
        sRequest.mReadData = NULL;
        sRequest.mSize = 0;
        sThreadActive = FALSE;
        sThreadFunc = NULL;
        return TRUE;
    }
    return FALSE;
}

// 800D2A24
BOOL isRetryResult(s32 result) {
    if (result == NAND_RESULT_AUTHENTICATION || result == NAND_RESULT_ECC_CRIT) {
        return TRUE;
    }
    return FALSE;
}

// 800D2A44
int getResult() {
    return sResult;
}

// 800D2A4C
BOOL isIdle() {
    return updateThread();
}

// 800D2A50
int create(const char *path) {
    s32 result = NANDCreate(path, NAND_PERM_RUSR | NAND_PERM_WUSR, 0);
    log("NANDCreate", result);
    if (result == NAND_RESULT_OK || result == NAND_RESULT_EXISTS) {
        return RESULT_OK;
    } else if (result == NAND_RESULT_CORRUPT) {
        return RESULT_CORRUPT;
    } else if (result == NAND_RESULT_MAXFILES) {
        return RESULT_MAXFILES;
    } else {
        return RESULT_ERROR;
    }
}

// 800D2ACC
int safeOpen(NANDFileInfo *info, const char *path, void *buffer, u32 size) {
    s32 result = NANDSimpleSafeOpen(path, info, NAND_ACCESS_WRITE, buffer, size);
    log("NANDSimpleSafeOpen", result);
    if (result == NAND_RESULT_OK) {
        return RESULT_OK;
    } else if (isRetryResult(result)) {
        return RESULT_RETRY;
    } else if (result == NAND_RESULT_CORRUPT) {
        return RESULT_CORRUPT;
    } else if (result == NAND_RESULT_MAXFILES) {
        return RESULT_MAXFILES;
    } else {
        int ret = RESULT_ERROR;
        if (result == NAND_RESULT_MAXBLOCKS) {
            ret = RESULT_MAXBLOCKS;
        }
        return ret;
    }
}

// 800D2B78
int safeCancel(NANDFileInfo *info) {
    s32 result = NANDSimpleSafeCancel(info);
    log("NANDSimpleSafeCancel", result);
    if (result == NAND_RESULT_OK) {
        return RESULT_OK;
    } else if (result == NAND_RESULT_CORRUPT) {
        return RESULT_CORRUPT;
    } else {
        return RESULT_ERROR;
    }
}

// 800D2BD4
int safeClose(NANDFileInfo *info) {
    s32 result = NANDSimpleSafeClose(info);
    log("NANDSimpleSafeClose", result);
    if (result == NAND_RESULT_OK) {
        return RESULT_OK;
    } else if (result == NAND_RESULT_CORRUPT) {
        return RESULT_CORRUPT;
    } else {
        return RESULT_ERROR;
    }
}

// 800D2C30
int open(NANDFileInfo *info, const char *path) {
    s32 result = NANDOpen(path, info, NAND_ACCESS_READ);
    log("NANDOpen", result);
    if (result == NAND_RESULT_OK) {
        return RESULT_OK;
    } else if (result == NAND_RESULT_NOEXISTS) {
        return RESULT_NOEXISTS;
    } else if (result == NAND_RESULT_CORRUPT) {
        return RESULT_CORRUPT;
    } else {
        return RESULT_ERROR;
    }
}

// 800D2CAC
int close(NANDFileInfo *info) {
    s32 result = NANDClose(info);
    log("NANDClose", result);
    if (result == NAND_RESULT_OK) {
        return RESULT_OK;
    } else if (result == NAND_RESULT_NOEXISTS) {
        return RESULT_NOEXISTS;
    } else if (result == NAND_RESULT_CORRUPT) {
        return RESULT_CORRUPT;
    } else {
        return RESULT_ERROR;
    }
}

// 800D2D18
int seek(NANDFileInfo *info, s32 offset) {
    s32 result = NANDSeek(info, offset, NAND_SEEK_BEG);
    log("NANDSeek", result);
    if (result == offset) {
        return RESULT_OK;
    } else if (isRetryResult(result)) {
        return RESULT_RETRY;
    } else if (result > 0) {
        return RESULT_SHORT;
    } else if (result == NAND_RESULT_NOEXISTS) {
        return RESULT_NOEXISTS;
    } else if (result == NAND_RESULT_CORRUPT) {
        return RESULT_CORRUPT;
    } else {
        return RESULT_ERROR;
    }
}

// 800D2DBC
int read(NANDFileInfo *info, void *buffer, s32 size) {
    s32 result = NANDRead(info, buffer, size);
    log("NANDRead", result);
    if (result == size) {
        return RESULT_OK;
    } else if (result > 0) {
        return RESULT_SHORT;
    } else if (isRetryResult(result)) {
        return RESULT_RETRY;
    } else if (result == NAND_RESULT_NOEXISTS) {
        return RESULT_NOEXISTS;
    } else if (result == NAND_RESULT_CORRUPT) {
        return RESULT_CORRUPT;
    } else {
        return RESULT_ERROR;
    }
}

// 800D2E5C
int deleteFile(const char *path) {
    s32 result = NANDDelete(path);
    log("NANDDelete", result);
    if (result == NAND_RESULT_OK) {
        return RESULT_OK;
    } else if (result == NAND_RESULT_CORRUPT) {
        return RESULT_CORRUPT;
    } else {
        return RESULT_ERROR;
    }
}

// 800D2EB8
int safeSeek(NANDFileInfo *info, s32 offset) {
    s32 result = NANDSeek(info, offset, NAND_SEEK_BEG);
    log("NANDSeek", result);
    if (result == offset) {
        return RESULT_OK;
    } else if (isRetryResult(result)) {
        return RESULT_RETRY;
    } else if (result == NAND_RESULT_CORRUPT) {
        return RESULT_CORRUPT;
    } else {
        return RESULT_ERROR;
    }
}

// 800D2F3C
int write(NANDFileInfo *info, const void *buffer, s32 size) {
    s32 result = NANDWrite(info, buffer, size);
    log("NANDWrite", result);
    if (result == size) {
        return RESULT_OK;
    } else if (isRetryResult(result)) {
        return RESULT_RETRY;
    } else if (result == NAND_RESULT_CORRUPT) {
        return RESULT_CORRUPT;
    } else {
        int ret = RESULT_ERROR;
        if (result == NAND_RESULT_MAXBLOCKS) {
            ret = RESULT_MAXBLOCKS;
        }
        return ret;
    }
}

// 800D2FCC
int writeFile(const char *path, u32 offset, const void *data, u32 size) {
    NANDFileInfo info;
    u8 buffer[NAND_SAFE_BUFFER_SIZE] ALIGN(32);
    int result;
    int tries = 0;
    while (true) {
        tries++;
        if (tries > NAND_WRITE_TRIES) {
            return RESULT_ERROR;
        }
        result = create(path);
        if (result != RESULT_OK) {
            return result;
        }
        result = safeOpen(&info, path, buffer, NAND_SAFE_BUFFER_SIZE);
        if (result == RESULT_RETRY) {
            result = safeCancel(&info);
            if (result != RESULT_OK) {
                return result;
            }
            result = deleteFile(path);
            if (result != RESULT_OK) {
                return result;
            }
            continue;
        }
        if (result != RESULT_OK) {
            return result;
        }
        result = safeSeek(&info, offset);
        if (result == RESULT_RETRY) {
            result = safeCancel(&info);
            if (result != RESULT_OK) {
                return result;
            }
            continue;
        }
        if (result != RESULT_OK) {
            return result;
        }
        result = write(&info, data, size);
        if (result == RESULT_RETRY) {
            result = safeCancel(&info);
            if (result != RESULT_OK) {
                return result;
            }
            continue;
        }
        if (result != RESULT_OK) {
            return result;
        }
        result = safeClose(&info);
        if (result != RESULT_OK) {
            return result;
        }
        return RESULT_OK;
    }
}

// 800D3118
int writeAll() {
    writeFile(lbl_8074B010, 0, dSaveData_c::getRaw(), lbl_80750AD0);
    return writeFile(dBanner_c::sFileName, 0, &g_banner.mBanner, dBanner_c::sFileSize);
}

// 800D3168
int writePlayer() {
    int player = getPlayerNo();
    return writeFile(lbl_8074B010, dSvMgr_c::getPlayerDataOffset(player), dPlayerMgr::Mgr_c::getCurrentPlayerRaw(),
                     PLAYER_SIZE);
}

// 800D31DC
int writeFriendList() {
    int player = getPlayerNo();
    dFriendList_c *friends = &dPlayerMgr::Mgr_c::getCurrentPlayer()->mFriends;
    return writeFile(lbl_8074B010, dSvMgr_c::getPlayerDataOffset(player), friends, FRIEND_LIST_SIZE);
}

// 800D324C
int writeAnimals() {
    dSaveData_c *save = dSaveData_c::getRaw();
    return writeFile(lbl_8074B010, dSvMgr_c::getVisitorDataOffset(), &save->mAnimals, ANIMALS_SIZE);
}

// 800D3298
int writeDLItems() {
    dSaveDLItemList_c *items = &dSaveData_c::getRaw()->mDLItems;
    return writeFile(lbl_8074B010, dSvMgr_c::getDLDataOffset(), items, DL_ITEMS_SIZE);
}

// 800D32E4
int writeTown() {
    dSaveData_c::getRaw();
    u32 size = dSaveData_c::getDLDataOffset();
    return writeFile(lbl_8074B010, 0, dSaveData_c::getRaw(), size);
}

// 800D332C
int readFile(const char *path, u32 offset, void *data, u32 size) {
    NANDFileInfo info;
    int result = open(&info, path);
    if (result != RESULT_OK) {
        return result;
    }
    result = seek(&info, offset);
    if (result != RESULT_OK) {
        int closeResult = close(&info);
        if (closeResult != RESULT_OK) {
            result = closeResult;
        }
        return result;
    }
    result = read(&info, data, size);
    if (result != RESULT_OK) {
        int closeResult = close(&info);
        if (closeResult != RESULT_OK) {
            result = closeResult;
        }
        return result;
    }
    int closeResult = close(&info);
    result = RESULT_OK;
    if (closeResult != RESULT_OK) {
        result = closeResult;
    }
    return result;
}

// 800D340C
int readAll() {
    int result = readFile(lbl_8074B010, 0, dSaveData_c::getRaw2(), lbl_80750AD0);
    if (result != RESULT_OK) {
        return result;
    }
    return readFile(dBanner_c::sFileName, 0, sBannerBuffer, dBanner_c::sFileSize);
}

// 800D3464
int deleteAllFiles() {
    int result;
    if (getType(dBanner_c::sFileName) == RESULT_OK) {
        result = deleteFile(dBanner_c::sFileName);
        if (result != RESULT_OK) {
            return result;
        }
    }
    result = deleteFile(lbl_8074B010);
    if (result != RESULT_OK) {
        return result;
    }
    sLoadErrorMessage = 0;
    return RESULT_OK;
}

// 800D34C4
int getType(const char *path) {
    u8 type;
    s32 result = NANDGetType(path, &type);
    log("NANDGetType", result);
    if (result == NAND_RESULT_OK) {
        return RESULT_OK;
    } else if (result == NAND_RESULT_NOEXISTS) {
        return RESULT_NOEXISTS;
    } else {
        return RESULT_ERROR;
    }
}

// 800D3524
int check(u32 blocks, u32 files, u32 *answer) {
    s32 result = NANDCheck(blocks, files, answer);
    log("NANDCheck", result);
    if (result == NAND_RESULT_OK) {
        return RESULT_OK;
    }
    return RESULT_ERROR;
}

// 800D3570
void log(const char *func, s32 result) {}

// 800D3574
int fn_800D3574() {
    return 0;
}

// 800D357C
int fn_800D357C() {
    return 0;
}

// 800D3584
dScene::dSceneChange_c *getSceneChange() {
    return &dScene::gSceneChange;
}

} // namespace dNand
