#ifndef D_SV_PROC_HPP
#define D_SV_PROC_HPP

#include <types.h>

// Global class name is confirmed by RTTI. Method and field names are inferred.
class dSvProc_c {
  public:
    enum Mode_e {
        MODE_NEW_TOWN,
        MODE_NEW_PLAYER,
        MODE_NEW_VISITOR_TOWN,
        MODE_NEW_VISITOR_PLAYER,
        MODE_LOAD,
        MODE_TITLE,
        MODE_GROWTH,
        MODE_COUNT
    };
    enum { STEP_COUNT = 3 };
    dSvProc_c() { }
    void reset();               // 80154358
    BOOL process(int mode);     // 8015436C
    BOOL createTown();          // 80154424
    BOOL startTown();           // 80154498
    BOOL finishTown();          // 801544FC
    BOOL createPlayer();        // 8015455C
    BOOL startPlayer();         // 80154594
    BOOL finishPlayer();        // 801545CC
    BOOL createVisitor();       // 80154634
    BOOL startVisitor();        // 801546A8
    BOOL finishVisitor();       // 8015470C
    BOOL createVisitorPlayer(); // 8015476C
    BOOL startVisitorPlayer();  // 801547A4
    BOOL finishVisitorPlayer(); // 801547D8
    BOOL prepareLoad();         // 80154840
    BOOL startLoad();           // 80154904
    BOOL finishLoad();          // 8015493C
    BOOL createTitleTown();     // 80154BD0
    BOOL startTitleTown();      // 80154CC4
    BOOL finishTitleTown();     // 80154D2C
    BOOL prepareGrowth();       // 80154D74
    BOOL startGrowth();         // 80154DDC
    BOOL finishGrowth();        // 80154E14
    void loadResources();       // 80154E5C
    void prepareTown();         // 80154EC4
    void updateTown();          // 80154F50
    void initializePlayer();    // 80155034
    void initializeVisitor();   // 801551E8
    void initializeTown();      // 80155214
    virtual ~dSvProc_c() {
    } // 801553A0

    /* 0x04 */ u8 mStep;
    /* 0x05 */ u8 mPreviousStep;
    /* 0x06 */ u8 mWasVisitor;
    /* 0x07 */ u8 _07;
}; // size 0x8

namespace dSvProc {
extern dSvProc_c sProc;
}

#endif
