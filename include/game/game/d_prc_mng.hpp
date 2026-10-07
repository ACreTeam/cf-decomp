#pragma once

// dPrcMng_c: the NG-word (profanity) checker. Source: src/dol/game/d_prc_mng.cpp
// (.text 80104770..80104C64). The class name is the game's (RTTI); the rest is inferred.
//
// The patterns are /PRC/<J|E|P|K>.bin by region, loaded at boot into dHeap::ngwordHeap_p (load), or
// the downloaded copy in the save (dSaveExtra_c::_08A960) when there is one for this region. The
// matcher (fn_801F7FC4 / fn_801F83D0, unsplit) runs on its own thread with its stack in
// dHeap::prcHeap_p and its work area in dHeap::prcWorkHeap_p:
//   startSetup()  builds the matcher from the patterns (boot, after load)
//   startCheck()  checks up to 50 UTF-16 strings; results[i] = 1 for each one that matches, *hits = count
//   isDone()      polled by the callers: TRUE once the thread has finished (and cleans it up)

#include <types.h>
#include <game/game/d_dvd.hpp>

class dPrcMng_c {
public:
    // The thread's argument block (startCheck's arguments).
    struct param_c {
        /* 0x0 */ const u16 **mWords;
        /* 0x4 */ int mNum;
        /* 0x8 */ u8 *mResults;
        /* 0xC */ int *mHits;
    }; // size 0x10

    dPrcMng_c() {
        mData = NULL;
        mSize = 0;
    }
    virtual ~dPrcMng_c() {}                          // 80104C08 (weak)

    static void clear();                             // 80104770
    static void *setupProc(void *arg);               // 8010479C: thread: builds the matcher
    static BOOL startSetup();                        // 801048D8
    static void *checkProc(void *arg);               // 801048E8: thread: runs the check
    static BOOL startCheck(const u16 **words, int num, u8 *results, int *hits); // 80104924
    static BOOL isDone();                            // 80104960
    static u32 getStackSize();                       // 80104964: dHeap::prcHeap_p's size
    static BOOL startThread(void *(*func)(void *), void *arg); // 80104970
    static BOOL endThread();                         // 80104A6C: TRUE once the thread is gone
    static BOOL load();                              // 80104B0C: /PRC/<region>.bin
    static u32 getWorkHeapSize();                    // 80104B90: dHeap::prcWorkHeap_p's size
    static u32 getNgwordHeapSize();                  // 80104B98: dHeap::ngwordHeap_p's size

    /* 0x00 vtable */
    /* 0x04 */ void *mData;                          // the loaded patterns
    /* 0x08 */ s32 mSize;
    /* 0x0C */ dDvd::loader_c mLoader;
}; // size 0x20

extern dPrcMng_c l_prcMng;                           // 805EBCE8
