#pragma once

// d_insect_fieldNP (REL): only what other modules call so far. Not decompiled
// yet; names unknown.

#include <types.h>
#include <game/mLib/m_vec.hpp>

class dPlayActorMng_c;
class mVec3_c;

// .text 1C74: called by d_fgobj_managerNP when a fallen honeycomb has landed.
// Unless the REL's flag at .bss 0x14 is set (fn_116_16524), it creates insect
// type 0xC (the bees) at pos through fn_116_B28 (with the manager's +0xB4 as
// the last argument). Its first argument is the DOL pointer g_playActorMng
// (8074E838), which d_insect_fieldNP also writes. The owning class and so the
// mangled name are not known until that REL is decompiled.
extern "C" void fn_116_1C74(dPlayActorMng_c *mng, const mVec3_c *pos);

namespace dInsectField {
void addNpcLink(void *insect, void *link);    // .text 6584
void removeNpcLink(void *insect, void *link); // .text 65B0
} // namespace dInsectField

// .text 675C: called by the player's doDelete on the insect it holds a link to,
// before unlinking. Owner / name unknown until d_insect_fieldNP is decompiled.
extern "C" void fn_116_675C(void *insect);

// More of the REL's functions the player REL calls (owners / names unknown).
// .text 1CDC: unless the REL's flag at .bss 0x40 is set, creates insect type 0x38 at pos
// (fn_116_B28); the player's net swing (STATE 0x0D) calls it at the tool position.
extern "C" BOOL fn_116_1CDC(dPlayActorMng_c *mng, const mVec3_c *pos);
// .text 1D34: the REL's .bss 0x40 (an insect, or NULL).
extern "C" void *fn_116_1D34(dPlayActorMng_c *mng);
// .text 5B1C: a float constant of the REL (the net reach is shortened by it).
extern "C" f32 fn_116_5B1C();
// .text F7C: creates an insect held by a player (STATE_SHOW_CATCH), like dFishField_c::addPlayerFish:
// the catalog type (dItem::seeker_c::findLike), a mode, the hand position, the player.
extern "C" void *fn_116_F7C(dPlayActorMng_c *mng, int type, int mode, const mVec3_c *pos, int player);
// .text 66EC: moves the held insect to pos.
extern "C" void fn_116_66EC(void *insect, const mVec3_c *pos);
// .text 6708: scales the insect's held model (dFIMdlMgr_c).
extern "C" void fn_116_6708(void *insect, const mVec3_c *scale);
// .text 6900: resets the insect for being held (caught in the net).
extern "C" void fn_116_6900(void *insect);
// .text 7DC4: the net reaches the insect: 0 = nothing, 1 = missed, 2 = caught.
extern "C" int fn_116_7DC4(void *insect, int player);
// .text 67F8: lets the held insect go (STATE_SHOW_CATCH_AT, the player releases it).
extern "C" void fn_116_67F8(void *insect);

// The field insect actor of d_insect_fieldNP (not decompiled): only what other modules read.
class dInsectFieldActor_c {
public:
    u8 _000[0x1A0];
    /* 0x1A0 */ mVec3_c mPos; // the player looks at it while it flies off (STATE_SHOW_CATCH_AT)
    u8 _1AC[0x2F0 - 0x1AC];
    /* 0x2F0 */ int mType; // dInsectType_e (0xC: the bees)
};

// A node of an insect's link list (someone who refers to the insect; unlinked on
// deletion). Same layout as dANpcOut::insectLink_s (d_a_npc_out.hpp).
struct dInsectLink_c {
    ~dInsectLink_c() {
        if (mpInsect) {
            dInsectField::removeNpcLink(mpInsect, this);
        }
    }
    // Moves the link to insect (none: just unlinks).
    void set(void *insect) {
        if (mpInsect) {
            dInsectField::removeNpcLink(mpInsect, this);
        }
        if (insect) {
            dInsectField::addNpcLink(insect, this);
        }
    }
    /* 0x0 */ void *mpInsect;
    /* 0x4 */ dInsectLink_c *mpNext;
}; // size 0x8
