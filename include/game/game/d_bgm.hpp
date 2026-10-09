#pragma once

#include <types.h>
#include <game/game/d_date.hpp>
#include <game/mLib/m_vec.hpp>
#include <lib/nw4r/ut/ut_list.h>

// The BGM manager (namespace dBgm). Source: src/dol/game/d_bgm.cpp (.text 80077754..800805BC).
// Class names come from the RTTI strings ("dBgm::StgField_c", "dBgm::VolCtrl_c", ...); the
// manager and its non-polymorphic parts have no RTTI, so their names (and every member name) are
// inferred. See notes/d_bgm.txt.
//
// The manager (l_mgr) keeps a priority queue of BGM requests (Queue_c). Each request is either a
// track (an id) or a "mute" at its priority (id -1): the head of the queue plays, so a mute
// above a track silences it. One stage object (StgXxx_c, chosen by the scene's param C low byte)
// runs per scene and enters / removes its requests. VolCtrl_c lowers the volume while one of its
// states (talk, menu, fishing, ...) "nominates" itself.
namespace dBgm {

class Mgr_c;

// ---------------------------------------------------------------------------------------------
// Stages: one per scene type (the low byte of getSceneParamC(scene) picks it).
// ---------------------------------------------------------------------------------------------

// The stage numbers (the low byte of getSceneParamC; Mgr_c::startStage's table order).
enum Stage_e {
    STAGE_NON,
    STAGE_FIELD,
    STAGE_ROOM,
    STAGE_OFFICE,
    STAGE_CHKP,
    STAGE_MUSEUM,
    STAGE_CAFE,
    STAGE_SHOP,
    STAGE_TAILOR,
    STAGE_TOWN,
    STAGE_BROKER,
    STAGE_BARBER,
    STAGE_FORTUNE,
    STAGE_GRACE,
    STAGE_HAPPY,
    STAGE_THEATER,
    STAGE_AUCTION,
    STAGE_RESET,
    STAGE_TITLE,
    STAGE_SAVE,
    STAGE_LOAD,
    STAGE_PLSEL,
    STAGE_BUS,
    STAGE_NUM
};

// The town's event jingles, part of StgField_c (StgField_c::mEvent).
class FieldEvent_c {
public:
    FieldEvent_c();                                   // 80077774
    ~FieldEvent_c();                                  // 800777A4
    void init();                                      // 800777E4
    void execute();                                   // 800777E8
    void stop();                                      // 8007789C
    void checkEvent(u32 *bgm, int event, u32 id, int prio); // 80077908
    void stopEvent(u32 *bgm);                         // 800779BC
    void checkToyDay();                               // 80077A0C
    void stopToyDay();                                // 80077B24
    void checkCountdown();                            // 80077B70
    void stopCountdown();                             // 80077DE8
    void checkNewYear();                              // 80077E58
    void stopNewYear();                               // 80078004

    /* 0x00 */ u32 mEventBgm[4];   // halloween, fireworks, festivale, harvest festival
    /* 0x10 */ u32 mToyDayBgm;
    /* 0x14 */ u32 mCountdownBgm;
    /* 0x18 */ u8 mCountdownMute;
    /* 0x1C */ u32 mNewYearBgm;
    /* 0x20 */ u32 mNewYearBgm2;
}; // size 0x24

class StgBase_c {
public:
    StgBase_c();                                      // 80078074
    virtual ~StgBase_c();                             // 8007808C
    virtual void start(u32 param);                    // 800780CC
    virtual void execute();                           // 800780D4
    virtual void end();                               // 800780D8
    // Whether the BGM keeps playing into the next scene (its param C).
    virtual bool isKeep(u32 nextParam) const;         // 800780DC
    virtual void reset();                             // 800780E4

    Stage_e getStage() const { return getStage(mParam); } // the low byte selects the stage (Mgr_c::startStage)
    u32 getType() const { return getType(mParam); }
    static Stage_e getStage(u32 param) { return (Stage_e)(param & 0xFF); }
    static u32 getType(u32 param) { return (param >> 16) & 0xF; }

    /* 0x04 */ u32 mParam; // getSceneParamC: stage (bits 0..7), bits 8..9, type (bits 16..19)
}; // size 0x8

class StgNon_c : public StgBase_c {
public:
    StgNon_c();                                       // 800780E8
    virtual ~StgNon_c();                              // 80078124
}; // size 0x8

class StgField_c : public StgBase_c {
public:
    StgField_c();                                     // 8007817C
    virtual ~StgField_c();                            // 80078214
    virtual void start(u32 param);                    // 8007827C
    virtual void execute();                           // 800782CC
    virtual void end();                               // 80078310
    virtual bool isKeep(u32 nextParam) const;         // 8007832C
    virtual void reset();                             // 8007835C

    void play18();                                    // 80078368
    void stop18();                                    // 800783D4
    void fn_80078408();                               // 80078408
    void fn_8007840C();                               // 8007840C
    void mute1C();                                    // 80078460
    void set25();                                     // 800784A8
    void play20();                                    // 800784B4
    void play1C();                                    // 800784D8
    void play28();                                    // 8007856C
    void fn_800785C8();                               // 800785C8
    void mute2C();                                    // 80078610
    void fn_80078658();                               // 80078658
    void mute2D();                                    // 800786A0
    void fn_800786E8();                               // 800786E8
    void set30(int value);                            // 80078730
    void play34();                                    // 80078738
    void mute38();                                    // 800787BC
    void unmute38();                                  // 80078804
    void fn_80078850();                               // 80078850
    void set3A();                                     // 80078898
    void setNoChime(bool noChime);                    // 800788A4
    void stopAll();                                   // 800788AC
    void playHour();                                  // 80078934
    void stopHour();                                  // 80078994
    void checkHour();                                 // 800789E0
    void muteChime();                                 // 80078A40
    void unmuteChime();                               // 80078A94
    void checkChime();                                // 80078AE0
    void checkHome();                                 // 80078BE0
    void setHomeBgm(u32 id);                          // 80078CC0
    void stop18Bgm();                                 // 80078D50
    void stop1C();                                    // 80078D9C
    void stop28();                                    // 80078E38
    void unmute2C();                                  // 80078E84
    void unmute2D();                                  // 80078ED0
    void stop34();                                    // 80078F1C

    /* 0x08 */ int mHour;      // 24: none
    /* 0x0C */ u32 mHourBgm;
    /* 0x10 */ u8 mChimeMute;
    /* 0x11 */ u8 mNoChime;
    /* 0x14 */ u32 mHomeBgm;
    /* 0x18 */ u32 mBgm18;
    /* 0x1C */ u32 mBgm1C;
    /* 0x20 */ u32 mBgm20;
    /* 0x24 */ u8 mMute24;
    /* 0x25 */ u8 m25;
    /* 0x28 */ u32 mBgm28;
    /* 0x2C */ u8 mMute2C;
    /* 0x2D */ u8 mMute2D;
    /* 0x30 */ int m30;
    /* 0x34 */ u32 mBgm34;
    /* 0x38 */ u8 mMute38;
    /* 0x39 */ u8 mMute39;
    /* 0x3A */ u8 m3A;
    /* 0x3C */ FieldEvent_c mEvent;
}; // size 0x60

class StgRoom_c : public StgBase_c {
public:
    StgRoom_c();                                      // 80078FB0
    virtual ~StgRoom_c();                             // 80079024
    virtual void start(u32 param);                    // 8007907C
    virtual void execute();                           // 80079170
    virtual void end();                               // 80079174

    void playMusic(int song);                         // 800791E4
    void stopMusic();                                 // 80079248
    void setMusicPos(const mVec3_c *pos);             // 80079294
    void fn_800792AC(int arg);                        // 800792AC
    void updateMusicPos();                            // 800792B4

    /* 0x08 */ u32 mMusicBgm;
    /* 0x0C */ u32 mHomeBgm;
    /* 0x10 */ u32 mRoomBgm;
    /* 0x14 */ mVec3_c mMusicPos;
    /* 0x20 */ u8 mMusicPosSet;
}; // size 0x24

class StgOffice_c : public StgBase_c {
public:
    StgOffice_c();                                    // 80079304
    virtual ~StgOffice_c();                           // 80079348
    virtual void start(u32 param);                    // 800793A0
    virtual void execute();                           // 80079418
    virtual void end();                               // 8007941C

    /* 0x08 */ u32 mBgm;
}; // size 0xC

class StgChkp_c : public StgBase_c {
public:
    StgChkp_c();                                      // 8007945C
    virtual ~StgChkp_c();                             // 800794A0
    virtual void start(u32 param);                    // 800794F8
    virtual void execute();                           // 80079554
    virtual void end();                               // 80079558
    virtual bool isKeep(u32 nextParam) const;         // 800795A0
    virtual void reset();                             // 800795C0

    void stopBgm();                                   // 800795C4

    /* 0x08 */ u32 mBgm;
}; // size 0xC

class StgMuseum_c : public StgBase_c {
public:
    StgMuseum_c();                                    // 80079610
    virtual ~StgMuseum_c();                           // 80079660
    virtual void start(u32 param);                    // 800796B8
    virtual void execute();                           // 80079750
    virtual void end();                               // 80079754
    virtual bool isKeep(u32 nextParam) const;         // 800797B8
    virtual void reset();                             // 80079800

    void stopBgm();                                   // 80079804
    static BOOL isSameMusic(int type, int otherType); // 8007985C
    void checkRoomChange();                           // 800798D4
    void updateRoomChange();                          // 80079920
    void changeRoom(u32 nextParam) const;             // 800799A8

    /* 0x08 */ u32 mBgm;
    /* 0x0C */ u32 mPrevParam;
    /* 0x10 */ u8 mRoomChange;
}; // size 0x14

class StgCafe_c : public StgBase_c {
public:
    StgCafe_c();                                      // 800799E0
    virtual ~StgCafe_c();                             // 80079A34
    virtual void start(u32 param);                    // 80079A8C
    virtual void execute();                           // 80079AC8
    virtual void end();                               // 80079ACC

    void playKK(int song, int fadeIn, u8 arg);        // 80079B50
    void stopKK();                                    // 80079BD4
    BOOL isKKStopped();                               // 80079C20
    void muteCafe();                                  // 80079C48
    void unmuteCafe();                                // 80079CA0
    BOOL isKKPlaying() const;                         // 80079CEC
    int getKKInfo0();                                 // 80079D44
    int getKKInfo1();                                 // 80079D7C
    int getKKInfo2();                                 // 80079DB4
    int getKKInfo3();                                 // 80079DEC
    int getKKInfo4();                                 // 80079E24
    f32 getKKInfo8();                                 // 80079E5C
    f32 getKKInfoC();                                 // 80079E90
    f32 getKKInfo10();                                // 80079EC4
    int getKKInfo5();                                 // 80079EF8
    int fn_80079F30();                                // 80079F30
    void muteKK();                                    // 80079F70
    void play1A92(int fade);                          // 80079FB8
    void stop1A92(int fadeArg, int fade);             // 8007A03C

    /* 0x08 */ u32 mKKBgm;
    /* 0x0C */ u8 mCafeMute;
    /* 0x10 */ u32 mBgm10;
    /* 0x14 */ u8 mKKMute;
}; // size 0x18

// Nook's store: one stage type per store size; the closing-time music. Window [start, close)
// plays the closing music, [start, chime) mutes the BGM for the closing chime.
struct ShopTime_s {
    /* 0x00 */ int mStartH, mStartM, mStartS;
    /* 0x0C */ int mCloseH, mCloseM, mCloseS;
    /* 0x18 */ int mChimeH, mChimeM, mChimeS;
}; // size 0x24

class StgShop_c : public StgBase_c {
public:
    StgShop_c();                                      // 8007A0E8
    virtual ~StgShop_c();                             // 8007A13C
    virtual void start(u32 param);                    // 8007A194
    virtual void execute();                           // 8007A244
    virtual void end();                               // 8007A368
    virtual bool isKeep(u32 nextParam) const;         // 8007A384
    virtual void reset();                             // 8007A3BC

    void stopAll();                                   // 8007A3C0
    void playClosing();                               // 8007A41C
    void stopClosing();                               // 8007A454
    void muteBell();                                  // 8007A4A0
    void unmuteBell();                                // 8007A4D0

    /* 0x08 */ u32 mType;
    /* 0x0C */ u32 mBgm;
    /* 0x10 */ u32 mClosingBgm;
    /* 0x14 */ u8 mChimeMute;
    /* 0x15 */ u8 mBellTime;
}; // size 0x18

class StgTailor_c : public StgBase_c {
public:
    StgTailor_c();                                    // 8007A4F8
    virtual ~StgTailor_c();                           // 8007A534
    virtual void start(u32 param);                    // 8007A58C
    virtual void execute();                           // 8007A5C8
    virtual void end();                               // 8007A5CC
}; // size 0x8

class StgTown_c : public StgBase_c {
public:
    StgTown_c();                                      // 8007A5DC
    virtual ~StgTown_c();                             // 8007A654
    virtual void start(u32 param);                    // 8007A6AC
    virtual void execute();                           // 8007A6B0
    virtual void end();                               // 8007A6F4

    void play1AB7();                                  // 8007A744
    void setMusicPos(const mVec3_c *pos);             // 8007A778
    void fn_8007A790();                               // 8007A790
    void fn_8007A7DC();                               // 8007A7DC
    void setNoChime(bool noChime);                    // 8007A7F4
    void playTown(u32 id);                            // 8007A7FC
    void stopTown();                                  // 8007A840
    void checkHour();                                 // 8007A88C
    void muteChime();                                 // 8007A91C
    void unmuteChime();                               // 8007A970
    void checkChime();                                // 8007A9BC
    void playBgm20();                                 // 8007AA78
    void stopBgm20();                                 // 8007AAD4
    void checkBgm20();                                // 8007AB20
    void updateMusicPos();                            // 8007AB84

    /* 0x08 */ u32 mBgm;
    /* 0x0C */ u8 mChimeMute;
    /* 0x0D */ u8 mNoChime;
    /* 0x10 */ mVec3_c mMusicPos;
    /* 0x1C */ u8 mMusicPosSet;
    /* 0x20 */ u32 mBgm20;
}; // size 0x24

class StgBroker_c : public StgBase_c {
public:
    StgBroker_c();                                    // 8007ABD4
    virtual ~StgBroker_c();                           // 8007AC10
    virtual void start(u32 param);                    // 8007AC68
    virtual void execute();                           // 8007ACA4
    virtual void end();                               // 8007ACA8
}; // size 0x8

class StgBarber_c : public StgBase_c {
public:
    StgBarber_c();                                    // 8007ACB8
    virtual ~StgBarber_c();                           // 8007ACFC
    virtual void start(u32 param);                    // 8007AD54
    virtual void execute();                           // 8007AD90
    virtual void end();                               // 8007AD94

    void mute();                                      // 8007ADD4
    void fn_8007ADF4();                               // 8007ADF4
    void unmute();                                    // 8007AE3C

    /* 0x08 */ u8 mMute;
}; // size 0xC

class StgFortune_c : public StgBase_c {
public:
    StgFortune_c();                                   // 8007AE88
    virtual ~StgFortune_c();                          // 8007AECC
    virtual void start(u32 param);                    // 8007AF24
    virtual void execute();                           // 8007AF60
    virtual void end();                               // 8007AF64

    void mute();                                      // 8007AFA4
    void unmute2();                                   // 8007AFC4
    void unmute();                                    // 8007AFC8

    /* 0x08 */ u8 mMute;
}; // size 0xC

class StgGrace_c : public StgBase_c {
public:
    StgGrace_c();                                     // 8007B014
    virtual ~StgGrace_c();                            // 8007B050
    virtual void start(u32 param);                    // 8007B0A8
    virtual void execute();                           // 8007B0E4
    virtual void end();                               // 8007B0E8
}; // size 0x8

class StgHappy_c : public StgBase_c {
public:
    StgHappy_c();                                     // 8007B0F8
    virtual ~StgHappy_c();                            // 8007B134
    virtual void start(u32 param);                    // 8007B18C
    virtual void execute();                           // 8007B1C8
    virtual void end();                               // 8007B1CC
}; // size 0x8

class StgTheater_c : public StgBase_c {
public:
    StgTheater_c();                                   // 8007B1DC
    virtual ~StgTheater_c();                          // 8007B234
    virtual void start(u32 param);                    // 8007B28C
    virtual void execute();                           // 8007B2E8
    virtual void end();                               // 8007B2EC
    virtual bool isKeep(u32 nextParam) const;         // 8007B398
    virtual void reset();                             // 8007B404

    void mute();                                      // 8007B408
    void unmute();                                    // 8007B450
    void playShow(int type);                          // 8007B49C
    void stopShow();                                  // 8007B4DC
    void playShowEnd();                               // 8007B51C
    void playEncore();                                // 8007B55C
    void playEncoreEnd();                             // 8007B5C4
    void checkShowEnd();                              // 8007B62C
    void stopBgm();                                   // 8007B6A4

    /* 0x08 */ u32 mBgm;
    /* 0x0C */ int mShowType;
    /* 0x10 */ u32 mShowBgm;
    /* 0x14 */ u32 mShowEndBgm;
    /* 0x18 */ u8 mMute;
}; // size 0x1C

class StgAuction_c : public StgBase_c {
public:
    StgAuction_c();                                   // 8007B6F0
    virtual ~StgAuction_c();                          // 8007B72C
    virtual void start(u32 param);                    // 8007B784
    virtual void execute();                           // 8007B7C0
    virtual void end();                               // 8007B7C4
}; // size 0x8

class StgReset_c : public StgBase_c {
public:
    StgReset_c();                                     // 8007B7D4
    virtual ~StgReset_c();                            // 8007B818
    virtual void start(u32 param);                    // 8007B870
    virtual void execute();                           // 8007B874
    virtual void end();                               // 8007B878

    void play();                                      // 8007B87C
    void stop2();                                     // 8007B8A0
    void stop();                                      // 8007B8A4

    /* 0x08 */ u32 mBgm;
}; // size 0xC

class StgTitle_c : public StgBase_c {
public:
    StgTitle_c();                                     // 8007B8F0
    virtual ~StgTitle_c();                            // 8007B93C
    virtual void start(u32 param);                    // 8007B994
    virtual void execute();                           // 8007B998
    virtual void end();                               // 8007B99C

    void requestPlay();                               // 8007B9A0
    void stop();                                      // 8007B9AC
    void mute();                                      // 8007BA00
    void unmute();                                    // 8007BA20
    void updatePlay();                                // 8007BA6C
    BOOL isEnd() const;                               // 8007BB14

    /* 0x08 */ u32 mBgm;
    /* 0x0C */ u8 mPlayReq;
    /* 0x0D */ u8 mMute;
}; // size 0x10

class StgSave_c : public StgBase_c {
public:
    StgSave_c();                                      // 8007BB78
    virtual ~StgSave_c();                             // 8007BBB4
    virtual void start(u32 param);                    // 8007BC0C
    virtual void execute();                           // 8007BC10
    virtual void end();                               // 8007BC14
}; // size 0x8

class StgLoad_c : public StgBase_c {
public:
    StgLoad_c();                                      // 8007BC18
    virtual ~StgLoad_c();                             // 8007BC54
    virtual void start(u32 param);                    // 8007BCAC
    virtual void execute();                           // 8007BCB0
    virtual void end();                               // 8007BCB4
}; // size 0x8

class StgPlSel_c : public StgBase_c {
public:
    StgPlSel_c();                                     // 8007BCB8
    virtual ~StgPlSel_c();                            // 8007BCFC
    virtual void start(u32 param);                    // 8007BD54
    virtual void execute();                           // 8007BD90
    virtual void end();                               // 8007BD94

    void unmute();                                    // 8007BDD4

    /* 0x08 */ u8 mMute;
}; // size 0xC

class StgBus_c : public StgBase_c {
public:
    StgBus_c();                                       // 8007BE20
    virtual ~StgBus_c();                              // 8007BE68
    virtual void start(u32 param);                    // 8007BEC0
    virtual void execute();                           // 8007BF14
    virtual void end();                               // 8007BF38

    void setMute(BOOL on);                            // 8007BF80
    void fn_8007BF90(int arg);                        // 8007BF90
    void mute();                                      // 8007BF98
    void unmute();                                    // 8007BFE0

    /* 0x08 */ bool mMute;
    /* 0x09 */ bool mMuteReq;
}; // size 0xC

// ---------------------------------------------------------------------------------------------
// The manager's parts.
// ---------------------------------------------------------------------------------------------

// The current and the previous frame's time.
class Time_c {
public:
    Time_c();                                         // 8007C02C
    ~Time_c();                                        // 8007C100
    void init();                                      // 8007C140
    void update();                                    // 8007C210
    BOOL isDateTime(int month, u32 day, int hour, u32 min) const; // 8007C2DC
    BOOL isDate(int month, u32 day) const;            // 8007C31C
    // Whether min:sec is in [min0:sec0, min1:sec1) (wrapping around the hour).
    BOOL isMinSec(int min0, int sec0, int min1, int sec1) const; // 8007C344
    // Whether the time of day is in [h0:m0:s0, h1:m1:s1) (wrapping around midnight).
    BOOL isTime(int h0, int m0, int s0, int h1, int m1, int s1) const; // 8007C3A0
    BOOL isSecChanged() const;                        // 8007C42C
    BOOL isHourChanged() const;                       // 8007C448
    BOOL isYearChanged() const;                       // 8007C464
    void print();                                     // 8007C480

    /* 0x00 */ dTime_c mNow;
    /* 0x28 */ dTime_c mPrev;
}; // size 0x50

// Scene state: a small state machine over the fader and the scene (entering the town from a
// gate or a building), plus flags the stages set.
class State_c {
public:
    enum Flag_e {
        FLAG_ENTER = 2,
        FLAG_4 = 4,
        FLAG_8 = 8,
        FLAG_10 = 0x10,
    };

    State_c();                                        // 8007C484
    ~State_c();                                       // 8007C49C
    void init();                                      // 8007C4DC
    void start();                                     // 8007C4F4
    void execute();                                   // 8007C4F8
    void executeState();                              // 8007C52C
    void stateWaitFader();                            // 8007C564
    void stateCheckScene();                           // 8007C5B8
    void stateWaitFader2();                           // 8007C650
    void stateWait();                                 // 8007C6AC
    void stateCheckLeave();                           // 8007C6FC
    void setMode();                                   // 8007C740
    void checkChime();                                // 8007C764

    /* 0x00 */ u32 mFlags;
    /* 0x04 */ u32 mMode;   // bits 8..9 of the current stage's param
    /* 0x08 */ int mState;
    /* 0x0C */ int mTimer;
}; // size 0x10

// Scene changes: mutes the BGM (priority 8) until the next scene is up, unless the stage keeps
// it playing.
class Change_c {
public:
    Change_c();                                       // 8007C824
    ~Change_c();                                      // 8007C838
    void init();                                      // 8007C878
    void execute();                                   // 8007C88C
    void start(int fade);                             // 8007C94C
    void end();                                       // 8007CA2C

    /* 0x00 */ int mState;
    /* 0x04 */ int mTimer;
    /* 0x08 */ u8 mKeep;
}; // size 0xC

class Silence_c {
public:
    Silence_c();                                      // 8007CA4C
    ~Silence_c();                                     // 8007CA58
    void init();                                      // 8007CA98
    void execute();                                   // 8007CAA4

    /* 0x00 */ u8 mOn;
}; // size 0x1

// Special event BGM (fishing tourney, bug-off, ...).
class Event_c {
public:
    Event_c();                                        // 8007CB2C
    ~Event_c();                                       // 8007CB70
    void fn_8007CBB0();                               // 8007CBB0
    void stopAll();                                   // 8007CBB4
    void init();                                      // 8007CC00
    void execute();                                   // 8007CC44
    void mute0();                                     // 8007CC48
    void play0(int fade);                             // 8007CC90
    void stop0(int fadeArg, int fade);                // 8007CD00
    void muteC(int type);                             // 8007CD58
    void playC(int fade);                             // 8007CDA4
    void stopC(int fadeArg, int fade);                // 8007CE20
    void mute18();                                    // 8007CE78
    void play14();                                    // 8007CEC0
    void stop14();                                    // 8007CEE4
    void mute20();                                    // 8007CF6C
    void play1C();                                    // 8007CFFC
    void stop1C();                                    // 8007D020
    void mute28();                                    // 8007D058
    void play24();                                    // 8007D078
    void stop24();                                    // 8007D09C
    void mute29();                                    // 8007D10C
    void unmute29Fade();                              // 8007D12C
    void mute30();                                    // 8007D164
    void play2C();                                    // 8007D184
    void stop2CFade();                                // 8007D1A8
    void stopBgm0();                                  // 8007D1E0
    void stopBgmC();                                  // 8007D250
    void stopBgm14();                                 // 8007D2C0
    void unmute29();                                  // 8007D378
    void stopBgm2C();                                 // 8007D3C4

    /* 0x00 */ u32 mBgm0;
    /* 0x04 */ u8 mMute4;
    /* 0x08 */ int mType;
    /* 0x0C */ u32 mBgmC;
    /* 0x10 */ u8 mMute10;
    /* 0x14 */ u32 mBgm14;
    /* 0x18 */ u8 mMute18;
    /* 0x1C */ u32 mBgm1C;
    /* 0x20 */ u8 mMute20;
    /* 0x24 */ u32 mBgm24;
    /* 0x28 */ u8 mMute28;
    /* 0x29 */ u8 mMute29;
    /* 0x2C */ u32 mBgm2C;
    /* 0x30 */ u8 mMute30;
}; // size 0x34

// The interface to the sound layer: requests made during the frame, sent in execute().
class Player_c {
public:
    Player_c();                                       // 8007D434
    ~Player_c();                                      // 8007D474
    void start(u32 id, int fadeIn, u8 weather);       // 8007D4B4
    void stop(int fade);                              // 8007D4CC
    void setVolume(f32 volume, int frames);           // 8007D4DC
    void setWeather(int weather);                     // 8007D4F0
    void setPos(const mVec3_c *pos);                  // 8007D500
    void init();                                      // 8007D508
    void execute();                                   // 8007D548

    /* 0x00 */ u8 mWeatherReq;
    /* 0x01 */ u8 mStartReq;
    /* 0x02 */ u8 mStopReq;
    /* 0x03 */ u8 mVolumeReq;
    /* 0x04 */ u8 mWeatherSet;
    /* 0x08 */ u32 mId;
    /* 0x0C */ int mFadeIn;
    /* 0x10 */ int mStopFade;
    /* 0x14 */ f32 mVolume;
    /* 0x18 */ int mVolumeFrames;
    /* 0x1C */ int mWeather;
    /* 0x20 */ const mVec3_c *mPos;
}; // size 0x24

// One BGM request. mPrio REQ_FREE marks a free slot.
class Request_c {
public:
    enum { REQ_FREE = 0x29 };

    Request_c();                                      // 8007D650
    ~Request_c();                                     // 8007D698
    void clear();                                     // 8007D6D8
    void set(int prio, u32 id, int fadeOut, int fadeIn, int timer, u8 weather, u8 duck,
             u8 pos);                                     // 8007D70C
    BOOL isTimeUp();                                  // 8007D73C
    void print(const char *msg);                      // 8007D78C

    /* 0x00 */ int mPrio;
    /* 0x04 */ u32 mId;        // -1: a mute
    /* 0x08 */ int mFadeOut;
    /* 0x0C */ int mFadeIn;
    /* 0x10 */ int mTimer;
    /* 0x14 */ u8 mDelete;
    /* 0x15 */ u8 mWeather;
    /* 0x16 */ u8 mDuck;
    /* 0x17 */ u8 mPos;
    /* 0x18 */ nw4r::ut::Link mLink;
}; // size 0x20

class Queue_c {
public:
    Queue_c();                                        // 8007D790
    ~Queue_c();                                       // 8007D7F0
    void entry(int prio, u32 id, int fadeOut, int fadeIn, int timer, u8 weather, u8 duck,
               u8 pos);                                   // 8007D858
    void removePrio(int prio);                        // 8007D974
    void remove(u32 id);                              // 8007D9E0
    void init();                                      // 8007DA28
    void execute();                                   // 8007DAAC
    BOOL isCurrent(u32 id);                           // 8007DAF0
    u32 getCurrentId();                               // 8007DB44
    f32 getCurrentVolume();                           // 8007DB5C
    BOOL isCurrentDuck();                             // 8007DB74
    BOOL isCurrentPos();                              // 8007DB8C
    void print(const char *msg);                      // 8007DBA4
    Request_c *findPrio(int prio);                    // 8007DBA8
    Request_c *findId(u32 id);                        // 8007DC24
    Request_c *findNext(Request_c *req);              // 8007DCA0
    Request_c *alloc();                               // 8007DD24
    void checkStop();                                 // 8007DD68
    void removeDeleted();                             // 8007DE04
    void checkStart();                                // 8007DE8C
    void checkTimers();                               // 8007DF1C

    /* 0x000 */ Request_c *mCurrent;
    /* 0x004 */ Request_c mRequests[16];
    /* 0x204 */ nw4r::ut::List mList;
}; // size 0x210

class Weather_c {
public:
    Weather_c();                                      // 8007DFB0
    ~Weather_c();                                     // 8007DFC0
    int getKind();                                    // 8007E000: 1 clear, 2 rain, 3 snow
    void init();                                      // 8007E058
    void set();                                       // 8007E068
    void update();                                    // 8007E084

    /* 0x00 */ int mWeather;
    /* 0x04 */ int mPrevWeather;
}; // size 0x8

// ---------------------------------------------------------------------------------------------
// Volume control: a state nominates itself with a volume; the first nominee (lowest id) wins.
// ---------------------------------------------------------------------------------------------

class VolState_c {
public:
    VolState_c(int id);                               // 8007E0DC
    virtual ~VolState_c();                            // 8007E130
    virtual void init();                              // 8007E18C
    virtual void execute() = 0;

    void clear();                                     // 8007E170

    /* 0x04 */ int mId;
    /* 0x08 */ f32 mVolume;
    /* 0x0C */ int mFrames;
    /* 0x10 */ u8 mChanged;
    /* 0x11 */ u8 mDelete;
    /* 0x14 */ nw4r::ut::Link mLink;        // VolCtrl_c::mStates
    /* 0x1C */ nw4r::ut::Link mNomineeLink; // VolCtrl_c::mNominees
}; // size 0x24

class VolStateAdjust_c : public VolState_c {
public:
    VolStateAdjust_c();                               // 8007E190
    virtual ~VolStateAdjust_c();                      // 8007E1D8
    virtual void init();                              // 8007E230
    virtual void execute();                           // 8007E264

    /* 0x24 */ u8 mOn;
}; // size 0x28

class VolStateWait_c : public VolState_c {
public:
    VolStateWait_c();                                 // 8007E2B8
    virtual ~VolStateWait_c();                        // 8007E300
    virtual void init();                              // 8007E358
    virtual void execute();                           // 8007E38C

    void start();                                     // 8007E3E8
    void end();                                       // 8007E3F4
    void cancel();                                    // 8007E400

    /* 0x24 */ int mState;
}; // size 0x28

class VolStateMenu_c : public VolState_c {
public:
    VolStateMenu_c();                                 // 8007E428
    virtual ~VolStateMenu_c();                        // 8007E478
    virtual void init();                              // 8007E4D0
    virtual void execute();                           // 8007E50C

    void start(int kind);                             // 8007E59C
    void end();                                       // 8007E5F0

    /* 0x24 */ int mKind;
    /* 0x28 */ int mState;
}; // size 0x2C

class VolStateTalk_c : public VolState_c {
public:
    VolStateTalk_c();                                 // 8007E648
    virtual ~VolStateTalk_c();                        // 8007E694
    virtual void init();                              // 8007E6EC
    virtual void execute();                           // 8007E724

    void start();                                     // 8007E7B0
    void end(u8 keep);                                // 8007E7BC

    /* 0x24 */ int mState;
    /* 0x28 */ u8 mKeep;
}; // size 0x2C

class VolStateCatapult_c : public VolState_c {
public:
    VolStateCatapult_c();                             // 8007E7CC
    virtual ~VolStateCatapult_c();                    // 8007E814
    virtual void init();                              // 8007E86C
    virtual void execute();                           // 8007E8A0

    void start();                                     // 8007E8FC
    void end();                                       // 8007E908

    /* 0x24 */ int mState;
}; // size 0x28

class VolStateFish_c : public VolState_c {
public:
    VolStateFish_c();                                 // 8007E914
    virtual ~VolStateFish_c();                        // 8007E95C
    virtual void init();                              // 8007E9B4
    virtual void execute();                           // 8007E9E8

    void start();                                     // 8007EA44
    void end();                                       // 8007EA50

    /* 0x24 */ int mState;
}; // size 0x28

class VolStateFirework_c : public VolState_c {
public:
    VolStateFirework_c();                             // 8007EA5C
    virtual ~VolStateFirework_c();                    // 8007EAA4
    virtual void init();                              // 8007EAFC
    virtual void execute();                           // 8007EB30

    void start();                                     // 8007EB8C
    void end();                                       // 8007EB98

    /* 0x24 */ int mState;
}; // size 0x28

class VolStateSky_c : public VolState_c {
public:
    VolStateSky_c();                                  // 8007EBA4
    virtual ~VolStateSky_c();                         // 8007EBEC
    virtual void init();                              // 8007EC44
    virtual void execute();                           // 8007EC78

    /* 0x24 */ u8 mOn;
}; // size 0x28

class VolStateBeach_c : public VolState_c {
public:
    VolStateBeach_c();                                // 8007ED30
    virtual ~VolStateBeach_c();                       // 8007ED78
    virtual void init();                              // 8007EDD0
    virtual void execute();                           // 8007EE04

    /* 0x24 */ u8 mOn;
}; // size 0x28

class VolStateFocus_c : public VolState_c {
public:
    VolStateFocus_c();                                // 8007EF20
    virtual ~VolStateFocus_c();                       // 8007EF90
    virtual void init();                              // 8007EFE8
    virtual void execute();                           // 8007F034

    void set(int type, const mVec3_c &pos);           // 8007F198

    /* 0x24 */ mVec3_c mPos;
    /* 0x30 */ int mType;   // 0 / 1: near / far radii; 2: none
    /* 0x34 */ u8 mOn;
}; // size 0x38

class VolStateStage_c : public VolState_c {
public:
    VolStateStage_c();                                // 8007F1B8
    virtual ~VolStateStage_c();                       // 8007F204
    virtual void init();                              // 8007F25C
    virtual void execute();                           // 8007F294

    void start();                                     // 8007F374
    void end();                                       // 8007F380
    void startShow();                                 // 8007F38C
    void endShow();                                   // 8007F398
    void cancelShow();                                // 8007F3A4

    /* 0x24 */ int mState;
    /* 0x28 */ int mShowState;
}; // size 0x2C

class VolCtrl_c {
public:
    VolCtrl_c();                                      // 8007F3C8
    virtual ~VolCtrl_c();                             // 8007F4FC
    void execute();                                   // 8007F5D0
    void init();                                      // 8007F60C
    void nominate(VolState_c *state, f32 volume, int frames); // 8007F6C8
    void denominate(VolState_c *state, int frames);   // 8007F7AC
    void setVolume(VolState_c *state, f32 volume);    // 8007F7D0
    void print(const char *msg);                      // 8007F7F0
    void executeStates();                             // 8007F7F4
    void calcVolume();                                // 8007F864
    void refreshVolume();                             // 8007F900
    void changeVolume();                              // 8007F9D0
    void removeDeleted();                             // 8007FAE4

    /* 0x004 */ VolStateAdjust_c mAdjust;
    /* 0x02C */ VolStateWait_c mWait;
    /* 0x054 */ VolStateMenu_c mMenu;
    /* 0x080 */ VolStateTalk_c mTalk;
    /* 0x0AC */ VolStateCatapult_c mCatapult;
    /* 0x0D4 */ VolStateFish_c mFish;
    /* 0x0FC */ VolStateFirework_c mFirework;
    /* 0x124 */ VolStateSky_c mSky;
    /* 0x14C */ VolStateBeach_c mBeach;
    /* 0x174 */ VolStateFocus_c mFocus;
    /* 0x1AC */ VolStateStage_c mStage;
    /* 0x1D8 */ nw4r::ut::List mStates;
    /* 0x1E4 */ nw4r::ut::List mNominees;
    /* 0x1F0 */ u8 mRefresh;
}; // size 0x1F4

// ---------------------------------------------------------------------------------------------
// The manager (l_mgr, 80582FC4).
// ---------------------------------------------------------------------------------------------

class Mgr_c {
public:
    Mgr_c();                                          // 8007FB6C
    ~Mgr_c();                                         // 8007FC9C
    void play(int prio, u32 id, int fadeOut, u8 duck);    // 8007FE6C
    void playField(u32 id);                           // 8007FEA8
    void playTown(u32 id);                            // 8007FEF0
    void playRoom(u32 id);                            // 8007FF38
    void playKK(u32 id, int fadeIn, u8 duck);         // 8007FF80
    void mute(int prio, int fadeOut, int timer);      // 8007FFC8
    void stop(u32 id);                                // 8008000C
    void unmute(int prio);                            // 80080014
    void fn_8008001C();                               // 8008001C
    void init();                                      // 80080020
    void execute();                                   // 80080098
    void sceneStart();                                // 8008010C
    void sceneEnd();                                  // 8008015C
    void sceneChange(int fade);                       // 80080190
    void startStage();                                // 800801C8
    void endStage();                                  // 80080304
    void fn_80080354();                               // 80080354
    void resetStages();                               // 80080358
    void executeStage();                              // 8008054C

    /* 0x000 */ Time_c mTime;
    /* 0x050 */ Weather_c mWeather;
    /* 0x058 */ State_c mState;
    /* 0x068 */ Change_c mChange;
    /* 0x074 */ Silence_c mSilence;
    /* 0x078 */ Event_c mEvent;
    /* 0x0AC */ VolCtrl_c mVolCtrl;
    /* 0x2A0 */ Queue_c mQueue;
    /* 0x4B0 */ Player_c mPlayer;
    /* 0x4D4 */ StgNon_c mStgNon;
    /* 0x4DC */ StgField_c mStgField;
    /* 0x53C */ StgRoom_c mStgRoom;
    /* 0x560 */ StgOffice_c mStgOffice;
    /* 0x56C */ StgChkp_c mStgChkp;
    /* 0x578 */ StgMuseum_c mStgMuseum;
    /* 0x58C */ StgCafe_c mStgCafe;
    /* 0x5A4 */ StgShop_c mStgShop;
    /* 0x5BC */ StgTailor_c mStgTailor;
    /* 0x5C4 */ StgTown_c mStgTown;
    /* 0x5E8 */ StgBroker_c mStgBroker;
    /* 0x5F0 */ StgBarber_c mStgBarber;
    /* 0x5FC */ StgFortune_c mStgFortune;
    /* 0x608 */ StgGrace_c mStgGrace;
    /* 0x610 */ StgHappy_c mStgHappy;
    /* 0x618 */ StgTheater_c mStgTheater;
    /* 0x634 */ StgAuction_c mStgAuction;
    /* 0x63C */ StgReset_c mStgReset;
    /* 0x648 */ StgTitle_c mStgTitle;
    /* 0x658 */ StgSave_c mStgSave;
    /* 0x660 */ StgLoad_c mStgLoad;
    /* 0x668 */ StgPlSel_c mStgPlSel;
    /* 0x674 */ StgBus_c mStgBus;
    /* 0x680 */ StgBase_c *mStage;
}; // size 0x684

extern Mgr_c l_mgr; // 80582FC4

} // namespace dBgm
