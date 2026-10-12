#pragma once

// The voice chat mark (DOL TU d_obj_voice.cpp, .text 801B700C..801B76BC; RTTI "dObjVoice_c"): the
// "Layout/chat_voice_mark.arc" layout shown while voice chat can be used, and the voice packets
// (NET_PACKET_VOICE, 1 byte) that switch a member's voice chat on or off and ask the members for their
// state. Member and function names are inferred.

#include <types.h>
#include <game/framework/f_base.hpp>
#include <game/game/d_m2d.hpp>

// The voice packet byte. ON/OFF switch the sender's voice chat; QUERY asks for a reply (READY, NOT_READY or
// NONE), which is kept per member (dObjVoice_c::getStatus). A member's status is QUERY while its reply is
// pending. Plain ints: the reply's (cond ? READY : NOT_READY) needs int operands to match.
#define VOICE_MSG_ON 0
#define VOICE_MSG_OFF 1
#define VOICE_MSG_READY 2
#define VOICE_MSG_NOT_READY 3
#define VOICE_MSG_NONE 4 // no reply needed (this console, a missing member, voice chat off)
#define VOICE_MSG_QUERY 5

class dObjVoice_c : public fBase_c {
public:
    enum FLAG_e {
        FLAG_SHOW = 1,
        FLAG_ANM_END = 2,
    };

    virtual int create();   // 801B7088
    virtual int doDelete(); // 801B7134
    virtual int execute();  // 801B7164
    virtual int draw();     // 801B71E4
    virtual ~dObjVoice_c() {} // 801B7634

    void initAnm(); // 801B7218
    void playAnm(); // 801B7274

    static BOOL isActive();          // 801B72C4: voice chat can be used now
    static void queryStatus();       // 801B7350: asks every other member (VOICE_MSG_QUERY)
    static BOOL isAllReplied();      // 801B7420: no member is still VOICE_MSG_QUERY
    static u8 getStatus(int member); // 801B747C
    static void receive(int member, const void *data); // 801B7488: NET_PACKET_VOICE
    static void sendOff();           // 801B7520: VOICE_MSG_OFF to every member
    static void sendOn();            // 801B752C: VOICE_MSG_ON to every member
    static void setAnmStart();       // 801B7538: the mark animation 0 ("..._tbBtn_Strt")
    static void setAnmFinish();      // 801B7544: the mark animation 1 ("..._tbBtn_Fnsh")
    static void setVoice(int member, int msg); // 801B7550: VOICE_MSG_ON connects the member's voice, else disconnects
    static void send(u8 msg, int member);       // 801B75E8
    static void sendReply(u8 msg, int member);  // 801B7630

    /* 0x064 */ m2d::ResAccLoader_c mResAcc;
    /* 0x138 */ m2d::SimpleEx_c mLyt;
    /* 0x1D0 */ m2d::AnmEx_c mAnm;
    /* 0x20C */ u16 mFlags;
}; // size 0x210
