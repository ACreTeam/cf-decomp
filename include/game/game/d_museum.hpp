#pragma once

// Museum donation record: who donated each fossil, fish, insect and painting.
// The City Folk counterpart of the GameCube Animal Crossing m_museum_display.c (mMmd_*), plus
// m_museum.c's completion letter (mMsm_SendCompMail). Source: src/dol/game/d_museum.cpp
// (.text 8011947C..8011A6C4). File name and function names are inferred. See notes/d_museum.txt.
// The save keeps one at dSaveData_c::_07352A.

#include <types.h>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_time_stamp.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_script.hpp>

#define MUSEUM_FOSSIL_NUM 58  // dItem::ITEM_IDX_AMBER..
#define MUSEUM_FISH_NUM 64    // dItem::ITEM_IDX_BITTERLING..
#define MUSEUM_INSECT_NUM 64  // dItem::ITEM_IDX_COMMON_BUTTERFLY..
#define MUSEUM_PICTURE_NUM 23 // dItem::ITEM_IDX_DYNAMIC_PAINTING..
#define MUSEUM_ITEM_NUM (MUSEUM_FOSSIL_NUM + MUSEUM_FISH_NUM + MUSEUM_INSECT_NUM + MUSEUM_PICTURE_NUM)

// Donor nibble: 0 not donated, 1..4 player + 1, 5 donated by a player who has since left.
enum {
    MUSEUM_DONOR_NONE,
    MUSEUM_DONOR_PLAYER_1,
    MUSEUM_DONOR_PLAYER_2,
    MUSEUM_DONOR_PLAYER_3,
    MUSEUM_DONOR_PLAYER_4,
    MUSEUM_DONOR_DELETED,
};

// getDonorKind
enum {
    MUSEUM_DONATED_BY_ME,      // the current player
    MUSEUM_DONATED_BY_OTHER,   // another player of this town
    MUSEUM_DONATED_BY_DELETED, // MUSEUM_DONOR_DELETED
    MUSEUM_NOT_DONATED,
};

class dMuseum_c {
public:
    dMuseum_c();                                                // 8011960C
    void clear();                                               // 80119654
    void clearDonor(const dItem::Item &item);                   // 8011980C
    void setDeleted(const dItem::Item &item);                   // 80119858
    u8 *getDonors(const dItem::Item &item, int *idx);           // 801198B4: the item's nibble array, and its index
    int getDonor(const dItem::Item &item);                      // 801199D0: MUSEUM_DONOR_*
    int getDonor(int kind, int idx);                            // 80119A1C
    int getDonorKind(const dItem::Item &item);                  // 80119A94: MUSEUM_DONATED_BY_* / NOT_DONATED
    BOOL isDonated(const dItem::Item &item);                    // 80119AF8
    BOOL isDonated(int kind, int idx);                          // 80119B24
    void donate(const dItem::Item &item);                       // 80119B50: by the current player
    void deletePlayer(int player);                              // 80119BE4: player's donations -> DELETED
    BOOL setDonorWord(dScript::Word_c *word, const dItem::Item &item); // 80119D48
    BOOL getDonorID(dPersonalID_c *out, const dItem::Item &item);      // 80119DB0
    BOOL isComplete();                                          // 80119EE8
    BOOL isInsectComplete();                                    // 80119F68
    BOOL isFossilComplete();                                    // 80119FDC
    BOOL isPictureComplete();                                   // 8011A050
    BOOL isFishComplete();                                      // 8011A0C4
    BOOL hasAnyDonation();                                      // 8011A138
    int getIndex(const dItem::Item &item);                      // 8011A26C: seeker_c::findLike
    BOOL isFossilSetComplete(const dItem::Item &item);          // 8011A29C: every fossil of item's BITM::m_fossil set
    int getProgress();                                          // 8011A3A8: percent donated, at least 1 once any is
    int getKindNum(int kind);                                   // 8011A4F8
    int countDonated(int kind);                                 // 8011A550
    u16 getNthDonated(int kind, int n);                         // 8011A5E8: or dItem::ITEM_ID_NONE
    void sendCompleteMail();                                    // 8011947C: Blathers' letter with the museum model

    /* 0x00 */ u8 mFossil[MUSEUM_FOSSIL_NUM / 2 + 1];   // 4 bits per item
    /* 0x1E */ u8 mFish[MUSEUM_FISH_NUM / 2 + 1];
    /* 0x3F */ u8 mInsect[MUSEUM_INSECT_NUM / 2 + 1];
    /* 0x60 */ u8 mPicture[MUSEUM_PICTURE_NUM / 2 + 1];
    /* 0x6C */ dTimeStamp_c mCompleteTime;              // set when the last item is donated
}; // size 0x74
