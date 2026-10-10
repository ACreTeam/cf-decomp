// Birthdays: the talk procedures for the player's and the npc's birthday. .text 8004902C..800493EC.
// See include/game/game/d_npc_talk_birthday.hpp. The npc's mood (dNpcMood_c) is the next
// TU, d_npc_talk_mood.cpp.
#include <game/game/d_npc_talk_birthday.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_player_mgr.hpp>

typedef dAcNpcNml_c::talk_c talk_c;

// 8046C828
static const char l_Ev_Birthday[] = "Ev_Birthday";

// 8004902C
int dAcNpcNml_c::talk_c::msgPlayerBirthday(msgInfo_s *info) {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    BOOL leapDay = player != NULL ? player->mBirthday.isSame(1, 29) : FALSE;
    u16 code = 1;
    if (leapDay) {
        dTime_c *now = dTime_c::getCurrent();
        if (now->month == 1 && now->mday == 29) {
            code = 0xE;
        } else {
            code = 0xB;
        }
    } else if (mpMemory != NULL && mpMemory->getFriendship() >= 0x40) {
        code = 4;
    }
    setLooksMsg(info, l_Ev_Birthday, code);
    setProcSet(&l_talkProcSets[TALK_PROC_PLAYER_BIRTHDAY]);
    return TRUE;
}

// 80049128
void dAcNpcNml_c::talk_c::endPlayerBirthday(int arg) {
    recordTalk(NULL);
    dNpc::msgMemorySecond_c *mem = getRememberedMsg();
    if (mem != NULL) {
        mem->clear();
    }
    dNpcEntry_c *entry = getEntry();
    if (entry != NULL) {
        entry->mMsg.set(mMessageLabel, mMessageCode, mTopicKind, mTopicGroup, mTopicIdx);
    }
    if (mpMemory != NULL) {
        mpMemory->mTalkCount.inc(0x44);
    }
}

// 800491B8
BOOL dAcNpcNml_c::talk_c::stepPlayerBirthday(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        mItem0.setFromIndex(dItem::ITEM_IDX_BIRTHDAY_CAKE);
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (player != NULL) {
            player->pickUp(&mItem0, TRUE);
        }
        requestItemAct(&mItem0, 1, 0);
        setMsgProc(&talk_c::msgPlayerBirthdayCake);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 8004926C
int dAcNpcNml_c::talk_c::msgPlayerBirthdayCake(msgInfo_s *info) {
    setLooksMsg(info, l_Ev_Birthday, mMessageCode + 1);
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player != NULL) {
        player->mBirthdayHost._08 = -1;
    }
    return TRUE;
}

// 800492B8
int dAcNpcNml_c::talk_c::msgNpcBirthday(msgInfo_s *info) {
    u16 code = 7;
    dNpcEntry_c *entry = getEntry();
    if (entry != NULL && entry->_28C.mBirthdayDone) {
        code = 8;
    }
    setLooksMsg(info, l_Ev_Birthday, code);
    setProcSet(&l_talkProcSets[TALK_PROC_NPC_BIRTHDAY]);
    return TRUE;
}

// 80049344
void dAcNpcNml_c::talk_c::endNpcBirthday(int arg) {
    recordTalk(NULL);
    dNpc::msgMemorySecond_c *mem = getRememberedMsg();
    if (mem != NULL) {
        mem->clear();
    }
    dNpcEntry_c *entry = getEntry();
    if (entry != NULL) {
        entry->mMsg.set(mMessageLabel, mMessageCode, mTopicKind, mTopicGroup, mTopicIdx);
        entry->_28C.mBirthdayDone = 1;
    }
    if (mpMemory != NULL) {
        mpMemory->mTalkCount.inc(0x44);
    }
}
