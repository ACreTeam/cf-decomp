#pragma once

// The post office: delivering held letters, the letters sent to other towns over WiiConnect24, the
// visit (Wi-Fi) sync of the post office, recycle bin and town tune, the reply scoring of letters to
// villagers ("mail check") and the downloaded letter. Source: src/dol/game/d_post_office.cpp
// (.text 8010263C..80104770). No class or namespace name survives; the names are inferred.

#include <types.h>
#include <game/game/d_mail.hpp>
#include <game/game/d_save_data.hpp>

class dPrivateData_c;
class dTime_c;
namespace dScript {
class Word_c;
}

// The letters held at the post office (dSaveExtra_c::_189C38, 0x12108). It is delivered at 9:00
// and 17:00. Its methods are in the unsplit code at 8013F098; the save field stays raw bytes (its
// constructor isn't modelled), so get() casts it.
struct dPostBox_c {
    static dPostBox_c *get() { return (dPostBox_c *)dSaveData_c::getExtra()->_189C38; }

    void set(int idx, dMail_c *mail);           // 8013F120
    void setHeld(dMail_c *mail);                // 8013F134
    int findEmpty();                            // 8013F1A0: -1 when full
    int pack();                                 // 8013F1A8: moves the letters to the front; returns the count
    void clear(int idx);                        // 8013F1B0
    void clearHeld();                           // 8013F1BC
    BOOL checkDelivery(BOOL reschedule);        // 8013F1C8: a delivery time has passed
    void scheduleDelivery();                    // 8013F354: the next 9:00 / 17:00

    /* 0x00000 */ dMail_c mMails[80];
    /* 0x11D00 */ dMail_c mHeld;       // index -1 in the send loop
    /* 0x12090 */ u8 _12090[8];        // last delivery time
    /* 0x12098 */ u8 _12098[8];        // next delivery time
    /* 0x120A0 */ u16 mFlags;          // bit 0: a delivery is scheduled
    /* 0x120A2 */ u8 _120A2[0x66];
}; // size 0x12108

// A WiiConnect24 message to send (fn_80178048): up to 4 attachments, the friend it goes to and the
// player sending it.
struct dPostRequest_c {
    dPostRequest_c() {
        for (int i = 0; i < 4; i++) {
            mData[i] = NULL;
            mSize[i] = 0;
        }
        mFriend = -1;
        mSender = NULL;
    }

    /* 0x00 */ const void *mData[4];  // [2] the present's download data, [3] the letter
    /* 0x10 */ u32 mSize[4];
    /* 0x20 */ int mFriend;           // friend index of the sender
    /* 0x24 */ dPrivateData_c *mSender;
}; // size 0x28

namespace dPostOffice {

// Delivery of the held letters.
int getToPlayerIdx(dMail_c *mail);              // 8010263C: -1 not to a player, -2 other town, -3 no such player
dPrivateData_c *getSender(dMail_c *mail);      // 801026C0: the local player sending a letter to another town
int getFriendIdx(dMail_c *mail);               // 8010278C: the recipient as the sender's friend; < 0 on errors
int getToAnimalIdx(dMail_c *mail);             // 80102830
BOOL deliverToAnimal(dMail_c *mail);           // 80102974
BOOL deliverToPlayer(dMail_c *mail);           // 801029C0
BOOL deliver(dMail_c *mail);                   // 80102A3C: TRUE when the letter can leave the post office
void deliverAll();                             // 80102AA4
BOOL hasMail();                                // 80102B38
BOOL isFull();                                 // 80102B84
BOOL add(dMail_c *mail);                       // 80102BBC
void holdMail(dMail_c *mail);                  // 80102C24: into dPostBox_c::mHeld
void update();                                 // 80102C60
BOOL hasFutureLetter();                        // 80102CE8: the current player has a letter to the future
void setFutureLetterTime(const dTime_c *time); // 80102D28
BOOL checkFull();                              // 80102D64

// Visit sync of the post office (packets 0x2D / 0x2E).
void resetSync();                              // 80102DC4
int getSyncResult();                           // 80102DF4
BOOL isOtherTownMailFull();                    // 80102E58: more than 30 letters held for other towns

// Sending to other towns.
void removeSent();                             // 80102EF0
void findNextSend();                           // 80102F40
void sendNext();                               // 80103084
void startSend();                              // 80103094
BOOL updateSend();                             // 801030D0

void setSyncWait();                            // 80103184
void setSyncState(u8 state);                   // 80103190
u8 getSyncState();                             // 80103198
void onSyncPacket(int player, const u8 *data); // 801031A0
void sendMail(dMail_c *mail);                  // 80103234
void onMailPacket(int player, const void *data); // 80103280
void sendSyncState(u8 state, int player);      // 8010330C
void replySyncState(u8 state, int player);     // 80103354

// Mail check: how a villager replies to a letter.
BOOL isSeparator(wchar_t c);                   // 80103358
BOOL isWord(const wchar_t *str);               // 80103404: 3 non-separators
int countWords(dMail_c *mail);                 // 80103464
void replaceNewlines(wchar_t *str, int len);   // 801034EC
void toHiragana(wchar_t *str, int len);        // 80103518
void toLower(wchar_t *str, int len);           // 80103590
void normalize(wchar_t *str, int len);         // 801035EC
int getLineLength(const wchar_t *str, int max); // 80103658
const wchar_t *nextLine(const wchar_t *str, int max); // 801036A0
int findWord(const wchar_t *word, const wchar_t *list); // 801036E4
int getWordId(const wchar_t *word);            // 80103784: STR_Mailcheck
BOOL bodyContains(dMail_c *mail, const wchar_t *str); // 801038F4
int scoreWords(dMail_c *mail);                 // 8010397C
int scoreNames(dMail_c *mail);                 // 80103A80
int getLengthType(dMail_c *mail);              // 80103BC8
int getReplyType(dMail_c *mail);               // 80103C10
int findSpecial(dMail_c *mail);                // 80103D24: STR_LetterSP

// Visit sync of the recycle bin (packets 0x62 / 0x63) and the town tune (0x67 / 0x68).
void clearRecycleSync();                       // 80103E3C
BOOL isRecycleSyncDone();                      // 80103E58
void sendRecycleBin(int player);               // 80103EB4
void broadcastRecycleBin();                    // 80103F04
void onRecycleBin(int player, const void *data); // 80103FB0
void onRecycleAck(int player);                 // 80104044
void clearMelodySync();                        // 80104054
BOOL isMelodySyncDone();                       // 80104070
void sendMelody(int player);                   // 801040CC
void broadcastMelody();                        // 8010411C
void onMelody(int player, const void *data);   // 801041C8
void onMelodyAck(int player);                  // 8010422C

// The downloaded letter (dSaveExtra_c::mDistMail).
BOOL canGetDistMail();                         // 8010423C
BOOL getDistMail();                            // 801043FC
void getDistMailSender(dScript::Word_c *word); // 801045E8

} // namespace dPostOffice
