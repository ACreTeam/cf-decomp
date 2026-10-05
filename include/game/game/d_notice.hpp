#pragma once

// The town notice board kept in the save (dSaveData_c+0x66B62). Source:
// src/dol/game/d_notice.cpp (.text 8011ACB4..8011B400). The file name follows the GameCube
// version; noticeWord_c comes from RTTI, the other names are inferred.
// The posting logic that fills the board is d_npc_notice.

#include <types.h>
#include <game/game/d_date.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_quest_time.hpp>
#include <game/game/d_string.hpp>

// One posted notice. 0x19A bytes, 2-byte aligned.
class dNotice_c {
public:
    dNotice_c();                                                     // 8011ACB4
    ~dNotice_c();                                                    // 8011ACC8

    dNotice_c &operator=(const dNotice_c &other);                    // 8011AD08
    void clear();                                                    // 8011AD3C
    void init(const dTime_c *date);                                  // 8011AD48: clear, mark valid, set the date
    void setSender(const wchar_t *name);                             // 8011AD94
    void setText(const wchar_t *text);                               // 8011AD9C
    wchar_t *getSender();                                            // 8011ADA8
    wchar_t *getText();                                              // 8011ADAC
    BOOL isValid();                                                  // 8011ADB4
    BOOL isRead(int player);                                         // 8011ADCC
    void setRead(int player);                                        // 8011ADF0
    void clearRead(int player);                                      // 8011AE0C
    u8 getDay();                                                     // 8011AE28
    u8 getMonth();                                                   // 8011AE30
    u16 getYear();                                                   // 8011AE38
    BOOL isFlag20();                                                 // 8011AE40
    void setFlag20();                                                // 8011AE58

    /* 0x000 */ wchar_t mSender[PLAYER_NAME_LEN + 1];
    /* 0x012 */ wchar_t mText[0xC1];
    /* 0x194 */ dYMD_c mDate;
    /* 0x198 */ u16 mFlags; // 1: valid, 2 << player: read by that player, 0x20: ?
}; // size 0x19A

// Word used to build notice text (RTTI name). A static instance at lbl_805F1430 is built by
// the sinit and otherwise unused in this TU.
class noticeWord_c : public dString::WordBase_c {
public:
    noticeWord_c();                                                  // 8011AE68
    virtual ~noticeWord_c();                                         // 8011AEAC
    virtual u32 getBufferSize();                                     // 8011AF04
    virtual wchar_t *getBuffer();                                    // 8011AF0C

    /* 0x024 */ wchar_t mBuffer[0xC1];
}; // size 0x1A8

// The board: 15 notices in a ring starting at mHead, plus the time of the last visit.
class dNoticeBoard_c {
public:
    enum { NOTICE_NUM = 15 };

    dNoticeBoard_c();                                                // 8011AF14
    void clear();                                                    // 8011AF70
    void init();                                                     // 8011AFD8: posts the two default notices
    dNotice_c *getNotice(int i);                                     // 8011B044
    dNotice_c *get(int i);                                           // 8011B048
    dNotice_c *at(int i);                                            // 8011B06C
    int getNum();                                                    // 8011B090: number of valid notices
    void add(const dNotice_c *notice);                               // 8011B0F4: drops the oldest when full
    void remove(int i);                                              // 8011B180
    void sort(int count);                                            // 8011B204: orders the newest `count` by date
    void setRead(int i, int player);                                 // 8011B2F0
    dTime_c getTime();                                               // 8011B324
    void setTime(const dTime_c *now);                                // 8011B328
    void clearRead(int player);                                      // 8011B35C

    /* 0x0000 */ dQuestTime_c mTime;
    /* 0x0008 */ dNotice_c mNotices[NOTICE_NUM];
    /* 0x180E */ u8 mHead;
}; // size 0x1810
