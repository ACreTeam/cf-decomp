#pragma once

// The night sky's star field (RTTI dStarDraw_c), part of the sky actor (dSky_c + 0xA4).
// Names other than the class's are inferred.

#include <game/game/d_date.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/mLib/m_3d.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_mtx.hpp>
#include <game/mLib/m_vec.hpp>
#include <nw4r/ut.h>

// A player's constellation (16 per town, after the town data at dSaveData_c + 0x72E0A).
struct dStarSign_c {
    /* 0x00 */ dPersonalID_c mOwner;
    /* 0x2C */ u16 mLines[16]; // indices into dStarDraw_c::sLines, 0xFFFF if unused
    /* 0x4C */ u8 _4C[0x6E - 0x4C];
}; // size 0x6E

class dStarDraw_c : public m3d::proc_c {
public:
    // A star of the chart.
    struct pos_c {
        /* 0x0 */ f32 x;
        /* 0x4 */ f32 y;
        /* 0x8 */ u8 mSize;    // 0..2
        /* 0x9 */ u8 mTwinkle; // index into mStars
        /* 0xA */ u8 mAlpha;
        /* 0xB */ u8 mVisible;
    }; // size 0xC

    struct line_c {
        /* 0x0 */ s16 mStar1;
        /* 0x2 */ s16 mStar2;
    }; // size 0x4

    // One of the twinkle cycles the stars share.
    struct star_c {
        star_c() : mPhase(0.0f), mTimer(0), mAngle(0) {}
        ~star_c() {}

        void init(int idx);    // 80165F50
        void calc();           // 80165FBC
        f32 getScale() const;  // 80166070

        /* 0x0 */ f32 mPhase;
        /* 0x4 */ int mTimer;
        /* 0x8 */ s16 mAngle;
    }; // size 0xC

    enum { LINE_NUM = 0x425 }; // lines in sLines, before its terminator

    typedef BOOL (*StarFilter)(int star, const dStarDraw_c *draw);
    typedef BOOL (*LineFilter)(u16 line, const dStarDraw_c *draw);

    dStarDraw_c()
        : mResFile(NULL), m0AC(0.0f), m0B0(0.0f), mMode(0), mCreated(false), mSign(NULL), mSignIdx(-1),
          mSelStar(-1), mSelLine(-1), mWhiteLines(false) {}

    virtual void drawXlu(); // 80164D14

    void setAngle(mAng angle) { mAngle = angle; }

    static BOOL isStarFree(int star, const dStarDraw_c *draw);                         // 80163AA0
    static BOOL isStarSelectable(int star, const dStarDraw_c *draw);                   // 80163C14
    static BOOL isLineFree(u16 line, const dStarDraw_c *draw);                         // 80163D68
    static BOOL isLineSelectable(u16 line, const dStarDraw_c *draw);                   // 80163E78
    static void traceSign(const dStarSign_c *sign, int star, u16 skipLine, u8 *visited); // 80163EFC
    static BOOL isLineRemovable(u16 line, const dStarDraw_c *draw);                    // 80163FD8

    bool create(EGG::Heap *heap);                                                     // 801640DC
    bool unload();                                                                    // 8016418C
    void setup(nw4r::g3d::ResFile file);                                              // 801641F0
    void calc(const mVec3_c *pos, const mAng &angle);                                 // 80164278
    void getStarScreenPos(int star, mVec2_c *out);                                    // 80164414
    int findStar(const mVec2_c *pos, StarFilter filter, f32 radius);                  // 8016456C
    int findLinkedStar(int star, const mVec2_c *pos, u16 *line, LineFilter filter, f32 radius); // 80164648
    int findLine(int star, const mVec2_c *pos, LineFilter filter, f32 radius);        // 80164760
    u16 findSignLine(const mVec2_c *pos, LineFilter filter, u16 *slot, f32 radius);   // 80164934
    int findSign(const mVec2_c *pos, f32 radius);                                     // 80164B30
    void drawSigns();                                                                 // 801650E8
    void drawBackground();                                                            // 80165300
    u8 calcAlpha(const nw4r::math::VEC3 *pos);                                        // 80165700
    bool drawStar(pos_c *star, f32 scale, const nw4r::ut::Color *color);              // 8016579C
    void drawLine(const pos_c *star1, const pos_c *star2, f32 width, const nw4r::ut::Color &color); // 80165A84
    void drawStarLines(int star, const nw4r::ut::Color &color);                       // 80165E18

    static mAng getSkyAngle(const dTime_c *time);                         // 801660B0
    static mAng getSignAngle(const dStarSign_c *sign);                    // 80166218
    static void getSignDate(const dStarSign_c *sign, dTime_c *date);      // 8016636C
    static void getSignTime(const dStarSign_c *sign, dTime_c *time);      // 8016646C
    static bool isSignVisible(const dStarSign_c *sign, const dTime_c *time); // 80166650
    static BOOL isSignLinked(const dStarSign_c *sign1, const dStarSign_c *sign2); // 80166708

    /* 0x008 */ nw4r::g3d::ResFile mResFile;
    /* 0x00C */ mMtx_c mMtx;
    /* 0x03C */ mVec3_c mPos;
    /* 0x048 */ mAng mAngle;
    /* 0x04A */ u8 mAlpha;
    /* 0x04C */ star_c mStars[8];
    /* 0x0AC */ f32 m0AC;
    /* 0x0B0 */ f32 m0B0;
    /* 0x0B4 */ mAllocator_c mAllocator;
    /* 0x0D0 */ dDvd::brresBank_c mRes;
    /* 0x128 */ int mMode;  // 1: no constellations, 2: with a dark background
    /* 0x12C */ bool mCreated;
    /* 0x130 */ dStarSign_c *mSign; // the constellation being edited
    /* 0x134 */ int mSignIdx;
    /* 0x138 */ int mSelStar;
    /* 0x13C */ int mSelLine;
    /* 0x140 */ bool mWhiteLines;

    static pos_c sStars[400];
    static pos_c sBgStars[120];
    static line_c sLines[LINE_NUM + 1];
}; // size 0x144
