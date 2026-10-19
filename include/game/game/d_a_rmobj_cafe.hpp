#ifndef D_A_RMOBJ_CAFE_HPP
#define D_A_RMOBJ_CAFE_HPP

#include <game/game/d_a_rmobj.hpp>
#include <game/game/d_effect.hpp>

// The cups of the Roost's guests (REL d_a_rmobj_cafeNP, profile
// A_RMOBJ_CAFE): a cup ("obj_cafe3") and saucer ("obj_cafe4") with steam
// ("af_obj_coffee_npc") for the villager guest and for the special npc guest,
// shown while that guest is there. Class name from RTTI ("daRmobjCafe_c").
class daRmobjCafe_c : public daRmobj_c {
  public:
    virtual int create();   // 0x3D8
    virtual int doDelete(); // 0x660
    virtual int execute();  // 0x4F8
    virtual int draw();     // 0x5F4

    /* 0x15C */ dLevelEffect_c mNpcSteam;
    /* 0x1F0 */ model_c mNpcCup;
    /* 0x39C */ model_c mNpcSaucer;
    /* 0x548 */ dLevelEffect_c mSpNpcSteam;
    /* 0x5DC */ model_c mSpNpcCup;
    /* 0x788 */ model_c mSpNpcSaucer;
}; // size 0x934

namespace dARmobjCafe {
extern u8 g_npcCreated;   // the villager guest (d_a_npc_cafe) is there
extern u8 g_spNpcCreated; // the special npc guest (d_a_npc_sp_cafe_guest)
} // namespace dARmobjCafe

#endif
