// .text 8008EDE8..8008FE50, .data 804DFB10..804DFB60, .bss 80587840..80588D10,
// .sdata 80749F20..80749F60, .sbss 8074E310..8074E330.
#include <game/game/d_screenshot.hpp>
#include <lib/egg/core/eggHeap.h>
#include <lib/revolution/OS/OSThread.h>
#include <cstdio>
#include <string.h>

using namespace dScreenshot;

struct dHeapOwner_c {
    /* 0x00 */ u8 _00[0x14];
    /* 0x14 */ EGG::Heap *mHeap;
};
extern "C" {
dHeapOwner_c *fn_800077C4();
u8 *fn_800FADCC();                  // the JPEG
u32 fn_800FADD4();                  // its size
int fn_8016CF08(int idx);
void fn_802B8D30(OSThread *thread, int arg);
void fn_802B8D90(OSThread *thread);

// The SD card library (wrappers return 0 / -1 unless noted).
void fn_8021C86C(u32 size);                                      // init
void fn_8021C990(int arg, void (*insert)(), void (*remove)());    // card callbacks
int fn_8021C4DC(int arg0, int arg1, int arg2, drive_c *drive);     // mount: 0, -2 no card
int fn_8021C6FC(char letter);                                    // unmount
int fn_8021C960(char letter);                                    // lock
int fn_8021CB14(char letter, int arg);                           // unlock: -1, 0, 1
int fn_8021C7A4();                                               // errno
int fn_8021C934(const char *path);                               // mkdir
int fn_8021CABC(const char *path);                               // remove
int fn_8021CAE8(const char *path);                               // rmdir
int fn_8021C810(const char *path, int attr, find_c *find);        // find first
int fn_8021C83C(find_c *find);                                   // find next
void *fn_8021C6D0(const char *path, int mode);                    // open (NULL on error)
int fn_8021C868(const void *data, u32 size, u32 num, void *file); // write: count
int fn_8021C7E4(void *file);                                     // close
}

// Explicit capacities preserve the original format storage and zero-filled tails.
static char l_dirSearchFormat[] = "%s\\%d?????";
static char l_dirFormat[12] = "%s\\%d%s";
static char l_jpegSearchFormat[20] = "%s\\????????.JPG";
static char l_fileFormat[] = "%s\\%s%04d.JPG";
static char l_photoSearchFormat[20] = "%s\\RUU_????.JPG";

static work_c l_work;
static u8 l_buf0[0x140] ALIGN(32);
static u8 l_buf1[0x1000] ALIGN(32);
static drive_c l_drive;
static OSThread l_thread;
static char l_rootPath[10];
static char l_dirPath[21];
static char l_filePath[33];

static BOOL l_mounted;
static BOOL l_locked;
static BOOL l_running;
static int l_result;
static int l_errno;
static BOOL l_inserted;
static BOOL l_busy;
static void *l_stack;

// 8008EDE8: empty; l_inserted is only set by a successful mount().
static void insertCallback() {}

// 8008EDEC
static void removeCallback() {
    l_inserted = FALSE;
}

// 8008EDF8
void dScreenshot::init() {
    fn_8021C86C(0x20000);
    fn_8021C990(0, insertCallback, removeCallback);
    l_work.mBuf0 = l_buf0;
    l_work.mBuf1 = l_buf1;
    l_work.mNum0 = 4;
    l_work.mNum1 = 4;
    l_work.m0C = 1;
    l_work.m10 = 1;
    l_drive.mWork = &l_work;
    l_mounted = FALSE;
    l_locked = FALSE;
    l_running = FALSE;
    l_result = RESULT_BUSY;
    l_errno = 0;
    l_busy = FALSE;
}

// 8008EE94
static int getErrorResult(int err) {
    switch (err) {
    case 5:
        if (!l_inserted) {
            return RESULT_REMOVED;
        }
        return 4;
    case 8:
    case 88:
        if (!l_inserted) {
            return RESULT_REMOVED;
        }
        return 6;
    case 28:
        return 8;
    case 201:
        return 7;
    case 90:
        return RESULT_FULL;
    default:
        return RESULT_ERROR;
    }
}

// 8008EF2C
static int mount() {
    switch (fn_8021C4DC(0, 0, 0, &l_drive)) {
    case 0:
        l_mounted = TRUE;
        if (!(l_drive.mFlags & 0x10)) {
            return RESULT_BAD_CARD;
        }
        l_inserted = TRUE;
        return RESULT_OK;
    case -2:
        return RESULT_NO_CARD;
    case -1:
    default:
        return RESULT_ERROR;
    }
}

// 8008EFC0
static int unmount() {
    switch (fn_8021C6FC(l_drive.mLetter)) {
    case 0:
        l_mounted = FALSE;
        return RESULT_OK;
    default:
        return RESULT_ERROR;
    }
}

// 8008F010
static int lock() {
    switch (fn_8021C960(l_drive.mLetter)) {
    case 0:
        l_locked = TRUE;
        return RESULT_OK;
    default:
        l_errno = fn_8021C7A4();
        return getErrorResult(l_errno);
    }
}

// 8008F068
static int unlock(int arg) {
    switch (fn_8021CB14(l_drive.mLetter, arg)) {
    case 1:
        l_locked = FALSE;
        return RESULT_OK;
    case 0:
        l_locked = FALSE;
        return RESULT_OK;
    default:
        return RESULT_ERROR;
    }
}

// 8008F0D8
static void startThread(OSThreadFunc func, void *arg) {
    l_stack = fn_800077C4()->mHeap->alloc(0x2800, 32);
    OSCreateThread(&l_thread, func, arg, (u8 *)l_stack + 0x2800, 0x2800, 24, OS_THREAD_DETACHED);
    fn_802B8D30(&l_thread, fn_8016CF08(11));
    OSResumeThread(&l_thread);
    l_running = TRUE;
    l_result = RESULT_BUSY;
}

// 8008F188
static BOOL endThread() {
    if (!l_running) {
        return TRUE;
    }
    if (OSIsThreadTerminated(&l_thread)) {
        if (l_stack) {
            EGG::Heap *heap = fn_800077C4()->mHeap;
            heap->free(l_stack);
            l_stack = NULL;
            fn_802B8D90(&l_thread);
        }
        l_running = FALSE;
        return TRUE;
    }
    return FALSE;
}

// 8008F220
BOOL dScreenshot::close() {
    if (!endThread()) {
        return FALSE;
    }
    if (l_locked) {
        unlock(1);
    }
    if (l_mounted) {
        unmount();
    }
    return TRUE;
}

// 8008F278
int dScreenshot::getResult() {
    if (endThread()) {
        if (l_result != RESULT_OK) {
            close();
        }
        int result = l_result;
        if (result == 4 || result == 6) {
            if (!l_inserted) {
                return RESULT_REMOVED;
            }
        }
        return result;
    }
    return RESULT_BUSY;
}

// 8008F2E0
static void *mountProc(void *arg) {
    int result = mount();
    if (result != RESULT_OK) {
        l_result = result;
        return NULL;
    }
    l_result = RESULT_OK;
    return NULL;
}

// 8008F320
void dScreenshot::startMount() {
    startThread(mountProc, NULL);
}

// 8008F330
static void *mountLockProc(void *arg) {
    int result = mount();
    if (result != RESULT_OK) {
        l_result = result;
        return NULL;
    }
    result = lock();
    if (result != RESULT_OK) {
        l_result = result;
        return NULL;
    }
    l_result = RESULT_OK;
    return NULL;
}

// 8008F388
void dScreenshot::startMountLock() {
    startThread(mountLockProc, NULL);
}

static const char *l_dcimName = "\\DCIM";

// 8008F398
static void *makeRootProc(void *arg) {
    memset(l_rootPath, 0, sizeof(l_rootPath));
    sprintf(l_rootPath, "%c:%s", l_drive.mLetter, l_dcimName);
    if (fn_8021C934(l_rootPath) != 0) {
        l_errno = fn_8021C7A4();
        if (l_errno != SD_ERROR_ALREADY_EXISTS) {
            l_result = getErrorResult(l_errno);
            return NULL;
        }
    }
    l_result = RESULT_OK;
    return NULL;
}

// 8008F430
void dScreenshot::startMakeRoot() {
    startThread(makeRootProc, NULL);
}

static const char *l_dirSuffix = "NIN01";

// 8008F440: Creates the folder after the last one.
static BOOL makeDir() {
    find_c find;
    int no = LAST_DIR_NUMBER;
    int ret = -1;
    while (ret != 0) {
        // Directory numbers are shared with other cameras, regardless of suffix.
        sprintf(l_dirPath, l_dirSearchFormat, l_rootPath, no);
        ret = fn_8021C810(l_dirPath, 0x10, &find);
        if (ret == 0) {
            if (no >= LAST_DIR_NUMBER) {
                l_result = RESULT_FULL;
                return FALSE;
            }
            no++;
            break;
        }
        l_errno = fn_8021C7A4();
        if (l_errno != SD_ERROR_NOT_FOUND) {
            l_result = getErrorResult(l_errno);
            return FALSE;
        }
        no--;
        if (no < FIRST_DIR_NUMBER) {
            no = FIRST_DIR_NUMBER;
            break;
        }
    }
    sprintf(l_dirPath, l_dirFormat, l_rootPath, no, l_dirSuffix);
    if (fn_8021C934(l_dirPath) != 0) {
        l_errno = fn_8021C7A4();
        l_result = getErrorResult(l_errno);
        return FALSE;
    }
    return TRUE;
}

// 8008F568: 1: l_dirPath is the last folder, 0: none, -1: error.
static int findDir() {
    find_c find;
    int no = LAST_DIR_NUMBER;
    int ret = -1;
    while (ret != 0) {
        sprintf(l_dirPath, l_dirFormat, l_rootPath, no, l_dirSuffix);
        ret = fn_8021C810(l_dirPath, 0x10, &find);
        if (ret == 0) {
            return 1;
        }
        l_errno = fn_8021C7A4();
        if (l_errno != SD_ERROR_NOT_FOUND) {
            l_result = getErrorResult(l_errno);
            return -1;
        }
        no--;
        if (no < FIRST_DIR_NUMBER) {
            break;
        }
    }
    return 0;
}

#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')

static const char *l_filePrefix = "RUU_";

// 8008F624: 1: l_filePath is the next file in l_dirPath, 0: the folder is full, -1: error.
static int makeFileName() {
    find_c find;
    int next = 1;
    memset(l_filePath, 0, sizeof(l_filePath));
    // Reserve numbers used by any JPEG prefix, including other cameras.
    sprintf(l_filePath, l_jpegSearchFormat, l_dirPath);
    int ret = fn_8021C810(l_filePath, 0x7F, &find);
    while (ret == 0) {
        if (IS_DIGIT(find.mName[4]) && IS_DIGIT(find.mName[5]) &&
            IS_DIGIT(find.mName[6]) && IS_DIGIT(find.mName[7])) {
            int no = (find.mName[4] - '0') * 1000 + (find.mName[5] - '0') * 100 + (find.mName[6] - '0') * 10 +
                     (find.mName[7] - '0');
            if (no >= next) {
                next = no + 1;
            }
        }
        ret = fn_8021C83C(&find);
    }
    l_errno = fn_8021C7A4();
    if (l_errno != SD_ERROR_NOT_FOUND) {
        l_result = getErrorResult(l_errno);
        return -1;
    }
    if (next >= FILE_NUMBER_LIMIT) {
        return 0;
    }
    sprintf(l_filePath, l_fileFormat, l_dirPath, l_filePrefix, next);
    return 1;
}

// 8008F798
static void *makePathProc(void *arg) {
    memset(l_dirPath, 0, sizeof(l_dirPath));
    int ret = findDir();
    if (ret == -1) {
        return NULL;
    }
    if (ret == 1) {
        ret = makeFileName();
        if (ret == -1) {
            return NULL;
        }
        if (ret == 1) {
            l_result = RESULT_OK;
            return NULL;
        }
    }
    if (!makeDir()) {
        return NULL;
    }
    if (makeFileName() == -1) {
        return NULL;
    }
    l_result = RESULT_OK;
    return NULL;
}

// 8008F844
void dScreenshot::startMakePath() {
    startThread(makePathProc, NULL);
}

// 8008F854
static void *writeProc(void *arg) {
    void *file = fn_8021C6D0(l_filePath, 0);
    if (file == NULL) {
        l_errno = fn_8021C7A4();
        l_result = getErrorResult(l_errno);
        return NULL;
    }
    u8 *data = fn_800FADCC();
    if (fn_8021C868(data, fn_800FADD4(), 1, file) == 0) {
        l_errno = fn_8021C7A4();
        l_result = getErrorResult(l_errno);
        fn_8021C7E4(file);
        fn_8021CABC(l_filePath);
        return NULL;
    }
    if (fn_8021C7E4(file) != 0) {
        l_errno = fn_8021C7A4();
        l_result = RESULT_REMOVED;
        return NULL;
    }
    l_result = RESULT_OK;
    return NULL;
}

// 8008F940
void dScreenshot::startWrite() {
    startThread(writeProc, NULL);
}

// 8008F950
static void *checkProc(void *arg) {
    find_c find;
    memset(l_rootPath, 0, sizeof(l_rootPath));
    sprintf(l_rootPath, "%c:%s", l_drive.mLetter, l_dcimName);
    if (fn_8021C810(l_rootPath, 0x10, &find) != 0) {
        l_errno = fn_8021C7A4();
        l_result = l_errno == SD_ERROR_NOT_FOUND ? RESULT_NO_PHOTO : getErrorResult(l_errno);
        return NULL;
    }
    for (int no = FIRST_DIR_NUMBER; no <= LAST_DIR_NUMBER; no++) {
        sprintf(l_dirPath, l_dirFormat, l_rootPath, no, l_dirSuffix);
        if (fn_8021C810(l_dirPath, 0x10, &find) == 0) {
            memset(l_filePath, 0, sizeof(l_filePath));
            sprintf(l_filePath, l_photoSearchFormat, l_dirPath);
            int ret = fn_8021C810(l_filePath, 0x7F, &find);
            while (ret == 0) {
                if (IS_DIGIT(find.mName[4]) && IS_DIGIT(find.mName[5]) &&
                    IS_DIGIT(find.mName[6]) && IS_DIGIT(find.mName[7])) {
                    l_result = RESULT_OK;
                    return NULL;
                }
                ret = fn_8021C83C(&find);
            }
        } else {
            l_errno = fn_8021C7A4();
            if (l_errno != SD_ERROR_NOT_FOUND) {
                l_result = getErrorResult(l_errno);
                return NULL;
            }
        }
    }
    l_result = RESULT_NO_PHOTO;
    return NULL;
}

// 8008FB38
void dScreenshot::startCheck() {
    startThread(checkProc, NULL);
}

// The original dot-entry check examines only the first two characters.
#define IS_DOT_DIR(name) ((name)[0] == '.' && ((name)[1] == '\0' || (name)[1] == '.'))

// 8008FB48
static void *deleteProc(void *arg) {
    find_c find;
    for (int no = FIRST_DIR_NUMBER; no <= LAST_DIR_NUMBER; no++) {
        sprintf(l_dirPath, l_dirFormat, l_rootPath, no, l_dirSuffix);
        if (fn_8021C810(l_dirPath, 0x10, &find) == 0) {
            memset(l_filePath, 0, sizeof(l_filePath));
            sprintf(l_filePath, l_photoSearchFormat, l_dirPath);
            int ret = fn_8021C810(l_filePath, 0x7F, &find);
            while (ret == 0) {
                if (IS_DIGIT(find.mName[4]) && IS_DIGIT(find.mName[5]) &&
                    IS_DIGIT(find.mName[6]) && IS_DIGIT(find.mName[7])) {
                    sprintf(l_filePath, "%s\\%s", l_dirPath, find.mName);
                    if (fn_8021CABC(l_filePath) != 0) {
                        l_errno = fn_8021C7A4();
                        l_result = getErrorResult(l_errno);
                        return NULL;
                    }
                }
                ret = fn_8021C83C(&find);
            }
            // Remove the folder once only . and .. are left.
            sprintf(l_filePath, "%s\\?*", l_dirPath);
            ret = fn_8021C810(l_filePath, 0x7F, &find);
            if (ret == 0) {
                if (IS_DOT_DIR(find.mName)) {
                    ret = fn_8021C83C(&find);
                }
                if (IS_DOT_DIR(find.mName)) {
                    ret = fn_8021C83C(&find);
                }
            }
            if (ret != 0) {
                l_errno = fn_8021C7A4();
                if (l_errno != SD_ERROR_NOT_FOUND) {
                    l_result = getErrorResult(l_errno);
                    return NULL;
                }
                if (fn_8021CAE8(l_dirPath) != 0) {
                    l_errno = fn_8021C7A4();
                    l_result = getErrorResult(l_errno);
                    return NULL;
                }
            }
        } else {
            l_errno = fn_8021C7A4();
            if (l_errno != SD_ERROR_NOT_FOUND) {
                l_result = getErrorResult(l_errno);
                return NULL;
            }
        }
    }
    l_result = RESULT_OK;
    return NULL;
}

// 8008FDB8
void dScreenshot::startDelete() {
    startThread(deleteProc, NULL);
}

// 8008FDC8
BOOL dScreenshot::isIdle() {
    return !l_busy && !l_locked;
}

// 8008FDEC
void dScreenshot::setBusy() {
    l_busy = TRUE;
}

// 8008FDF8
void dScreenshot::clearBusy() {
    l_busy = FALSE;
}

// 8008FE04
BOOL dScreenshot::tryClose() {
    if (isIdle()) {
        return TRUE;
    }
    if (close()) {
        clearBusy();
        return TRUE;
    }
    return FALSE;
}
