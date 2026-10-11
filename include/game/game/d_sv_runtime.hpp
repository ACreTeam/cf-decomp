#pragma once

#include <types.h>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_reset.hpp>
#include <cstring>
#include <game/game/d_net.hpp>

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
int fn_800D2A4C();
int fn_800D2404();
int fn_800D240C();
int fn_800D241C();
int fn_8017B18C();
int fn_8017B190();
extern const int lbl_80750AD0;
extern const int lbl_80750330;
extern unsigned char lbl_8074E7F8[8];
extern const char *lbl_8074B010;
dSceneChange_c *fn_801BB7B8(); // 801BB7B8: &gSceneChange
int fn_801782EC();
int fn_80178374();
void fn_8010DDE0();
void fn_8010DED0();
void fn_8010DCF0(void *);
void fn_800D22F8();
int fn_800D2780();
int fn_800D27E0();
int fn_800D2720();
int fn_800D25D8();
void fn_800A949C(int, int);
int fn_800D34C4(const char *);
int fn_801781D8();
int fn_80178448();
int fn_800D2A44();
int fn_8017BBB8(void *);
void fn_8017C3EC(int, void *, void *, u64 *);
void fn_80136C7C(void *, void *);
void fn_8010DFC0();
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
