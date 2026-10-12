// d_obj_voice.cpp: the voice chat mark and the voice packets. .text 801B700C..801B76BC,
// .data 804FC850..804FC938, .sdata 8074C300..8074C318, .sbss 8074EB70..8074EB78.
// See include/game/game/d_obj_voice.hpp. Names other than the class name are inferred.
#include <game/game/d_obj_voice.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_net_mgr.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>

#define VOICE_PRIORITY 0x89
#define VOICE_ANM_NUM 2

CUSTOM_BASE_PROFILE(OBJ_VOICE, dObjVoice_c, 0xF8, 0x127);

static int l_anmIdx;
static u8 l_status[NET_MEMBER_NONE];

// dSaveOption_c bits of a player (dPrivateData_c::_83E8).
static inline dSaveOption_c *getOption(dPrivateData_c *player) {
    return (dSaveOption_c *)&player->_83E8;
}

// 801B7088
int dObjVoice_c::create() {
    if (!mResAcc.load("Layout/chat_voice_mark.arc")) {
        return NOT_READY;
    }
    mLyt.build(&mResAcc, "chat_voice_mark_00.brlyt");
    mLyt.mPriority = VOICE_PRIORITY;
    if (isActive()) {
        mFlags |= FLAG_SHOW;
    } else {
        mFlags &= ~FLAG_SHOW;
    }
    initAnm();
    mLyt.calc();
    return SUCCEEDED;
}

// 801B7134
int dObjVoice_c::doDelete() {
    bool ok = mResAcc.remove();
    return ok;
}

// 801B7164
int dObjVoice_c::execute() {
    playAnm();
    if (isActive()) {
        mFlags |= FLAG_SHOW;
    } else {
        mFlags &= ~FLAG_SHOW;
    }
    if (mFlags & FLAG_ANM_END) {
        mFlags &= ~FLAG_ANM_END;
        mLyt.calc();
    }
    return SUCCEEDED;
}

// 801B71E4
int dObjVoice_c::draw() {
    if (mFlags & FLAG_SHOW) {
        mLyt.entry();
    }
    return SUCCEEDED;
}

static const char *l_anmNames[VOICE_ANM_NUM] = {
    "chat_voice_mark_00_tbBtn_Strt.brlan",
    "chat_voice_mark_00_tbBtn_Fnsh.brlan",
};

// 801B7218
void dObjVoice_c::initAnm() {
    mAnm.init(&mLyt, &mResAcc, l_anmNames, VOICE_ANM_NUM);
    mAnm.start(1);
    mAnm.setToEnd();
    l_anmIdx = 1;
}

// 801B7274
void dObjVoice_c::playAnm() {
    mAnm.setNext(l_anmIdx);
    if (mAnm.play()) {
        mFlags |= FLAG_ANM_END;
    }
}

// 801B72C4
BOOL dObjVoice_c::isActive() {
    if (isSceneAttr(getCurrentScene(), SCENE_ATTR_DEMO)) {
        return FALSE;
    }
    if (fn_8010D9FC() == TRUE) {
        return FALSE;
    }
    if (!dNet::isSessionActive()) {
        return FALSE;
    }
    if (dNet::getMemberNum() <= 1) {
        return FALSE;
    }
    return dNetMgr_c::sInstance->mInfo.m1A31 != 0;
}

// 801B7350
void dObjVoice_c::queryStatus() {
    l_status[0] = VOICE_MSG_QUERY;
    l_status[1] = VOICE_MSG_QUERY;
    l_status[2] = VOICE_MSG_QUERY;
    l_status[3] = VOICE_MSG_QUERY;
    int myNo = dNet::getMyNo();
    for (int i = 0; i < NET_MEMBER_NONE; i++) {
        if (i == myNo) {
            l_status[i] = VOICE_MSG_NONE;
        } else if (dNet::isMember(i)) {
            if (getOption(dPlayerMgr_c::getNetPlayer(i))->isBit1() == TRUE) {
                l_status[i] = VOICE_MSG_NONE;
            } else {
                sendReply(VOICE_MSG_QUERY, i);
            }
        } else {
            l_status[i] = VOICE_MSG_NONE;
        }
    }
}

// 801B7420
BOOL dObjVoice_c::isAllReplied() {
    for (int i = 0; i < NET_MEMBER_NONE; i++) {
        if (l_status[i] == VOICE_MSG_QUERY) {
            return FALSE;
        }
    }
    return TRUE;
}

// 801B747C
u8 dObjVoice_c::getStatus(int member) {
    return l_status[member];
}

// 801B7488
void dObjVoice_c::receive(int member, const void *data) {
    u8 msg = *(const u8 *)data;
    switch (msg) {
        case VOICE_MSG_ON:
        case VOICE_MSG_OFF:
            setVoice(member, msg);
            break;
        case VOICE_MSG_READY:
        case VOICE_MSG_NOT_READY:
        case VOICE_MSG_NONE:
            l_status[member] = msg;
            break;
        case VOICE_MSG_QUERY:
            int reply;
            if (fn_8010D9FC() == TRUE) {
                reply = VOICE_MSG_NONE;
            } else {
                reply = dNetMgr_c::sInstance->mInfo.m1A31 != 0 ? VOICE_MSG_READY : VOICE_MSG_NOT_READY;
            }
            sendReply(reply, member);
            break;
    }
}

// 801B7520
void dObjVoice_c::sendOff() {
    send(VOICE_MSG_OFF, NET_MEMBER_NONE);
}

// 801B752C
void dObjVoice_c::sendOn() {
    send(VOICE_MSG_ON, NET_MEMBER_NONE);
}

// 801B7538
void dObjVoice_c::setAnmStart() {
    l_anmIdx = 0;
}

// 801B7544
void dObjVoice_c::setAnmFinish() {
    l_anmIdx = 1;
}

// 801B7550
void dObjVoice_c::setVoice(int member, int msg) {
    if (dNet::isMember(member)) {
        dPrivateData_c *player = dPlayerMgr_c::getNetPlayer(member);
        if (player != NULL) {
            if (msg == VOICE_MSG_ON) {
                dNetMgr_c::connect(member);
                getOption(player)->mBit1 = 0;
            } else {
                dNetMgr_c::disconnect(member);
                getOption(player)->mBit1 = 1;
            }
        }
    }
}

// 801B75E8
void dObjVoice_c::send(u8 msg, int member) {
    u8 data = msg;
    dNet::beginPacket();
    dNet::addPacket(&data, sizeof(data));
    dNet::sendPacket(NET_PACKET_VOICE, member);
}

// 801B7630
void dObjVoice_c::sendReply(u8 msg, int member) {
    send(msg, member);
}
