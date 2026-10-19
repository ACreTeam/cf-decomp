#ifndef D_A_RMOBJ_BUS_HPP
#define D_A_RMOBJ_BUS_HPP

// Include order: the weak RTTI / vtables in .data follow class completion
// (as d_a_rmobj_sample.hpp).
#include <game/mLib/m_3d.hpp>
#include <game/framework/f_base.hpp>
#include <game/game/d_dvd.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_msg_rcpt.hpp>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_a_rmobj.hpp>
#include <game/game/d_bg.hpp>

// The bus interior (REL d_a_rmobj_busNP, profile A_RMOBJ_BUS): the view out of
// the windows ("obj_bus_bg", a scrolling srt animation with the season's
// textures from a dBG::packBank_c), the wipers ("obj_bus_wiper", running in
// rain or snow) and, on the opening ride (SCENE_DM_BUS_PL_CRT), the tunnel
// ("obj_bus_bg_tunnel"). Kapp'n's opening npc (d_a_npc_sp_player_create)
// drives it through the static functions. Class name from RTTI
// ("daRmobjBus_c").
class daRmobjBus_c : public daRmobj_c {
  public:
    // The steps (l_initSteps / l_executeSteps).
    enum Step_e {
        STEP_NONE,        // the view only
        STEP_TUNNEL,      // in the tunnel (its animation 0 loops)
        STEP_TUNNEL_EXIT, // out of the tunnel ("obj_bus_bg_tunnel1"), lighting
                          // up
        STEP_NUM
    };

    // The tunnel model's animation (index 0 of each kind) inside the tunnel.
    enum Anm_e { ANM_TUNNEL };

    virtual int create();   // 0x410
    virtual int doDelete(); // 0x93C
    virtual int execute();  // 0x688
    virtual int draw();     // 0x8E0

    // Used by Kapp'n (FALSE / 0 without the bus).
    static BOOL startTunnelExit();   // 0x364: setStep(STEP_TUNNEL_EXIT)
    static BOOL isTunnelExitStarted(); // 0x384
    static void reloadSeason();      // 0x3A4: after a season change
    static BOOL isSeasonReloaded();  // 0x3E8

    BOOL setStep(int step); // 0x9BC: runs the step's init; FALSE if it fails
    void executeStep();     // 0xA30

    BOOL initNone();          // 0xA70
    void executeNone();       // 0xA78
    BOOL initTunnel();        // 0xA7C
    void executeTunnel();     // 0xAB4
    BOOL initTunnelExit();    // 0xABC
    void executeTunnelExit(); // 0xAF8

    static daRmobjBus_c *m_instance; // .bss 0x20: set while it exists

    /* 0x15C */ bool mIsPlayerCreate; // the opening ride
    /* 0x15D */ u8 mIsSaveGood;       // dSaveData_c::isGood
    /* 0x15E */ u8 mIsRain;
    /* 0x15F */ u8 mIsSnow;
    /* 0x160 */ model_c mBgModel;
    /* 0x30C */ model_c mWiperModel;
    /* 0x4B8 */ model_c mTunnelModel;
    /* 0x664 */ f32 mBusRatio; // dEnvironment_c::setBusRatio (0 in the tunnel)
    /* 0x668 */ u8 mReloadSeason;      // reload the season's textures
    /* 0x669 */ u8 mTunnelExitStarted; // the tunnel exit animation is playing
    /* 0x66C */ dBG::packBank_c mPackBank;
    /* 0x6C8 */ int mStep; // Step_e
}; // size 0x6CC

#endif
