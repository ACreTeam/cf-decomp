#pragma once

// d_activity_log (DOL TU d_activity_log.cpp, .text 800C76C0..800C7BE4, .bss 8059FF90..8059FFA8): tracks the
// player's progress on a jinx (dSaveJinx_c, see d_sv_activity.hpp) heard about another town while visiting it.
// Other systems report the jinx conditions (onCutTree, onCatchFish, ...); once the jinx is fulfilled
// (isDone) its outcome is applied to the save's jinx results (applyResult / applyVisitorDone / applyTravelResult).
// No RTTI; the namespace comes from the file name. All names are inferred.

#include <types.h>
#include <game/game/d_sv_activity.hpp>

namespace dActivityLog {

class Log_c {
public:
    Log_c();                              // 800C76C0
    ~Log_c() {}                           // 800C7BA4
    void set(const dSaveJinx_c *jinx);    // 800C7764: also resets the progress
    void count(int cond, int player, int value); // 800C7890: dJinxCondition_e
    void setTownBits(u16 bits);           // 800C7AF8
    
    const dSaveJinx_c* const getJinx() const { return &mJinx; }

    /* 0x0 */ dSaveJinx_c mJinx;          // the jinx being attempted
    /* 0x4 */ int mCount;                 // JINX_COND_CUT_TREE..PLANT_FLOWERS: count so far
    /* 0x8 */ u8 mDone;                   // the jinx condition is fulfilled
    /* 0xA */ u16 mTownBits;              // backup of the town's dSvActivity_c::mToday.mOutcomes while travelling
}; // size 0xC

void create();                            // 800C7704 (dScene create)
void destroy();                           // 800C7708 (dScene destroy)
void clear();                             // 800C770C
void setJinx(const dSaveJinx_c *jinx);    // 800C7754
const dSaveJinx_c *getJinx();             // 800C7784
u8 isDone();                              // 800C7790
void onHeartLetter(int player);           // 800C77A0
void onCutTree(int player);               // 800C77B8
void onPullWeeds(int player);             // 800C77D0
void onPlantTree(int player);             // 800C77E8
void onPlantFlower(int player);           // 800C7800
void onFindClover(int player);            // 800C7818
void onPlantRedTurnip(int player);        // 800C7830
void onCatchBug(int bug, int player);     // 800C7848: bug index from ITEM_IDX_COMMON_BUTTERFLY
void onCatchFish(int fish, int player);   // 800C7860: fish index from ITEM_IDX_BITTERLING
void onPlantMoneyTree(int player);        // 800C7878
void applyVisitorDone(BOOL done);         // 800C7994: host only (net sync of isDone)
void applyResult();                       // 800C79DC: applies a fulfilled jinx to the town, then clear()
void applyTravelResult(int outcome);      // 800C7A5C: dJinxOutcome_e, JINX_OUTCOME_NUM = none
void backupTownBits();                    // 800C7AC0
u16 getTownBits();                        // 800C7B00
void restoreTownBits();                   // 800C7B10
BOOL isBusy();                            // 800C7B9C: always FALSE (d_s_boot_static)

} // namespace dActivityLog
