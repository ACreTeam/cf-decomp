// d_npc_info.cpp: the special NPCs' event state. .text 800F59C8..800F9424, .ctors 804656E0, .rodata
// 80475418..80475728, .data 804ED108..804ED130, .bss 805CE800..805CE9D0, .sdata 8074AEE8, .sbss
// 8074E570..8074E5A8, .sdata2 80750980..80750988. See include/game/game/d_npc_info.hpp.
#include <game/game/d_npc_info.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_a_npc.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_sv_npc_pos.hpp>
#include <cstring>

using namespace dNpcInfo;

static param_c l_param;
static opTanukichi_c l_opTanukichi;
static playerCreate_c l_playerCreate;
static tanukichi_c l_tanukichi;
static jingle_c l_jingle;
static franklin_c l_franklin;
static pyontaro_c l_pyontaro;
static uomasa_c l_uomasa;
static bug_c l_bug;
static pumpking_c l_pumpking;
static cafe_c l_cafe;
static soncho_c l_soncho;
static gatekeeperB_c l_gatekeeperB;
static kappei_c l_kappei;
static resetsan_c l_resetsan;
static goddess_c l_goddess;
static dancer_c l_dancer;
static totakeke_c l_totakeke;
static gatekeeperA_c l_gatekeeperA;
static kaburiba_c l_kaburiba;
static fuko_c l_fuko;
static futa_c l_futa;

static bool l_resetsanEnabled = true;

// 800F59C8
void fn_800F59C8() {}

// 800F59CC
int fn_800F59CC() {
    return 0;
}

// 800F59D4
int fn_800F59D4() {
    return 0;
}

// 800F59DC
int dNpcInfo::getJingleHomeChance() {
    return l_param.mJingleHomeChance;
}

// 800F59EC
int dNpcInfo::getWalkChance() {
    return l_param.mWalkChance;
}

// 800F59FC
int dNpcInfo::getWaitBase() {
    return l_param.mWaitBase;
}

// 800F5A0C
int dNpcInfo::getWaitRand() {
    return l_param.mWaitRand;
}

// 800F5A1C
void dNpcInfo::init() {
    l_opTanukichi.clear();
    l_playerCreate.clear();
    l_tanukichi.clear();
    l_jingle.clear();
    l_franklin.clear();
    l_pyontaro.clear();
    l_uomasa.clear();
    l_bug.clear();
    l_pumpking.clear();
    l_cafe.clear();
    l_soncho.clear();
    l_gatekeeperB.clear();
    l_kappei.clear();
    l_goddess.clear();
    l_dancer.clear();
    l_totakeke.clear();
    l_gatekeeperA.clear();
    l_kaburiba.clear();
    l_fuko.clear();
    l_futa.clear();
}

// 800F5AE8
void dNpcInfo::reset() {
    l_tanukichi.clear();
    l_jingle.clear();
    l_franklin.clear();
    l_pyontaro.clear();
    l_uomasa.clear();
    l_bug.clear();
    l_pumpking.clear();
    l_cafe.clear();
    l_soncho.clear();
    l_dancer.clear();
    l_totakeke.clear();
    l_kaburiba.clear();
}

// 800F5B74
opTanukichi_c *opTanukichi_c::get() {
    return &l_opTanukichi;
}

// 800F5B7C
void opTanukichi_c::clear() {
    mViewed[0] = 0;
    mViewed[1] = 0;
    mViewed[2] = 0;
    mViewed[3] = 0;
}

// 800F5B94
playerCreate_c *playerCreate_c::get() {
    return &l_playerCreate;
}

// 800F5B9C
void playerCreate_c::clear() {
    mFromDs = 0;
}

// 800F5BA8
tanukichi_c *tanukichi_c::get() {
    return &l_tanukichi;
}

// 800F5BB0
void tanukichi_c::clear() {
    mJobDone = 0;
    _1 = 0;
    mTimeSpecialTold = 0;
}

// 800F5BC4
BOOL tanukichi_c::isAllVillagersMet() {
    if (!dPlayerMgr_c::getCurrentPlayerRaw()->isFlag0(0xF)) {
        return FALSE;
    }
    dAnimalBlock_c *animals = &dSaveData_c::getTown()->mAnimals.mTown;
    dPersonalID_c *pid = &dPlayerMgr_c::getCurrentPlayer()->mPID;
    for (int i = 0; i < 10; i++) {
        dAnimal_c *animal = animals->getAnimal(i);
        if (animal != NULL && animal->mID.isValid() && animal->findMemory(pid) == NULL) {
            return FALSE;
        }
    }
    return TRUE;
}

// 800F5C88
jingle_c *jingle_c::get() {
    return &l_jingle;
}

// 800F5C94
void jingle_c::clear() {
    mRing.clear();
}

// 800F5C98
bool jingle_c::isMet() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->mItemPairRing.mMet;
    }
    return mRing.mMet;
}

// 800F5CFC
bool jingle_c::isGiven() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->mItemPairRing.mGiven;
    }
    return mRing.mGiven;
}

// 800F5D60
bool jingle_c::isTalked() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->mItemPairRing.mTalked;
    }
    return mRing.mTalked;
}

// 800F5DC4
bool jingle_c::isGivenHere() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->mItemPairRing.mGivenHere;
    }
    return mRing.mGivenHere;
}

// 800F5E28
int jingle_c::getCount() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->mItemPairRing.mCount;
    }
    return mRing.mCount;
}

// 800F5E74
bool jingle_c::isMoved() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->mItemPairRing.mMoved;
    }
    return mRing.mMoved;
}

// 800F5ED8
void jingle_c::meet() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->mItemPairRing.meet();
    } else {
        mRing.meet();
    }
}

// 800F5F24
void jingle_c::push(dItem::Item hat, dItem::Item acc) {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->mItemPairRing.push(hat, acc);
    } else {
        mRing.push(hat, acc);
    }
}

// 800F5FB8
void jingle_c::incCount() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->mItemPairRing.incCount();
    } else {
        mRing.incCount();
    }
}

// 800F6004
BOOL jingle_c::contains(dItem::Item hat, dItem::Item acc) {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->mItemPairRing.contains(hat, acc);
    }
    return mRing.contains(hat, acc);
}

// 800F6098
void jingle_c::move() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->mItemPairRing.move();
    } else {
        mRing.move();
    }
}

// 800F60E4
void jingle_c::setSpot() {
    mVec3_c pos;
    pos.x = pos.y = pos.z = 0.0f;
    BOOL atHome = TRUE;
    int kind = 0x44;
    int homeRoll = cM::rndInt(100);
    if (homeRoll >= getJingleHomeChance()) {
        int eligibleCount = 0;
        for (int i = 0; i < 10; i++) {
            dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(i);
            if (animal != NULL && animal->mID.isValid()) {
                dSaveTown_c *town = dSaveData_c::getTown();
                int houseKind = i + 0x11;
                if (houseKind != town->mNpcSpotA.mKind && animal->mPlace == 0) {
                    eligibleCount++;
                }
            }
        }
        if (eligibleCount > 0) {
            int selectedIndex = cM::rndInt(eligibleCount);
            int candidateIndex = 0;
            for (int i = 0; i < 10; i++) {
                dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(i);
                if (animal != NULL && animal->mID.isValid()) {
                    dSaveTown_c *town = dSaveData_c::getTown();
                    int houseKind = i + 0x11;
                    if (houseKind != town->mNpcSpotA.mKind && animal->mPlace == 0) {
                        candidateIndex++;
                        if (candidateIndex > selectedIndex) {
                            kind = houseKind;
                            atHome = FALSE;
                            break;
                        }
                    }
                }
            }
        }
    }
    if (atHome) {
        dSaveTown_c *town = dSaveData_c::getTown();
        mVec3_c home = town->mNpcSpotA.getPos();
        if (!dAcNpc_c::searchSpot(&pos, 4, &home, lbl_80750520)) {
            return;
        }
        kind = 0;
    }
    dSaveTown_c *town = dSaveData_c::getTown();
    town->mNpcSpotA.set(kind, &pos);
    if (dAcNpc_c::isMultiPlay()) {
        dNet::beginPacket();
        u8 packet[4];
        packet[0] = kind;
        packet[1] = worldToUnit(pos.x);
        packet[2] = worldToUnit(pos.z);
        dNet::addPacket(packet, sizeof(packet));
        dNet::sendPacket(NET_PACKET_NPC_SPOT_A, NET_MEMBER_NONE);
    }
}

// 800F62F4
bool jingle_c::isFirstGift() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->mItemPairRing.mFirstGift;
    }
    return mRing.mFirstGift;
}

// 800F6358
bool jingle_c::isRefusedNewLook() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->mItemPairRing.mRefusedNewLook;
    }
    return mRing.mRefusedNewLook;
}

// 800F63BC
void jingle_c::setGreetedNewLook() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dItemPairRing_c *ring = &dPlayerMgr_c::getCurrentPlayer()->mItemPairRing;
        ring->mGreetedNewLook = 1;
    } else {
        mRing.mGreetedNewLook = 1;
    }
}

// 800F6410
bool jingle_c::isGreetedNewLook() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->mItemPairRing.mGreetedNewLook;
    }
    return mRing.mGreetedNewLook;
}

// 800F6474
void jingle_c::setRefusedNewLook() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dItemPairRing_c *ring = &dPlayerMgr_c::getCurrentPlayer()->mItemPairRing;
        ring->mRefusedNewLook = 1;
    } else {
        mRing.mRefusedNewLook = 1;
    }
}

// 800F64C8
void jingle_c::resetEncounter() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->mItemPairRing.resetEncounter();
    } else {
        mRing.resetEncounter();
    }
}

// 800F6514
franklin_c *franklin_c::get() {
    return &l_franklin;
}

// 800F6520
void franklin_c::clear() {
    mHandedOver = 0;
    _1 = 0;
    mAskedForHelp = 0;
    mThanked = 0;
    mTalked = 0;
    mTalkedHere = 0;
    mRelocated = 0;
    mGaveHere = 0;
    mGaveKnifeFork = 0;
    mGaveKnifeFork2 = 0;
}

// 800F6550
void franklin_c::setTalked() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x18);
    } else {
        mTalked = 1;
    }
    mTalkedHere = 1;
}

// 800F65A4
void franklin_c::setGaveKnifeFork() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        if (dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x19)) {
            dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x1A);
        } else {
            dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x19);
        }
    } else if (mGaveKnifeFork) {
        mGaveKnifeFork2 = 1;
    } else {
        mGaveKnifeFork = 1;
    }
    mGaveHere = 1;
}

// 800F6634
void franklin_c::relocate() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        if (dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x18)) {
            mRelocated = 1;
        }
    } else if (mTalked) {
        mRelocated = 1;
    }
    _1 = 0;
    mTalkedHere = 0;
    mGaveHere = 0;
}

// 800F66AC
void franklin_c::setSpot(const mVec3_c *center, f32 radius) {
    mVec3_c pos;
    pos.x = pos.y = pos.z = 0.0f;
    if (dAcNpc_c::searchSpot2(&pos, 4, center, 0x400, FALSE, TRUE, radius) ||
        dAcNpc_c::searchSpot2(&pos, 4, center, 0x400, FALSE, FALSE, radius)) {
        dSaveData_c::getTown()->mHarvestSpot.set(&pos);
        if (dAcNpc_c::isMultiPlay()) {
            dNet::beginPacket();
            u8 packet[2];
            packet[0] = worldToUnit(pos.x);
            packet[1] = worldToUnit(pos.z);
            dNet::addPacket(packet, sizeof(packet));
            dNet::sendPacket(NET_PACKET_HARVEST_SPOT, NET_MEMBER_NONE);
        }
    }
}

// 800F67B0
BOOL franklin_c::isTalked() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x18);
    }
    return mTalked;
}

// 800F67F8
pyontaro_c *pyontaro_c::get() {
    return &l_pyontaro;
}

// 800F6804
void pyontaro_c::clear() {
    mIntroduced = 0;
    mShovelOffered = 0;
    mBackTalks = 0;
    resetTalks();
}

// 800F6818
void pyontaro_c::resetTalks() {
    mFoilTalks = 0;
    mEggTalks = 0;
    mEggsLeftTalks = 0;
    mNoEggsTalks = 0;
}

// 800F6830
void pyontaro_c::setIntroduced() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x28);
    } else {
        mIntroduced = 1;
    }
    mFoilTalks = 0;
    mEggTalks = 0;
    mEggsLeftTalks = 0;
    mNoEggsTalks = 0;
}

// 800F6890
void pyontaro_c::setShovelOffered() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x3F);
    } else {
        mShovelOffered = 1;
    }
    mFoilTalks = 0;
    mEggTalks = 0;
    mEggsLeftTalks = 0;
    mNoEggsTalks = 0;
}

// 800F68F0
void pyontaro_c::incFoilTalks() {
    if (mFoilTalks < 2) {
        mFoilTalks++;
    }
    mEggTalks = 0;
    mEggsLeftTalks = 0;
    mNoEggsTalks = 0;
}

// 800F6918
void pyontaro_c::incEggTalks() {
    if (mEggTalks < 5) {
        mEggTalks++;
    }
    mFoilTalks = 0;
    mEggsLeftTalks = 0;
    mNoEggsTalks = 0;
}

// 800F6940
void pyontaro_c::incEggsLeftTalks() {
    if (mEggsLeftTalks < 4) {
        mEggsLeftTalks++;
    }
    mFoilTalks = 0;
    mEggTalks = 0;
    mNoEggsTalks = 0;
}

// 800F6968
void pyontaro_c::incNoEggsTalks() {
    if (mNoEggsTalks < 2) {
        mNoEggsTalks++;
    }
    mFoilTalks = 0;
    mEggTalks = 0;
    mEggsLeftTalks = 0;
}

// 800F6990
void pyontaro_c::incBackTalks() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        if (!dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x46)) {
            dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x46);
        } else {
            dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x47);
        }
    } else if (mBackTalks < 2) {
        mBackTalks++;
    }
    mFoilTalks = 0;
    mEggTalks = 0;
    mEggsLeftTalks = 0;
    mNoEggsTalks = 0;
}

// 800F6A20
int pyontaro_c::getBackTalks() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        if (dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x47)) {
            return 2;
        }
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x46);
    }
    return mBackTalks;
}

// 800F6A84
BOOL pyontaro_c::isIntroduced() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x28);
    }
    return mIntroduced;
}

// 800F6ACC
BOOL pyontaro_c::isShovelOffered() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x3F);
    }
    return mShovelOffered;
}

// 800F6B14
uomasa_c *uomasa_c::get() {
    return &l_uomasa;
}

// 800F6B1C
void uomasa_c::clear() {
    mJoined = 0;
    mTalkedBeforeStart = 0;
    mResultHeard = 0;
}

// 800F6B30
void uomasa_c::setJoined() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->setFlag1(PRIVATE_FLAG1_FISHING_JOINED);
    } else {
        mJoined = 1;
    }
}

// 800F6B7C
void uomasa_c::setTalkedBeforeStart() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->setFlag1(PRIVATE_FLAG1_FISHING_TALKED_BEFORE_START);
    } else {
        mTalkedBeforeStart = 1;
    }
}

// 800F6BC8
void uomasa_c::setResultHeard() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->setFlag1(PRIVATE_FLAG1_FISHING_RESULT_HEARD);
    } else {
        mResultHeard = 1;
    }
}

// 800F6C14
BOOL uomasa_c::isJoined() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(PRIVATE_FLAG1_FISHING_JOINED);
    }
    return mJoined;
}

// 800F6C5C
BOOL uomasa_c::isTalkedBeforeStart() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(PRIVATE_FLAG1_FISHING_TALKED_BEFORE_START);
    }
    return mTalkedBeforeStart;
}

// 800F6CA4
BOOL uomasa_c::isResultHeard() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(PRIVATE_FLAG1_FISHING_RESULT_HEARD);
    }
    return mResultHeard;
}

// 800F6CEC
bug_c *bug_c::get() {
    return &l_bug;
}

// 800F6CF4
void bug_c::clear() {
    mJoined = 0;
    mTalkedBeforeStart = 0;
    mResultHeard = 0;
    mJudged = 0;
}

// 800F6D0C
void bug_c::setJoined() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->setFlag1(PRIVATE_FLAG1_BUG_OFF_JOINED);
    } else {
        mJoined = 1;
    }
}

// 800F6D58
void bug_c::setTalkedBeforeStart() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->setFlag1(PRIVATE_FLAG1_BUG_OFF_TALKED_BEFORE_START);
    } else {
        mTalkedBeforeStart = 1;
    }
}

// 800F6DA4
void bug_c::setResultHeard() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->setFlag1(PRIVATE_FLAG1_BUG_OFF_RESULT_HEARD);
    } else {
        mResultHeard = 1;
    }
}

// 800F6DF0
void bug_c::setJudged() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->setFlag1(PRIVATE_FLAG1_BUG_OFF_JUDGED);
    } else {
        mJudged = 1;
    }
}

// 800F6E3C
BOOL bug_c::isJoined() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(PRIVATE_FLAG1_BUG_OFF_JOINED);
    }
    return mJoined;
}

// 800F6E84
BOOL bug_c::isTalkedBeforeStart() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(PRIVATE_FLAG1_BUG_OFF_TALKED_BEFORE_START);
    }
    return mTalkedBeforeStart;
}

// 800F6ECC
BOOL bug_c::isResultHeard() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(PRIVATE_FLAG1_BUG_OFF_RESULT_HEARD);
    }
    return mResultHeard;
}

// 800F6F14
BOOL bug_c::isJudged() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(PRIVATE_FLAG1_BUG_OFF_JUDGED);
    }
    return mJudged;
}

// 800F6F5C
pumpking_c *pumpking_c::get() {
    return &l_pumpking;
}

// 800F6F64
void pumpking_c::clear() {
    mNoCandyTold = 0;
    mCandyGiven = 0;
    mOtherGiven = 0;
    mRefused = 0;
}

// 800F6F7C
void pumpking_c::setNoCandyTold() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x14);
    } else {
        mNoCandyTold = 1;
    }
}

// 800F6FC8
void pumpking_c::setCandyGiven() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x15);
    } else {
        mCandyGiven = 1;
    }
}

// 800F7014
void pumpking_c::setOtherGiven() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x16);
    } else {
        mOtherGiven = 1;
    }
}

// 800F7060
void pumpking_c::setRefused() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x17);
    } else {
        mRefused = 1;
    }
}

// 800F70AC
BOOL pumpking_c::isNoCandyTold() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x14);
    }
    return mNoCandyTold;
}

// 800F70F4
BOOL pumpking_c::isCandyGiven() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x15);
    }
    return mCandyGiven;
}

// 800F713C
BOOL pumpking_c::isOtherGiven() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x16);
    }
    return mOtherGiven;
}

// 800F7184
BOOL pumpking_c::isRefused() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x17);
    }
    return mRefused;
}

// 800F71CC
void pumpking_c::setSpot(const mVec3_c *center, f32 radius) {
    mVec3_c pos;
    pos.x = pos.y = pos.z = 0.0f;
    if (dAcNpc_c::searchSpot(&pos, 4, center, radius) || dAcNpc_c::searchSpotAny(&pos, center, radius)) {
        int unitX = worldToUnit(pos.x);
        int unitZ = worldToUnit(pos.z);
        dSaveData_c::getTown()->mNpcSpotB.set(unitX, unitZ);
        if (dAcNpc_c::isMultiPlay()) {
            dNet::beginPacket();
            u8 packet[2];
            packet[0] = worldToUnit(pos.x);
            packet[1] = worldToUnit(pos.z);
            dNet::addPacket(packet, sizeof(packet));
            dNet::sendPacket(NET_PACKET_NPC_SPOT_B, NET_MEMBER_NONE);
        }
    }
}

// 800F72E8
cafe_c *cafe_c::get() {
    return &l_cafe;
}

// 800F72F0
void cafe_c::clear() {
    mTalked = 0;
    mServed = 0;
}

// 800F7300
soncho_c *soncho_c::get() {
    return &l_soncho;
}

// 800F730C
void soncho_c::clear() {
    mTalked = 0;
    mFullTold = 0;
    mTalkedAfter = 0;
    mPopperGiven = 0;
    mHfRegiveTold = 0;
    mHfLostAsked = 0;
    mHfGreeted = 0;
    mHfTalkedAfter = 0;
    mHfFullTold = 0;
    mHfTalkedBeforeStart = 0;
    mGiven = 0;
    mPresent = dItem::ITEM_ID_NONE;
}

// 800F734C
dItem::Item *soncho_c::getPresent() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return &dPlayerMgr_c::getCurrentPlayer()->mSonchoPresent;
    }
    return &mPresent;
}

// 800F7394
void soncho_c::setPresent(dItem::Item item) {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->mSonchoPresent = item;
    } else {
        mPresent = item;
    }
}

// 800F73F0
void soncho_c::setTalked() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x06);
    } else {
        mTalked = 1;
    }
}

// 800F743C
void soncho_c::setFullTold() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x0B);
    } else {
        mFullTold = 1;
    }
}

// 800F7488
void soncho_c::setTalkedAfter() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x0C);
    } else {
        mTalkedAfter = 1;
    }
}

// 800F74D4
void soncho_c::setPopperGiven() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x22);
    } else {
        mPopperGiven = 1;
    }
}

// 800F7520
void soncho_c::setHfRegiveTold() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x23);
    } else {
        mHfRegiveTold = 1;
    }
}

// 800F756C
void soncho_c::setHfLostAsked() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x24);
    } else {
        mHfLostAsked = 1;
    }
}

// 800F75B8
void soncho_c::setHfGreeted() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x25);
    } else {
        mHfGreeted = 1;
    }
}

// 800F7604
void soncho_c::setHfTalkedAfter() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x26);
    } else {
        mHfTalkedAfter = 1;
    }
}

// 800F7650
void soncho_c::setHfFullTold() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x41);
    } else {
        mHfFullTold = 1;
    }
}

// 800F769C
void soncho_c::setHfTalkedBeforeStart() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x42);
    } else {
        mHfTalkedBeforeStart = 1;
    }
}

// 800F76E8
void soncho_c::resetPopperGiven() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->clearFlag1(0x22);
    } else {
        mPopperGiven = 0;
    }
}

// 800F7734
void soncho_c::resetHfLostAsked() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->clearFlag1(0x24);
    } else {
        mHfLostAsked = 0;
    }
}

// 800F7780
void soncho_c::setGiven() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x07);
    } else {
        mGiven = 1;
    }
}

// 800F77CC
BOOL soncho_c::isTalked() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x06);
    }
    return mTalked;
}

// 800F7814
BOOL soncho_c::isFullTold() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x0B);
    }
    return mFullTold;
}

// 800F785C
BOOL soncho_c::isTalkedAfter() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x0C);
    }
    return mTalkedAfter;
}

// 800F78A4
BOOL soncho_c::isPopperGiven() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x22);
    }
    return mPopperGiven;
}

// 800F78EC
BOOL soncho_c::isHfRegiveTold() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x23);
    }
    return mHfRegiveTold;
}

// 800F7934
BOOL soncho_c::isHfLostAsked() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x24);
    }
    return mHfLostAsked;
}

// 800F797C
BOOL soncho_c::isHfGreeted() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x25);
    }
    return mHfGreeted;
}

// 800F79C4
BOOL soncho_c::isHfTalkedAfter() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x26);
    }
    return mHfTalkedAfter;
}

// 800F7A0C
BOOL soncho_c::isHfFullTold() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x41);
    }
    return mHfFullTold;
}

// 800F7A54
BOOL soncho_c::isHfTalkedBeforeStart() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x42);
    }
    return mHfTalkedBeforeStart;
}

// 800F7A9C
BOOL soncho_c::isGiven() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x07);
    }
    return mGiven;
}

// 800F7AE4
BOOL soncho_c::testTalked() {
    BOOL wasSet = isTalked();
    setTalked();
    return wasSet;
}

// 800F7B28
BOOL soncho_c::testFullTold() {
    BOOL wasSet = isFullTold();
    setFullTold();
    return wasSet;
}

// 800F7B6C
BOOL soncho_c::testTalkedAfter() {
    BOOL wasSet = isTalkedAfter();
    setTalkedAfter();
    return wasSet;
}

// 800F7BB0
BOOL soncho_c::testPopperGiven() {
    BOOL wasSet = isPopperGiven();
    setPopperGiven();
    return wasSet;
}

// 800F7BF4
BOOL soncho_c::testHfRegiveTold() {
    BOOL wasSet = isHfRegiveTold();
    setHfRegiveTold();
    return wasSet;
}

// 800F7C38
BOOL soncho_c::testHfTalkedAfter() {
    BOOL wasSet = isHfTalkedAfter();
    setHfTalkedAfter();
    return wasSet;
}

// 800F7C7C
BOOL soncho_c::testHfFullTold() {
    BOOL wasSet = isHfFullTold();
    setHfFullTold();
    return wasSet;
}

// 800F7CC0
BOOL soncho_c::testHfTalkedBeforeStart() {
    BOOL wasSet = isHfTalkedBeforeStart();
    setHfTalkedBeforeStart();
    return wasSet;
}

// 800F7D04
gatekeeperB_c *gatekeeperB_c::get() {
    return &l_gatekeeperB;
}

// 800F7D0C
void gatekeeperB_c::clear() {
    mGreeted = 0;
}

// 800F7D18
kappei_c *kappei_c::get() {
    return &l_kappei;
}

// 800F7D20
void kappei_c::clear() {
    mLyleTalked = 0;
}

// 800F7D2C
resetsan_c *resetsan_c::get() {
    return &l_resetsan;
}

// 800F7D38
void resetsan_c::init() {
    mVisit = 0;
    mSorryType = -1;
}

// 800F7D4C
void resetsan_c::set(int visit) {
    mVisit = visit;
}

// 800F7D54
int resetsan_c::getVisit() {
    int visit = 0;
    if (l_resetsanEnabled) {
        visit = mVisit;
    }
    return visit;
}

// 800F7D70
goddess_c *goddess_c::get() {
    return &l_goddess;
}

// 800F7D78
void goddess_c::clear() {
    mItem = dItem::ITEM_ID_NONE;
}

// 800F7D88
void goddess_c::setItem(dItem::Item item) {
    mItem = item;
}

// 800F7D94
dItem::Item goddess_c::getItem() {
    return mItem;
}

// 800F7DA0
dancer_c *dancer_c::get() {
    return &l_dancer;
}

// 800F7DAC
void dancer_c::clear() {
    mTalked = 0;
    mCandyAsked[0] = 0;
    mCandyAsked[1] = 0;
    mCandyAsked[2] = 0;
    mCandyAsked[3] = 0;
    resetEncounter();
}

// 800F7DC8
BOOL dancer_c::isTalked() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x2B);
    }
    return mTalked;
}

// 800F7E10
void dancer_c::setTalked() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x2B);
    } else {
        mTalked = 1;
    }
}

// 800F7E5C
BOOL dancer_c::isCandyAsked() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        switch (dSaveData_c::getTown()->mDailyRand.mValue) {
        case 0:
            return dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x2C);
        case 1:
            return dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x2D);
        case 2:
            return dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x2E);
        case 3:
            return dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x2F);
        default:
            return FALSE;
        }
    }
    return mCandyAsked[dSaveData_c::getTown()->mDailyRand.mValue];
}

// 800F7F20
void dancer_c::setCandyAsked() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        switch (dSaveData_c::getTown()->mDailyRand.mValue) {
        case 0:
            dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x2C);
            break;
        case 1:
            dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x2D);
            break;
        case 2:
            dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x2E);
            break;
        case 3:
            dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x2F);
            break;
        }
    } else {
        int color = dSaveData_c::getTown()->mDailyRand.mValue;
        mCandyAsked[color] = 1;
    }
}

// 800F7FE0
void dancer_c::resetEncounter() {
    mNoCandyTold = 0;
    mWrongCandyTold = 0;
    mTooFewTold = 0;
    mGiftGiven = 0;
}

// 800F7FF8
void dancer_c::resetCandyHints() {
    mNoCandyTold = 0;
    mWrongCandyTold = 0;
    mTooFewTold = 0;
}

// 800F800C
totakeke_c *totakeke_c::get() {
    return &l_totakeke;
}

// 800F8018
void totakeke_c::clear() {
    mTalked = 0;
    mRequested = 0;
    resetSong();
    mHadAudience = 0;
    mPlayReq = 0;
    mReceived = 0;
    mIsRequester = 0;
    mWorkWarned = 0;
    _05 = 0;
    mPerforming = 0;
}

// 800F8074
void totakeke_c::resetSong() {
    mRequestKind = 8;
    mSong = dItem::ITEM_ID_NONE;
    mRequesterNo = NET_MEMBER_NONE;
    memset(mRequestTitle, 0, sizeof(mRequestTitle));
    mUnknownTitle = 0;
    mAudienceNum = 0;
    mAudienceDoneNum = 0;
    mPlayReq = 0;
    mReceived = 0;
    mInAudience = 0;
}

// 800F80E4
void totakeke_c::setTalked() {
    if (!get()->isMet()) {
        _05 = 1;
    }
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x30);
    } else {
        mTalked = 1;
    }
}

// 800F8148
BOOL totakeke_c::isMet() {
    return dPlayerMgr_c::getCurrentPlayerRaw()->isFlag0(0x4D);
}

// 800F8170
BOOL totakeke_c::isTalked() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x30);
    }
    return mTalked;
}

// 800F81B8
void totakeke_c::start(int kind, u16 song, const void *request) {
    resetSong();
    dItem::Item requestedSong(song);
    mRequestKind = kind;
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x31);
    } else {
        mRequested = 1;
    }
    int from = dItem::FROM_COUNT;
    switch (get()->mRequestKind) {
    case 0:
        from = dItem::FROM_MUSIC_GOKIGEN;
        break;
    case 1:
        from = dItem::FROM_MUSIC_FUKIGEN;
        break;
    case 2:
        from = dItem::FROM_MUSIC_MATTARI;
        break;
    case 3:
        from = dItem::FROM_MUSIC_BLUE;
        break;
    case 4:
        from = dItem::FROM_MUSIC_UNKNOWN;
        break;
    case 7:
        if (requestedSong.isValid() && requestedSong.getFrom() != dItem::FROM_MUSIC_HAZURE) {
            mSong = requestedSong;
            mUnknownTitle = 0;
            break;
        }
        memcpy(mRequestTitle, request, sizeof(mRequestTitle));
        from = dItem::FROM_MUSIC_HAZURE;
        mUnknownTitle = 1;
        break;
    case 8:
        break;
    }
    if (!mSong.isValid()) {
        // Prefer a song the player does not own.
        u8 *ownedSongs = dPlayerMgr_c::getCurrentPlayer()->mFlags2;
        if (from == dItem::FROM_MUSIC_HAZURE) {
            ownedSongs = NULL;
        }
        if (from == dItem::FROM_COUNT) {
            dItem::musicCandCB_c cb(ownedSongs);
            cb.mExcludeFrom = dItem::FROM_MUSIC_SECRET;
            cb.mFrom[0] = dItem::FROM_MUSIC_GOKIGEN;
            cb.mFrom[1] = dItem::FROM_MUSIC_FUKIGEN;
            cb.mFrom[2] = dItem::FROM_MUSIC_MATTARI;
            cb.mFrom[3] = dItem::FROM_MUSIC_BLUE;
            cb.mFrom[4] = dItem::FROM_MUSIC_UNKNOWN;
            dItem::seeker_c::get()->search(dItem::KIND_MUSIC, 6, &cb);
        } else {
            dItem::musicCandCB_c cb(ownedSongs);
            cb.mExcludeFrom = dItem::FROM_MUSIC_SECRET;
            cb.mFrom[0] = from;
            cb.mFrom[1] = dItem::FROM_COUNT;
            cb.mFrom[2] = dItem::FROM_COUNT;
            cb.mFrom[3] = dItem::FROM_COUNT;
            cb.mFrom[4] = dItem::FROM_COUNT;
            dItem::seeker_c::get()->search(dItem::KIND_MUSIC, 6, &cb);
        }
        if (dItem::seeker_c::get()->mCount != 0) {
            mSong = dItem::seeker_c::get()->getRandom();
        } else {
            // Retry the requested mood without excluding owned songs.
            if (from == dItem::FROM_COUNT) {
                dItem::musicCandCB_c cb(NULL);
                cb.mExcludeFrom = dItem::FROM_MUSIC_SECRET;
                cb.mFrom[0] = dItem::FROM_MUSIC_GOKIGEN;
                cb.mFrom[1] = dItem::FROM_MUSIC_FUKIGEN;
                cb.mFrom[2] = dItem::FROM_MUSIC_MATTARI;
                cb.mFrom[3] = dItem::FROM_MUSIC_BLUE;
                cb.mFrom[4] = dItem::FROM_MUSIC_UNKNOWN;
                dItem::seeker_c::get()->search(dItem::KIND_MUSIC, 6, &cb);
            } else {
                dItem::musicCandCB_c cb(NULL);
                cb.mExcludeFrom = dItem::FROM_MUSIC_SECRET;
                cb.mFrom[0] = from;
                cb.mFrom[1] = dItem::FROM_COUNT;
                cb.mFrom[2] = dItem::FROM_COUNT;
                cb.mFrom[3] = dItem::FROM_COUNT;
                cb.mFrom[4] = dItem::FROM_COUNT;
                dItem::seeker_c::get()->search(dItem::KIND_MUSIC, 6, &cb);
            }
            if (dItem::seeker_c::get()->mCount != 0) {
                mSong = dItem::seeker_c::get()->getRandom();
            } else {
                // Fall back to any of the regular moods.
                dItem::musicCandCB_c cb(NULL);
                cb.mExcludeFrom = dItem::FROM_MUSIC_SECRET;
                cb.mFrom[0] = dItem::FROM_MUSIC_GOKIGEN;
                cb.mFrom[1] = dItem::FROM_MUSIC_FUKIGEN;
                cb.mFrom[2] = dItem::FROM_MUSIC_MATTARI;
                cb.mFrom[3] = dItem::FROM_MUSIC_BLUE;
                cb.mFrom[4] = dItem::FROM_MUSIC_UNKNOWN;
                dItem::seeker_c::get()->search(dItem::KIND_MUSIC, 6, &cb);
                mSong = dItem::seeker_c::get()->getRandom();
            }
        }
    }
    mRequesterNo = dNet::getMyNo();
    setPlaying(TRUE);
    mIsRequester = 1;
    mHadAudience = 0;
    mAudienceNum = 0;
    if (dAcNpc_c::isMultiPlay()) {
        dNet::beginPacket();
        dNet::sendPacket(NET_PACKET_NPC_EVENT_START, NET_MEMBER_NONE);
    }
}

// 800F8554
void totakeke_c::sendSong() {
    if (dAcNpc_c::isMultiPlay()) {
        if (mIsRequester) {
            mPerforming = 1;
            dNet::beginPacket();
            totakekePacket_c packet;
            toPacket(&packet);
            dNet::addPacket(&packet, sizeof(packet));
            dNet::sendPacket(NET_PACKET_NPC_EVENT_DATA, NET_MEMBER_NONE);
        } else {
            mHadAudience = 1;
        }
    }
    totakeke_c *info = get();
    info->mInAudience = 1;
}

// 800F85DC
void totakeke_c::stop() {
    if (dAcNpc_c::isMultiPlay()) {
        if (mIsRequester) {
            setPlaying(FALSE);
            mPerforming = 0;
            dNet::beginPacket();
            dNet::sendPacket(NET_PACKET_NPC_EVENT_END, NET_MEMBER_NONE);
        } else if (get()->mInAudience) {
            dNet::beginPacket();
            dNet::sendPacket(NET_PACKET_NPC_EVENT_LEAVE, mRequesterNo);
        }
    } else {
        setPlaying(FALSE);
        mPerforming = 0;
    }
    resetSong();
    mIsRequester = 0;
    mHadAudience = 0;
}

// 800F8690
BOOL totakeke_c::isRequested() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x31);
    }
    return mRequested;
}

// 800F86D8
BOOL totakeke_c::isPlaying() {
    return dSaveData_c::getRaw()->isFlag(0x14);
}

// 800F8700
void totakeke_c::setPlaying(BOOL on) {
    if (on) {
        dSaveData_c::getTown()->setFlag(0x14);
    } else {
        dSaveData_c::getTown()->clearFlag(0x14);
    }
}

// 800F8740
BOOL totakeke_c::isOnStage() {
    if (fn_800FBC48(4) == NULL) {
        return FALSE;
    }
    if (fn_800FD664() && dScene::getCurrentScene() == dScene::SCENE_RM_MM_CAFE && fn_800FBC48(4)->z >= 352.0f) {
        return TRUE;
    }
    return FALSE;
}

// 800F87B8
void totakeke_c::fromPacket(const totakekePacket_c *packet) {
    memcpy(mRequestTitle, packet->mRequestTitle, sizeof(mRequestTitle));
    mSong = packet->mSong;
    mRequesterNo = packet->mRequesterNo;
    mRequestKind = packet->mRequestKind;
    mUnknownTitle = packet->mUnknownTitle != 0;
    mPerforming = packet->mPerforming != 0;
    mReceived = 1;
}

// 800F8840
void totakeke_c::toPacket(totakekePacket_c *packet) {
    memcpy(packet->mRequestTitle, mRequestTitle, sizeof(packet->mRequestTitle));
    packet->mSong = mSong.getId();
    packet->mRequesterNo = mRequesterNo;
    packet->mRequestKind = mRequestKind;
    packet->mUnknownTitle = mUnknownTitle;
    packet->mPerforming = mPerforming;
}

#define SP_NPC_NUM 0x61

// 80475418: STR_SPNpc_name string ids.
static const u16 l_spNameStr[SP_NPC_NUM] = {
    0x0001, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000A, 0x000B, 0x000C,
    0x000D, 0x000E, 0x000F, 0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019,
    0x001A, 0x002E, 0x001C, 0x001D, 0x001E, 0x001F, 0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026,
    0x0027, 0x0002, 0x000C, 0x0001, 0x0028, 0x000D, 0x000D, 0x000D, 0x000D, 0x000D, 0x000D, 0x000D, 0x000D,
    0x000D, 0x000D, 0x000D, 0x000D, 0x000D, 0x000D, 0x000D, 0x000D, 0x000D, 0x000D, 0x000D, 0x000D, 0x000D,
    0x000D, 0x000D, 0x000D, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x001B, 0x0029, 0x002A, 0x002B,
    0x002C, 0x002D, 0x0011, 0x0012, 0x0024, 0x0024, 0x0024, 0x0024, 0x0024, 0x0024, 0x0024, 0x0024, 0x0024,
    0x0024, 0x0011, 0x0012, 0x002E, 0x0033, 0x000D,
};

// 804754DC: sound ids.
static const u16 l_spSound[SP_NPC_NUM] = {
    0x0001, 0x0BB9, 0x0DE9, 0x0C31, 0x0DAD, 0x0D0D, 0x158B, 0x0C1D, 0x0C6D, 0x0C59, 0x0D35, 0x0D49, 0xFFFF,
    0x0E39, 0x0D21, 0x0CA9, 0x0C95, 0x0CD1, 0x1577, 0x0CBD, 0x0E75, 0x0E11, 0x0E89, 0x0D5D, 0x0CF9, 0x0C81,
    0x0CE5, 0x1563, 0x0E25, 0x0BE1, 0x15B3, 0x0C45, 0x0E61, 0x162B, 0x0BCD, 0x0D85, 0x159F, 0x154F, 0x0D99,
    0x0D71, 0x0DE9, 0x0E9D, 0xFFFF, 0x1603, 0x0E4D, 0x0E4D, 0x0E4D, 0x0E4D, 0x0E4D, 0x0E4D, 0x0E4D, 0x0E4D,
    0x0E4D, 0x0E4D, 0x0E4D, 0x0E4D, 0x0E4D, 0x0E4D, 0x0E4D, 0x0E4D, 0x0E4D, 0x0E4D, 0x0E4D, 0x0E4D, 0x0E4D,
    0x0E4D, 0x0E4D, 0x0E4D, 0x0DC1, 0x0DFD, 0x0DD5, 0x0DC1, 0x0DFD, 0x0DD5, 0x1653, 0x1667, 0x15DB, 0x163F,
    0x1617, 0x15EF, 0x0CD1, 0x1577, 0x159F, 0x159F, 0x159F, 0x159F, 0x159F, 0x159F, 0x159F, 0x159F, 0x159F,
    0x159F, 0x0CD1, 0x1577, 0x1563, 0x15C7, 0x0E4D,
};

// 804755A0: voice types.
static const int l_spVoice[SP_NPC_NUM] = {
    2, 3, 0, 0, 1, 0, 2, 0, 1, 1, 1, 1, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0, 2, 2, 0, 1, 1, 0, 2, 0, 2, 1, 0,
    2, 1, 0, 0, 2, 0, 2, 0, 0, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0,
};

// 800F88AC
u16 dNpcInfo::getSpNameStr(const dItem::Item *npc) {
    u16 id = npc->getId();
    u16 nameStr = 0;
    if (ITEM_NAME_TYPE(id) == 8) {
        u32 index = ITEM_NAME_INDEX(id);
        if (index < SP_NPC_NUM) {
            nameStr = l_spNameStr[index];
        }
    }
    return nameStr;
}

// 800F88E0
int dNpcInfo::getSpSound(const dItem::Item *npc) {
    u16 id = npc->getId();
    int sound = 0xFF;
    if (ITEM_NAME_TYPE(id) == 8) {
        u32 index = ITEM_NAME_INDEX(id);
        if (index < SP_NPC_NUM) {
            sound = l_spSound[index];
        }
    }
    return sound;
}

// 800F8914
int dNpcInfo::getSpVoice(const dItem::Item *npc) {
    u16 id = npc->getId();
    int voice = 2;
    if (ITEM_NAME_TYPE(id) == 8) {
        u32 index = ITEM_NAME_INDEX(id);
        if (index < SP_NPC_NUM) {
            voice = l_spVoice[index];
        }
    }
    return voice;
}

// 800F8948
BOOL dNpcInfo::isSameFashion(const dItem::Item *a, const dItem::Item *b) {
    if (!a->isValid()) {
        return FALSE;
    }
    if (!b->isValid()) {
        return FALSE;
    }
    int fashionA = dItem::infoBank_c::get()->getBITM(*a)->getFashion();
    int fashionB = dItem::infoBank_c::get()->getBITM(*b)->getFashion();
    if (fashionA == dItem::FASHION_NONE) {
        return FALSE;
    }
    if (fashionB == dItem::FASHION_NONE) {
        return FALSE;
    }
    return fashionA == fashionB;
}

// 800F8A20
BOOL dNpcInfo::isPlayerFashionMatching() {
    if (isSameFashion(&dPlayerMgr_c::getCurrentPlayer()->mEquipment.mShirt,
                      &dPlayerMgr_c::getCurrentPlayer()->mEquipment.mHat)) {
        return TRUE;
    }
    return isSameFashion(&dPlayerMgr_c::getCurrentPlayer()->mEquipment.mShirt,
                         &dPlayerMgr_c::getCurrentPlayer()->mEquipment.mAcc);
}

// 800F8A84
gatekeeperA_c *gatekeeperA_c::get() {
    return &l_gatekeeperA;
}

// 800F8A8C
void gatekeeperA_c::clear() {
    mEscorting = 0;
    mAtGate = 0;
}

// 800F8A9C
kaburiba_c *kaburiba_c::get() {
    return &l_kaburiba;
}

// 800F8AA4
void kaburiba_c::clear() {
    mTalked = 0;
}

// 800F8AB0
void kaburiba_c::setTalked() {
    dPlayerMgr_c::getCurrentPlayer()->setFlag0(PRIVATE_FLAG0_JOAN_MET);
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x00);
    } else {
        mTalked = 1;
    }
}

// 800F8B08
BOOL kaburiba_c::isTalked() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x00);
    }
    return mTalked;
}

// 800F8B50
fuko_c *fuko_c::get() {
    return &l_fuko;
}

// 800F8B58
void fuko_c::clear() {
    mTalked = 0;
}

// 800F8B64
void fuko_c::setTalked() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        dPlayerMgr_c::getCurrentPlayer()->setFlag1(0x02);
    } else {
        mTalked = 1;
    }
}

// 800F8BB0
BOOL fuko_c::isTalked() {
    if (dAcNpc_c::isPlayerFromThisTown()) {
        return dPlayerMgr_c::getCurrentPlayer()->isFlag1(0x02);
    }
    return mTalked;
}

// 800F8BF8
futa_c *futa_c::get() {
    return &l_futa;
}

// 800F8C00
void futa_c::clear() {
    mGreeted = 0;
}

// 800F8C0C
void futa_c::setGreeted(u8 greeted) {
    mGreeted = greeted;
}

// 800F8C14
u8 futa_c::isGreeted() {
    return mGreeted;
}
