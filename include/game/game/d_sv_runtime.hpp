#pragma once

#include <types.h>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_reset.hpp>
#include <cstring>
#include <game/game/d_net.hpp>
#include <game/game/d_nand.hpp>
#include <game/game/d_banner.hpp>
#include <game/game/d_save_data.hpp>

// Narrow ABI boundary for dependencies whose class interfaces are unrecovered.
// Address names deliberately avoid assigning speculative ownership to them.
namespace dSvRuntime {

typedef void (dMsg::Comp_c::*DemoMethod)();

struct IdentityFlags {
    u8 state : 6;
    u8 other : 2;
};

struct TownIdentity {
    u16 id;
    u8 name[0x12];
    u8 extra;

    inline bool isSame(const TownIdentity &town) const {
        return id == town.id && extra == town.extra &&
               memcmp(name, town.name, sizeof(name)) == 0;
    }
};
template <class T> inline T &field(void *object, unsigned int offset) {
    return *reinterpret_cast<T *>(static_cast<char *>(object) + offset);
}
inline bool isDemoMode(void *demo, const DemoMethod &expected) {
    return field<DemoMethod>(demo, 0x70) == expected;
}

extern "C" {
void *fn_8017D8E8();
void fn_80154358(void *process);
int fn_8015436C(void *process, int mode);
extern unsigned char lbl_8074E7F8[8];
dScene::dSceneChange_c *fn_801BB7B8(); // 801BB7B8: &dScene::gSceneChange
void fn_8010DED0();
void fn_800A949C(int, int);
}

inline void releaseMessage(dMsg::Comp_c *controller) {
    controller->mLock = 0;
    controller->mWaitIconSe = 0;
    controller->mWaitIcon = 0;
}
inline void showSaveError(dMsg::Comp_c *controller, u16 code, const char *label) {
    releaseMessage(controller);
    controller->setNextMsgCode(code);
    controller->setNextMsgLabel(label);
}
inline void enableReset() {
    field<u32>(fn_8017D8E8(), 0x1AC) &= ~0x200;
    dReset::Manage_c::GetInstance()->SetResetEnable();
}

} // namespace dSvRuntime
