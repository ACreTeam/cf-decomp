#pragma once

#include <types.h>
#include <game/game/d_demo.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_reset.hpp>
#include <cstring>

// Narrow ABI boundary for dependencies whose class interfaces are unrecovered.
// Address names deliberately avoid assigning speculative ownership to them.
namespace dSvRuntime {

typedef void (dDemo_c::*DemoMethod)();

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
int fn_800DCF58(); // 800DCF58: this console's net member index (d_net.hpp)
void fn_801A316C(void *demo, int mode);
int fn_8017B18C();
int fn_8017B190();
extern const int lbl_80750AD0;
extern const int lbl_80750330;
extern unsigned char lbl_8074E7F8[8];
extern const char *lbl_8074B010;
dSceneChange_c *fn_801BB7B8(); // 801BB7B8: &gSceneChange
void fn_801A4E44(void *, u16);
void fn_801A4E34(void *, const char *);
bool fn_800DCEDC();
int fn_800DCDFC(int);
bool fn_800DCF30();
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
int fn_800DDB08();
int fn_800DDB58();
int fn_800DDBA8();
int fn_800DD2A4();
void fn_800D3E14();
void fn_800DD4C8();
void fn_800DD588(int, int);
int fn_800DDA7C();
int fn_800DDAA4();
void fn_800DDAE4();
void fn_800DD4A0();
void fn_800DDE4C();
void fn_80168DC0(u16 *, int);
void fn_800A949C(int, int);
int fn_800D34C4(const char *);
int fn_801781D8();
int fn_80178448();
int fn_800D2A44();
u64 fn_800DDDAC(int);
int fn_8017BBB8(void *);
void fn_8017C3EC(int, void *, void *, u64 *);
int fn_800DCF90();
void fn_80136C7C(void *, void *);
void fn_8010DFC0();
}

inline void releaseMessage(void *controller) {
    field<u8>(controller, 0x6C7F) = 0;
    field<u8>(controller, 0x6C7D) = 0;
    field<u8>(controller, 0x6C7C) = 0;
}
inline void showSaveError(void *controller, u16 code, const char *label) {
    releaseMessage(controller);
    fn_801A4E44(controller, code);
    fn_801A4E34(controller, label);
}
inline void enableReset() {
    field<u32>(fn_8017D8E8(), 0x1AC) &= ~0x200;
    dReset::Manage_c::GetInstance()->SetResetEnable();
}

} // namespace dSvRuntime
