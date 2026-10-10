// The villager's small talk about the town and the "3P_*" remarks about another villager.
// .text 800613BC..80061ADC. See include/game/game/d_npc_talk_town.hpp.
#include <game/game/d_npc_talk_town.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_npc_mng.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_shop_grace.hpp>
#include <game/game/d_theater_common.hpp>
#include <game/game/d_demo.hpp>

typedef dAcNpcNml_c::talk_c talk_c;

static const char l_Town_Rumor[] = "Town_Rumor";   // 8046CD60
static const char l_Town_Always[] = "Town_Always"; // 8046CD6C

// 800613BC
int dAcNpcNml_c::talk_c::msgTownRumor(msgInfo_s *info) {
    dAnimal_c *animal = getAnimal();
    if (animal == NULL) {
        return FALSE;
    }
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL) {
        return FALSE;
    }
    dPersonalID_c *self = &player->mPID;
    u32 kind = 10;
    u16 code = 0;
    dPersonalID_c pid;
    pid.clear();
    if (fn_80116A74(dSaveData_c::getRawExtra(), &pid, &kind, self) && pid.isValid() && kind < 10 &&
        !(pid == *self)) {
        dAnimalMemory_c *memory = animal->findMemory2(&pid);
        switch (kind) {
        case 1:
            code = pid.player.mGender == GENDER_MALE ? (u16)1 : (u16)2;
            if (memory == NULL) {
                code += 2;
            }
            break;
        case 2:
            code = 5;
            if (memory == NULL) {
                code = 6;
            }
            break;
        case 3:
            code = 7;
            if (memory == NULL) {
                code = 8;
            }
            break;
        case 4:
            code = 9;
            if (memory == NULL) {
                code = 10;
            }
            break;
        case 5:
            code = 11;
            if (memory == NULL) {
                code = 12;
            }
            break;
        case 6:
            code = 13;
            if (memory == NULL) {
                code = 15;
            }
            break;
        case 7:
            code = 14;
            if (memory == NULL) {
                code = 16;
            }
            break;
        case 8:
            code = 17;
            if (memory == NULL) {
                code = 18;
            }
            break;
        case 9:
            code = 19;
            if (memory == NULL) {
                code = 20;
            }
            break;
        }
        setLandName(&pid.land, 0);
        setPersonalName(&pid, 1);
        u8 gender = pid.player.getGender();
        u16 unit = fn_800F3F38(gender, animal->mID.getLooks(1));
        if (unit == 0) {
            clearWord(2);
        } else {
            getController()->fn_801A5874(2, unit, "sys_STRING/STR_Unit");
        }
    }
    if (code != 0) {
        setProcSet(&l_talkProcSets[TALK_PROC_TOWN]);
        setLooksMsg(info, l_Town_Rumor, code);
        return TRUE;
    }
    return FALSE;
}

// 80061670
int dAcNpcNml_c::talk_c::msgTownAlways(msgInfo_s *info) {
    setProcSet(&l_talkProcSets[TALK_PROC_TOWN]);
    setLooksMsg(info, l_Town_Always, 0);
    return TRUE;
}

// 800616D0
int dAcNpcNml_c::talk_c::msgTown(msgInfo_s *info) {
    static const u8 l_odds[2] = {20, 80};
    static const msgFunc l_procs[2] = {&talk_c::msgTownRumor, &talk_c::msgTownAlways};
    const u8 *odds = l_odds;
    u32 idx = 2;
    int rnd = cM::rndInt(100);
    for (u32 i = 0; i < 2; i++) {
        if (rnd < odds[i]) {
            idx = i;
            break;
        }
        rnd -= odds[i];
    }
    if (idx >= 2) {
        idx = 1;
    }
    if (!(this->*l_procs[idx])(info)) {
        msgTownAlways(info);
    }
    return TRUE;
}

static const char l_Town_Theater[] = "Town_Theater"; // 8046CD90

// 80061790
int dAcNpcNml_c::talk_c::msgTownTheater(msgInfo_s *info) {
    u16 code = 0x1F;
    if (lbl_80600874.mOpen != 0) {
        if (fn_8016CD8C(&lbl_80600874)) {
            code = 0x1E;
        } else if (lbl_8074EAC8 > 0 && lbl_8074EAC8 < 0x1E) {
            code = lbl_8074EAC8;
        }
    }
    setProcSet(&l_talkProcSets[TALK_PROC_TOWN_THEATER]);
    setLooksMsg(info, l_Town_Theater, code);
    return TRUE;
}

static const char l_Town_Grace[] = "Town_Grace"; // 8046CDA0

// 80061844
int dAcNpcNml_c::talk_c::msgTownGrace(msgInfo_s *info) {
    setProcSet(&l_talkProcSets[TALK_PROC_TOWN_GRACE]);
    u16 code = 1;
    if (dSaveData_c::getTown()->mShops.mShopGrace.isSoldOut()) {
        code = 6;
    } else {
        switch (dSaveData_c::getTown()->mShops.mShopGrace.getSaleStage()) {
        case 1:
            code = 3;
            break;
        case 2:
            code = 4;
            break;
        case 3:
            code = 5;
            break;
        default:
            if (dSaveData_c::getTown()->isFlag(0x17)) {
                code = 2;
            }
            break;
        }
    }
    setLooksMsg(info, l_Town_Grace, code);
    return TRUE;
}

// The "3P_*" labels by personality (getLabelByTable(0x14, ..)).
static const char *l_3pLabels[6] = {"3P_Bo", "3P_Ha", "3P_Ko", "3P_Fu", "3P_Ge", "3P_Ta"};

// 80061934
const char *dAcNpcNml_c::talk_c::get3PLabel(u32 looks) {
    const char *label = NULL;
    if (looks < 6) {
        label = l_3pLabels[looks];
    }
    return label;
}

// 80061958
int dAcNpcNml_c::talk_c::msgTown3P(msgInfo_s *info) {
    u32 looks = 6;
    dAcNpcNml_c *partner = mpNpc != NULL ? static_cast<dAcNpcNml_c *>(static_cast<dAcNpcNml_c *>(mpNpc)->mpChatPartner) : NULL;
    dAnimal_c *other = partner != NULL ? partner->mpAnimal : NULL;
    u32 otherLooks = other != NULL ? other->mID.getLooks(1) : 6;
    if (otherLooks < 6) {
        looks = otherLooks;
    }
    if (looks >= 6) {
        looks = 0;
    }
    if (looks < 6) {
        const char *label = getLabelByTable(0x14, looks, 0, 0);
        if (label != NULL) {
            setProcSet(&l_talkProcSets[TALK_PROC_TOWN_3P]);
            setLooksMsg(info, label, 0);
            if (other != NULL && other->mID.isValid()) {
                setAnmPersonalName(&other->mID, 0);
            }
            dAnimal_c *animal = getAnimal();
            if (animal != NULL && animal->mID.isValid()) {
                setAnmPersonalName(&animal->mID, 1);
            }
            dNpc::msgMemorySecond_c *mem = getRememberedMsg();
            if (mem != NULL) {
                mem->clear();
            }
            return TRUE;
        }
    }
    return FALSE;
}
