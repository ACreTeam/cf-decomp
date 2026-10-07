// dPrcMng_c: the NG-word checker (see include/game/game/d_prc_mng.hpp).
// .text 80104770..80104C64.
#include <game/game/d_prc_mng.hpp>
#include <game/game/d_region.hpp>
#include <game/game/d_save_data.hpp>
#include <lib/egg/core/eggFrmHeap.h>
#include <lib/revolution/OS/OSThread.h>
#include <string.h>

// dHeap (unsplit): ngwordHeap_p, prcWorkHeap_p, prcHeap_p (their heap name strings).
extern EGG::FrmHeap *lbl_8074E428;
extern EGG::FrmHeap *lbl_8074E42C;
extern EGG::FrmHeap *lbl_8074E430;

// Not split yet (C linkage keeps the target names).
extern "C" {
int fn_801F7FC4(void *work, size_t workSize, const void *data, s32 dataSize); // build the matcher
int fn_801F83D0(const u16 **words, int num, u8 *results, int *hits);         // run it
void fn_801F8958();
void fn_8010EEB4(dSaveDistBlock_c *block); // clear the downloaded patterns
int fn_8016CF08(int idx);
void fn_802B8D30(OSThread *thread, int arg);
void fn_802B8D90(OSThread *thread);
}

static OSThread l_thread;
static dPrcMng_c::param_c l_param;
dPrcMng_c l_prcMng; // also read by fn_8010EEB4 (d_save_data: resets the downloaded patterns)
static void *l_stack;
static bool l_running;
static int l_result;

static const char *l_fileNames[] = {"/PRC/J.bin", "/PRC/E.bin", "/PRC/P.bin", "/PRC/K.bin"};

// 80104770
void dPrcMng_c::clear() {
    l_param.mWords = NULL;
    l_param.mNum = 0;
    l_param.mResults = NULL;
    l_param.mHits = NULL;
    l_running = false;
    l_stack = NULL;
    l_result = 0;
}

// 8010479C
void *dPrcMng_c::setupProc(void *arg) {
    EGG::FrmHeap *heap = lbl_8074E42C;
    heap->free(3);
    size_t size = heap->getAllocatableSize(4);
    void *work = heap->alloc(size, 4);
    const void *data = dSaveData_c::getRawExtra()->_08A960.mData;
    s32 dataSize = dSaveData_c::getRawExtra()->_08A960.mSize;
    if (dataSize == 0) {
        data = l_prcMng.mData;
        dataSize = l_prcMng.mSize;
    }
    if (dSaveData_c::getTownRegion() != getRegion()) {
        data = l_prcMng.mData;
        dataSize = l_prcMng.mSize;
    }
    l_result = fn_801F7FC4(work, size, data, dataSize);
    if (l_result == -8) {
        fn_8010EEB4(&dSaveData_c::getExtra()->_08A960);
        l_result = fn_801F7FC4(work, size, l_prcMng.mData, l_prcMng.mSize);
    }
    fn_801F8958();
    return NULL;
}

// 801048D8
BOOL dPrcMng_c::startSetup() {
    return startThread(setupProc, NULL);
}

// 801048E8
void *dPrcMng_c::checkProc(void *arg) {
    param_c *param = (param_c *)arg;
    fn_801F83D0(param->mWords, param->mNum, param->mResults, param->mHits);
    fn_801F8958();
    return NULL;
}

// 80104924
BOOL dPrcMng_c::startCheck(const u16 **words, int num, u8 *results, int *hits) {
    param_c param;
    param.mWords = words;
    param.mNum = num;
    param.mResults = results;
    param.mHits = hits;
    return startThread(checkProc, &param);
}

// 80104960
BOOL dPrcMng_c::isDone() {
    return endThread();
}

// 80104964
u32 dPrcMng_c::getStackSize() {
    return 0xA000;
}

// 80104970
BOOL dPrcMng_c::startThread(void *(*func)(void *), void *arg) {
    if (!OSIsThreadTerminated(&l_thread) || l_stack != NULL) {
        endThread();
        return FALSE;
    }
    l_stack = lbl_8074E430->alloc(0xA000, 4);
    if (arg != NULL) {
        memcpy(&l_param, arg, sizeof(param_c));
    }
    OSCreateThread(&l_thread, func, &l_param, (u8 *)l_stack + 0xA000, 0xA000, 0x12, 1);
    fn_802B8D30(&l_thread, fn_8016CF08(8));
    OSResumeThread(&l_thread);
    l_running = true;
    return TRUE;
}

// 80104A6C
BOOL dPrcMng_c::endThread() {
    if (!l_running) {
        return TRUE;
    }
    if (OSIsThreadTerminated(&l_thread)) {
        if (l_stack != NULL) {
            lbl_8074E430->free(3);
            l_stack = NULL;
            fn_802B8D90(&l_thread);
        }
        l_param.mWords = NULL;
        l_param.mNum = 0;
        l_param.mResults = NULL;
        l_param.mHits = NULL;
        l_running = false;
        return TRUE;
    }
    return FALSE;
}

// 80104B0C
BOOL dPrcMng_c::load() {
    if (l_prcMng.mData != NULL) {
        return TRUE;
    }
    void *data = l_prcMng.mLoader.request(l_fileNames[getRegion()], 0, lbl_8074E428);
    if (data == NULL) {
        return FALSE;
    }
    l_prcMng.mData = data;
    l_prcMng.mSize = l_prcMng.mLoader.getStatus();
    return TRUE;
}

// 80104B90
u32 dPrcMng_c::getWorkHeapSize() {
    return 0x80000;
}

// 80104B98
u32 dPrcMng_c::getNgwordHeapSize() {
    return 0xC800;
}
