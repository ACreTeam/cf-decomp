#pragma once

// The New Year's countdown board (REL d_a_str_cdbbsNP, profile A_STR_CDBBS,
// BUILDING_COUNTDOWN_BOARD): a structure (daSTR_c, RTTI "daStrCount_c") that
// shows the time left until the new year on Dec 31 (six digits and two
// colons), switches its texture animation to the New Year one on Jan 1, flies
// the town flag (the town flag's design on its model) and creates the
// countdown's flag actor (COUNTDOWN_FLAG). Source:
// src/d_a_str_cdbbsNP/d_a_str_cdbbs.cpp. Names are inferred.

#include <game/game/d_a_str.hpp>
#include <game/game/d_item.hpp>

namespace dAStrCdbbs {

// The digits (obj_cdbbs_number in obj_cdnum): hours, minutes and seconds,
// tens first.
enum {
    DIGIT_HOUR_TENS,
    DIGIT_HOUR_ONES,
    DIGIT_MIN_TENS,
    DIGIT_MIN_ONES,
    DIGIT_SEC_TENS,
    DIGIT_SEC_ONES,
    DIGIT_NUM,
};

// The countdown runs on Dec 31 (MONTH_DECEMBER); the board's New Year
// animation (obj_cdbbs_ny) shows until Jan 1 (MONTH_JANUARY) ends.
enum {
    COUNTDOWN_MDAY = 31,
    NEW_YEAR_LAST_MDAY = 1,
};

// The offset of the digits and colons on the board model (with their x).
#define PART_OFFSET_Y 36.0f
#define PART_OFFSET_Z 8.25f

// The colons (obj_cdbbs_colon in obj_cdnum): between the hours and the
// minutes, and between the minutes and the seconds.
enum {
    COLON_NUM = 2,
};

// One digit of the countdown: a model whose texture animation frame is the
// digit.
class number_c {
  public:
    BOOL create(nw4r::g3d::ResFile file, u32 idx);
    BOOL execute();
    BOOL draw(const mVec3_c *pos);
    BOOL remove();
    void update();

    /* 0x00 */ m3d::mdl_c mMdl;
    /* 0x40 */ m3d::anmTexSrt_c mAnm;
    /* 0x6C */ nw4r::g3d::ResFile mResFile;
    /* 0x70 */ u32 mIdx;  // DIGIT_*
    /* 0x74 */ f32 mPosX; // on the board
    /* 0x78 */ u8 mDigit; // the frame of mAnm
}; // size 0x7C

// A colon of the countdown.
class colon_c {
  public:
    BOOL create(nw4r::g3d::ResFile file, u32 idx);
    BOOL execute();
    BOOL draw(const mVec3_c *pos);
    BOOL remove();

    /* 0x00 */ m3d::mdl_c mMdl;
    /* 0x40 */ nw4r::g3d::ResFile mResFile;
    /* 0x44 */ u32 mIdx;
    /* 0x48 */ f32 mPosX; // on the board
}; // size 0x4C

// The time left on Dec 31 until the new year (all 0 on the other days).
void getRemainTime(u32 *hour, u32 *min, u32 *sec);
// The matrix of a digit or colon at offset on the board at pos, on the curved
// ground.
void calcPartMtx(mMtx_c *mtx, const mVec3_c *pos, const mVec3_c *offset);

} // namespace dAStrCdbbs

class daStrCount_c : public daSTR_c {
  public:
    virtual int doDelete();
    virtual int execute();
    virtual void beforeCreateMdl();
    virtual void afterCreateMdl();
    virtual void drawMdl();

    /* 0x498 */ dAStrCdbbs::number_c mNumbers[dAStrCdbbs::DIGIT_NUM];
    /* 0x780 */ dAStrCdbbs::colon_c mColons[dAStrCdbbs::COLON_NUM];
    /* 0x818 */ dItem::resLoader_c mFlagLoader; // the town flag's design
    /* 0x888 */ m3d::anmTexSrt_c mBoardAnm;     // obj_cdbbs_cd / obj_cdbbs_ny
    /* 0x8B4 */ u8 mIsNewYear;                  // mBoardAnm is obj_cdbbs_ny
    /* 0x8B5 */ u8 mHideCount; // no digits (Nook's part-time job)
}; // size 0x8B8
