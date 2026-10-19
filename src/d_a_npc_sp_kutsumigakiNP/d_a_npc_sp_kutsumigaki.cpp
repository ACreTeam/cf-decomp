#include <game/game/d_a_npc_sp_kutsumigaki.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_a_player.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_effect.hpp>
#include <game/game/d_hmn_anm.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_money.hpp>
#include <game/game/d_msg_comp.hpp>
#include <game/game/d_npc_mng.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_save_town.hpp>
#include <game/game/d_scene_check.hpp>
#include <game/game/d_sv_villager_move.hpp>
#include <game/sLib/s_lib.hpp>
#include <game/snd/snd_id.hpp>

#ifdef MUST_MATCH
// Matching-only emission-order workaround (as in d_a_npc_sp_fuko / _jijyo); no
// evidence of an original helper. Queue d_a_npc.hpp's weak section before the
// implicit mVec3_c destructor (setShinePos's l_basePos) joins an earlier
// section. MWCC drops this unused body.
static void orderNpcWeakInlines(dAcNpc_c *npc) {
    npc->dAcNpc_c::getHoldItem();
}
#endif

// Kicks the shoe-shine skunk. Functions are in target .text order.

// He talks to a player in front of the stoop: closer than this in x/z and
// within this yaw of his front.
#define KUTSUMIGAKI_TALK_DIST 60.0f
#define KUTSUMIGAKI_TALK_YAW 0x1100

namespace dANpcSpKutsumigaki {
static inline dPrivateData_c *getPlayer() {
    return dPlayerMgr::Mgr_c::getCurrentPlayer();
}
static inline int getShineCount() {
    return getPlayer()->mShoeShineCount;
}

// .text 0xCC: the item colour of a hair colour.
static int getHairColor(int hairColor) {
    switch (hairColor) {
    case 0:
        return dItem::COLOR_BLACK;
    case 1:
        return dItem::COLOR_BROWN;
    case 2:
        return dItem::COLOR_RED;
    case 3:
        return dItem::COLOR_BLUE;
    case 4:
        return dItem::COLOR_YELLOW;
    case 5:
        return dItem::COLOR_GREEN;
    case 6:
        return dItem::COLOR_PINK;
    case 7:
        return dItem::COLOR_WHITE;
    }
    return dItem::COLOR_NEUTRAL;
}

// The hat leaves the hair visible (caps and hair accessories).
static inline BOOL showsHair(int bone) {
    BOOL shows = FALSE;
    if (bone == dItem::HIDE_BONE_CAP ||
        bone == dItem::HIDE_BONE_HAIR_ACCESSORY || bone == 7) {
        shows = TRUE;
    }
    return shows;
}

// The colours of the outfit (shirt, hat, accessory: colour A and B; the hair
// colour where it shows); COLOR_COUNT: none.
static int l_colors[6] = {
    dItem::COLOR_COUNT, dItem::COLOR_COUNT, dItem::COLOR_COUNT,
    dItem::COLOR_COUNT, dItem::COLOR_COUNT, dItem::COLOR_COUNT,
};

// .text 0x134: the shoe colour that matches the colours of the outfit.
static u8 getColorShoeColor(u8 kind) {
    struct colorShoe_s {
        int mColor;
        u8 mShoe[2]; // the shoe colour, by kind
    };
    struct colorPairShoe_s {
        int mColorA;
        int mColorB;
        u8 mShoe[2];
    };
    static const colorShoe_s l_colorShoes[] = {
        {0, {6, 6}},  {1, {2, 2}},  {2, {9, 9}},  {3, {0, 0}},  {4, {3, 3}},
        {5, {4, 4}},  {6, {5, 5}},  {7, {6, 6}},  {8, {7, 7}},  {9, {6, 6}},
        {10, {1, 1}}, {11, {6, 6}}, {12, {6, 6}}, {14, {8, 8}}, {13, {4, 4}},
    };
    static const colorPairShoe_s l_colorPairShoes[] = {
        {0, 1, {0, 0}},   {0, 2, {1, 1}},   {0, 3, {2, 2}},   {0, 4, {3, 3}},
        {0, 5, {4, 4}},   {0, 6, {5, 5}},   {0, 7, {6, 6}},   {0, 8, {7, 7}},
        {0, 9, {8, 8}},   {0, 10, {9, 9}},  {0, 11, {6, 6}},  {0, 14, {8, 8}},
        {0, 13, {4, 4}},  {0, 12, {6, 6}},  {12, 1, {0, 0}},  {12, 2, {1, 1}},
        {12, 3, {2, 2}},  {12, 4, {3, 3}},  {12, 5, {4, 4}},  {12, 6, {5, 5}},
        {12, 7, {6, 6}},  {12, 8, {7, 7}},  {12, 9, {8, 8}},  {12, 10, {9, 9}},
        {12, 11, {6, 6}}, {12, 14, {8, 8}}, {12, 13, {4, 4}}, {1, 2, {2, 2}},
        {1, 3, {0, 0}},   {1, 4, {4, 4}},   {1, 5, {5, 5}},   {1, 6, {2, 2}},
        {1, 7, {1, 1}},   {1, 8, {7, 7}},   {1, 9, {3, 3}},   {1, 10, {9, 9}},
        {1, 11, {6, 6}},  {1, 14, {8, 8}},  {1, 13, {6, 6}},  {2, 3, {2, 2}},
        {2, 4, {1, 1}},   {2, 5, {3, 3}},   {2, 6, {1, 1}},   {2, 7, {6, 6}},
        {2, 8, {8, 8}},   {2, 9, {2, 2}},   {2, 10, {9, 9}},  {2, 11, {6, 6}},
        {2, 14, {8, 8}},  {2, 13, {4, 4}},  {3, 4, {8, 8}},   {3, 5, {3, 3}},
        {3, 6, {0, 0}},   {3, 7, {6, 6}},   {3, 8, {7, 7}},   {3, 9, {2, 2}},
        {3, 10, {1, 1}},  {3, 11, {0, 0}},  {3, 14, {8, 8}},  {3, 13, {4, 4}},
        {4, 5, {4, 4}},   {4, 6, {2, 2}},   {4, 7, {8, 8}},   {4, 8, {6, 6}},
        {4, 9, {0, 0}},   {4, 10, {5, 5}},  {4, 11, {5, 5}},  {4, 14, {8, 8}},
        {4, 13, {4, 4}},  {5, 6, {1, 1}},   {5, 7, {6, 6}},   {5, 8, {7, 7}},
        {5, 9, {4, 4}},   {5, 10, {9, 9}},  {5, 11, {4, 4}},  {5, 14, {5, 5}},
        {5, 13, {4, 4}},  {6, 7, {6, 6}},   {6, 8, {7, 7}},   {6, 9, {8, 8}},
        {6, 10, {1, 1}},  {6, 11, {6, 6}},  {6, 14, {1, 1}},  {6, 13, {4, 4}},
        {7, 8, {7, 7}},   {7, 9, {6, 6}},   {7, 10, {1, 1}},  {7, 11, {6, 6}},
        {7, 14, {6, 6}},  {7, 13, {6, 6}},  {8, 9, {7, 7}},   {8, 10, {9, 9}},
        {8, 11, {7, 7}},  {8, 14, {7, 7}},  {8, 13, {4, 4}},  {9, 10, {8, 8}},
        {9, 11, {8, 8}},  {9, 14, {2, 2}},  {9, 13, {8, 8}},  {10, 11, {9, 9}},
        {10, 14, {9, 9}}, {10, 13, {9, 9}}, {11, 14, {8, 8}}, {11, 13, {4, 4}},
        {14, 13, {4, 4}},
    };
    static const u8 l_defaultShoe[2] = {4, 1};

    for (int i = 0; i < 6; i++) {
        l_colors[i] = dItem::COLOR_COUNT;
    }
    l_colors[0] = getPlayer()->mEquipment.mShirt.getColorA();
    l_colors[1] = getPlayer()->mEquipment.mShirt.getColorB();
    if (getPlayer()->mMiiMask) {
        l_colors[2] = l_colors[3] = l_colors[4] = l_colors[5] =
            dItem::COLOR_COUNT;
    } else if (getPlayer()->mEquipment.mHat.getKind() == dItem::KIND_ORG_CAP) {
        l_colors[2] = l_colors[3] = getHairColor(getPlayer()->mHairColor);
        if (getPlayer()->mEquipment.mAcc == dItem::ITEM_ID_NONE) {
            l_colors[4] = l_colors[5] = dItem::COLOR_COUNT;
        } else {
            l_colors[4] = getPlayer()->mEquipment.mAcc.getColorA();
            l_colors[5] = getPlayer()->mEquipment.mAcc.getColorB();
        }
    } else {
        if (getPlayer()->mEquipment.mHat == dItem::ITEM_ID_NONE) {
            l_colors[2] = l_colors[3] = getHairColor(getPlayer()->mHairColor);
        } else {
            l_colors[2] = getPlayer()->mEquipment.mHat.getColorA();
            l_colors[3] = getPlayer()->mEquipment.mHat.getColorB();
        }
        if (getPlayer()->mEquipment.mAcc == dItem::ITEM_ID_NONE) {
            if (getPlayer()->mEquipment.mHat == dItem::ITEM_ID_NONE ||
                !showsHair(getPlayer()->mEquipment.mHat.getHideBone())) {
                l_colors[4] = l_colors[5] = dItem::COLOR_COUNT;
            } else {
                l_colors[4] = l_colors[5] =
                    getHairColor(getPlayer()->mHairColor);
            }
        } else {
            l_colors[4] = getPlayer()->mEquipment.mAcc.getColorA();
            l_colors[5] = getPlayer()->mEquipment.mAcc.getColorB();
        }
    }

    int counts[dItem::COLOR_COUNT];
    for (int i = 0; i < dItem::COLOR_COUNT; i++) {
        counts[i] = 0;
    }
    for (int i = 0; i < 6; i++) {
        if (l_colors[i] < dItem::COLOR_COUNT) {
            counts[l_colors[i]]++;
        }
    }
    int max = 0;
    for (int i = 0; i < dItem::COLOR_COUNT; i++) {
        if (max < counts[i]) {
            max = counts[i];
        }
    }
    int num = 0;
    int first = dItem::COLOR_NEUTRAL;
    int second = dItem::COLOR_NEUTRAL;
    for (int i = 0; i < dItem::COLOR_COUNT; i++) {
        if (counts[i] == max) {
            num++;
            if (num == 1) {
                first = i;
            } else if (num == 2) {
                second = i;
            }
        }
    }

    if (num == 1) {
        int i = 0;
        while (first != l_colorShoes[i].mColor) {
            i++;
        }
        switch (kind) {
        case dPlayerMgr::SHOE_KIND_MALE:
            return l_colorShoes[i].mShoe[dPlayerMgr::SHOE_KIND_MALE];
        case dPlayerMgr::SHOE_KIND_FEMALE:
            return l_colorShoes[i].mShoe[dPlayerMgr::SHOE_KIND_FEMALE];
        }
    } else if (num == 2) {
        int i = 0;
        while (!((first == l_colorPairShoes[i].mColorA &&
                  second == l_colorPairShoes[i].mColorB) ||
                 (second == l_colorPairShoes[i].mColorA &&
                  first == l_colorPairShoes[i].mColorB))) {
            i++;
        }
        switch (kind) {
        case dPlayerMgr::SHOE_KIND_MALE:
            return l_colorPairShoes[i].mShoe[dPlayerMgr::SHOE_KIND_MALE];
        case dPlayerMgr::SHOE_KIND_FEMALE:
            return l_colorPairShoes[i].mShoe[dPlayerMgr::SHOE_KIND_FEMALE];
        }
    } else {
        switch (kind) {
        case dPlayerMgr::SHOE_KIND_MALE:
            return l_defaultShoe[dPlayerMgr::SHOE_KIND_MALE];
        case dPlayerMgr::SHOE_KIND_FEMALE:
            return l_defaultShoe[dPlayerMgr::SHOE_KIND_FEMALE];
        }
    }
    return 1;
}

// The looks of the outfit (shirt, hat, accessory, the hairstyle's looks);
// LOOK_COUNT: none.
static int l_looks[6] = {
    dItem::LOOK_COUNT, dItem::LOOK_COUNT, dItem::LOOK_COUNT,
    dItem::LOOK_COUNT, dItem::LOOK_COUNT,
};
// The last look picked, by kind (unused).
static int l_lastLook[2] = {dItem::LOOK_COUNT, dItem::LOOK_COUNT};

// .text 0x6F8: the shoe colour that matches the looks of the outfit.
static u8 getLookShoeColor(u8 kind) {
    struct lookShoe_s {
        int mLook;
        int mShoeLook[2]; // the look of the shoes, by kind
    };
    struct lookPairShoe_s {
        int mLookA;
        int mLookB;
        int mShoeLook[2];
    };
    static const lookShoe_s l_lookShoes[] = {
        {10, {5, 5}}, {0, {0, 0}}, {1, {1, 1}}, {2, {2, 2}},
        {3, {3, 3}},  {4, {4, 4}}, {5, {5, 5}}, {6, {6, 6}},
        {7, {7, 7}},  {8, {8, 8}}, {9, {9, 9}},
    };
    static const lookShoe_s l_defaultShoe = {0, {5, 5}};
    static const lookPairShoe_s l_lookPairShoes[] = {
        {0, 1, {0, 0}}, {0, 2, {0, 0}}, {0, 3, {0, 0}}, {0, 4, {0, 0}},
        {0, 5, {0, 0}}, {0, 6, {0, 0}}, {0, 7, {0, 0}}, {0, 8, {0, 0}},
        {0, 9, {0, 0}}, {1, 2, {1, 1}}, {1, 3, {1, 1}}, {1, 4, {1, 1}},
        {1, 5, {1, 1}}, {1, 6, {1, 1}}, {1, 7, {1, 1}}, {1, 8, {1, 1}},
        {1, 9, {1, 1}}, {2, 3, {2, 2}}, {2, 4, {2, 2}}, {2, 5, {2, 2}},
        {2, 6, {2, 2}}, {2, 7, {2, 2}}, {2, 8, {2, 2}}, {2, 9, {2, 2}},
        {3, 4, {3, 3}}, {3, 5, {3, 3}}, {3, 6, {3, 3}}, {3, 7, {3, 3}},
        {3, 8, {3, 3}}, {3, 9, {3, 3}}, {4, 5, {4, 4}}, {4, 6, {4, 4}},
        {4, 7, {4, 4}}, {4, 8, {4, 4}}, {4, 9, {4, 4}}, {5, 6, {5, 5}},
        {5, 7, {5, 5}}, {5, 8, {5, 5}}, {5, 9, {5, 5}}, {6, 7, {6, 6}},
        {6, 8, {6, 6}}, {6, 9, {6, 6}}, {7, 8, {7, 7}}, {7, 9, {7, 7}},
        {8, 9, {8, 8}},
    };

    int look = dItem::LOOK_COUNT;
    for (int i = 0; i < 5; i++) {
        l_looks[i] = dItem::LOOK_COUNT;
    }
    l_looks[0] = getPlayer()->mEquipment.mShirt.getClothStyle();
    int n = 1;
    if (!getPlayer()->mMiiMask) {
        BOOL hair = FALSE;
        if (getPlayer()->mEquipment.mHat.getKind() == dItem::KIND_ORG_CAP) {
            hair = TRUE;
            if (getPlayer()->mEquipment.mAcc.isValid()) {
                l_looks[1] = getPlayer()->mEquipment.mAcc.getClothStyle();
                n = 2;
            }
        } else {
            if (getPlayer()->mEquipment.mHat == dItem::ITEM_ID_NONE) {
                hair = TRUE;
            } else {
                l_looks[1] = getPlayer()->mEquipment.mHat.getClothStyle();
                n = 2;
            }
            if (getPlayer()->mEquipment.mAcc == dItem::ITEM_ID_NONE) {
                if (!hair &&
                    showsHair(getPlayer()->mEquipment.mHat.getHideBone())) {
                    hair = TRUE;
                }
            } else {
                l_looks[n++] = getPlayer()->mEquipment.mAcc.getClothStyle();
            }
        }
        if (hair) {
            int i;
            u8 hairStyle = getPlayer()->mHair;
            int *look = &l_looks[n];
            for (i = 0; i < dItem::LOOK_COUNT; i++) {
                if (dPlayerMgr::hasHairFeature(hairStyle, i)) {
                    *look++ = i;
                }
            }
        }
    }

    int counts[dItem::LOOK_COUNT];
    for (int i = 0; i < dItem::LOOK_COUNT; i++) {
        counts[i] = 0;
    }
    for (int i = 0; i < 5; i++) {
        if (l_looks[i] < dItem::LOOK_COUNT) {
            counts[l_looks[i]]++;
        }
    }
    int max = 0;
    for (int i = 0; i < dItem::LOOK_COUNT; i++) {
        if (max < counts[i]) {
            max = counts[i];
        }
    }
    int num = 0;
    int first = dItem::LOOK_NONE;
    int second = dItem::LOOK_NONE;
    for (int i = 0; i < dItem::LOOK_COUNT; i++) {
        if (counts[i] == max) {
            num++;
            if (num == 1) {
                first = i;
            } else if (num == 2) {
                second = i;
            }
        }
    }

    if (num == 2) {
        if (first != dItem::LOOK_NONE && second != dItem::LOOK_NONE) {
            int i = 0;
            while (!((first == l_lookPairShoes[i].mLookA &&
                      second == l_lookPairShoes[i].mLookB) ||
                     (second == l_lookPairShoes[i].mLookA &&
                      first == l_lookPairShoes[i].mLookB))) {
                i++;
            }
            switch (kind) {
            case dPlayerMgr::SHOE_KIND_MALE:
                look = l_lookPairShoes[i].mShoeLook[dPlayerMgr::SHOE_KIND_MALE];
                break;
            case dPlayerMgr::SHOE_KIND_FEMALE:
                look =
                    l_lookPairShoes[i].mShoeLook[dPlayerMgr::SHOE_KIND_FEMALE];
                break;
            }
        } else {
            if (first == dItem::LOOK_NONE) {
                first = second;
            }
            int i = 0;
            while (first != l_lookShoes[i].mLook) {
                i++;
            }
            switch (kind) {
            case dPlayerMgr::SHOE_KIND_MALE:
                look = l_lookShoes[i].mShoeLook[dPlayerMgr::SHOE_KIND_MALE];
                break;
            case dPlayerMgr::SHOE_KIND_FEMALE:
                look = l_lookShoes[i].mShoeLook[dPlayerMgr::SHOE_KIND_FEMALE];
                break;
            }
        }
    } else if (num == 1) {
        int i = 0;
        while (first != l_lookShoes[i].mLook) {
            i++;
        }
        switch (kind) {
        case dPlayerMgr::SHOE_KIND_MALE:
            look = l_lookShoes[i].mShoeLook[dPlayerMgr::SHOE_KIND_MALE];
            break;
        case dPlayerMgr::SHOE_KIND_FEMALE:
            look = l_lookShoes[i].mShoeLook[dPlayerMgr::SHOE_KIND_FEMALE];
            break;
        }
    } else {
        switch (kind) {
        case dPlayerMgr::SHOE_KIND_MALE:
            look = l_defaultShoe.mShoeLook[dPlayerMgr::SHOE_KIND_MALE];
            break;
        case dPlayerMgr::SHOE_KIND_FEMALE:
            look = l_defaultShoe.mShoeLook[dPlayerMgr::SHOE_KIND_FEMALE];
            break;
        }
    }
    l_lastLook[kind] = look;
    return dPlayerMgr::getShoeColor(look, kind);
}

// .text 0xBB4: the debug page hook (empty in this build).
static BOOL entryDebugPage(dAcNpcSpKutsumigaki_c *npc) {
    return FALSE;
}

// .text 0xBBC
BASE_PROFILE(A_NPC_SP_KUTSUMIGAKI, dAcNpcSpKutsumigaki_c);
} // namespace dANpcSpKutsumigaki

// .text 0xE04
void dAcNpcSpKutsumigaki_c::onDemoCmd(int cmd, u8 member) {
    switch (cmd) {
    case DEMO_CMD_START:
        changeState(STATE_IDLE);
        break;
    case DEMO_CMD_TALK:
        mShineTalk.start(
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT));
        changeState(STATE_TALK);
        break;
    case CMD_RELEASE_PERMIT:
    case DEMO_CMD_END:
        if (mAnmSet.mWaitAnm == HMN_ANM_SKK_WAIT_NORMAL) {
            mAnmSet.setAnm0(HMN_ANM_NML_WAIT1);
        }
        changeState(STATE_IDLE);
        break;
    }
    dAcNpcSp_c::onDemoCmd(cmd, member);
}

namespace dANpcSpKutsumigaki {
static const dAcNpcSpKutsumigaki_c::State_s l_states[] = {
    {&dAcNpcSpKutsumigaki_c::initIdle, &dAcNpcSpKutsumigaki_c::executeIdle},
    {&dAcNpcSpKutsumigaki_c::initTalk, &dAcNpcSpKutsumigaki_c::executeTalk},
    {&dAcNpcSpKutsumigaki_c::initShine, &dAcNpcSpKutsumigaki_c::executeShine},
};
} // namespace dANpcSpKutsumigaki

// .text 0xEC8: the player can talk to him in front of the stoop.
bool dAcNpcSpKutsumigaki_c::canInteract(dDemoActor_c *actor) {
    if (dAcNpcSp_c::canInteract(actor)) {
        if ((dANpcSpKutsumigaki::getPlayer()->isFlag1(
                 PRIVATE_FLAG1_SHOES_SHINED) ||
             mShined) &&
            mAnmSet.mWaitAnm != HMN_ANM_SKK_WAIT_SIT) {
            return true;
        }
        if (mLookAt.mLocked) {
            return false;
        }
        dAcPlayer_c *player =
            dPlayerMgr::getNpcTargetActor(dPlayerMgr::PLAYER_INDEX_CURRENT);
        if (player == NULL) {
            return false;
        }
        mVec3_c playerPos = player->mPos;
        if (playerPos == mVec3_c::Zero) {
            return false;
        }
        const mVec3_c *myPos = &mPos;
        mVec3_c *pos = player->demoHook58();
        if (std::fabs(myPos->xzDistTo(*pos)) < KUTSUMIGAKI_TALK_DIST) {
            mAng angle(dActor_c::targetAngleY(myPos, pos) - mAngle.y.mAngle);
            if (dAcNpc_c::lookAt_c::isWithin(angle,
                                             mAng(KUTSUMIGAKI_TALK_YAW)) &&
                !mLookAt.mHeadPos.isZero()) {
                return true;
            }
        }
    }
    return false;
}

// .text 0x109C
void dAcNpcSpKutsumigaki_c::changeState(int state) {
    mState = state;
    (this->*dANpcSpKutsumigaki::l_states[state].mInit)();
}

// .text 0x10D4: the next place of the shine, next to the player on the
// stoop.
void dAcNpcSpKutsumigaki_c::setShinePos(int pos) {
    static mVec3_c l_basePos(1084.8f, 0.0f, 385.6f);
    mVec3_c ofs;
    switch (pos) {
    case SHINE_POS_FEET:
        ofs.set(0.0f, 0.0f, -13.0f);
        break;
    case SHINE_POS_JUMP:
        ofs.set(38.0f, 0.0f, 28.0f);
        break;
    case SHINE_POS_SHOES:
        ofs.set(0.0f, 0.0f, 38.0f);
        break;
    case SHINE_POS_BRUSH:
        ofs.set(0.0f, 0.0f, 32.0f);
        break;
    case SHINE_POS_BACK:
        ofs.set(-28.0f, 0.0f, 32.0f);
        break;
    }
    ofs.rotY(mAng(-0x2000));
    mShinePos = l_basePos + ofs;
}

// .text 0x124C
int dAcNpcSpKutsumigaki_c::preCreate() {
    if (!dAcNpcSp_c::preCreate()) {
        return NOT_READY;
    }
    setDemoState1();
    setRecept(&mShineTalk);
    return SUCCEEDED;
}

// .text 0x12A0
int dAcNpcSpKutsumigaki_c::create() {
    if (!dAcNpcSp_c::create()) {
        return NOT_READY;
    }
    if (!mClothMdl.create("skk_cloth", 0)) {
        return NOT_READY;
    }
    dTime_c now = *dTime_c::getCurrent();
    if (!dNpcMng_c::isWeekVisitWeek() || now.hour < 9 || now.hour >= 21) {
        return FAILED;
    }
    mAnmSet.setAnm0(HMN_ANM_SKK_WAIT_SIT);
    mObjc.setDefWeight(dAcNpc_c::objc_c::WEIGHT_IMMOVABLE);
    if (!dANpcSpKutsumigaki::getPlayer()->isFlag1(PRIVATE_FLAG1_SHOES_SHINED)) {
        mLookAt.mYaw.mMax = KUTSUMIGAKI_TALK_YAW;
        mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                          KUTSUMIGAKI_TALK_DIST);
    }
    mShined = 0;
    mTurnBack = 0;
    dSceneCheck::fn_80081858(dANpcSpKutsumigaki::entryDebugPage(this));
    return SUCCEEDED;
}

// .text 0x13F4
int dAcNpcSpKutsumigaki_c::doDelete() {
    if (!mClothMdl.remove()) {
        return NOT_READY;
    }
    if (!dAcNpcSp_c::doDelete()) {
        return NOT_READY;
    }
    dANpcSpKutsumigaki::entryDebugPage(this);
    dSceneCheck::fn_8008185C();
    return SUCCEEDED;
}

// .text 0x145C: the cloth is drawn from the jump on.
int dAcNpcSpKutsumigaki_c::draw() {
    if (mState == STATE_SHINE && mStep >= SHINE_STEP_JUMP) {
        mMtx_c mtx;
        dActor_c::makeMtx(&mtx, &mPos, mAngle.y);
        mClothMdl.mMdl.setLocalMtx(&mtx);
        mClothMdl.entry();
    }
    return dAcNpc_c::draw();
}

// .text 0x14D0
int dAcNpcSpKutsumigaki_c::vtB0() {
    if (mState < STATE_COUNT) {
        (this->*dANpcSpKutsumigaki::l_states[mState].mExecute)();
    }
    return 1;
}

// .text 0x1518
void dAcNpcSpKutsumigaki_c::initIdle() {
    if (mAnmSet.mWaitAnm == HMN_ANM_SKK_WAIT_NORMAL) {
        mAction.requestWait(PRIORITY_IDLE, 0.0f);
    } else if (mTurnBack) {
        mAction.requestTurn(PRIORITY_IDLE, mAng(0), l_turnSpeed, 0,
                            cNpcMorphFrames);
        mTurnBack = 0;
    } else {
        mAction.requestWait(PRIORITY_IDLE, cNpcMorphFrames);
    }
    mCheckGround = 1;
}

// .text 0x15C8
void dAcNpcSpKutsumigaki_c::executeIdle() {
}

// .text 0x15CC
void dAcNpcSpKutsumigaki_c::initTalk() {
    if (!mTalk.isBusy()) {
        if (mAnmSet.mWaitAnm == HMN_ANM_NML_WAIT1) {
            mAng angle = getTalkAngle();
            mAng speed = getTalkTurnSpeed();
            mTalk.requestTurnTalk(angle, speed, FALSE);
        } else {
            mTalk.requestTalk(true, FALSE);
        }
    }
    mStep = 0;
}

// .text 0x166C
void dAcNpcSpKutsumigaki_c::stepTalk() {
    if (!mTalk.isBusy()) {
        dDemo::finishTalk(this);
        mStep++;
    }
}

namespace dANpcSpKutsumigaki {
static const dAcNpcSpKutsumigaki_c::Proc l_talkSteps[] = {
    &dAcNpcSpKutsumigaki_c::stepTalk};
} // namespace dANpcSpKutsumigaki

// .text 0x16B8
void dAcNpcSpKutsumigaki_c::executeTalk() {
    if (mStep < ARRAY_SIZE(dANpcSpKutsumigaki::l_talkSteps)) {
        (this->*dANpcSpKutsumigaki::l_talkSteps[mStep])();
    }
}

// .text 0x16F8
void dAcNpcSpKutsumigaki_c::initShine() {
    mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_SKK_STAND, 0.0f, cNpcMorphFrames,
                       m3d::PLAYMODE_INHERIT, 0);
    mStep = SHINE_STEP_STAND;
    mSpeedF = 0.0f;
    mAccelY = 0.0f;
    mMaxSpeedY = 0.0f;
    mSpeed.x = 0.0f;
    mSpeed.y = 0.0f;
    mSpeed.z = 0.0f;
    mCheckGround = 0;
    setShinePos(SHINE_POS_FEET);
    mTimer = 0;
}

// .text 0x1788: he gets up and hops in front of the player's feet.
void dAcNpcSpKutsumigaki_c::stepStand() {
    f32 frame = mTimer;
    mTimer++;
    if (frame < 18.0f) {
        // still getting up
    } else if (frame < 36.0f) {
        if (frame < 19.0f) {
            mAudioObj.startSound(SE_Shoeshine_SmallJump);
        }
        f32 rate = 1.0f / (36.0f - frame);
        sLib::addCalc(&mPos.x, mShinePos.x, rate, 32.0f, 0.0f);
        sLib::addCalc(&mPos.z, mShinePos.z, rate, 32.0f, 0.0f);
    } else {
        if (frame < 37.0f) {
            mAudioObj.startSound(SE_Shoeshine_SmallJumpLanding);
        }
        mPos.x = mShinePos.x;
        mPos.z = mShinePos.z;
    }
    mLastPos = mPos;
    if (mAction.mActionId == ACTION_ANM && mAction.mCanChange) {
        mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_SKK_STAND_WAIT, 0.0f, 0.0f,
                           m3d::PLAYMODE_INHERIT, 0);
        mStep = SHINE_STEP_WAIT;
        mTimer = 5;
        dPlayerMgr::startShoeShine(750, 0.75f);
    }
}

// .text 0x1910
void dAcNpcSpKutsumigaki_c::stepWait() {
    if (--mTimer <= 0) {
        mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_SKK_JUMP, 0.0f,
                           cNpcMorphFrames, m3d::PLAYMODE_INHERIT, 0);
        mClothMdl.setAnm("skk_jump_cloth", m3d::FORWARD_ONCE, 0.0f);
        mStep = SHINE_STEP_JUMP;
        mTimer = 0;
        setShinePos(SHINE_POS_JUMP);
    }
}

// .text 0x19AC: the big jump around the player, then to the shoes.
void dAcNpcSpKutsumigaki_c::stepJump() {
    f32 frame = mTimer;
    mTimer++;
    if (frame < 10.0f) {
        // the run-up
    } else if (frame < 30.0f) {
        if (frame < 11.0f) {
            mAudioObj.startSound(SE_Shoeshine_BigJump);
        }
        f32 rate = 1.0f / (31.0f - frame);
        sLib::addCalc(&mPos.x, mShinePos.x, rate, 32.0f, 0.0f);
        sLib::addCalc(&mPos.z, mShinePos.z, rate, 32.0f, 0.0f);
    } else if (frame < 31.0f) {
        mAudioObj.startSound(SE_Shoeshine_BigJumpLanding);
        mPos.x = mShinePos.x;
        mPos.z = mShinePos.z;
        setShinePos(SHINE_POS_SHOES);
    } else if (frame < 56.0f) {
        // the pose
    } else if (frame < 84.0f) {
        if (frame < 57.0f) {
            mAudioObj.startSound(SE_Shoeshine_SmallJump);
        }
        f32 rate = 1.0f / (85.0f - frame);
        sLib::addCalc(&mPos.x, mShinePos.x, rate, 32.0f, 0.0f);
        sLib::addCalc(&mPos.z, mShinePos.z, rate, 32.0f, 0.0f);
    } else if (frame < 85.0f) {
        mAudioObj.startSound(SE_Shoeshine_SmallJumpLanding);
        mPos.x = mShinePos.x;
        mPos.z = mShinePos.z;
        setShinePos(SHINE_POS_BRUSH);
    } else if (frame < 159.0f) {
        // the pose
    } else if (frame < 176.0f) {
        f32 rate = 1.0f / (177.0f - frame);
        sLib::addCalc(&mPos.x, mShinePos.x, rate, 32.0f, 0.0f);
        sLib::addCalc(&mPos.z, mShinePos.z, rate, 32.0f, 0.0f);
    } else {
        mPos.x = mShinePos.x;
        mPos.z = mShinePos.z;
    }
    mLastPos = mPos;
    mClothMdl.play();
    if (mClothMdl.mAnmVis.checkFrame(113.0f)) {
        mAudioObj.startSound(SE_Shoeshine_CleanStartPose);
    }
    if (mAction.mActionId == ACTION_ANM && mAction.mCanChange) {
        mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_SKK_BRUSH, 0.0f, 0.0f,
                           m3d::PLAYMODE_INHERIT, 0);
        dEffect::create("af_skk_polish", &mPos, &mAngle, NULL);
        mClothMdl.setAnm("skk_brush_cloth", m3d::FORWARD_LOOP, 0.0f);
        mStep = SHINE_STEP_BRUSH;
        mTimer = 80;
        mAudioObj.startSound(SE_Shoeshine_Clean);
    }
}

// .text 0x1CDC
void dAcNpcSpKutsumigaki_c::stepBrush() {
    mClothMdl.play();
    if (--mTimer <= 0) {
        mAction.requestAnm(PRIORITY_DEMO, HMN_ANM_SKK_BRUSH_END, 0.0f, 6.0f,
                           m3d::PLAYMODE_INHERIT, 0);
        mClothMdl.setAnm("skk_brush_end_cloth", m3d::FORWARD_ONCE, 0.0f);
        mStep = SHINE_STEP_FINISH;
        mTimer = 0;
        setShinePos(SHINE_POS_BACK);
        dPlayerMgr::setShoeColor(mShineTalk.mShoeColor, mShineTalk.mShoeKind);
        mAudioObj.startSound(SE_Shoeshine_CleanEnd);
    }
}

// .text 0x1DA4: he hops back and sits down again.
void dAcNpcSpKutsumigaki_c::stepFinish() {
    mClothMdl.play();
    f32 frame = mTimer;
    mTimer++;
    if (frame < 5.0f) {
        // the turn
    } else if (frame < 24.0f) {
        f32 rate = 1.0f / (24.0f - frame);
        sLib::addCalc(&mPos.x, mShinePos.x, rate, 32.0f, 0.0f);
        sLib::addCalc(&mPos.z, mShinePos.z, rate, 32.0f, 0.0f);
    } else {
        mPos.x = mShinePos.x;
        mPos.z = mShinePos.z;
    }
    mLastPos = getPos();
    if (mAction.mActionId == ACTION_ANM && mAction.mCanChange) {
        mAngle.y += 0x6222;
        mAngle3D.y = mAngle.y;
        mPos.y = mLastPos.y = dBGCF::getGroundY(&mPos, FALSE);
        mAnmSet.setAnm0(HMN_ANM_SKK_WAIT_NORMAL);
        mShined = 1;
        mTurnBack = 1;
        mAction.requestWait(PRIORITY_IDLE, 0.0f);
        changeState(STATE_TALK);
        mLookAt.mYaw.mMax = l_lookYawMax;
        mLookAt.setPlayer(PRIORITY_IDLE, true, l_lookPitchStep, l_lookYawStep,
                          cNpcLookRange);
        dPlayerMgr::finishShoeShine();
    }
}

namespace dANpcSpKutsumigaki {
static const dAcNpcSpKutsumigaki_c::Proc l_shineSteps[] = {
    &dAcNpcSpKutsumigaki_c::stepStand, &dAcNpcSpKutsumigaki_c::stepWait,
    &dAcNpcSpKutsumigaki_c::stepJump, &dAcNpcSpKutsumigaki_c::stepBrush,
    &dAcNpcSpKutsumigaki_c::stepFinish};
} // namespace dANpcSpKutsumigaki

// .text 0x1F44
void dAcNpcSpKutsumigaki_c::executeShine() {
    if (mStep < ARRAY_SIZE(dANpcSpKutsumigaki::l_shineSteps)) {
        (this->*dANpcSpKutsumigaki::l_shineSteps[mStep])();
    }
}

// .text 0x1F84
dAcNpcSpKutsumigaki_c::talk_c::talk_c(dAcNpcSpKutsumigaki_c *npc)
    : mpOwner(npc) {
    init();
}

// .text 0x1FE4
dAcNpcSpKutsumigaki_c::talk_c::~talk_c() {
}

// .text 0x203C
void dAcNpcSpKutsumigaki_c::talk_c::init() {
    dAcNpc_c::recept_c::init();
    mMsgNo = 0;
    mOnMessageEnd = NULL;
    mOnRequestEnd = NULL;
    mProc = NULL;
    mShoeColor = mOrgShoeColor = dANpcSpKutsumigaki::getPlayer()->mShoeColor;
    mShoeKind = mOrgShoeKind = dANpcSpKutsumigaki::getPlayer()->mShoeKind;
}

// .text 0x20E4
void dAcNpcSpKutsumigaki_c::talk_c::start(dActor_c *target) {
    init();
    mpActActor = target;
    if (dANpcSpKutsumigaki::getPlayer()->isFlag1(PRIVATE_FLAG1_SHOES_SHINED) ||
        mpOwner->mShined) {
        mMsgNo = MSG_SHINED;
    } else {
        int count = dANpcSpKutsumigaki::getShineCount();
        if (count <= 2) {
            mMsgNo = MSG_FIRST_OFFER;
        } else if (count <= 6) {
            mMsgNo = MSG_REGULAR_OFFER;
        } else if (count <= 10) {
            mMsgNo = MSG_OFFER;
        } else {
            mMsgNo = MSG_OFFER_RANDOM;
        }
        dMoney_c::onForceShow();
    }
}

// .text 0x21B0
void dAcNpcSpKutsumigaki_c::talk_c::getMsgInfo(msgInfo_s *info) {
    info->mLabel = "npc_SP/NPC_shoeshine";
    info->mCode = mMsgNo;
}

// .text 0x21C8
void dAcNpcSpKutsumigaki_c::talk_c::onMessageStart(int arg) {
}

// .text 0x21CC
void dAcNpcSpKutsumigaki_c::talk_c::onMessageEnd(int kind) {
    switch (mMessageCode) {
    case MSG_BY_COLOR:
        mShoeColor = dANpcSpKutsumigaki::getColorShoeColor(mShoeKind);
        mOnMessageEnd = &talk_c::closeMsg;
        mOnRequestEnd = &talk_c::onShineRequested;
        break;
    case MSG_BY_LOOK:
        mShoeColor = dANpcSpKutsumigaki::getLookShoeColor(mShoeKind);
        mOnMessageEnd = &talk_c::closeMsg;
        mOnRequestEnd = &talk_c::onShineRequested;
        break;
    case MSG_PAY: {
        dSaveTown_c *town = dSaveData_c::getTown();
        town->mVillagerMove.setPlayerValue(dPlayerMgr::getCurrentPlayerSlot(),
                                           dSvVillagerMove::PLAYER_VALUE_KICKS);
        break;
    }
    case MSG_DONE_FREE: {
        dSaveTown_c *town = dSaveData_c::getTown();
        town->mVillagerMove.setPlayerValue(dPlayerMgr::getCurrentPlayerSlot(),
                                           dSvVillagerMove::PLAYER_VALUE_KICKS);
        break;
    }
    case MSG_ASK_SWAP:
        if (mpController->mNextMsgCode == MSG_NO_MONEY) {
            break;
        }
        if (dANpcSpKutsumigaki::getShineCount() < SHINE_COUNT_MAX) {
            break;
        }
        if (dANpcSpKutsumigaki::getPlayer()->isFlag0(
                PRIVATE_FLAG0_KICKS_SWAP_ASKED)) {
            mpController->setNextMsgCode(MSG_ASK_KIND_REPEAT);
        } else {
            dANpcSpKutsumigaki::getPlayer()->setFlag0(
                PRIVATE_FLAG0_KICKS_SWAP_ASKED);
            if (dANpcSpKutsumigaki::getPlayer()->mPID.player.mGender ==
                GENDER_MALE) {
                mpController->setNextMsgCode(MSG_SWAP_TO_FEMALE);
            } else {
                mpController->setNextMsgCode(MSG_SWAP_TO_MALE);
            }
        }
        break;
    }
    if (mOnMessageEnd) {
        (this->*mOnMessageEnd)();
        mOnMessageEnd = NULL;
    }
}

// .text 0x23C0
void dAcNpcSpKutsumigaki_c::talk_c::onAnswer(int arg) {
    int answer = mpController->mSelect.mCursor;
    switch (mMessageCode) {
    case MSG_SWAP_TO_FEMALE:
    case MSG_SWAP_TO_MALE:
        if (answer == ANSWER_SWAP_YES) {
            if (dANpcSpKutsumigaki::getPlayer()->mPID.player.mGender ==
                GENDER_MALE) {
                mShoeKind = dPlayerMgr::SHOE_KIND_FEMALE;
            } else {
                mShoeKind = dPlayerMgr::SHOE_KIND_MALE;
            }
            mpController->setNextMsgCode(MSG_SWAP_HOW);
        }
        break;
    case MSG_ASK_KIND:
        if (answer == ANSWER_MASCULINE) {
            mShoeKind = dPlayerMgr::SHOE_KIND_MALE;
        } else {
            mShoeKind = dPlayerMgr::SHOE_KIND_FEMALE;
        }
        break;
    case MSG_ASK_KIND_2:
        if (answer == ANSWER_2_MASCULINE) {
            mShoeKind = dPlayerMgr::SHOE_KIND_MALE;
        } else {
            mShoeKind = dPlayerMgr::SHOE_KIND_FEMALE;
        }
        break;
    }
}

// .text 0x2498
void dAcNpcSpKutsumigaki_c::talk_c::preExecute() {
    if (mProc) {
        (this->*mProc)();
    }
}

// .text 0x24E0
void dAcNpcSpKutsumigaki_c::talk_c::onTalkEnd() {
    dMoney_c::offForceShow();
}

// .text 0x24E4
void dAcNpcSpKutsumigaki_c::talk_c::onRequestEnd(int kind) {
    if (mOnRequestEnd) {
        (this->*mOnRequestEnd)();
        mOnRequestEnd = NULL;
    }
}

// .text 0x2548: the window closes for the shine.
void dAcNpcSpKutsumigaki_c::talk_c::closeMsg() {
    if (mpController->mNextMsgCode == dMsg::MSG_CODE_NONE) {
        mpController->setNextMsgCode(dMsg::MSG_CODE_INVALID);
    }
    setRequest1(0);
}

// .text 0x25A0
void dAcNpcSpKutsumigaki_c::talk_c::onShineRequested() {
    startShine();
}

// .text 0x25A4
void dAcNpcSpKutsumigaki_c::talk_c::startShine() {
    mProc = &talk_c::execShineWait;
    mpOwner->changeState(STATE_SHINE);
}

// .text 0x25CC: once the shine is over, the talk goes on.
void dAcNpcSpKutsumigaki_c::talk_c::execShineWait() {
    if (mpOwner->mShined &&
        dPlayerMgr::isWaitingForDemo(dPlayerMgr::PLAYER_INDEX_CURRENT)) {
        reqMsgClose();
        mOnRequestEnd = &talk_c::clearProc;
        if (mShoeColor != mOrgShoeColor || mShoeKind != mOrgShoeKind) {
            mpController->setNextMsgCode(MSG_DONE);
            dANpcSpKutsumigaki::getPlayer()->setFlag1(
                PRIVATE_FLAG1_SHOES_SHINED);
            dPrivateData_c *player = dANpcSpKutsumigaki::getPlayer();
            if (player->mShoeShineCount < SHINE_COUNT_MAX) {
                player->mShoeShineCount++;
            }
        } else {
            mpController->setNextMsgCode(MSG_DONE_FREE);
        }
        mProc = NULL;
    }
}

// .text 0x26B8
void dAcNpcSpKutsumigaki_c::talk_c::clearProc() {
    mProc = NULL;
}

// .text 0x26D8
int dAcNpcSpKutsumigaki_c::getEarType() {
    return NPC_EAR_TYPE_KICKS;
}

#ifdef MUST_MATCH
// Matching-only emission order (the d_a_npc_sp_cafe / _fortune convention).
// There is no evidence for this helper in the original source. MWCC drops
// the unused function; its reference puts anmChr_c::getType before
// anmVis_c::getType among the retained weak inlines.
static void orderKutsumigakiWeakInlines(m3d::anmChr_c *anm) {
    anm->m3d::anmChr_c::getType();
}
#endif
