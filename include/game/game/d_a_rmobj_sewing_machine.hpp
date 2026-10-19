#ifndef D_A_RMOBJ_SEWING_MACHINE_HPP
#define D_A_RMOBJ_SEWING_MACHINE_HPP

// Include order: the RTTI/vtable group order in .data (reverse class
// completion).
#include <game/mLib/m_3d.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_a_rmobj.hpp>
#include <game/game/d_item.hpp>

// The sewing machine of the Able Sisters' shop (REL d_a_rmobj_sewing_machineNP,
// profile A_RMOBJ_SEWING_MACHINE). Class name from RTTI
// ("daRmobjSewingMachine_c"). The machine
// ("obj_tailor_saw") runs its animations; the cloth under the needle
// ("obj_tailor_cloth") is textured with a random piece of clothing
// (KIND_CLOTH), chosen by the scene owner and shared over the net (record
// NET_REC_SEWING_CLOTH).
class daRmobjSewingMachine_c : public daRmobj_c {
  public:
    enum State_e {
        STATE_SEW,   // running at full speed
        STATE_START, // speeding up to full speed, then sewing
        STATE_STOP,  // slowing down, the cloth held at its frame
        STATE_NUM
    };

    typedef BOOL (daRmobjSewingMachine_c::*InitFunc)();
    typedef void (daRmobjSewingMachine_c::*ExecFunc)();

    daRmobjSewingMachine_c();          // 0x8C
    virtual ~daRmobjSewingMachine_c(); // 0x284

    virtual int create();   // 0x3F8
    virtual int doDelete(); // 0x62C
    virtual int execute();  // 0x57C
    virtual int draw();     // 0x5F0

    void initRate();                  // 0x69C
    BOOL changeState(int state);      // 0x6AC
    void executeState();              // 0x718
    BOOL initSew();                   // 0x758
    void executeSew();                // 0x760
    BOOL initStart();                 // 0x7A0
    void executeStart();              // 0x7A8
    BOOL initStop();                  // 0x804
    void executeStop();               // 0x83C
    BOOL loadItem();                  // 0x874
    void slowDown();                  // 0x9CC
    bool speedUp();                   // 0xA18
    BOOL isNeedleMoving();            // 0xA74
    void setRecord(dItem::Item item); // 0xB50
    u16 getRecord();                  // 0xB84

    static daRmobjSewingMachine_c *s_instance;

    /* 0x15C */ int mState;
    /* 0x160 */ f32 mFrame; // the cloth's chr frame
    /* 0x164 */ model_c mClothMdl;
    /* 0x310 */ mVec3_c mNeedlePos;
    /* 0x31C */ u8 _31C[4];
    /* 0x320 */ f32 mRate; // the machine's animation rate
    /* 0x324 */ model_c mMachineMdl;
    /* 0x4D0 */ dItem::resLoader_c mResLoader; // the cloth's texture
    /* 0x540 */ dItem::Item mItem;
}; // size 0x544

#endif
