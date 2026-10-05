// Villager (animal) save data TU. Draft: only the class anchors are written so far.
// .text 8011B400..80135E88 (sinit fn_80135E40). See notes/d_animal.txt.
#include <game/game/d_event.hpp>
#include <game/game/d_animal.hpp>
#include <game/sLib/s_crc.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_string.hpp>
#include <lib/egg/core/eggHeap.h>
#include <game/game/d_date.hpp>
#include <game/game/d_catalog.hpp>
#include <cstring>
#include <cstddef>

// Dependencies whose owners are not recovered yet.
extern "C" {
// Member objects owned by other TUs.
void fn_801192F4(dUnk300C_c *obj); // clear

// Memory-kind helpers.
u32 fn_80162548();
BOOL fn_80162594(u8 kind, int);

}

// Later functions of this TU, not written yet (C linkage keeps the target names).


struct dEventId_c {
    dEventId_c(int id) : mId(id) {}
    int mId;
};

typedef void (*dAnimalEventFunc)(dAnimal_c *animals, int num, s8 *buf, int a, int b);

struct dAnimalEvent_c {
    u8 mEvent;
    dAnimalEventFunc mFunc;
};

// Callees in other TUs declared with differing signatures by the first-pass chunks.
extern "C" {
int fn_80091370(dTime_c *time, int, int, int);
int fn_800BD6A8(dTime_c *time, int, int, int);
int fn_800C60B4(dItem::Item *out, int num, const void *table, int tableNum, const void *filter, const dItem::Item *exclude, int excludeNum, int);
BOOL fn_800DCEDC();
u16 fn_800FABF4(int looks, int season);
dDesign_c *fn_80147F80(dDesign_c *designs, int idx);
BOOL fn_8014B0F0(void *fg, int *x, int *z, dItem::Item item, int);
}

// Free functions of this TU.
int getMinCountMask(u32 *mask, const u32 *counts, u32 num, u32 skipMask, BOOL useSkip);
int getRarestLooksMask(u32 *mask, dAnimal_c *animals, u32 num, u32 skipMask, BOOL useSkip);
BOOL isBitSet(u16 idx, const u8 *bits, u32 size, u16 max);
dAnimalTemplate_c *pickTemplateByLooks(dAnimalTemplate_c *tmpls, u32 num, u32 looks, dAnimal_c *animals,
                                           u32 animalNum, const u8 *bits, u32 size, u32 max, u8 flag);
dAnimalTemplate_c *pickTemplate(dAnimalTemplate_c *tmpls, u32 num, u32 mask, u32 count, u32 looksNum,
                                           dAnimal_c *animals, u32 animalNum, const u8 *bits, u32 size, u32 max,
                                           u8 flag);
dAnimal_c *fn_8011B910(dAnimal_c *animals, u32 num, u32 idx);
dAnimal_c *fn_8011B92C(dAnimal_c *animals, u32 num, u32 idx);
int findFreeAnimalIdx(dAnimal_c *animals, u32 num, BOOL flag);
dAnimal_c *getFreeAnimal(dAnimal_c *animals, u32 num, BOOL flag);
u32 countAnimals(dAnimal_c *animals, u32 num);
dAnimal_c *fn_8011BA8C(u16 npcIdx, dAnimal_c *animals, u32 num);
dAnimal_c *fn_8011BB0C(u16 npcIdx, dAnimal_c *animals, u32 num);
int findAnimalIdx(dAnmPersonalID_c *id, dAnimal_c *animals, u32 num);
int pickAnimalIdx(dAnmPersonalID_c **exclude, u32 numExclude, dAnimal_c *animals, u32 num, BOOL flag);
void initHarvestFestival(dAnimal_c *animals, u32 num);
void initHalloween(dAnimal_c *animals, u32 num, const s8 *order, u32 want, u32 special);
void initFishingTourney(dAnimal_c *animals, u32 num);
void initBugOff(dAnimal_c *animals, u32 num);
void initCountdown(dAnimal_c *animals, u32 num);
void initFireworks(dAnimal_c *animals, u32 num);
void initFleaMarket(dAnimal_c *animals, u32 num);
void initFestivale(dAnimal_c *animals, u32 num);
void initToyDay(dAnimal_c *animals, u32 num);
BOOL getOwnHouseSpot(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block);
BOOL getOtherHouseSpot(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block);
BOOL getPlayerHouseSpot(int *x, int *z, u32 player);
BOOL getCurPlayerHouseSpot(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block);
BOOL getOtherPlayerHouseSpot(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block);
BOOL getStructSpot(int *x, int *z, const dItem::Item *item);
BOOL getStructSpotD013(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block);
BOOL D016(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block);
BOOL D017(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block);
BOOL D015(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block);
BOOL D014(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block);
BOOL getBlockTypeSpot(int *x, int *z, u32 type);
BOOL getBlockSpot0FE00000(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block);
BOOL getBottomRowSpot(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block);
BOOL fn_8011DD5C(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block);
BOOL fn_8011DD64(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block);
BOOL FE000(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block);
BOOL getRandomSpot(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block);
BOOL isMovedOutPlayer(const dPersonalID_c *pid, const dLandID_c *land, dPrivateData_c *players);
BOOL isOtherTownPlayer(const dPersonalID_c *pid, const dLandID_c *land, dPrivateData_c *players);
void fn_8011F19C(dScript::Word_c *out, dScript::Word_c *def, BOOL flag);
dItem::Item getTemplateFtr(u32 idx, const dAnimalTemplate_c *tmpl);
u32 getFtrSlotOfPlaceholder(const dItem::Item *item);
BOOL isHouseItem(const dItem::Item *item);
u32 getRoomFtrSlot(const dItem::Item *room, u32 x, u32 z);
BOOL getFtrSlotRange(int *start, int *end, u32 size, BOOL wish);
int getFossilSlot(const dItem::Item *item, int start, int end, BOOL wish);
int getFossilSet(const dItem::Item *item);
dItem::Item getMovingBox(const dItem::Item *item);
BOOL isNewItemKind(const dItem::Item *item);
BOOL isOrgCloth(const dItem::Item *item);
BOOL isFossilRequestMatch(const dItem::Item *item, int mode, const dItem::Item *other);
BOOL isNoneOrRoyalItem(const dItem::Item *item);
BOOL isHoldableItem(const dItem::Item *item);
u8 calcHeldItemChangeMinute(int minute, BOOL soon);
dItem::Item pickFancyUmbrella();
BOOL isFancyUmbrella(const dItem::Item *item);
dItem::Item pickRandomUmbrella();
dDesign_c *pickTownDesign(int style, u32 sameStyle);
dAnimal_c *findAnimalByCompat(dAnimal_c *animals, dAnimal_c *animal, BOOL (*cmp)(u32, u32), u32 best);
BOOL isCompatHigher(u32 best, u32 value);
BOOL isCompatLower(u32 best, u32 value);
dAnimal_c *getBestCompatAnimal(dAnimal_c *animals, dAnimal_c *animal);
dAnimal_c *getWorstCompatAnimal(dAnimal_c *animals, dAnimal_c *animal);
void clearIdxList(s8 *buf, u32 num);
BOOL hasIdxInList(const s8 *buf, u32 num);
int findIdxInList(s8 idx, const s8 *buf, u32 num);
u32 halfRoundUp(u32 n);
const dAnimalEvent_c *findTodayEvent();
const dAnimalEvent_c *findEventOnDay(const dTime_c *time, u32 arg);
int pickRandomWish(int exclude);
dPrivateData_c *findPlayerByID(dPrivateData_c *players, const dPlayerID_c *pid);
dItem::Item *allocRoomItems(u32 *outNum, dPrivateData_c *player, EGG::Heap *heap);
dItem::Item pickAppointmentPresent(dPrivateData_c *player, u32 kind, BOOL flag);
BOOL sendPresentLetter(const char *label, const dPersonalID_c *to, dAnmPersonalID_c *id,
                            const dItem::Item *present);
BOOL sendQ8Letter(const dPersonalID_c *to, dAnmPersonalID_c *id, const dItem::Item *present);
int pickAppointmentPresent2(dItem::Item *out, dPrivateData_c *player, dAnimal_c *animal, BOOL flag);
BOOL sendQ9Letter(const dPersonalID_c *to, dAnmPersonalID_c *id, const dItem::Item *present);
BOOL sendQ10Letter(const dPersonalID_c *to, dAnmPersonalID_c *id, const dItem::Item *present, BOOL flag);
BOOL isAnimalIdx(u32 idx);
void clearNpcIdxList(u16 *buf, u32 num);
int getBirthdayHostIdxIn(dAnimal_c *animals, u32 num, int playerNo, BOOL checkEvent, BOOL checkFlag,
                           BOOL checkPocket);
BOOL isBirthdayHostIn(const dAnmPersonalID_c *id, dAnimal_c *animals, u32 num, int playerNo,
                            BOOL checkEvent, BOOL checkFlag, BOOL checkPocket);
BOOL fn_80134D54(const dPrivateData_c *player);
BOOL fn_80134DA8(const dPrivateData_c *player);
BOOL isHoldingNet(const dPrivateData_c *player);
BOOL isHoldingFishingrod(const dPrivateData_c *player);
BOOL isHoldingWatering(const dPrivateData_c *player);
BOOL isHoldingAxe(const dPrivateData_c *player);
BOOL isWearingKingOutfit(const dPrivateData_c *player);
BOOL isWearingRoyalCrown(const dPrivateData_c *player);
BOOL isWearingCrown(const dPrivateData_c *player);
BOOL isWearingChefsHat(const dPrivateData_c *player);
BOOL isWearingBridalVeil(const dPrivateData_c *player);
BOOL isWearingSwimCapGoggles(const dPrivateData_c *player);
BOOL isWearingOutbackHat(const dPrivateData_c *player);
BOOL isWearingHalo(const dPrivateData_c *player);
BOOL isWearingJesterCap(const dPrivateData_c *player);
BOOL isWearingWitchHat(const dPrivateData_c *player);
BOOL isWearingBabyOutfit(const dPrivateData_c *player);
BOOL isWearingAfroWig(const dPrivateData_c *player);
BOOL isWearingBunnyHood(const dPrivateData_c *player);
BOOL isWearingGeishaWig(const dPrivateData_c *player);
BOOL isWearingSamuraiWig(const dPrivateData_c *player);
BOOL isWearingKingTutMask(const dPrivateData_c *player);
BOOL isWearingNinjaHood(const dPrivateData_c *player);
BOOL isWearingWrestlingMask(const dPrivateData_c *player);
BOOL isWearingDressing(const dPrivateData_c *player);
BOOL isWearingMohawkWig(const dPrivateData_c *player);
BOOL isWearingRegentWig(const dPrivateData_c *player);
BOOL isRaining(const dPrivateData_c *player);
BOOL isSnowing(const dPrivateData_c *player);
BOOL isMorning(const dPrivateData_c *player);
BOOL isLateNight(const dPrivateData_c *player);
BOOL isInsectCatalogComplete(const dPrivateData_c *player);
BOOL isFishCatalogComplete(const dPrivateData_c *player);
BOOL isFullyTanned(const dPrivateData_c *player);
BOOL hasNoPocketMoney(const dPrivateData_c *player);
BOOL hasSavings100k(const dPrivateData_c *player);
BOOL isNever(const dPrivateData_c *player);
BOOL fn_8013585C(const dPrivateData_c *player);
BOOL fn_80135908(const dPrivateData_c *player);
BOOL fn_80135948(const dPrivateData_c *player);
BOOL hasFemaleHairNoHat(const dPrivateData_c *player);
BOOL fn_801359DC(const dPrivateData_c *player);
BOOL fn_80135A40(const dPrivateData_c *player);
BOOL hasMaleHairNoHat(const dPrivateData_c *player);
int getImpression(dPrivateData_c *player);
BOOL sendVisitorLetter(dPrivateData_c *player);

// Chunk 8 passes event ids through a struct so each call gets its own stack temporary.
static inline BOOL isEventActive(const dEventId_c &event) {
    return dEvent::isActive(reinterpret_cast<const dQuestEvent_e &>(event));
}

static inline BOOL isEventOngoing(const dEventId_c &event) {
    return dEvent::isOngoing(reinterpret_cast<const dQuestEvent_e &>(event));
}

static inline BOOL isEventOver(const dEventId_c &event) {
    return dEvent::isOver(reinterpret_cast<const dQuestEvent_e &>(event));
}

// 805F15E8; constructed by __sinit (80135E40), first used by writeReplyLetter.
static dMail_c sMail;

// 8011B400
dGreetingWord_c::dGreetingWord_c() {
    clear();
}

// 8011B444
dGreetingWord_c::~dGreetingWord_c() {}

// 8011B49C
u32 dGreetingWord_c::getBufferSize() {
    return sizeof(mBuffer);
}

// 8011B4A4
wchar_t *dGreetingWord_c::getBuffer() {
    return mBuffer;
}

// 8011B4AC
dHabitWord_c::dHabitWord_c() {
    clear();
}

// 8011B4F0
dHabitWord_c::~dHabitWord_c() {}

// 8011B548
u32 dHabitWord_c::getBufferSize() {
    return sizeof(mBuffer);
}

// 8011B550
wchar_t *dHabitWord_c::getBuffer() {
    return mBuffer;
}

// 8011B558
// Finds the smallest counts. Sets a bit in *mask for each index tied at the
// minimum and returns how many there are. Indices set in skipMask are ignored
// when useSkip is nonzero.
int getMinCountMask(u32 *mask, const u32 *counts, u32 num, u32 skipMask, BOOL useSkip) {
    int found = 0;
    u32 min = 0xFFFFFFFF;
    *mask = 0;

    for (u32 i = 0; i < num; i++, counts++) {
        if (skipMask != 0 && useSkip != 0 && ((skipMask >> i) & 1)) {
            continue;
        }

        if (*counts < min) {
            min = *counts;
            *mask = 1 << i;
            found = 1;
        } else if (*counts == min) {
            *mask |= 1 << i;
            found++;
        }
    }

    return found;
}

// 8011B5E0
// Least common personality among the given villagers.
int getRarestLooksMask(u32 *mask, dAnimal_c *animals, u32 num, u32 skipMask, BOOL useSkip) {
    u32 counts[LOOKS_TYPE_NUM];
    memset(counts, 0, sizeof(counts));

    dAnmPersonalID_c *id;
    for (u32 i = 0; i < num; i++, animals++) {
        id = &animals->mID;
        if (id->isValid() && id->getLooks(1) < LOOKS_TYPE_NUM) {
            counts[id->getLooks(1)]++;
        }
    }

    return getMinCountMask(mask, counts, LOOKS_TYPE_NUM, skipMask, useSkip);
}

// Temporary declarations (chunk 0)
extern "C" {
// This TU, other chunks.

// This TU, chunk 0 (used before their definition).

// Other TUs.
BOOL fn_8014A034(void *fg, int *x, int *z, u32 player);
dNpcFieldMap_c *fn_80190C44(int outdoor);
BOOL fn_8008C39C(dNpcFieldMap_c *map, int bx, int bz, u32 type);
}
BOOL isFurnitureKind(int kind);

// 8011B6AC
// Bit idx of a bit array, if idx <= max.
BOOL isBitSet(u16 idx, const u8 *bits, u32 size, u16 max) {
    if (bits == NULL || size == 0) {
        return FALSE;
    }

    if (idx <= max) {
        u32 byte = idx >> 3;
        u32 bit = idx & 7;
        if (byte < size) {
            return (bits[byte] >> bit) & 1;
        }
    }
    return FALSE;
}

// 8011B6F4
// Random template of the given personality that is not in town and not in the bit array.
dAnimalTemplate_c *pickTemplateByLooks(dAnimalTemplate_c *tmpls, u32 num, u32 looks, dAnimal_c *animals,
                                           u32 animalNum, const u8 *bits, u32 size, u32 max, u8 flag) {
    dAnimalTemplate_c *result = NULL;
    u32 count = 0;

    for (u32 i = 0; i < num; i++, tmpls++) {
        if (looks != tmpls->mLooks) {
            continue;
        }

        u16 npcIdx = (int)tmpls->mNpcIdx;
        if (fn_8011BA8C(npcIdx, animals, animalNum) != NULL) {
            continue;
        }

        if (flag && !tmpls->mIsStarter) {
            continue;
        }

        if (!isBitSet(npcIdx, bits, size, max)) {
            f32 chance = 100.0f / (count + 1);
            if (cM::rndF(100.0f) <= chance) {
                result = tmpls;
            }
            count++;
        }
    }

    return result;
}

// 8011B838
dAnimalTemplate_c *pickTemplate(dAnimalTemplate_c *tmpls, u32 num, u32 mask, u32 count, u32 looksNum,
                                           dAnimal_c *animals, u32 animalNum, const u8 *bits, u32 size, u32 max,
                                           u8 flag) {
    dAnimalTemplate_c *result = NULL;

    while (count != 0 && mask != 0) {
        u8 looks = pickRandomBit(mask, count, looksNum);
        if (looks >= looksNum) {
            break;
        }

        dAnimalTemplate_c *tmpl = pickTemplateByLooks(tmpls, num, looks, animals, animalNum, bits, size, max, flag);
        if (tmpl != NULL) {
            result = tmpl;
            break;
        }

        mask &= ~(1 << looks);
        count--;
    }

    return result;
}

// 8011B910
dAnimal_c *fn_8011B910(dAnimal_c *animals, u32 num, u32 idx) {
    if (idx < num) {
        return &animals[idx];
    }
    return NULL;
}

// 8011B92C
dAnimal_c *fn_8011B92C(dAnimal_c *animals, u32 num, u32 idx) {
    if (idx < num) {
        return &animals[idx];
    }
    return NULL;
}

// 8011B948
// First free slot (or, with flag, one whose mJoinType is 3).
int findFreeAnimalIdx(dAnimal_c *animals, u32 num, BOOL flag) {
    int result = -1;

    for (u32 i = 0; i < num; i++, animals++) {
        if (!animals->mID.isValid() || (flag && animals->mJoinType == 3)) {
            result = i;
            break;
        }
    }

    return result;
}

// 8011B9CC
dAnimal_c *getFreeAnimal(dAnimal_c *animals, u32 num, BOOL flag) {
    return fn_8011B910(animals, num, findFreeAnimalIdx(animals, num, flag));
}

// 8011BA14
u32 countAnimals(dAnimal_c *animals, u32 num) {
    u32 count = 0;

    for (u32 i = 0; i < num; i++, animals++) {
        if (animals->mID.isValid()) {
            count++;
        }
    }

    return count;
}

// 8011BA8C
dAnimal_c *fn_8011BA8C(u16 npcIdx, dAnimal_c *animals, u32 num) {
    dAnmPersonalID_c *id;
    dAnimal_c *result = NULL;

    for (u32 i = 0; i < num; i++, animals++) {
        id = &animals->mID;
        if (id->isValid() && npcIdx == id->mNpcIdx) {
            result = animals;
            break;
        }
    }

    return result;
}

// 8011BB0C
dAnimal_c *fn_8011BB0C(u16 npcIdx, dAnimal_c *animals, u32 num) {
    dAnmPersonalID_c *id;
    dAnimal_c *result = NULL;

    for (u32 i = 0; i < num; i++, animals++) {
        id = &animals->mID;
        if (id->isValid() && npcIdx == id->mNpcIdx) {
            result = animals;
            break;
        }
    }

    return result;
}

// 8011BB8C
int findAnimalIdx(dAnmPersonalID_c *id, dAnimal_c *animals, u32 num) {
    if (id->isValid()) {
        for (u32 i = 0; i < num; i++, animals++) {
            if (*id == animals->mID) {
                return i;
            }
        }
    }

    return -1;
}

// 8011BC84
// Random villager index, skipping the ids in exclude (with flag, also those where isMoving is set).
int pickAnimalIdx(dAnmPersonalID_c **exclude, u32 numExclude, dAnimal_c *animals, u32 num, BOOL flag) {
    u32 mask = 0;
    int count = 0;
    int result = -1;

    u32 i;
    for (i = 0; i < num; i++, animals++) {
        if (animals->mID.isValid() && (!flag || !animals->isMoving())) {
            mask |= 1 << i;
            count++;
        }
    }
    animals -= num;

    if (exclude != NULL) {
        for (i = 0; i < numExclude; exclude++, i++) {
            if (*exclude != NULL && (*exclude)->isValid()) {
                u32 idx = findAnimalIdx((dAnmPersonalID_c *)*exclude, animals, num);
                if (idx < num && ((mask >> idx) & 1)) {
                    mask &= ~(1 << idx);
                    count--;
                }
            }
        }
    }

    if (count > 0) {
        result = pickRandomBit(mask, count, num);
    }

    return result;
}

// 8011BDB4
void initHarvestFestival(dAnimal_c *animals, u32 num) {
    for (u32 i = 0; i < num; i++, animals++) {
        if (animals->mID.isValid() && !animals->isMoving()) {
            animals->mEvent.setPlace(2);
        }
    }
}

static inline BOOL isPlayerIdx(int idx) {
    return idx >= 0 && idx < 4;
}

// 8011BE34
void initHalloween(dAnimal_c *animals, u32 num, const s8 *order, u32 want, u32 special) {
    u32 left = want < num ? want : num;

    if (left != 0) {
        u32 mask = 0xF;
        u32 count = 4;
        int player = fn_801017B8();
        if (isPlayerIdx(player)) {
            u32 idx = getBirthdayHostIdxIn(animals, num, player, 0, 0, 0);
            if (idx < num) {
                dAnimal_c *animal = fn_8011B910(animals, num, idx);
                if (animal != NULL && animal->mID.isValid()) {
                    dAnimalEventState_c *obj = &animal->mEvent;
                    if ((int)obj->mPlace == 3) {
                        obj->setPlace(0);
                        left--;
                    }
                }
            }
            count = 3;
            mask &= ~(1 << player);
        }

        if (left != 0) {
            while (count != 0 && mask != 0) {
                int p = pickRandomBit(mask, count, 4);
                if (p == -1) {
                    break;
                }

                u32 idx = getBirthdayHostIdxIn(animals, num, p, 0, 0, 0);
                if (idx < num) {
                    dAnimal_c *animal = fn_8011B910(animals, num, idx);
                    if (animal != NULL && animal->mID.isValid()) {
                        dAnimalEventState_c *obj = &animal->mEvent;
                        if ((int)obj->mPlace == 3) {
                            obj->setPlace(0);
                            if (--left == 0) {
                                break;
                            }
                        }
                    }
                }

                mask &= ~(1 << p);
                count--;
            }
        }
    }

    if (left != 0) {
        for (u32 i = 0; i < num; i++) {
            dAnimal_c *animal = fn_8011B910(animals, num, i);
            if (animal != NULL && animal->mID.isValid() && (int)animal->mEvent.mPlace == 3) {
                switch (animal->getDayPlace(0, i == special, 0)) {
                    case 3:
                        break;
                    case 0:
                        left--;
                        break;
                }
            }
            if (left == 0) {
                break;
            }
        }
    }

    if (left != 0) {
        for (u32 i = 0; i < num; i++, order++) {
            int idx = *order;
            dAnimal_c *animal = fn_8011B910(animals, num, idx);
            if (animal != NULL && animal->mID.isValid() && (int)animal->mEvent.mPlace == 3) {
                switch (animal->getDayPlace(0, idx == special, 0)) {
                    case 3:
                        animal->mEvent.setPlace(0);
                        left--;
                        break;
                    case 0:
                        left--;
                        break;
                }
            }
            if (left == 0) {
                break;
            }
        }
    }

    if (left != 0) {
        dAnimal_c *animal = animals;
        for (u32 i = 0; i < num; i++, animal++) {
            if (animal != NULL && animal->mID.isValid() && (int)animal->mEvent.mPlace == 3) {
                switch (animal->getDayPlace(0, i == special, 0)) {
                    case 3:
                        animal->mEvent.setPlace(0);
                        left--;
                        break;
                    case 0:
                        left--;
                        break;
                }
            }
            if (left == 0) {
                break;
            }
        }
    }

    for (u32 i = 0; i < num; i++, animals++) {
        if (animals->mID.isValid() && animals->mEvent.mPlace != 0 && !animals->isMoving()) {
            animals->mEvent.setPlace(2);
        }
    }
}

// 8011C220
void initFishingTourney(dAnimal_c *animals, u32 num) {
    for (u32 i = 0; i < num; i++, animals++) {
        if (animals->mID.isValid() && !animals->isMoving()) {
            animals->mEvent.setPlace(2);
        }
    }
}

// 8011C2A0
void initBugOff(dAnimal_c *animals, u32 num) {
    for (u32 i = 0; i < num; i++, animals++) {
        if (animals->mID.isValid() && !animals->isMoving()) {
            animals->mEvent.setPlace(2);
        }
    }
}

// 8011C320
void initCountdown(dAnimal_c *animals, u32 num) {
    for (u32 i = 0; i < num; i++, animals++) {
        if (animals->mID.isValid() && !animals->isMoving()) {
            animals->mEvent.setPlace(2);
        }
    }
}

// 8011C3A0
void initFireworks(dAnimal_c *animals, u32 num) {
    for (u32 i = 0; i < num; i++, animals++) {
        if (animals->mID.isValid() && !animals->isMoving()) {
            animals->mEvent.setPlace(2);
        }
    }
}

// 8011C420
void initFleaMarket(dAnimal_c *animals, u32 num) {
    dAnimal_c *animal = animals;
    for (u32 i = 0; i < num; i++, animal++) {
        if (animal->mID.isValid() && !animal->isMoving()) {
            animal->mEvent.setPlace(2);
        }
    }

    for (u32 i = 0; i < num; i++, animals++) {
        if (animals->mID.isValid() && !animals->isMoving()) {
            dAnimalEventState_c *obj = &animals->mEvent;
            if (obj->mPlace != 0) {
                obj->setPlace(2);
            }
            if (obj->isInEvent()) {
                animals->pickBoxedFtrMoveOut();
            }
        }
    }
}

// 8011C518
void initFestivale(dAnimal_c *animals, u32 num) {
    for (u32 i = 0; i < num; i++, animals++) {
        if (animals->mID.isValid() && !animals->isMoving()) {
            animals->mEvent.setPlace(2);
        }
    }
}

// 8011C598
void initToyDay(dAnimal_c *animals, u32 num) {
    for (u32 i = 0; i < num; i++, animals++) {
        if (animals->mID.isValid() && !animals->isMoving()) {
            animals->mEvent.setPlace(2);
        }
    }
}

// 8011C618
dAnimalTalkCount_c::dAnimalTalkCount_c() {}

// 8011C61C
dAnimalTalkCount_c::~dAnimalTalkCount_c() {}

// 8011C65C
void dAnimalTalkCount_c::clear() {
    mCount = 0;
    mCountNoAttr5 = 0;
}

// 8011C66C
void dAnimalTalkCount_c::inc(u32 kind) {
    if (mCount < 5) {
        mCount++;
    }

    if (mCountNoAttr5 < 5) {
        if (kind == 0x44) {
            kind = fn_80162548();
        }
        if (!fn_80162594(kind, 5)) {
            mCountNoAttr5++;
        }
    }
}

// 8011C6E4
u8 dAnimalTalkCount_c::get(u32 kind) {
    if (kind == 0x44) {
        kind = fn_80162548();
    }
    if (fn_80162594(kind, 5)) {
        return mCount;
    }
    return mCountNoAttr5;
}

// 8011C73C
dAnimalMemory_c::dAnimalMemory_c() {
    clear();
}

// 8011C79C
dAnimalMemory_c::~dAnimalMemory_c() {}

// 8011C7F8
void dAnimalMemory_c::clear() {
    mPlayer.clear();
    mLastTalkTime.reset();
    mLand.clear();
    memset(mNickname, 0, sizeof(mNickname));
    memset(mGreeting, 0, sizeof(mGreeting));
    mPresent = dItem::ITEM_ID_NONE;
    memset(&mFlags, 0, sizeof(mFlags));
    mFriendship = 0;
    mImpression = '1';
    mTalkCount.clear();
    mEventFlags = 0;
    _86 = dItem::ITEM_ID_NONE;
}

// 8011C8A8
BOOL dAnimalMemory_c::init(const dPersonalID_c *pid, const dLandID_c *land, const dTime_c *time) {
    if (pid->isValid()) {
        clear();
        updateTalk(pid, land, time);
        memcpy(mNickname, pid->player.mName, sizeof(mNickname));
        return TRUE;
    }
    return FALSE;
}

// 8011C93C
BOOL dAnimalMemory_c::init(const dPersonalID_c *pid, s8 a, u16 count, const dLandID_c *land, const dTime_c *time) {
    if (pid->isValid()) {
        clear();
        set(a, count, pid, land, time);
        memcpy(mNickname, pid->player.mName, sizeof(mNickname));
        return TRUE;
    }
    return FALSE;
}

// 8011C9D0
void dAnimalMemory_c::updateTalk(const dPersonalID_c *pid, const dLandID_c *land, const dTime_c *time) {
    if (pid != NULL && pid->isValid()) {
        mPlayer.copy(pid);
    }

    dLandID_c town = dSaveData_c::getRaw()->mLandID;
    if (land == NULL) {
        land = &town;
    }

    if (pid != NULL && pid->land == town) {
        mFlags.mSameTown = 1;
    }

    if (time == NULL) {
        time = dTime_c::getCurrent();
    }

    if (calcTalkDays(time)) {
        addTalkFriendship();
    }

    mLand.copy(land);
    dTime_c cal;
    memcpy(&cal, time, sizeof(dTime_c));
    mLastTalkTime.set(&cal);
}

// 8011CB4C
void dAnimalMemory_c::set(s8 a, u16 count, const dPersonalID_c *pid, const dLandID_c *land, const dTime_c *time) {
    if (pid != NULL && pid->isValid()) {
        mPlayer.copy(pid);
    }

    dLandID_c town = dSaveData_c::getTown()->mLandID;
    if (land == NULL) {
        land = &town;
    }

    if (pid != NULL && pid->land == town) {
        mFlags.mSameTown = 1;
    }

    if (time == NULL) {
        time = dTime_c::getCurrent();
    }

    setFriendship(a);
    setTalkDays(count);
    mLand.copy(land);
    dTime_c cal;
    memcpy(&cal, time, sizeof(dTime_c));
    mLastTalkTime.set(&cal);
}

// 8011CCCC
void dAnimalMemory_c::setTalkDays(u16 count) {
    if (count > 4) {
        count = 4;
    }
    mFlags.mTalkDays = count;
}

static inline BOOL isTimeSet(dTimeStamp_c *time) {
    return !time->isNone();
}

// 8011CCE8
// Updates the meeting count for a meeting at now. TRUE if this is the first meeting of the day.
BOOL dAnimalMemory_c::calcTalkDays(const dTime_c *now) {
    u16 count = 0;
    BOOL first = FALSE;

    if (isTimeSet(&mLastTalkTime) && mPlayer.isValid()) {
        dTime_c lastDay = mLastTalkTime.get();
        dTime_c nowDay = *now;
        lastDay.add(0, -6, 0, 0);
        nowDay.add(0, -6, 0, 0);
        if (dTime_c::isSameOrAfter(nowDay, lastDay)) {
            int days = dTime_c::diffDays(&nowDay, &lastDay, FALSE);
            if (days == 0) {
                count = mFlags.mTalkDays;
            } else {
                if (days == 1) {
                    count = mFlags.mTalkDays + 1;
                }
                first = TRUE;
            }
        } else {
            first = TRUE;
        }
    } else {
        first = TRUE;
    }

    setTalkDays(count);
    return first;
}

// 8011CF38
s8 dAnimalMemory_c::addTalkFriendship() {
    int count = mFlags.mTalkDays;
    if (count > 4) {
        count = 4;
    }
    return addFriendship(count + 1);
}

// 8011CF58
s8 dAnimalMemory_c::getFriendship() {
    return mFriendship;
}

// 8011CF64
void dAnimalMemory_c::setFriendship(s8 value) {
    mFriendship = value;
}

// 8011CF6C
s8 dAnimalMemory_c::addFriendship(s8 delta) {
    int value = delta + getFriendship();
    if (value > 127) {
        value = 127;
    } else if (value < -128) {
        value = -128;
    }
    setFriendship(value);
    return getFriendship();
}

// 8011CFE0
int dAnimalMemory_c::getFriendshipLevel(s8 value) {
    int result = 2;
    if (value >= 64) {
        return 0;
    }
    if (value < 0) {
        result = 1;
    }
    return result;
}

// 8011D008
int dAnimalMemory_c::getFriendshipLevel() {
    return getFriendshipLevel(getFriendship());
}

// 8011D044
u32 dAnimalMemory_c::hasLetter() {
    return mFlags.mGotLetter;
}

// 8011D050
void dAnimalMemory_c::setNickname(const wchar_t *name, u32 len) {
    memset(mNickname, 0, sizeof(mNickname));
    if (len >= PLAYER_NAME_LEN + 1) {
        len = PLAYER_NAME_LEN;
    }
    memcpy(mNickname, name, len * sizeof(wchar_t));
    mFlags.mHasNickname = 1;
}

// 8011D0C8
void dAnimalMemory_c::setNickname(dScript::Word_c *word) {
    wchar_t *buf = word->getBuffer();
    setNickname(buf, word->getCapacity());
}

// 8011D134
void dAnimalMemory_c::getNickname(dScript::Word_c *word) {
    word->set(mNickname, 0);
}

// 8011D148
void dAnimalMemory_c::decNicknameWait(int num) {
    if (num <= 0) {
        return;
    }
    // Reload the flag word for the bitfield write, preserving the other flags.
    if (num > mFlags.mNicknameWait) {
        ((volatile dAnimalMemoryFlags_c *)&mFlags)->mNicknameWait = 0;
    } else {
        ((volatile dAnimalMemoryFlags_c *)&mFlags)->mNicknameWait = mFlags.mNicknameWait - num;
    }
}

// 8011D184
void dAnimalMemory_c::copyNickname(dAnimalMemory_c *other) {
    dHmnName::Word_c word;
    other->getNickname(&word);
    setNickname(&word);
    mFlags.mNicknameWait = other->mFlags.mNicknameWait;
}

// 8011D1F4
u32 dAnimalMemory_c::hasGreeting() {
    return mFlags.mHasGreeting;
}

// 8011D200
void dAnimalMemory_c::setGreeting(const wchar_t *str, u32 len) {
    memset(mGreeting, 0, sizeof(mGreeting));
    if (len >= 17) {
        len = 16;
    }
    memcpy(mGreeting, str, len * sizeof(wchar_t));
    mFlags.mHasGreeting = 1;
}

// 8011D278
void dAnimalMemory_c::getGreeting(dScript::Word_c *word) {
    word->set(mGreeting, 0);
}

// 8011D28C
void dAnimalMemory_c::decGreetingWait(int num) {
    if (num <= 0) {
        return;
    }
    // Reload the flag word for the bitfield write, preserving the other flags.
    if (num > mFlags.mGreetingWait) {
        ((volatile dAnimalMemoryFlags_c *)&mFlags)->mGreetingWait = 0;
    } else {
        ((volatile dAnimalMemoryFlags_c *)&mFlags)->mGreetingWait = mFlags.mGreetingWait - num;
    }
}

// 8011D2C8
// Stores item in mPresent if it is furniture, wallpaper, carpet or clothing.
BOOL dAnimalMemory_c::setPresent(const dItem::Item *item) {
    if (item->mId == dItem::ITEM_ID_NONE) {
        return FALSE;
    }

    dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    if (bitm == NULL) {
        return FALSE;
    }

    BOOL ok = FALSE;
    int kind = bitm->getKind();
    if (isFurnitureKind(kind)) {
        ok = TRUE;
    } else {
        switch (kind) {
            case dItem::KIND_WALL:
            case dItem::KIND_CARPET:
            case dItem::KIND_CLOTH:
                ok = TRUE;
                break;
        }
    }

    if (ok) {
        mPresent = *item;
        return TRUE;
    }
    return FALSE;
}

// 8011D3B4
void dAnimalMemory_c::updateLetterCond(int value) {
    u32 r = getImpression((dPrivateData_c *)value);
    if (r < 50) {
        mImpression = r;
    }
}

// 8011D3F0
void dAnimalMemory_c::onEventFlag(u32 bit) {
    if (bit < 8) {
        mEventFlags |= 1 << bit;
    }
}

// 8011D410
void dAnimalMemory_c::offEventFlag(u32 bit) {
    if (bit < 8) {
        mEventFlags &= ~(1 << bit);
    }
}

// 8011D430
BOOL dAnimalMemory_c::isEventFlag(u32 bit) {
    if (bit < 8) {
        return (mEventFlags >> bit) & 1;
    }
    return FALSE;
}

// 8011D450
dAnimalEventState_c::dAnimalEventState_c() {}

// 8011D454
dAnimalEventState_c::~dAnimalEventState_c() {}

// 8011D494
void dAnimalEventState_c::clear() {
    mPlace = 3;
    _1 = 0;
    mFlags = 0;
}

// 8011D4AC
void dAnimalEventState_c::setPlace(u8 state) {
    clear();
    mPlace = state;
}

// 8011D4E8
BOOL dAnimalEventState_c::isInEvent() {
    return mPlace < 3;
}

// 8011D4F8
void dAnimalEventState_c::setFlag(u32 bit) {
    if (bit < 8) {
        mFlags |= 1 << bit;
    }
}

// 8011D518
void dAnimalEventState_c::clearFlag(u32 bit) {
    if (bit < 8) {
        mFlags &= ~(1 << bit);
    }
}

// 8011D538
BOOL dAnimalEventState_c::isFlag(u32 bit) {
    if (bit < 8) {
        return (mFlags >> bit) & 1;
    }
    return FALSE;
}

// 8011D558
dAnimalSpot_c::dAnimalSpot_c() {}

// 8011D594
dAnimalSpot_c::~dAnimalSpot_c() {}

// 8011D5D4
void dAnimalSpot_c::clear() {
    mTime.reset();
    mX = -1;
    mZ = -1;
    mGroup = 0;
    mType = 4;
}

// setD013Spot fills a dAnimalSpot_c.
// 8011D61C
BOOL dAnimalSpot_c::isValid() {
    BOOL valid = FALSE;
    if (mX != -1 && mZ != -1 && mType < 4) {
        valid = TRUE;
    }
    return valid;
}

// 8011D650
dTime_c dAnimalSpot_c::getTime() {
    return mTime.get();
}

// 8011D654
s64 dAnimalSpot_c::getTimeRaw() {
    return mTime.getTicks();
}

// 8011D658
void dAnimalSpot_c::setTime(const dTime_c *time, int mins) {
    dTime_c cal = *time;
    cal.add(0, 0, mins, 0);
    mTime.set(&cal);
}

// 8011D6FC
void dAnimalSpot_c::setTimeRaw(const s64 *value) {
    mTime.set(*value);
}

// Spot pickers for pickWishSpot: fill *x / *z with a unit position, return FALSE if none.
typedef BOOL (*dAnimalSpotFunc)(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block);

// 8011D708
// At the villager's house.
BOOL getOwnHouseSpot(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block) {
    mVec3_c pos = mVec3_c::Zero;
    if (block->getHousePosById(&pos, &animal->mID)) {
        *x = (int)pos.x >> 5;
        *z = (int)pos.z >> 5;
        return TRUE;
    }
    return FALSE;
}

// 8011D7B0
// At another villager's house.
BOOL getOtherHouseSpot(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block) {
    // The target tests this result but does nothing with it: the branch body is a
    // copy that coalesces away, leaving the compare.
    dAnimalBlock_c *b = block;
    if (animal->mID.isValid()) {
        b = block;
    }
    dAnimal_c *other = b->pickRandomAnimalConst(NULL, 0, FALSE);
    if (other == NULL || !other->mID.isValid()) {
        return FALSE;
    }

    mVec3_c pos = mVec3_c::Zero;
    if (block->getHousePosById(&pos, &other->mID)) {
        *x = (int)pos.x >> 5;
        *z = (int)pos.z >> 5;
        return TRUE;
    }
    return FALSE;
}

// 8011D8B0
BOOL getPlayerHouseSpot(int *x, int *z, u32 player) {
    if (player >= 4) {
        return FALSE;
    }
    return fn_8014A034(&dSaveData_c::getTown()->_05EB04, x, z, player) != 0;
}

// 8011D928
// At the current player's house.
BOOL getCurPlayerHouseSpot(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block) {
    u32 player = fn_801017B8();
    return getPlayerHouseSpot(x, z, dSaveData_c::getRaw()->mHomes.findPlayer(player));
}

// 8011D990
// At another player's house.
BOOL getOtherPlayerHouseSpot(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block) {
    u32 player = fn_801017B8();
    int home = dSaveData_c::getRaw()->mHomes.findPlayer(player);
    int other = cM::rndInt(3);
    if (other == home) {
        other++;
    }
    return getPlayerHouseSpot(x, z, other);
}

// The save pointer is fetched before the item copy is made.
static inline BOOL findFgItem(dSaveData_c *save, int *x, int *z, const dItem::Item &item) {
    return fn_8014B0F0(&save->_05EB04, x, z, item, 1);
}

// 8011DA10
BOOL getStructSpot(int *x, int *z, const dItem::Item *item) {
    if (!item->isExtId()) {
        return FALSE;
    }
    return findFgItem(dSaveData_c::getTown(), x, z, *item) != 0;
}

// 8011DAB0
BOOL getStructSpotD013(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block) {
    dItem::Item item((u16)0xD013);
    return getStructSpot(x, z, &item);
}

// 8011DAE0
BOOL D016(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block) {
    dItem::Item item((u16)0xD016);
    return getStructSpot(x, z, &item);
}

// 8011DB10
BOOL D017(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block) {
    dItem::Item item((u16)0xD017);
    return getStructSpot(x, z, &item);
}

// 8011DB40
BOOL D015(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block) {
    dItem::Item item((u16)0xD015);
    return getStructSpot(x, z, &item);
}

// 8011DB70
BOOL D014(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block) {
    dItem::Item item((u16)0xD014);
    return getStructSpot(x, z, &item);
}

// 8011DBA0
// Random block (not on the map edge) of the given type; returns its center unit.
BOOL getBlockTypeSpot(int *x, int *z, u32 type) {
    dNpcFieldMap_c *map = fn_80190C44(1);
    if (map == NULL) {
        return FALSE;
    }

    int w = map->mBlockW - 1;
    int h = map->mBlockH - 1;
    int bx = -1;
    int bz = -1;
    u32 count = 0;

    for (int j = 1; j < h; j++) {
        for (int i = 1; i < w; i++) {
            if (fn_8008C39C(map, i, j, type)) {
                count++;
                f32 chance = 100.0f / count;
                if (cM::rndF(100.0f) < chance) {
                    bx = i;
                    bz = j;
                }
            }
        }
    }

    if (count == 0 || bx == -1 || bz == -1) {
        return FALSE;
    }

    *x = bx * 16 + 7;
    *z = bz * 16 + 7;
    return TRUE;
}

// 8011DCFC
BOOL getBlockSpot0FE00000(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block) {
    return getBlockTypeSpot(x, z, 0x0FE00000);
}

// 8011DD04
BOOL getBottomRowSpot(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block) {
    *x = (cM::rndInt(5) + 1) * 16 + 7;
    *z = 0x57;
    return TRUE;
}

// 8011DD5C
BOOL fn_8011DD5C(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block) {
    return getBlockTypeSpot(x, z, 0x100000);
}

// 8011DD64
BOOL fn_8011DD64(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block) {
    return getBlockTypeSpot(x, z, 0x200);
}

// 8011DD6C
BOOL FE000(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block) {
    return getBlockTypeSpot(x, z, 0xFE000);
}

// 8011DD78
// Anywhere in the 5x5 inner blocks.
BOOL getRandomSpot(int *x, int *z, dAnimal_c *animal, dAnimalBlock_c *block) {
    int bx = cM::rndInt(5) + 1;
    int bz = cM::rndInt(5) + 1;
    *x = bx * 16 + 7;
    *z = bz * 16 + 7;
    return TRUE;
}

// 80475D98: chance (percent) of each spot per wish kind.
static const u8 sSpotChance[8][15] = {
    {5, 3, 5, 5, 3, 3, 5, 5, 3, 3, 10, 10, 10, 20, 10},
    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 30},
    {5, 5, 5, 5, 5, 5, 10, 5, 5, 15, 5, 5, 5, 5, 15},
    {5, 5, 5, 5, 5, 20, 5, 5, 5, 5, 5, 5, 5, 5, 15},
    {10, 5, 5, 5, 5, 5, 5, 20, 5, 5, 5, 5, 5, 5, 10},
    {5, 10, 15, 10, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 10},
    {20, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 15},
    {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 30},
};

// 80475E10
static dAnimalSpotFunc const sSpotFuncs[15] = {
    getOwnHouseSpot, getOtherHouseSpot, getCurPlayerHouseSpot, getOtherPlayerHouseSpot, getStructSpotD013, D016, D017, D015,
    D014, getBlockSpot0FE00000, getBottomRowSpot, fn_8011DD5C, fn_8011DD64, FE000, getRandomSpot,
};

static inline void setSpot(dAnimalSpot_c *obj, int x, int z, u8 type) {
    obj->mX = x;
    obj->mZ = z;
    obj->mType = type;
    obj->mGroup = 1;
}

// 8011DDE8
// Picks a random spot for the villager by its wish kind.
BOOL dAnimalSpot_c::pickWishSpot(dAnimal_c *animal, dAnimalBlock_c *block) {
    if (!animal->mID.isValid()) {
        return FALSE;
    }

    u32 idx = block->getAnimalIdx(&animal->mID);
    if (idx >= ANIMAL_NUM) {
        return FALSE;
    }

    u32 kind = animal->mQuest.mWish.mKind;
    if (kind >= 8) {
        return FALSE;
    }

    const u8 *chance = sSpotChance[kind];
    int spot = 15;
    int r = cM::rndInt(100);
    for (int i = 0; i < 15; i++, chance++) {
        if (r < *chance) {
            spot = i;
            break;
        }
        r -= *chance;
    }

    if (spot >= 15) {
        return FALSE;
    }

    int x = -1;
    int z = -1;
    if (!sSpotFuncs[spot](&x, &z, animal, block)) {
        getRandomSpot(&x, &z, animal, block);
    }

    setSpot(this, x, z, 0);
    fn_800F0B64(idx, x, z, 0);
    return TRUE;
}

// 8011DFC4
BOOL dAnimalSpot_c::setHomeSpot1(dAnimal_c *animal, dAnimalBlock_c *block) {
    int x = -1;
    int z = -1;
    if (!getOwnHouseSpot(&x, &z, animal, block)) {
        return FALSE;
    }

    u32 idx = block->getAnimalIdx(&animal->mID);
    if (idx >= ANIMAL_NUM) {
        return FALSE;
    }

    setSpot(this, x, z, 1);
    fn_800F0B64(idx, x, z, 1);
    return TRUE;
}

// 8011E080
BOOL dAnimalSpot_c::fn_8011E080(dAnimal_c *animal, dAnimalBlock_c *block) {
    int x = -1;
    int z = -1;
    if (!getOwnHouseSpot(&x, &z, animal, block)) {
        return FALSE;
    }

    u32 idx = block->getAnimalIdx(&animal->mID);
    if (idx >= ANIMAL_NUM) {
        return FALSE;
    }

    setSpot(this, x, z, 2);
    fn_800F0B64(idx, x, z, 2);
    return TRUE;
}

// 8011E140
BOOL dAnimalSpot_c::fn_8011E140(dAnimal_c *animal, dAnimalBlock_c *block) {
    int x = -1;
    int z = -1;
    if (!getOwnHouseSpot(&x, &z, animal, block)) {
        return FALSE;
    }

    u32 idx = block->getAnimalIdx(&animal->mID);
    if (idx >= ANIMAL_NUM) {
        return FALSE;
    }

    setSpot(this, x, z, 3);
    fn_800F0B64(idx, x, z, 3);
    return TRUE;
}

// 8011E200
BOOL dAnimalSpot_c::setD013Spot(dAnimal_c *animal, dAnimalBlock_c *block) {
    int x = -1;
    int z = -1;
    if (!getStructSpotD013(&x, &z, animal, block)) {
        return FALSE;
    }

    u32 idx = block->getAnimalIdx(&animal->mID);
    if (idx >= ANIMAL_NUM) {
        return FALSE;
    }

    setSpot(this, x, z, 0);
    fn_800F0B64(idx, x, z, 0);
    return TRUE;
}

// 8011E2C0
dAnimal_c::dAnimal_c() {
    clear();
}

// 8011E39C
dAnimal_c::~dAnimal_c() {}

// 8011E458
void dAnimal_c::clear() {
    memset(this, 0, sizeof(dAnimal_c));
    mID.clear();
    mJoinType = 4;

    for (int i = 0; i < ANIMAL_MEMORY_NUM; i++) {
        mMemories[i].clear();
    }

    mPrevLand.clear();
    clearNewItems();
    mWall = dItem::ITEM_ID_NONE;
    mCarpet = dItem::ITEM_ID_NONE;
    mClothDesign.clear();
    mLetter.clear();
    mQuest.clear();
    mDayPlace = 3;
    mPlace = 4;
    mEvent.clear();
    mBoxedFtrMask = 0;
    mHeldItem = dItem::ITEM_ID_NONE;
    fn_801192F4(&_300C);
    mSpot.clear();
    clearPlaceChangeTime();
}

// 8011E544
void dAnimal_c::copy(const dAnimal_c *other) {
    memcpy(this, other, sizeof(dAnimal_c));
}

// 8011E54C
BOOL dAnimal_c::isChecksumValid() {
    if (!mID.isValid() || !mSetupLoaded) {
        return TRUE;
    }

    u32 stored = getChecksum();
    return calcChecksum() == stored;
}

// 8011E5C0
u32 dAnimal_c::getChecksum() {
    u32 sum = 0;
    if (mID.isValid() && this != NULL) {
        cLib::memCpy(&sum, &mChecksum, sizeof(sum));
    }
    return sum;
}

// 8011E61C
u32 dAnimal_c::calcChecksum() {
    u32 sum = 0;
    if (mID.isValid() && this != NULL) {
        sum = sCrc::calcCRC32(this, offsetof(dAnimal_c, mChecksum), 0x04201018, -1);
    }
    return sum;
}

// 8011E688
void dAnimal_c::init(u16 npcIdx, u8 arg, const dLandID_c *land, const dAnimalTemplate_c *tmpl) {
    clear();
    mID.set(npcIdx, tmpl->mLooks, land, tmpl->mNames[0], tmpl->mNames[1], tmpl->mNames[2], tmpl->mNames[3],
            tmpl->mNames[4], tmpl->mNames[5], tmpl->mNames[6], tmpl->mNames[7]);
    mJoinType = arg;
    mCloth.mId = (int)tmpl->mCloth;

    dItem::Item item0;
    item0.mId = (int)tmpl->mWall;
    setWall(&item0);
    dItem::Item item1;
    item1.mId = (int)tmpl->mCarpet;
    setCarpet(&item1);

    memcpy(&mTemplate, tmpl, sizeof(dAnimalTemplate_c));

    dQuestVillagerWish_c *quest = &mQuest;
    quest->mWish.setRandom();
    if (quest->mWish.isValid() && (int)quest->mWish.mKind == 2) {
        clearFossilRoomFtr();
    }

    initVersion();
}

// 8011E790
void dAnimal_c::setSetupData(const void *src) {
    memcpy(this, src, offsetof(dAnimal_c, mTemplate));
    mSetupLoaded = 1;
}

// 8011E7C8
// Index of the memory of pid, or -1.
int dAnimal_c::getMemoryIdx(const dPersonalID_c *pid) {
    dPersonalID_c *player;
    dAnimalMemory_c *mem = getMemory2(0);
    int result = -1;

    for (int i = 0; i < ANIMAL_MEMORY_NUM; i++, mem++) {
        player = &mem->mPlayer;
        if (*player == *pid) {
            result = i;
            break;
        }
    }

    return result;
}

// 8011E888
dAnimalMemory_c *dAnimal_c::getMemory(u32 idx) {
    if (idx < ANIMAL_MEMORY_NUM) {
        return &mMemories[idx];
    }
    return NULL;
}

// Temporary declarations (chunk 1)
extern "C" {
// This TU, other chunks.

// Other TUs.
dScript::Word_c *fn_801A3158(dDemo_c *demo, int idx);
u32 fn_800DCF30();
extern u8 lbl_8059FF80[];
}

// Flag bit 26 of the memory's first word.
static inline BOOL dAnimalMemory_isFlag26(const dAnimalMemory_c *mem) {
    return (*(const u32 *)&mem->mFlags >> 26) & 1;
}

// Room items 0xF000..0xF02F stand for the villager's furniture slots.
static inline bool isFurnitureSlotId(u16 id) {
    return id >= 0xF000 && id <= 0xF02F;
}

static inline int getFurnitureSlotIdx(u16 id) {
    return isFurnitureSlotId(id) ? (id - 0xF000) >> 2 : -1;
}

static inline int getFtrSize(const dItem::BITM *bitm) {
    u32 raw = static_cast<s8>(bitm->m_ftrSize);
    int size = 0;
    if (raw < 3) {
        size = raw;
    }
    return size;
}

// 8011E8A8
dAnimalMemory_c *dAnimal_c::getMemory2(u32 idx) {
    if (idx < ANIMAL_MEMORY_NUM) {
        return &mMemories[idx];
    }
    return NULL;
}

// 8011E8C8
dAnimalMemory_c *dAnimal_c::findMemory(const dPersonalID_c *pid) {
    return getMemory(getMemoryIdx(pid));
}

// 8011E900
dAnimalMemory_c *dAnimal_c::findMemory2(const dPersonalID_c *pid) {
    return getMemory2(getMemoryIdx(pid));
}

// 8011E938
// First unused memory slot, or -1.
int dAnimal_c::getFreeMemoryIdx() {
    dAnimalMemory_c *mem = mMemories;
    int res = -1;
    for (int i = 0; i < ANIMAL_MEMORY_NUM; i++, mem++) {
        if (!mem->mPlayer.isValid()) {
            res = i;
            break;
        }
    }
    return res;
}

// 8011E9A4
// Memory slot to overwrite among those accepted by the filter: an unused one if any,
// else the one with the lowest _88, then the oldest _04.
int dAnimal_c::getReplaceMemoryIdx(dAnimalMemoryFilter filter) {
    dLandID_c *land = &dSaveData_c::getTown()->mLandID;
    dPrivateData_c *players = dSaveData_c::getTown()->mPlayers;
    int res = -1;

    if (land->isValid()) {
        dAnimalMemory_c *mem = getMemory2(0);
        dAnimalMemory_c *best = NULL;
        for (int i = 0; i < ANIMAL_MEMORY_NUM; i++, mem++) {
            if (mem->mPlayer.isValid()) {
                if (filter(&mem->mPlayer, land, players)) {
                    if (best != NULL) {
                        if (best->hasLetter() == mem->hasLetter()) {
                            if (best->getFriendship() > mem->getFriendship()) {
                                best = mem;
                                res = i;
                            } else if (best->getFriendship() == mem->getFriendship()) {
                                if (!dTime_c::isSameOrAfter(mem->mLastTalkTime.get(), best->mLastTalkTime.get())) {
                                    best = mem;
                                    res = i;
                                }
                            }
                        } else if (mem->hasLetter() == 0) {
                            best = mem;
                            res = i;
                        }
                    } else {
                        best = mem;
                        res = i;
                    }
                }
            } else {
                res = i;
                break;
            }
        }
    }

    return res;
}

// 8011EBD4
// Players of this town who have moved out.
BOOL isMovedOutPlayer(const dPersonalID_c *pid, const dLandID_c *land, dPrivateData_c *players) {
    BOOL res = FALSE;
    if (*land == pid->land && dPrivateData_c::find(players, pid) == -1) {
        res = TRUE;
    }
    return res;
}

// 8011EC68
int dAnimal_c::getMovedOutPlayerMemoryIdx() {
    return getReplaceMemoryIdx(isMovedOutPlayer);
}

// 8011EC74
// Players from other towns.
BOOL isOtherTownPlayer(const dPersonalID_c *pid, const dLandID_c *land, dPrivateData_c *players) {
    return *land != pid->land;
}

// 8011ECE0
int dAnimal_c::getOtherTownMemoryIdx() {
    return getReplaceMemoryIdx(isOtherTownPlayer);
}

// 8011ECEC
u32 dAnimal_c::getNewMemoryIdx() {
    u32 idx = getFreeMemoryIdx();
    if (idx >= ANIMAL_MEMORY_NUM) {
        idx = getMovedOutPlayerMemoryIdx();
    }
    if (idx >= ANIMAL_MEMORY_NUM) {
        idx = getOtherTownMemoryIdx();
    }
    return idx;
}

// 8011ED38
// Random remembered player, skipping those in exclude and (if checkTown) local players
// who are not loaded.
int dAnimal_c::getRandomMemoryIdx(const dPersonalID_c **exclude, u32 num, BOOL checkTown) {
    dAnimalMemory_c *mem = getMemory2(0);
    const dLandID_c *townLand = &dSaveData_c::getTown()->mLandID;
    u32 count = 0;
    int res = -1;

    for (int i = 0; i < ANIMAL_MEMORY_NUM; i++, mem++) {
        if (mem->mPlayer.isValid()) {
            const dPersonalID_c *pid = &mem->mPlayer;
            BOOL ok = TRUE;
            if (checkTown && pid->land == *townLand && dPlayerMgr_c::getPlayer(pid) == NULL) {
                ok = FALSE;
            } else if (exclude != NULL && num != 0) {
                const dPersonalID_c **p = exclude;
                for (u32 j = 0; j < num; j++, p++) {
                    if (*p != NULL && (*p)->isValid() && **p == *pid) {
                        ok = FALSE;
                        break;
                    }
                }
            }

            if (ok) {
                f32 rate = 100.0f / (count + 1);
                if (cM::rndF(100.0f) <= rate) {
                    res = i;
                }
                count++;
            }
        }
    }

    return res;
}

// 8011EF48
dAnimalMemory_c *dAnimal_c::getRandomMemory(const dPersonalID_c **exclude, u32 num, BOOL checkTown) {
    return getMemory(getRandomMemoryIdx(exclude, num, checkTown));
}

// 8011EF80
int dAnimal_c::getMemoryNum() {
    dAnimalMemory_c *mem = getMemory2(0);
    int count = 0;
    for (int i = 0; i < ANIMAL_MEMORY_NUM; i++, mem++) {
        if (mem->mPlayer.isValid()) {
            count++;
        }
    }
    return count;
}

// 8011EFF0
// Drops remembered items that no longer exist.
void dAnimal_c::clearInvalidPresents() {
    dAnimalMemory_c *mem = getMemory(0);
    for (int i = 0; i < ANIMAL_MEMORY_NUM; i++, mem++) {
        if (mem->mPlayer.isValid() && mem->mPresent.mId != dItem::ITEM_ID_NONE) {
            dItem::BITM *bitm = dItem::getBITM(mem->mPresent.mId);
            if (bitm == NULL) {
                mem->setEmptyItem();
            }
        }
    }
}

// 8011F090
BOOL dAnimal_c::usesNickname(const dPersonalID_c *pid) {
    if (pid->isValid()) {
        dAnimalMemory_c *mem = findMemory2(pid);
        if (mem != NULL && dAnimalMemory_isFlag26(mem) && mem->getFriendship() > -10) {
            return TRUE;
        }
    }
    return FALSE;
}

// 8011F110
// The player's nickname if the villager uses one, else the player's name.
void dAnimal_c::getPlayerCallName(dScript::Word_c *word, const dPersonalID_c *pid) {
    dAnimalMemory_c *mem = findMemory2(pid);
    if (!usesNickname(pid) || mem == NULL) {
        pid->setWord(word);
    } else {
        mem->getNickname(word);
    }
}

// 8011F19C
// Random non-empty demo word that differs from def, else def.
void fn_8011F19C(dScript::Word_c *out, dScript::Word_c *def, BOOL flag) {
    u32 mask = 0xF;

    if (flag == FALSE) {
        mask |= 0x70;
    } else {
        mask |= 0x380;
    }
    dDemo_c *demo = dDemo_c::mInstance;

    if (demo != NULL) {
        while (mask != 0) {
            u32 count = 0;
            int pick = -1;
            for (u32 i = 0; i < 10; i++) {
                if ((mask >> i) & 1) {
                    f32 rate = 100.0f / (count + 1);
                    if (cM::rndF(100.0f) <= rate) {
                        pick = i;
                    }
                    count++;
                }
            }

            if (pick == -1) {
                mask = 0;
                break;
            }

            if (fn_801A3158(demo, pick)->getLength(0) != 0 && !fn_801A3158(demo, pick)->isSame(def)) {
                out->copy(fn_801A3158(demo, pick), 0);
                break;
            }

            mask &= ~(1 << pick);
        }

        if (mask == 0) {
            out->copy(def, 0);
        }
    } else {
        out->copy(def, 0);
    }
}

// 8011F334
BOOL dAnimal_c::getGreeting(dScript::Word_c *word, const dPersonalID_c *pid) {
    dAnimalMemory_c *mem = findMemory2(pid);
    if (mem != NULL && pid->isValid() && mem->hasGreeting()) {
        mem->getGreeting(word);
        return TRUE;
    }
    return FALSE;
}

// 8011F3BC
dItem::Item getTemplateFtr(u32 idx, const dAnimalTemplate_c *tmpl) {
    dItem::Item item;
    if (idx < 10) {
        switch (idx) {
        case 0: item.mId = (int)tmpl->mFurniture[0]; break;
        case 1: item.mId = (int)tmpl->mFurniture[1]; break;
        case 2: item.mId = (int)tmpl->mFurniture[2]; break;
        case 3: item.mId = (int)tmpl->mFurniture[3]; break;
        case 4: item.mId = (int)tmpl->mFurniture[4]; break;
        case 5: item.mId = (int)tmpl->mFurniture[5]; break;
        case 6: item.mId = (int)tmpl->mFurniture[6]; break;
        case 7: item.mId = (int)tmpl->mFurniture[7]; break;
        case 8: item.mId = (int)tmpl->mFurniture[8]; break;
        case 9: item.mId = (int)tmpl->mFurniture[9]; break;
        }
    }
    return item;
}

// 8011F468
dItem::Item dAnimal_c::getFtr(u32 idx) {
    return getTemplateFtr(idx, &mTemplate);
}

// 8011F478
void dAnimal_c::clearFtr(u32 idx) {
    if (idx < 10) {
        switch (idx) {
        case 0: mTemplate.mFurniture[0] = dItem::ITEM_ID_NONE; break;
        case 1: mTemplate.mFurniture[1] = dItem::ITEM_ID_NONE; break;
        case 2: mTemplate.mFurniture[2] = dItem::ITEM_ID_NONE; break;
        case 3: mTemplate.mFurniture[3] = dItem::ITEM_ID_NONE; break;
        case 4: mTemplate.mFurniture[4] = dItem::ITEM_ID_NONE; break;
        case 5: mTemplate.mFurniture[5] = dItem::ITEM_ID_NONE; break;
        case 6: mTemplate.mFurniture[6] = dItem::ITEM_ID_NONE; break;
        case 7: mTemplate.mFurniture[7] = dItem::ITEM_ID_NONE; break;
        case 8: mTemplate.mFurniture[8] = dItem::ITEM_ID_NONE; break;
        case 9: mTemplate.mFurniture[9] = dItem::ITEM_ID_NONE; break;
        }
    }
}

// 8011F518
BOOL dAnimal_c::setFtr(u32 idx, const dItem::Item *item) {
    if (!isHouseItem(item)) {
        return FALSE;
    }

    BOOL res = FALSE;
    if (idx < 10) {
        switch (idx) {
        case 0: mTemplate.mFurniture[0] = (u16)item->mId; res = TRUE; break;
        case 1: mTemplate.mFurniture[1] = (u16)item->mId; res = TRUE; break;
        case 2: mTemplate.mFurniture[2] = (u16)item->mId; res = TRUE; break;
        case 3: mTemplate.mFurniture[3] = (u16)item->mId; res = TRUE; break;
        case 4: mTemplate.mFurniture[4] = (u16)item->mId; res = TRUE; break;
        case 5: mTemplate.mFurniture[5] = (u16)item->mId; res = TRUE; break;
        case 6: mTemplate.mFurniture[6] = (u16)item->mId; res = TRUE; break;
        case 7: mTemplate.mFurniture[7] = (u16)item->mId; res = TRUE; break;
        case 8: mTemplate.mFurniture[8] = (u16)item->mId; res = TRUE; break;
        case 9: mTemplate.mFurniture[9] = (u16)item->mId; res = TRUE; break;
        }
    }
    return res;
}

// 8011F638
int dAnimal_c::countFtr() {
    int count = 0;
    for (int i = 0; i < 10; i++) {
        dItem::Item item = getFtr(i);
        if (item.mId != dItem::ITEM_ID_NONE && isHouseItem(&item)) {
            count++;
        }
    }
    return count;
}

// 8011F6B8
int dAnimal_c::findFtr(const dItem::Item *item) {
    if (item->mId != dItem::ITEM_ID_NONE) {
        for (int i = 0; i < 10; i++) {
            if (item->isSame(getFtr(i))) {
                return i;
            }
        }
    }
    return -1;
}

// 8011F740
// Furniture slot of a room placeholder item, or 10.
u32 getFtrSlotOfPlaceholder(const dItem::Item *item) {
    u32 res = 10;
    if (isFurnitureSlotId(item->mId)) {
        int idx = getFurnitureSlotIdx(item->mId);
        if (idx != -1) {
            if (idx >= 3) {
                res = idx - 2;
            } else {
                res = idx;
            }
        }
    }
    return res;
}

// 8011F7B4
// Item a villager can keep in the house.
BOOL isHouseItem(const dItem::Item *item) {
    if (item->mId == dItem::ITEM_ID_NONE) {
        return FALSE;
    }

    dItem::BITM *bitm = dItem::getBITM(item->mId);
    int kind = bitm != NULL ? bitm->getKind() : dItem::KIND_COUNT;
    BOOL res = FALSE;

    switch (kind) {
    case dItem::KIND_FTR:
    case dItem::KIND_CAP:
    case dItem::KIND_ACC:
    case dItem::KIND_FOSSIL:
    case dItem::KIND_HANIWA:
        res = TRUE;
        break;
    case dItem::KIND_CLOTH:
    case dItem::KIND_INSECT:
    case dItem::KIND_FISH:
    case dItem::KIND_PICTURE:
    case dItem::KIND_FAKE_PICTURE_BEFORE:
    case dItem::KIND_FAKE_PICTURE_AFTER:
    case dItem::KIND_UMBRELLA:
    case dItem::KIND_FLOWER:
        if (dItem::clampField(bitm->m_ftrFunc, 0x41, 1) != 0) {
            res = TRUE;
        }
        break;
    }
    return res;
}

// 8011F8B8
// Furniture slot of the room item at (x, z) of a 16x16 room grid.
u32 getRoomFtrSlot(const dItem::Item *room, u32 x, u32 z) {
    if (room != NULL && x < 16 && z < 16) {
        int idx = z * 16;
        idx += x;
        return getFtrSlotOfPlaceholder(&room[idx]);
    }
    return 10;
}

// 8011F8EC
// Furniture slot range [*start, *end) for a size class.
BOOL getFtrSlotRange(int *start, int *end, u32 size, BOOL wish) {
    static const int sStart[3] = {5, 1, 0};
    static const int sEnd[3] = {10, 5, 1};
    static const int sWishStart[3] = {5, 3, 0};
    static const int sWishEnd[3] = {10, 5, 3};

    if (size >= 3) {
        return FALSE;
    }

    const int *s = sStart;
    const int *e = sEnd;
    if (wish) {
        s = sWishStart;
        e = sWishEnd;
    }
    *start = s[size];
    *end = e[size];
    return TRUE;
}

// 8011F938
// Fixed slot for a fossil (by its position in the fossil set), or 10.
int getFossilSlot(const dItem::Item *item, int start, int end, BOOL wish) {
    if (!wish) {
        return 10;
    }

    if (item->mId == dItem::ITEM_ID_NONE) {
        return 10;
    }

    dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    if (bitm == NULL || bitm->getKind() != dItem::KIND_FOSSIL) {
        return 10;
    }

    int size = getFtrSize(bitm);
    int res = 10;
    if (size >= 1 && size <= 2) {
        int fossil = 0;
        u32 raw = bitm->m_fossil;
        if (raw < 0x1C) {
            fossil = raw;
        }

        if (fossil != 0 && (u32)fossil < 0x1C) {
            u32 num = dItem::seeker_c::get()->searchFossil(fossil);
            if (num != 0 && num <= (u32)(end - start)) {
                u32 idx = dItem::seeker_c::get()->findFossil(*item);
                if (idx < num) {
                    res = start + idx;
                }
            }
        }
    }
    return res;
}

// 8011FA74
// Fossil set of an item, or 0.
int getFossilSet(const dItem::Item *item) {
    if (item->mId == dItem::ITEM_ID_NONE) {
        return 0;
    }

    dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    if (bitm == NULL || bitm->getKind() != dItem::KIND_FOSSIL) {
        return 0;
    }

    int fossil = 0;
    u32 raw = bitm->m_fossil;
    if (raw < 0x1C) {
        fossil = raw;
    }

    if (fossil != 0 && fossil < 0x1C) {
        return fossil;
    }
    return 0;
}

// 8011FB14
u32 dAnimal_c::countFossilSetFtr(int fossil, int start, int end) {
    u32 count = 0;
    for (int i = start; i < end; i++) {
        dItem::Item item = getFtr(i);
        int f = getFossilSet(&item);
        if (f != 0 && f == fossil) {
            count++;
        }
    }
    return count;
}

// 8011FB94
// Puts an item into a furniture slot. *out gets the replaced item (TRUE) or the item
// itself (FALSE).
BOOL dAnimal_c::placeFtr(const dItem::Item *item, int idx, BOOL notify, dItem::Item *out) {
    dItem::Item dummy;
    if (out == NULL) {
        out = &dummy;
    }
    *out = dItem::Item();

    if (item->mId == dItem::ITEM_ID_NONE) {
        return FALSE;
    }

    dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    if (bitm == NULL) {
        return FALSE;
    }

    if (isHouseItem(item)) {
        dQuestWish_c *wish = &mQuest.mWish;
        int size = getFtrSize(bitm);
        wish->isWishItem(item);
        BOOL isWish = wish->isValid() ? wish->mKind == 2 : FALSE;

        int start = 10;
        int end = 10;
        if (!getFtrSlotRange(&start, &end, size, isWish)) {
            return FALSE;
        }

        int slot = getFossilSlot(item, start, end, isWish);
        if (slot < 10) {
            dItem::Item cur = getFtr(slot);
            if (cur.isSame(*item)) {
                *out = *item;
                return FALSE;
            }

            if (cur.mId != dItem::ITEM_ID_NONE) {
                u32 curNum = countFossilSetFtr(getFossilSet(&cur), start, end);
                u32 newNum = countFossilSetFtr(getFossilSet(item), start, end);
                if (newNum + 1 < curNum) {
                    *out = *item;
                    return FALSE;
                }
            }

            if (setFtr(slot, item)) {
                if (notify) {
                    fn_800F0DD4(idx, slot, item);
                }
                *out = cur;
                return TRUE;
            }

            *out = *item;
            return FALSE;
        }

        int pick = 10;
        u32 count = 0;
        for (int i = start; i < end; i++) {
            dItem::Item cur = getFtr(i);
            if (cur.mId == dItem::ITEM_ID_NONE) {
                count++;
                f32 rate = 100.0f / count;
                if (cM::rndF(100.0f) <= rate) {
                    pick = i;
                }
            }
        }

        if (pick == 10) {
            count = 0;
            for (int i = start; i < end; i++) {
                dItem::Item cur = getFtr(i);
                if (!wish->isWishItem(&cur)) {
                    count++;
                    f32 rate = 100.0f / count;
                    if (cM::rndF(100.0f) <= rate) {
                        pick = i;
                    }
                }
            }
        }

        if (pick == 10) {
            count = 0;
            for (int i = start; i < end; i++) {
                dItem::Item cur = getFtr(i);
                if (wish->isWishItem(&cur)) {
                    count++;
                    f32 rate = 100.0f / count;
                    if (cM::rndF(100.0f) <= rate) {
                        pick = i;
                    }
                }
            }
        }

        if (pick < 10) {
            dItem::Item cur = getFtr(pick);
            if (setFtr(pick, item)) {
                if (notify) {
                    fn_800F0DD4(idx, pick, item);
                }
                *out = cur;
                return TRUE;
            }
        }
    }

    *out = *item;
    return FALSE;
}

// 8011FFE0
// 10% chance to throw out a random furniture item that is not wished for.
BOOL dAnimal_c::throwAwayFtr() {
    if (cM::rndF(100.0f) < 90.0f) {
        return FALSE;
    }

    u32 pick = 10;
    dQuestWish_c *wish = &mQuest.mWish;
    u32 count = 0;
    for (int i = 0; i < 10; i++) {
        dItem::Item item = getFtr(i);
        if (item.mId != dItem::ITEM_ID_NONE && !wish->isWishItem(&item)) {
            count++;
            f32 rate = 100.0f / count;
            if (cM::rndF(100.0f) < rate) {
                pick = i;
            }
        }
    }

    if (pick < 10) {
        dItem::Item item = getFtr(pick);
        clearFtr(pick);
        dSaveData_c::getTown()->mRecycleBin.add(item.mId);
        return TRUE;
    }
    return FALSE;
}

// 80120124
dItem::Item getMovingBox(const dItem::Item *item) {
    static const dItem::Item sItems[3] = {dItem::Item(dItem::ITEM_IDX_NOT_USED_FTR_03), dItem::Item(dItem::ITEM_IDX_NOT_USED_FTR_02), dItem::Item(dItem::ITEM_IDX_NOT_USED_FTR_01)};

    if (item->mId != dItem::ITEM_ID_NONE) {
        dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
        if (bitm != NULL) {
            u32 size = getFtrSize(bitm);
            if (size < 3) {
                return sItems[size];
            }
        }
    }
    return dItem::Item();
}

// 80120238
// Replaces the furniture placeholders of a 16x16 room with the villager's furniture.
void dAnimal_c::applyRoomFtr(dItem::Item *room) {
    if (room == NULL) {
        return;
    }

    BOOL flag = isMoving();
    for (int i = 0; i < 256; i++, room++) {
        if (isFurnitureSlotId(room->mId)) {
            u32 slot = getFtrSlotOfPlaceholder(room);
            if (slot < 10) {
                dItem::Item item = getFtr(slot);
                BOOL ok = isHouseItem(&item);
                if (flag && ok) {
                    if (isFtrBoxed(slot)) {
                        dItem::Item alt = getMovingBox(&item);
                        dItem::Item copy = alt;
                        if (alt.mId != dItem::ITEM_ID_NONE) {
                            item = copy;
                        }
                    } else {
                        ok = FALSE;
                    }
                }

                if (ok) {
                    item = item.withVariant(room->mId & 3);
                    *room = item;
                }
            }

            if (isFurnitureSlotId(room->mId)) {
                room->mId = dItem::ITEM_ID_NONE;
            }
        }
    }
}

struct dAnimalItemRange_c {
    dAnimalItemRange_c(s32 kind, s32 sub) {
        mKind = kind;
        mSub = sub;
    }

    s32 mKind;
    s32 mSub;
};

// 80120388
// Drops furniture that no longer exists and fills the house up to 5 pieces.
// The wallpaper/carpet items are temporaries (stack slots in expression order).
static inline const dItem::Item *ptrTo(const dItem::Item &item) {
    return &item;
}

void dAnimal_c::validateHouse() {
    if (!mID.isValid()) {
        return;
    }

    for (int i = 0; i < 10; i++) {
        dItem::Item item = getFtr(i);
        dItem::Item copy = item;
        if (item.mId != dItem::ITEM_ID_NONE) {
            dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(copy);
            if (bitm == NULL) {
                clearFtr(i);
            }
        }
    }

    u32 count = countFtr();
    u32 i = 0;
    static dAnimalItemRange_c sRange(dItem::KIND_FTR, 0);
    for (; i < 10 && count < 5; i++) {
        dItem::Item item;
        if (fn_800C60B4(&item, 1, &sRange.mKind, 1, lbl_8059FF80, 0, 0, 0) && item.mId != dItem::ITEM_ID_NONE &&
            placeFtr(&item, -1, FALSE, NULL)) {
            count++;
        }
    }

    dItem::BITM *wall = dItem::infoBank_c::get()->getBITM(*getWall());
    if (wall == NULL) {
        setWall(ptrTo(dItem::Item((u16)(int)mTemplate.mWall)));
    }

    dItem::BITM *carpet = dItem::infoBank_c::get()->getBITM(*getCarpet());
    if (carpet == NULL) {
        setCarpet(ptrTo(dItem::Item((u16)(int)mTemplate.mCarpet)));
    }
}

// 80120548
void dAnimal_c::clearFossilRoomFtr() {
    clearFtr(1);
    clearFtr(2);
}

// 80120584
void dAnimal_c::clearNewItems() {
    mNewItems[0] = dItem::ITEM_ID_NONE;
    mNewItems[1] = dItem::ITEM_ID_NONE;
    mNewItems[2] = dItem::ITEM_ID_NONE;
    mNewItems[3] = dItem::ITEM_ID_NONE;
}

// 801205A0
dItem::Item *dAnimal_c::fn_801205A0(u32 idx) {
    if (idx < 4) {
        return &mNewItems[idx];
    }
    return NULL;
}

// 801205C0
dItem::Item *dAnimal_c::fn_801205C0(u32 idx) {
    if (idx < 4) {
        return &mNewItems[idx];
    }
    return NULL;
}

// 801205E0
// Moves the _2FFC items to the front.
void dAnimal_c::packNewItems() {
    for (int i = 0; i < 3; i++) {
        if (mNewItems[i].mId == dItem::ITEM_ID_NONE) {
            int j;
            for (j = i + 1; j < 4; j++) {
                if (mNewItems[j].mId != dItem::ITEM_ID_NONE) {
                    mNewItems[i] = mNewItems[j];
                    mNewItems[j] = dItem::ITEM_ID_NONE;
                    break;
                }
            }
            if (j == 4) {
                return;
            }
        }
    }
}

// 80120664
int dAnimal_c::findNewItem(const dItem::Item *item) {
    dItem::Item *p = fn_801205C0(0);
    for (int i = 0; i < 4; i++, p++) {
        if (item->isSame(*p)) {
            return i;
        }
    }
    return -1;
}

// 801206DC
BOOL isNewItemKind(const dItem::Item *item) {
    if (item->mId == dItem::ITEM_ID_NONE) {
        return FALSE;
    }

    dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    if (bitm == NULL) {
        return FALSE;
    }

    switch (bitm->getKind()) {
    case dItem::KIND_WALL:
    case dItem::KIND_CARPET:
    case dItem::KIND_FTR:
    case dItem::KIND_CLOTH:
    case dItem::KIND_CAP:
    case dItem::KIND_ACC:
    case dItem::KIND_INSECT:
    case dItem::KIND_FISH:
    case dItem::KIND_FOSSIL:
    case dItem::KIND_HANIWA:
    case dItem::KIND_PICTURE:
    case dItem::KIND_MUSIC:
    case dItem::KIND_FAKE_PICTURE_BEFORE:
    case dItem::KIND_FAKE_PICTURE_AFTER:
    case dItem::KIND_UMBRELLA:
        return TRUE;
    }
    return FALSE;
}

// 80120794
BOOL dAnimal_c::addNewItem(const dItem::Item *item) {
    if (isNewItemKind(item)) {
        packNewItems();
        dItem::Item none;
        u32 idx = findNewItem(&none);
        if (idx < 4) {
            mNewItems[idx] = *item;
            return TRUE;
        }

        mNewItems[0] = dItem::ITEM_ID_NONE;
        packNewItems();
        mNewItems[3] = *item;
        return TRUE;
    }
    return FALSE;
}

// 80120844
BOOL dAnimal_c::removeNewItem(const dItem::Item *item) {
    if (item->mId == dItem::ITEM_ID_NONE) {
        return FALSE;
    }

    u32 idx = findNewItem(item);
    if (idx < 4) {
        mNewItems[idx] = dItem::ITEM_ID_NONE;
        packNewItems();
        return TRUE;
    }
    return FALSE;
}

// 801208B4
void dAnimal_c::validateNewItems() {
    dItem::Item *p = mNewItems;
    for (int i = 0; i < 4; i++, p++) {
        if (p->mId != dItem::ITEM_ID_NONE) {
            dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(dItem::Item(p->mId));
            if (bitm == NULL) {
                *p = dItem::ITEM_ID_NONE;
            }
        }
    }
    packNewItems();
}

// 80120944
// Random _2FFC item not in exclude.
dItem::Item *dAnimal_c::pickNewItem(const dItem::Item *exclude, u32 num, BOOL all) {
    const dItem::Item *e;
    dItem::Item *p = fn_801205C0(0);
    int count = 0;
    u32 mask = 0;
    for (int i = 0; i < 4; i++, p++) {
        if (p->mId != dItem::ITEM_ID_NONE) {
            dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(dItem::Item(p->mId));
            if (bitm != NULL && (all || !bitm->m_noPurchase)) {
                e = exclude;
                BOOL found = FALSE;
                for (u32 j = 0; j < num; e++, j++) {
                    if (p->isSame(*e)) {
                        found = TRUE;
                        break;
                    }
                }
                if (!found) {
                    mask |= 1 << i;
                    count++;
                }
            }
        }
    }
    return fn_801205C0(pickRandomBit(mask, count, 4));
}

// 80120A50
// Most valuable-first pick among the _2FFC items not in exclude.
dItem::Item *dAnimal_c::pickPricedNewItem(const dItem::Item *exclude, u32 num, BOOL all) {
    const dItem::Item *e;
    dItem::Item *res = NULL;
    dItem::Item *p = fn_801205C0(0);
    u32 count = 0;
    for (int i = 0; i < 4; i++, p++) {
        if (p->mId != dItem::ITEM_ID_NONE) {
            dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(dItem::Item(p->mId));
            if (bitm != NULL && (all || !bitm->m_noPurchase)) {
                e = exclude;
                BOOL found = FALSE;
                for (u32 j = 0; j < num; e++, j++) {
                    if (p->isSame(*e)) {
                        found = TRUE;
                        break;
                    }
                }
                if (!found) {
                    int price = p->getPrice();
                    if (price > 0) {
                        res = p;
                        count = 1;
                    } else if (price == 0) {
                        f32 rate = 100.0f / (count + 1);
                        if (cM::rndF(100.0f) <= rate) {
                            res = p;
                        }
                        count++;
                    }
                }
            }
        }
    }
    return res;
}

// 80120BC0
// Random _2FFC item of a kind, other than exclude.
// The target materialises the negated result (cntlzw) before testing it.
static inline BOOL isNotSame(const dItem::Item *a, const dItem::Item *b) {
    return !a->isSame(*b);
}

dItem::Item dAnimal_c::pickNewItemOfKind(int kind, const dItem::Item *exclude) {
    dItem::Item res;
    u32 count = 0;
    for (int i = 0; i < 4; i++) {
        dItem::Item *p = fn_801205C0(i);
        if (p != NULL && p->mId != dItem::ITEM_ID_NONE && isNotSame(p, exclude)) {
            dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(dItem::Item(p->mId));
            if (bitm != NULL && kind == bitm->getKind()) {
                count++;
                f32 rate = 100.0f / count;
                if (cM::rndF(100.0f) <= rate) {
                    res = *p;
                }
            }
        }
    }
    return res;
}

// 80120D00
// Daily update of the villager's wallpaper, carpet, music and furniture from the _2FFC items.
void dAnimal_c::applyNewItems(u8 idx, BOOL notify) {
    if (!mID.isValid()) {
        return;
    }
    if (idx >= ANIMAL_NUM) {
        return;
    }
    if (isMoving()) {
        return;
    }

    dRecycleBin_c *bin = &dSaveData_c::getTown()->mRecycleBin;
    BOOL noBin = FALSE;
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        noBin = TRUE;
    }

    dItem::Item cur = *getWall();
    dItem::Item next = pickNewItemOfKind(dItem::KIND_WALL, &cur);
    if (next.mId != dItem::ITEM_ID_NONE) {
        if (cur.mId != dItem::ITEM_ID_NONE && !noBin && cM::rndF(100.0f) < 10.0f) {
            bin->add(cur.mId);
        }
        setWall(&next);
        if (notify) {
            fn_800F109C(idx, getWall());
        }
    }

    cur = *getCarpet();
    next = pickNewItemOfKind(dItem::KIND_CARPET, &cur);
    if (next.mId != dItem::ITEM_ID_NONE) {
        if (cur.mId != dItem::ITEM_ID_NONE && !noBin && cM::rndF(100.0f) < 10.0f) {
            bin->add(cur.mId);
        }
        setCarpet(&next);
        if (notify) {
            fn_800F1154(idx, getCarpet());
        }
    }

    if (cM::rndF(100.0f) < 50.0f) {
        cur = getMusic();
        next = pickNewItemOfKind(dItem::KIND_MUSIC, &cur);
        if (next.mId != dItem::ITEM_ID_NONE) {
            setMusic(&next);
            if (notify) {
                fn_800F120C(idx, ptrTo(getMusic()));
            }
        }
    }

    dQuestBase_c *quest;
    dItem::Item best[3];
    for (int i = 0; i < 4; i++) {
        dItem::Item *p = fn_801205A0(i);
        if (p != NULL && p->mId != dItem::ITEM_ID_NONE) {
            dItem::Item got;
            placeFtr(p, idx, notify, &got);
            if (!noBin) {
            dItem::Item copy = got;
            if (got.mId != dItem::ITEM_ID_NONE) {
                dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(copy);
                if (bitm != NULL) {
                    int kind = bitm->getKind();
                    u32 size = 3;
                    switch (kind) {
                    case dItem::KIND_FTR:
                    case dItem::KIND_FOSSIL:
                    case dItem::KIND_HANIWA:
                        size = getFtrSize(bitm);
                        break;
                    case dItem::KIND_CLOTH:
                    case dItem::KIND_PICTURE:
                    case dItem::KIND_FAKE_PICTURE_BEFORE:
                    case dItem::KIND_FAKE_PICTURE_AFTER:
                    case dItem::KIND_UMBRELLA:
                        if (dItem::clampField(bitm->m_ftrFunc, 0x41, 1) != 0) {
                            size = getFtrSize(bitm);
                        }
                        break;
                    }

                    if (size < 3) {
                        dItem::Item *slot = &best[size];
                        if (slot->mId == dItem::ITEM_ID_NONE) {
                            *slot = got;
                        } else {
                            int price = got.getPrice();
                            if (price > 0) {
                                int slotPrice = slot->getPrice();
                                if (slotPrice > 0) {
                                    if (price < slotPrice) {
                                        *slot = got;
                                    } else if (price == slotPrice && cM::rndF(100.0f) < 50.0f) {
                                        *slot = got;
                                    }
                                } else {
                                    *slot = got;
                                }
                            }
                        }
                    }
                }
            }
            }
        }
    }

    if (!noBin) {
        int i;
        dItem::Item *p = best;
        for (i = 0; i < 3; i++, p++) {
            if (p->mId != dItem::ITEM_ID_NONE && cM::rndF(100.0f) < 50.0f) {
                bin->add(p->mId);
            }
        }
    }

    if (countNewItems()) {
        clearNewItems();
        if (notify) {
            fn_800F0F3C(idx);
        }
    }

    quest = &mQuest.mQuest.mBase;
    if (quest->isActive()) {
        u8 state = quest->mState;
        switch (quest->mKind) {
        case 2:
            if (state == 1 || state == 2) {
                quest->mState = 3;
                if (notify) {
                    fn_800F12D8(idx, 2, 3);
                }
            }
            break;
        case 4:
            if (state == 1 || state == 2) {
                quest->mState = 3;
                if (notify) {
                    fn_800F12D8(idx, 4, 3);
                }
            }
            break;
        }
    }
}

// Temporary declarations (chunk 2)
extern "C" {
// This TU, other chunks.

// Other TUs.
extern u8 lbl_8059FF80[0x10];
struct dNpcPair_c;
u32 fn_800DCF30();
u16 fn_8016AF68(const char *group);
BOOL fn_8016AE68(dScript::Word_c *word, u16 index, const char *group);
u16 fn_800CBA14(const char *label);
u16 fn_800CBA40(const char *label);
u16 fn_800CBA6C(const char *label);
u16 fn_800CBA98(const char *label);
u16 fn_800CBAC4(const char *label);
int fn_80103BC8(dMail_c *mail);
int fn_80103C10(dMail_c *mail);
u16 fn_80103D24(dMail_c *mail);
BOOL fn_80102BBC(dMail_c *mail);
int fn_801017B8();
}

// C++ linkage (mangled in symbols.txt), d_npc.
void fn_800F46CC(const dAnmPersonalID_c *animal, int slot);

// Same layout as d_npc's pair table entries for fn_800C60B4.
struct dNpcPair_c {
    dNpcPair_c(int a, int b) : mA(a), mB(b) {}

    /* 0x0 */ int mA;
    /* 0x4 */ int mB;
};

static inline BOOL isItemKind(const dItem::Item &item, int kind) {
    dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(item);
    return bitm != NULL ? bitm->getKind() == kind : FALSE;
}

static inline u32 &memFlagWord(dAnimalMemory_c *mem) {
    return *(u32 *)&mem->mFlags;
}

// 80121234
u32 dAnimal_c::countNewItems() {
    dItem::Item *items = fn_801205C0(0);
    u32 num = 0;
    for (int i = 0; i < 4; i++) {
        if (items[i].mId != dItem::ITEM_ID_NONE) {
            num++;
        }
    }
    return num;
}

// 801212A0
dItem::Item dAnimal_c::pickOwnItem() {
    if (!mID.isValid()) {
        return dItem::Item();
    }
    if (isMoving()) {
        return dItem::Item();
    }

    dQuestVillagerWish_c *quest = &mQuest;
    int kind = quest->mWish.isValid() ? quest->mWish.mKind : 8;
    dQuestBase_c *base = &quest->mQuest.mBase;
    dItem::Item prev = base->isActive() ? base->mItem : dItem::Item();
    dTime_c *now = dTime_c::getCurrent();
    dItem::Item item;

    switch (kind) {
    case 0:
        if (cM::rndF(100.0f) < 50.0f) {
            u32 idx = fn_800BD6A8(now, 4, 0, 1);
            if (idx < 0x40) {
                item.setFromIndex(dItem::ITEM_IDX_COMMON_BUTTERFLY, idx, 0);
                if (item.isSame(prev)) {
                    item = dItem::ITEM_ID_NONE;
                }
            }
        }
        break;
    case 1:
        if (cM::rndF(100.0f) < 50.0f) {
            u32 idx = fn_80091370(now, 4, 0, 1);
            if (idx < 0x40) {
                item.setFromIndex(dItem::ITEM_IDX_BITTERLING, idx, 0);
                if (item.isSame(prev)) {
                    item = dItem::ITEM_ID_NONE;
                }
            }
        }
        break;
    case 2: {
        f32 rnd = cM::rndF(100.0f);
        if (rnd < 30.0f) {
            static dNpcPair_c sPair(0xB, 0x5C);
            // Integer select: the target computes this branchless.
            const dItem::Item *exclude = (const dItem::Item *)(prev.mId != dItem::ITEM_ID_NONE ? (int)&prev : 0);
            fn_800C60B4(&item, 1, &sPair, 1, lbl_8059FF80, exclude, exclude != NULL, 0);
            if (item.isSame(prev)) {
                item = dItem::ITEM_ID_NONE;
            }
        } else if (rnd < 50.0f) {
            static dNpcPair_c sPair(0xC, 0x5C);
            fn_800C60B4(&item, 1, &sPair, 1, lbl_8059FF80, NULL, FALSE, 0);
            if (item.isSame(prev)) {
                item = dItem::ITEM_ID_NONE;
            }
        }
        break;
    }
    case 4:
        if (cM::rndF(100.0f) < 50.0f) {
            dItem::seriesCandCB_c cb(mTemplate.mFavFtrSeries, prev, TRUE);
            dItem::seeker_c::get()->search(3, 6, &cb);
            item = dItem::seeker_c::get()->getRandom();
            if (item.isSame(prev)) {
                item = dItem::ITEM_ID_NONE;
            }
        }
        break;
    case 3:
        if (cM::rndF(100.0f) < 30.0f) {
            int style = mTemplate.mLikedStyle;
            dItem::clothCandCB_c cb(mCloth, style, 10, TRUE);
            dItem::seeker_c::get()->search(4, 6, &cb);
            item = dItem::seeker_c::get()->getRandom();
            if (item.isSame(prev)) {
                item = dItem::ITEM_ID_NONE;
            }
        }
        break;
    }

    static dNpcPair_c sDefault(3, 0);
    if (item.mId == dItem::ITEM_ID_NONE) {
        if (!fn_800C60B4(&item, 1, &sDefault, 1, lbl_8059FF80, NULL, FALSE, 0)) {
            item = dItem::ITEM_ID_NONE;
        }
    }
    if (item.isSame(prev)) {
        item = dItem::ITEM_ID_NONE;
    }
    return item;
}

// 80121738
BOOL dAnimal_c::updateOwnItems() {
    if (!mID.isValid()) {
        return FALSE;
    }
    if (isMoving()) {
        return FALSE;
    }
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        return FALSE;
    }

    u32 num = countFtr();
    dItem::Item item;
    if (num <= 5) {
        if (countNewItems() < 4 && cM::rndF(100.0f) < 50.0f) {
            item = pickOwnItem();
            if (item.mId != dItem::ITEM_ID_NONE) {
                addNewItem(&item);
            }
        }
    } else if (num == 6 || num == 7) {
        if (countNewItems() < 4 && cM::rndF(100.0f) < 25.0f) {
            item = pickOwnItem();
            if (item.mId != dItem::ITEM_ID_NONE) {
                addNewItem(&item);
            }
        }
    } else if (num >= 8) {
        throwAwayFtr();
    }
    return FALSE;
}

// 8012188C
int dAnimal_c::getRoomLayout(int *type) {
    int dummy = 1;
    dQuestWish_c *wish;
    int value = 0;
    int *out = type;
    if (out == NULL) {
        out = &dummy;
    }
    *out = 1;

    if (mID.isValid()) {
        value = mTemplate.mRoomLayout;
        wish = &mQuest.mWish;
        if (wish->isValid() && (int)wish->mKind == 2) {
            *out = 2;
            value %= 10;
        }
    }
    return value;
}

// 8012194C
void dAnimal_c::setCloth(const dItem::Item *item) {
    dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    if (bitm != NULL) {
        switch (bitm->getKind()) {
        case dItem::KIND_CLOTH:
            mCloth = *item;
            break;
        case dItem::KIND_TA_CLOTH: {
            dItem::seeker_c *seeker = dItem::seeker_c::get();
            seeker->search(dItem::KIND_TA_CLOTH, 6, NULL);
            wearTailorDesign(seeker->find(*item));
            break;
        }
        }
    }
}

// 80121A10
BOOL dAnimal_c::wearTailorDesign(u32 idx) {
    if (idx < 8 && setDesignFromTailor(idx)) {
        mCloth.setFromIndex(dItem::ITEM_IDX_ORG_CLOTH_00);
        return TRUE;
    }
    return FALSE;
}

// 80121A64
BOOL isOrgCloth(const dItem::Item *item) {
    dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    if (bitm != NULL && bitm->getKind() == dItem::KIND_ORG_CLOTH) {
        return TRUE;
    }
    return FALSE;
}

// 80121AC8
BOOL dAnimal_c::isWearingOrgCloth() {
    return isOrgCloth(&mCloth);
}

// 80121AD0
BOOL dAnimal_c::isWearingCloth() {
    dItem::Item item = mCloth;
    if (item.mId != dItem::ITEM_ID_NONE) {
        dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(item);
        if (bitm != NULL && bitm->getKind() == dItem::KIND_CLOTH) {
            return TRUE;
        }
    }
    return FALSE;
}

// 80121B3C
BOOL dAnimal_c::setDesignFromTailor(u32 idx) {
    if (idx < 8) {
        mClothDesign = *fn_80147F80(dSaveData_c::getTown()->_05EC80, idx);
        return TRUE;
    }
    return FALSE;
}

// 80121D94
BOOL dAnimal_c::wearDesign(const dDesign_c *design) {
    if (design->mCreator.isValid()) {
        mClothDesign = *design;
        mCloth.setFromIndex(dItem::ITEM_IDX_ORG_CLOTH_00);
        return TRUE;
    }
    return FALSE;
}

// 80121FEC
// 0 or 1 if the clothing's style is one of the villager's two styles, else 2.
int dAnimal_c::getStyleMatch(const dItem::Item *item) {
    dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    int result = 2;
    if (bitm != NULL && bitm->getKind() == dItem::KIND_CLOTH) {
        int style = 0;
        if (bitm->m_style < STYLE_NUM) {
            style = bitm->m_style;
        }
        if (style == mTemplate.mLikedStyle) {
            result = 0;
        } else if (style == mTemplate.mDislikedStyle) {
            result = 1;
        }
    }
    return result;
}

// 8012209C
void dAnimal_c::setWall(const dItem::Item *item) {
    if (item->mId != dItem::ITEM_ID_NONE && isItemKind(*item, dItem::KIND_WALL)) {
        mWall = *item;
    }
}

// 80122130
dItem::Item *dAnimal_c::getWall() {
    return &mWall;
}

// 80122138
void dAnimal_c::setCarpet(const dItem::Item *item) {
    if (item->mId != dItem::ITEM_ID_NONE && isItemKind(*item, dItem::KIND_CARPET)) {
        mCarpet = *item;
    }
}

// 801221CC
dItem::Item *dAnimal_c::getCarpet() {
    return &mCarpet;
}

// 801221D4
dItem::Item dAnimal_c::getMusic() {
    int id = mTemplate.mMusic;
    return dItem::Item((u16)id);
}

// 801221E0
void dAnimal_c::setMusic(const dItem::Item *item) {
    if (item->mId != dItem::ITEM_ID_NONE && isItemKind(*item, dItem::KIND_MUSIC)) {
        (u16 &)mTemplate.mMusic = item->mId;
    }
}

// 80122274
void dAnimal_c::setMovingIn() {
    mIsMoving = 1;
    mIsMovingIn = 1;
}

// 80122284
BOOL dAnimal_c::isMovingIn() {
    BOOL result = FALSE;
    if (isMoving() && mIsMovingIn) {
        result = TRUE;
    }
    return result;
}

// 801222D8
void dAnimal_c::setMovingOut() {
    mIsMoving = 1;
    mIsMovingIn = 0;
}

// 801222EC
BOOL dAnimal_c::isMovingOut() {
    BOOL result = FALSE;
    if (isMoving() && !mIsMovingIn) {
        result = TRUE;
    }
    return result;
}

// 80122340
void dAnimal_c::clearMoving() {
    mIsMoving = 0;
    mIsMovingIn = 0;
}

// 80122350
BOOL dAnimal_c::isMoving() {
    return mIsMoving != 0;
}

// 80122368
void dAnimal_c::setQuestStarted() {
    mQuestStarted = 1;
}

// 80122378
void dAnimal_c::clearQuestStarted() {
    mQuestStarted = 0;
}

// 80122388
BOOL dAnimal_c::isQuestStarted() {
    return mQuestStarted != 0;
}

// 801223A0
void dAnimal_c::clearPlaceChangeTime() {
    mPlaceChangeTime.reset();
}

// 801223A8
dTime_c dAnimal_c::getPlaceChangeTime() {
    return mPlaceChangeTime.get();
}

// 801223B0
BOOL dAnimal_c::isPlaceChangeTimeSet() {
    return mPlaceChangeTime.isNone() == FALSE;
}

// 801223DC
void dAnimal_c::setPlaceChangeTime(int mins, const dTime_c *time) {
    if (time == NULL) {
        time = dTime_c::getCurrent();
    }
    dTime_c t = *time;
    t.add(0, 0, mins, 0);
    mPlaceChangeTime.set(&t);
}

// 80122498
BOOL dAnimal_c::isPlaceChangeTimeReached(const dTime_c *now) {
    if (!isPlaceChangeTimeSet()) {
        return TRUE;
    }
    if (now == NULL) {
        now = dTime_c::getCurrent();
    }
    dTime_c time = getPlaceChangeTime();
    if (dTime_c::isSameOrAfter(*now, time)) {
        return TRUE;
    }
    return FALSE;
}

// 801225F4
BOOL dAnimal_c::isVersionValid() {
    return getVersion() == 2;
}

// 80122620
s32 dAnimal_c::getVersion() {
    return mVersion;
}

// 80122628
void dAnimal_c::initVersion() {
    mVersion = 2;
}

// 80122634
dItem::Item dAnimal_c::getKey() {
    dSaveData_c *town = dSaveData_c::getTown();
    return town->mAnimals.getAnimalKey(&mID);
}

// 80122684
void dAnimal_c::setHabitFromWord(dScript::Word_c *word, int arg) {
    setHabit(word->getBuffer(), arg);
}

// 801226DC
void dAnimal_c::getHabitWord(dScript::Word_c *word, int arg, BOOL itchy) {
    static const char sItchy[] = "sys_STRING/STR_Itchy";
    if (itchy) {
        u16 num = fn_8016AF68(sItchy);
        u16 idx = num >= 1 ? (u16)(cM::rndInt(num - 1) + 1) : 1;
        fn_8016AE68(word, idx, sItchy);
    } else {
        const wchar_t *str = getHabit(arg);
        if (str != NULL) {
            word->set(str, 0);
        }
    }
}

// 80122770
void dAnimal_c::decHabitCooldown(int num) {
    if (num <= 0) {
        return;
    }
    if ((u32)num > mHabitCooldown) {
        mHabitCooldown = 0;
    } else {
        mHabitCooldown -= num;
    }
}

// 8012279C
// Takes a letter from a player: remembers it and handles its present.
BOOL dAnimal_c::receiveLetter(dMail_c *mail, int slot) {
    if (mail == NULL) {
        return FALSE;
    }

    dPersonalID_c *pid = mail->getFromPlayer();
    if (pid == NULL || !pid->isValid()) {
        return FALSE;
    }

    int idx = getMemoryIdx(pid);
    dAnimalMemory_c *mem = getMemory(idx);
    if (mem == NULL) {
        return FALSE;
    }

    memFlagWord(mem) |= 0x80000000;
    mLetter.copy(mail);
    dItem::Item present = mail->getPresent();
    if (present.mId != dItem::ITEM_ID_NONE) {
        if (addNewItem(&present)) {
            fn_800F0E9C(slot, &present);
        }
        if (mem->setPresent(&present)) {
            fn_800F1514(slot, idx, &present);
        }
    }
    return TRUE;
}

// 801228B4
BOOL dAnimal_c::hasAnyLetter() {
    if (!mID.isValid()) {
        return FALSE;
    }

    dAnimalMemory_c *mem = getMemory2(0);
    for (int i = 0; i < ANIMAL_MEMORY_NUM; i++, mem++) {
        if (mem->mPlayer.isValid() && mem->hasLetter()) {
            return TRUE;
        }
    }
    return FALSE;
}

// 8012294C
BOOL dAnimal_c::hasLetterFrom(const dPersonalID_c *pid) {
    if (!pid->isValid()) {
        return FALSE;
    }

    dPersonalID_c *from = mLetter.getFromPlayer();
    if (from == NULL || !from->isValid()) {
        return FALSE;
    }
    if (*pid != *from) {
        return FALSE;
    }

    dAnimalMemory_c *mem = findMemory2(pid);
    if (mem == NULL || !mem->hasLetter()) {
        return FALSE;
    }
    return TRUE;
}

static inline u16 rndCount(u16 num) {
    return num > 1 ? (u16)(cM::rndInt(num - 1) + 1) : 1;
}

static const char *sReSP[] = {"MAIL_BO_ReSP", "MAIL_HA_ReSP", "MAIL_KO_ReSP",
                              "MAIL_FU_ReSP", "MAIL_GE_ReSP", "MAIL_TA_ReSP"};
static const char *sReSP1[] = {"MAIL_BO_ReSP1", "MAIL_HA_ReSP1", "MAIL_KO_ReSP1",
                               "MAIL_FU_ReSP1", "MAIL_GE_ReSP1", "MAIL_TA_ReSP1"};
static const char *sReSP2[] = {"MAIL_BO_ReSP2", "MAIL_HA_ReSP2", "MAIL_KO_ReSP2",
                               "MAIL_FU_ReSP2", "MAIL_GE_ReSP2", "MAIL_TA_ReSP2"};
static const char *sReBad[] = {"MAIL_BO_ReBad", "MAIL_HA_ReBad", "MAIL_KO_ReBad",
                               "MAIL_FU_ReBad", "MAIL_GE_ReBad", "MAIL_TA_ReBad"};
static const char *sReply[] = {"MAIL_BO_Reply", "MAIL_HA_Reply", "MAIL_KO_Reply",
                               "MAIL_FU_Reply", "MAIL_GE_Reply", "MAIL_TA_Reply"};
static const char *sPresent[] = {"MAIL_BO_Present", "MAIL_HA_Present", "MAIL_KO_Present",
                                 "MAIL_FU_Present", "MAIL_GE_Present", "MAIL_TA_Present"};

// 80122A60
// Writes the villager's reply to a player's letter into sMail.
BOOL dAnimal_c::writeReplyLetter(dMail_c *mail, dPrivateData_c *player) {
    if (!mID.isValid()) {
        return FALSE;
    }

    dPersonalID_c *pid = &player->mPID;
    if (!pid->isValid() || !pid->isFromTown()) {
        return FALSE;
    }
    if (player->isFlag0(0xD)) {
        return FALSE;
    }

    u8 looks = mID.getLooks(1);
    if (looks >= LOOKS_TYPE_NUM) {
        return FALSE;
    }

    int kind = fn_80103C10(mail);
    BOOL send = FALSE;
    int delta = 0;
    switch (kind) {
    case 3: {
        const char *label = sReSP[looks];
        u16 footer = rndCount(fn_800CBA6C(label));
        u16 header = rndCount(fn_800CBA14(label));
        dItem::Item present;
        if (mail->getPresent() != dItem::ITEM_ID_NONE) {
            switch (fn_80103BC8(mail)) {
            case 1: {
                static dNpcPair_c sPair(3, 0);
                fn_800C60B4(&present, 1, &sPair, 1, lbl_8059FF80, NULL, FALSE, 0);
                break;
            }
            case 2:
                fn_800F45A4(&present, 1, lbl_8059FF80, NULL, 0);
                break;
            default: {
                static dNpcPair_c sPair(4, 0);
                if (cM::rndF(100.0f) < 50.0f) {
                    fn_800C60B4(&present, 1, &sPair, 1, lbl_8059FF80, NULL, FALSE, 0);
                }
                if (present.mId == dItem::ITEM_ID_NONE) {
                    present.setFromIndex(dItem::ITEM_IDX_APPLE, cM::rndInt(6), 0);
                }
                break;
            }
            }
        }

        const char *body = present.mId != dItem::ITEM_ID_NONE ? sReSP2[looks] : sReSP1[looks];
        u16 idx = fn_80103D24(mail);
        u16 num = fn_800CBA40(body);
        u16 bodyIdx = idx;
        if (idx >= num) {
            bodyIdx = rndCount(num);
        }
        dItem::Item paper(fn_800FABF4(looks, dTime_c::getCurrent()->getSeason()));
        fn_800F46CC(&mID, 0);
        sMail.clear();
        sMail.setupFromAnimal(&header, &bodyIdx, &footer, (int)label, (int)body, (int)label, &mID, pid,
                              &paper);
        dItem::Item copy = present;
        if (present.mId != dItem::ITEM_ID_NONE) {
            sMail.setPresent(copy.mId, 0xFF);
            delta = 5;
        } else {
            delta = 3;
        }
        send = TRUE;
        break;
    }
    case 1: {
        const char *label = sReBad[looks];
        u16 footer = rndCount(fn_800CBA6C(label));
        u16 header = rndCount(fn_800CBA14(label));
        u16 body = rndCount(fn_800CBA40(label));
        dItem::Item paper(fn_800FABF4(looks, dTime_c::getCurrent()->getSeason()));
        fn_800F46CC(&mID, 0);
        sMail.clear();
        sMail.setupFromAnimal(&header, &body, &footer, (int)label, (int)label, (int)label, &mID, pid,
                              &paper);
        send = TRUE;
        break;
    }
    case 2: {
        const char *label = sReply[looks];
        u16 num = fn_800CBAC4(label);
        u16 e = rndCount(num);
        u16 a = rndCount(num);
        dItem::Item present;
        if (mail->getPresent() != dItem::ITEM_ID_NONE) {
            switch (fn_80103BC8(mail)) {
            case 1: {
                static dNpcPair_c sPair(3, 0);
                fn_800C60B4(&present, 1, &sPair, 1, lbl_8059FF80, NULL, FALSE, 0);
                break;
            }
            case 2:
                fn_800F45A4(&present, 1, lbl_8059FF80, NULL, 0);
                break;
            default: {
                static dNpcPair_c sPair(4, 0);
                if (cM::rndF(100.0f) < 50.0f) {
                    fn_800C60B4(&present, 1, &sPair, 1, lbl_8059FF80, NULL, FALSE, 0);
                }
                if (present.mId == dItem::ITEM_ID_NONE) {
                    present.setFromIndex(dItem::ITEM_IDX_APPLE, cM::rndInt(6), 0);
                }
                break;
            }
            }
        }

        const char *body = present.mId != dItem::ITEM_ID_NONE ? sPresent[looks] : sReply[looks];
        u16 num2 = fn_800CBA98(body);
        u16 b = rndCount(num2);
        u16 c = rndCount(num2);
        u16 d = rndCount(num2);
        dItem::Item paper(fn_800FABF4(looks, dTime_c::getCurrent()->getSeason()));
        fn_800F46CC(&mID, 0);
        sMail.clear();
        sMail.setupFromAnimal(&a, &b, &c, &d, &e, (int)body, (int)label, &mID, pid, &paper);
        dItem::Item copy = present;
        if (present.mId != dItem::ITEM_ID_NONE) {
            sMail.setPresent(copy.mId, 0xFF);
            delta = 3;
        } else {
            delta = 1;
        }
        send = TRUE;
        break;
    }
    }

    if (delta != 0) {
        dAnimalMemory_c *mem = findMemory(pid);
        if (mem != NULL && mem->mPlayer.isValid()) {
            mem->addFriendship(delta);
        }
    }

    if (send && fn_80102BBC(&sMail)) {
        return TRUE;
    }
    return FALSE;
}

// 80123240
int dAnimal_c::getDayPlace(BOOL checkPlayer, BOOL arg2, BOOL useDefault) {
    if (!mID.isValid()) {
        return 3;
    }

    int result = 3;
    BOOL found = FALSE;
    if (checkPlayer) {
        int player = fn_801017B8();
        dSaveData_c *save = dSaveData_c::getRaw();
        found = save->mAnimals.mTown.isBirthdayHost(&mID, player, 1, 1, 1);
    }

    if (found) {
        result = 0;
    } else if (!isLostItemRequestDone(0, 0)) {
        result = 0;
    } else if (arg2) {
        result = 0;
    } else if (isMoving()) {
        result = 1;
    } else if ((int)mQuest.mQuest.mBase.mKind == 5) {
        result = 1;
    }

    if (useDefault && result == 3) {
        switch (mEvent.mPlace) {
        case 0:
            result = 0;
            break;
        case 1:
            result = 1;
            break;
        }
    }
    return result;
}

// 80123374
void dAnimal_c::clearTalkCounts() {
    dAnimalMemory_c *mem = getMemory(0);
    for (int i = 0; i < ANIMAL_MEMORY_NUM; i++, mem++) {
        mem->mTalkCount.clear();
    }
}

// 801233C8
void dAnimal_c::clearMemoryFlag24() {
    dAnimalMemory_c *mem = getMemory(0);
    for (int i = 0; i < ANIMAL_MEMORY_NUM; i++) {
        memFlagWord(&mem[i]) &= ~0x01000000;
    }
}

// 801234AC
void dAnimal_c::clearEventFlags() {
    dAnimalMemory_c *mem = getMemory(0);
    for (int i = 0; i < ANIMAL_MEMORY_NUM; i++) {
        mem[i].mEventFlags = 0;
    }
}

// 80123514
void dAnimal_c::clearMemoryFlag23() {
    dAnimalMemory_c *mem = getMemory(0);
    for (int i = 0; i < ANIMAL_MEMORY_NUM; i++) {
        memFlagWord(&mem[i]) &= ~0x00800000;
    }
}

// 801235F8
void dAnimal_c::fn_801235F8() {
    dAnimalMemory_c *mem = getMemory(0);
    for (int i = 0; i < ANIMAL_MEMORY_NUM; i++) {
        memFlagWord(&mem[i]) &= ~0x00000100;
    }
}

// 801236DC
void dAnimal_c::fn_801236DC() {
    dAnimalMemory_c *mem = getMemory(0);
    for (int i = 0; i < ANIMAL_MEMORY_NUM; i++) {
        memFlagWord(&mem[i]) &= ~0x00200000;
    }
}

// 801237C0
void dAnimal_c::fn_801237C0() {
    dAnimalMemory_c *mem = getMemory(0);
    for (int i = 0; i < ANIMAL_MEMORY_NUM; i++) {
        memFlagWord(&mem[i]) &= ~0x00100000;
    }
}

// 801238A4
void dAnimal_c::fn_801238A4() {
    dAnimalMemory_c *mem = getMemory(0);
    for (int i = 0; i < ANIMAL_MEMORY_NUM; i++) {
        memFlagWord(&mem[i]) &= ~0x00000800;
    }
}

// 80123988
void dAnimal_c::fn_80123988() {
    dAnimalMemory_c *mem = getMemory(0);
    for (int i = 0; i < ANIMAL_MEMORY_NUM; i++) {
        memFlagWord(&mem[i]) &= ~0x00000400;
    }
}

// 80123A6C
void dAnimal_c::fn_80123A6C() {
    dAnimalMemory_c *mem = getMemory(0);
    for (int i = 0; i < ANIMAL_MEMORY_NUM; i++) {
        memFlagWord(&mem[i]) &= ~0x00000200;
    }
}

// Temporary declarations (chunk 3)
extern "C" {
// This TU, other chunks.

// Other TUs.
u8 *fn_800AC28C();
u16 fn_800CBA14(const char *label);
u16 fn_800CBA40(const char *label);
u16 fn_800CBA6C(const char *label);
BOOL fn_801029C0(dMail_c *mail);
BOOL fn_80102BBC(dMail_c *mail);
extern u8 lbl_8059FF80[];
}

int getCategoryQ5(const dItem::Item &item);

// {a, b} pairs passed to fn_800C60B4 (cf. dNpcPair_c in d_npc.cpp).
struct dAnimalPair_c {
    dAnimalPair_c(int a, int b) : mA(a), mB(b) {}

    /* 0x0 */ int mA;
    /* 0x4 */ int mB;
};

// Item filter passed to fn_800F45A4 and friends: skips items in the player's catalog.
struct dAnimalCatalogFilter_c {
    dAnimalCatalogFilter_c(dCatalog_c *catalog, int kind) : mCatalog(catalog), _4(0), _8(kind) {}

    /* 0x0 */ dCatalog_c *mCatalog;
    /* 0x4 */ int _4;
    /* 0x8 */ int _8;
};

static inline int getBITMFossil(const dItem::BITM *bitm) {
    int fossil = 0;
    if ((u32)bitm->m_fossil < 0x1C) {
        fossil = bitm->m_fossil;
    }
    return fossil;
}

static inline int getBITMStyle(const dItem::BITM *bitm) {
    return bitm->m_style < STYLE_NUM ? (STYLE_LOOK)bitm->m_style : STYLE_UNSET;
}

static inline int getBITMSeries(const dItem::BITM *bitm) {
    int series = bitm->m_series;
    int ret = 0;
    if ((u32)series < 0x84) {
        ret = series;
    }
    return ret;
}

// 80123B50
void dAnimal_c::decNicknameWait(int n) {
    if (n > 0) {
        dAnimalMemory_c *memory = getMemory(0);
        for (int i = 0; i < ANIMAL_MEMORY_NUM; i++, memory++) {
            memory->decNicknameWait(n);
        }
    }
}

// 80123BBC
void dAnimal_c::decGreetingWait(int n) {
    if (n > 0) {
        dAnimalMemory_c *memory = getMemory(0);
        for (int i = 0; i < ANIMAL_MEMORY_NUM; i++, memory++) {
            memory->decGreetingWait(n);
        }
    }
}

// 80123C28
void dAnimal_c::growWish() {
    dQuestWish_c *wish = &mQuest.mWish;
    if (wish->isValid()) {
        wish->addValue(10);
    }
}

// 80123C6C
// Picks the reward for a finished errand: 0 random item, 1 villager's item, 2 money, 3 none.
u32 dAnimal_c::pickErrandReward(dItem::Item *item, int *price, dPrivateData_c *player) {
    static const u8 sRates[3] = {60, 20, 20};

    if (item == NULL || price == NULL) {
        return 3;
    }
    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayer();
    }
    if (player == NULL) {
        return 3;
    }
    if (!mID.isValid()) {
        return 3;
    }

    item->mId = dItem::ITEM_ID_NONE;
    *price = 0;
    u32 ret = 3;
    int r = cM::rndF(100.0f);
    for (u32 i = 0; i < 3; i++) {
        if (r < sRates[i]) {
            ret = i;
            break;
        }
        r -= sRates[i];
    }

    switch (ret) {
    case 0:
        fn_800F45A4(item, 1, lbl_8059FF80, NULL, 0);
        break;
    case 1: {
        dItem::Item *p = pickNewItem(NULL, 0, FALSE);
        if (p != NULL) {
            *item = *p;
        }
        break;
    }
    default:
        if ((u32)dSaveData_c::getTown()->mHomes.findOwner(player) < 4) {
            int rate = *fn_800AC28C();
            int room = player->getMoneyRoom(0);
            int amount = rate * 4 + 500;
            if (amount <= room) {
                *price = amount;
                item->setFromIndex(dItem::ITEM_IDX_100_BELLS);
                ret = 2;
            }
        }
        break;
    }

    if (item->mId == dItem::ITEM_ID_NONE) {
        fn_800F45A4(item, 1, lbl_8059FF80, NULL, 0);
        ret = 3;
        if (item->mId != dItem::ITEM_ID_NONE) {
            ret = 0;
        }
    }
    return ret;
}

static const char *sErrandThanksLabels[LOOKS_TYPE_NUM] = {
    "MAIL_BO_Q67", "MAIL_HA_Q67", "MAIL_KO_Q67", "MAIL_FU_Q67", "MAIL_GE_Q67", "MAIL_TA_Q67",
};

// 80123E5C
// Sends the thank-you letter for an errand delivered to other.
BOOL dAnimal_c::sendErrandThanksLetter(const dPersonalID_c *to, dAnmPersonalID_c *other, const dItem::Item *present, BOOL tryDirect) {
    if (!mID.isValid()) {
        return FALSE;
    }
    if (!other->isValid()) {
        return FALSE;
    }

    u8 looks = mID.getLooks(1);
    if (looks >= LOOKS_TYPE_NUM) {
        return FALSE;
    }

    const char *label = sErrandThanksLabels[looks];
    u16 n;
    n = fn_800CBA14(label);
    u16 a = n > 1 ? (u16)(cM::rndInt(n - 1) + 1) : 1;
    n = fn_800CBA40(label);
    u16 b = n > 1 ? (u16)(cM::rndInt(n - 1) + 1) : 1;
    n = fn_800CBA6C(label);
    u16 c = n > 1 ? (u16)(cM::rndInt(n - 1) + 1) : 1;
    dItem::Item paper(fn_800FABF4(looks, dTime_c::getCurrent()->getSeason()));

    fn_800F46CC(&mID, 0);
    fn_800F46CC(other, 1);
    looks = mID.getLooks(1);
    fn_800F4794(other->getGender(1), looks, 2);

    sMail.clear();
    sMail.setupFromAnimal(&a, &b, &c, (int)label, (int)label, (int)label, &mID, to, &paper);
    if (present->mId != dItem::ITEM_ID_NONE) {
        sMail.setPresent(present->mId, 0xFF);
    }

    if (tryDirect && fn_801029C0(&sMail)) {
        return TRUE;
    }
    if (fn_80102BBC(&sMail)) {
        return TRUE;
    }
    return FALSE;
}

// 80124084
// Completes a delivery errand (kind 7) at this villager.
BOOL dAnimal_c::completeErrandRequest(u32 idx, dPrivateData_c *player, BOOL tryDirect) {
    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayer();
    }
    if (player == NULL || !player->mPID.isValid()) {
        return FALSE;
    }
    if (!mID.isValid()) {
        return FALSE;
    }

    dQuestErrand_c *errand = player->mErrand.get(0);
    if (errand == NULL) {
        return FALSE;
    }
    if (!errand->mBase.isActive() || (int)errand->mBase.mKind != QUEST_KIND_ERRAND_REQUEST || errand->mBase.mState != 1 ||
        errand->_18E != 1) {
        return FALSE;
    }
    if (!(*errand->getAnimal(0) == mID)) {
        return FALSE;
    }

    dItem::Item item;
    int price = 0;
    int ret = pickErrandReward(&item, &price, player);
    if ((u32)ret >= 3) {
        return FALSE;
    }
    if (ret == 2) {
        item.setFromIndex(dItem::ITEM_IDX_500_BELLS);
    }

    if (sendErrandThanksLetter(&player->mPID, errand->getAnimal(1), &item, tryDirect)) {
        if (ret == 1 && removeNewItem(&item) && idx < ANIMAL_NUM) {
            fn_800F0FE4(idx, &item);
        }
        errand->clear();
        return TRUE;
    }
    return FALSE;
}

// 801242C8
// Like pickErrandReward, with the reward kind taken from a table: 0 random item, 1 villager's item,
// 2 money, 3 catalog item, 4 none.
u32 dAnimal_c::pickErrandFinalReward(dItem::Item *item, int *price, dPrivateData_c *player, u32 a, u32 b) {
    static const u8 sKinds[3][3] = {
        {0, 2, 1},
        {1, 0, 2},
        {1, 2, 0},
    };

    if (item == NULL || price == NULL) {
        return 4;
    }
    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayer();
    }
    if (player == NULL) {
        return 4;
    }
    if (!mID.isValid()) {
        return 4;
    }

    if (a >= 3) {
        a = 2;
    }
    if (b >= 3) {
        b = 2;
    }

    item->mId = dItem::ITEM_ID_NONE;
    *price = 0;
    int ret = sKinds[a][b];

    switch (ret) {
    case 0: {
        static dAnimalPair_c sPairs[6] = {
            dAnimalPair_c(1, 4), dAnimalPair_c(3, 4), dAnimalPair_c(2, 4),
            dAnimalPair_c(1, 5), dAnimalPair_c(3, 5), dAnimalPair_c(2, 5),
        };
        fn_800C60B4(item, 1, sPairs, 6, lbl_8059FF80, 0, 0, 0);
        break;
    }
    case 1: {
        dItem::Item *p = pickNewItem(NULL, 0, FALSE);
        if (p != NULL) {
            *item = *p;
        }
        break;
    }
    default:
        if ((u32)dSaveData_c::getTown()->mHomes.findOwner(player) < 4) {
            int rate = *fn_800AC28C();
            int room = player->getMoneyRoom(0);
            int amount = rate * 4 + 500;
            if (amount <= room) {
                *price = amount;
                item->setFromIndex(dItem::ITEM_IDX_100_BELLS);
                ret = 2;
            }
        }
        break;
    }

    if (item->mId == dItem::ITEM_ID_NONE) {
        dAnimalCatalogFilter_c filter(&player->mCatalog, 0x23);
        if (!fn_800F4608(item, 1, &filter, NULL, 0)) {
            fn_800F4608(item, 1, lbl_8059FF80, NULL, 0);
        }
        ret = 4;
        if (item->mId != dItem::ITEM_ID_NONE) {
            ret = 3;
        }
    }
    return ret;
}

// 8012453C
// Completes a delivery errand (kind 8) at this villager.
BOOL dAnimal_c::completeErrandRequestFinal(u32 idx, dPrivateData_c *player, BOOL tryDirect) {
    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayer();
    }
    if (player == NULL || !player->mPID.isValid()) {
        return FALSE;
    }
    if (!mID.isValid()) {
        return FALSE;
    }

    u32 state;
    dQuestErrand_c *errand = player->mErrand.get(0);
    if (errand == NULL) {
        return FALSE;
    }
    state = errand->mBase.mState;
    if (!errand->mBase.isActive() || (int)errand->mBase.mKind != QUEST_KIND_ERRAND_REQUEST_FINAL || state < 1 ||
        state >= 4 || errand->_18E == 0) {
        return FALSE;
    }
    if (!(*errand->getAnimal(0) == mID)) {
        return FALSE;
    }

    dItem::Item item;
    int price = 0;
    u32 a = state - 1;
    u32 b = errand->_18E - 1;
    if (b >= 3) {
        b = 2;
    }
    int ret = pickErrandFinalReward(&item, &price, player, a, b);
    if ((u32)ret >= 4) {
        return FALSE;
    }
    if (ret == 2) {
        item.setFromIndex(dItem::ITEM_IDX_500_BELLS);
    }

    if (sendErrandThanksLetter(&player->mPID, errand->getAnimal(1), &item, tryDirect)) {
        if (ret == 1 && removeNewItem(&item) && idx < ANIMAL_NUM) {
            fn_800F0FE4(idx, &item);
        }
        errand->clear();
        return TRUE;
    }
    return FALSE;
}

// 801247A0
// Ends an expired errand from this villager, flagging the errand item in the player's pockets.
BOOL dAnimal_c::expireErrandRequest(dPrivateData_c *player, BOOL enable) {
    if (!enable) {
        return FALSE;
    }
    if (player == NULL || !player->mPID.isValid()) {
        return FALSE;
    }
    if (!mID.isValid()) {
        return FALSE;
    }

    dQuestErrand_c *errand = player->mErrand.get(0);
    if (errand == NULL || errand->_18E != 0) {
        return FALSE;
    }
    if (!errand->mBase.isActive() || errand->mBase.getSubType() != QUEST_ERRAND_TYPE_REQUEST ||
        errand->mBase.mItem.mId == dItem::ITEM_ID_NONE) {
        return FALSE;
    }
    if (!(*errand->getAnimal(0) == mID)) {
        return FALSE;
    }

    if (errand->mBase.isExpired(NULL)) {
        for (int i = 0; i < PLAYER_POCKETS_COUNT; i++) {
            if (player->mPockets[i].isSame(errand->mBase.mItem) && player->getPocketFlag(i) == 2) {
                player->setPocketFlag(i, 4);
            }
        }

        dAnimalMemory_c *memory = findMemory(&player->mPID);
        if (memory != NULL) {
            memory->addFriendship(-10);
        }
        errand->clear();
        return TRUE;
    }
    return FALSE;
}

// 801249B4
void dAnimal_c::updateLostItemRequest() {
    if (mID.isValid()) {
        dQuestVillager_c *quest = &mQuest.mQuest;
        if (quest->mBase.isActive() && (int)quest->mBase.mKind == QUEST_KIND_REQUEST_6 && quest->mBase.mState != 2 &&
            quest->mBase.isPastDeadline(NULL)) {
            quest->mBase.mState = 2;
        }
    }
}

// 80124A34
BOOL dAnimal_c::clearExpiredRequest(dLostQuest_c *lost, BOOL enable) {
    if (!enable) {
        return FALSE;
    }
    if (!mID.isValid()) {
        return FALSE;
    }

    dQuestVillager_c *quest = &mQuest.mQuest;
    if (!quest->mBase.isActive()) {
        return FALSE;
    }
    if (quest->mBase.isExpired(NULL)) {
        if ((int)quest->mBase.mKind == QUEST_KIND_REQUEST_6) {
            lost->setTime(*dTime_c::getCurrent());
        }
        quest->clear();
        return TRUE;
    }
    return FALSE;
}

// 80124AF0
u32 dAnimal_c::pickInsectRequest(dItem::Item *item, u8 *arg) {
    item->setFromIndex(dItem::ITEM_IDX_COMMON_BUTTERFLY, fn_800BD6A8(dTime_c::getCurrent(), 4, 0, 1), FALSE);
    *arg = dQuestVillager_c::getInsectPriceRank(item);
    return 0;
}

// 80124B5C
u32 dAnimal_c::pickFishRequest(dItem::Item *item, u8 *arg) {
    item->setFromIndex(dItem::ITEM_IDX_BITTERLING, fn_80091370(dTime_c::getCurrent(), 4, 0, 1), FALSE);
    *arg = dQuestVillager_c::getFishPriceRank(item);
    return 0;
}

// 80124BC8
u32 dAnimal_c::pickFossilRequest(dItem::Item *item, u8 *arg) {
    if (cM::rndF(100.0f) <= 33.0f) {
        return 1;
    }
    *item = dItem::seeker_c::get()->getRandomFossil(0);
    if (item->mId == dItem::ITEM_ID_NONE) {
        item->setFromIndex(dItem::ITEM_IDX_AMBER);
    }
    return 0;
}

// 80124C44
u32 dAnimal_c::pickClothRequestNotDisliked(dItem::Item *item, u8 *arg) {
    return 1;
}

// 80124C4C
u32 dAnimal_c::pickClothRequestLikedStyle(dItem::Item *item, u8 *arg) {
    return 2;
}

// 80124C54
u32 dAnimal_c::pickClothRequestItemNotDisliked(dItem::Item *item, u8 *arg) {
    int style = mTemplate.mDislikedStyle;
    dItem::Item exclude = mCloth;
    dItem::clothCandCB_c cb(exclude, 10, style, TRUE);
    dItem::seeker_c::get()->search(4, 6, &cb);
    *item = dItem::seeker_c::get()->getRandom();
    if (item->mId == dItem::ITEM_ID_NONE) {
        item->setFromIndex(dItem::ITEM_IDX_WORK_UNIFORM, 2, FALSE);
    }
    return 0;
}

// 80124CEC
u32 dAnimal_c::pickClothRequestItemLikedStyle(dItem::Item *item, u8 *arg) {
    int style = mTemplate.mLikedStyle;
    dItem::Item exclude = mCloth;
    dItem::clothCandCB_c cb(exclude, style, 10, TRUE);
    dItem::seeker_c::get()->search(4, 6, &cb);
    *item = dItem::seeker_c::get()->getRandom();
    if (item->mId == dItem::ITEM_ID_NONE) {
        item->setFromIndex(dItem::ITEM_IDX_WORK_UNIFORM, 2, FALSE);
    }
    return 0;
}

// 80124D84
u32 dAnimal_c::pickClothRequest(dItem::Item *item, u8 *arg) {
    static const RequestPickFunc sFuncs[4] = {
        &dAnimal_c::pickClothRequestNotDisliked,
        &dAnimal_c::pickClothRequestLikedStyle,
        &dAnimal_c::pickClothRequestItemNotDisliked,
        &dAnimal_c::pickClothRequestItemLikedStyle,
    };

    u32 r = cM::rndInt(4);
    u32 ret = 7;
    if (r < 4) {
        ret = (this->*sFuncs[r])(item, arg);
    }
    if (ret >= 7) {
        ret = 1;
    }
    return ret;
}

// 80124E10
u32 dAnimal_c::pickFtrRequestCategory(dItem::Item *item, u8 *arg) {
    *arg = cM::rndInt(10) + 1;
    return 3;
}

// 80124E4C
u32 dAnimal_c::pickFtrRequestColor(dItem::Item *item, u8 *arg) {
    u32 value = 0;
    if (cM::rndF(2.0f) < 1.0f) {
        value = (s8)mTemplate.mFavFtrColor;
    }
    if (value == 0 || value >= 15) {
        value = (u32)cM::rndF(14.0f) + 1;
    }
    *arg = value;
    return 4;
}

// 80124ED0
u32 dAnimal_c::pickFtrRequestTaste(dItem::Item *item, u8 *arg) {
    u32 value = 4;
    if (cM::rndF(2.0f) < 1.0f) {
        u32 n = 0;
        // Raw byte reads: the target loads 0x196 once for these two tests.
        if ((((u8*)&mTemplate)[0x196] >> 1) & 1) {
            value = 0;
            n = 1;
        }
        if (((u8*)&mTemplate)[0x196] & 1) {
            f32 chance = 100.0f / (n + 1);
            if (cM::rndF(100.0f) <= chance) {
                value = 1;
            }
            n++;
        }
        if (mTemplate.mLikesNewFtr) {
            f32 chance = 100.0f / (n + 1);
            if (cM::rndF(100.0f) <= chance) {
                value = 2;
            }
            n++;
        }
        if (mTemplate.mLikesOldFtr) {
            f32 chance = 100.0f / (n + 1);
            if (cM::rndF(100.0f) <= chance) {
                value = 3;
            }
        }
    }
    if (value >= 4) {
        value = (u32)cM::rndF(4.0f);
    }
    *arg = value;
    return 5;
}

// 80125044
u32 dAnimal_c::pickFtrRequestSeries(dItem::Item *item, u8 *arg) {
    static const int sValues[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

    u32 value = 0x84;
    if (cM::rndF(100.0f) < 50.0f) {
        value = (s8)mTemplate.mFavFtrSeries;
    }
    if (value >= 0x84) {
        value = sValues[cM::rndInt(12)];
    }
    *arg = value;
    return 6;
}

// 801250C8
u32 dAnimal_c::pickFtrRequest(dItem::Item *item, u8 *arg) {
    static const RequestPickFunc sFuncs[4] = {
        &dAnimal_c::pickFtrRequestCategory,
        &dAnimal_c::pickFtrRequestColor,
        &dAnimal_c::pickFtrRequestTaste,
        &dAnimal_c::pickFtrRequestSeries,
    };

    u32 r = (u32)cM::rndF(4.0f);
    u32 ret = 7;
    if (r < 4) {
        ret = (this->*sFuncs[r])(item, arg);
    }
    return ret;
}

// 8012514C
// Picks the item for a villager request of the given quest kind.
u32 dAnimal_c::pickRequestItem(dItem::Item *item, u8 *arg, int kind) {
    static dAnimal_c::RequestPickFunc sFuncs[7] = {
        &dAnimal_c::pickInsectRequest,
        &dAnimal_c::pickFishRequest,
        &dAnimal_c::pickFossilRequest,
        &dAnimal_c::pickClothRequest,
        &dAnimal_c::pickFtrRequest,
        NULL,
        NULL,
    };

    u32 ret = 7;
    if (dQuestBase_c::getKindType(kind) == QUEST_TYPE_REQUEST) {
        int idx = -1;
        if (dQuestBase_c::getKindIndex(&idx, kind) && (u32)idx < 7 && sFuncs[idx]) {
            dItem::Item tmpItem;
            u8 tmpArg = 0;
            if (item == NULL) {
                item = &tmpItem;
            }
            if (arg == NULL) {
                arg = &tmpArg;
            }
            ret = (this->*sFuncs[idx])(item, arg);
        }
    }
    return ret;
}

// 8012528C
u32 dAnimal_c::pickInsectFishReward(dItem::Item *item, int *price, dPrivateData_c *player, int mode, const dItem::Item *exclude) {
    if (item == NULL || price == NULL) {
        return 3;
    }
    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayer();
    }
    if (player == NULL) {
        return 3;
    }
    if (!mID.isValid()) {
        return 3;
    }

    int ret = 3;
    int base;
    switch (mode) {
    case 2: {
        dAnimalCatalogFilter_c filter(&player->mCatalog, 0x20);
        if (fn_800F45A4(item, 1, &filter, NULL, 0)) {
            ret = 0;
        } else if (fn_800F4668(item, 1, lbl_8059FF80, NULL, 0)) {
            ret = 0;
        }
        base = 700;
        break;
    }
    case 1: {
        dItem::Item *p = pickPricedNewItem(exclude, 1, FALSE);
        if (p != NULL) {
            ret = 1;
            *item = *p;
        }
        base = 700;
        break;
    }
    default: {
        dItem::Item *p = pickNewItem(exclude, 1, FALSE);
        if (p != NULL) {
            ret = 1;
            *item = *p;
        }
        base = 500;
        break;
    }
    }

    if (item->mId == dItem::ITEM_ID_NONE || ret == 3) {
        f32 r = cM::rndF(100.0f);
        int owner = dSaveData_c::getTown()->mHomes.findOwner(player);
        if (r < 30.0f && (u32)owner < 4) {
            int rate = *fn_800AC28C();
            int room = player->getMoneyRoom(0);
            int amount = base + rate * 4;
            if (amount <= room) {
                *price = amount;
                item->setFromIndex(dItem::ITEM_IDX_100_BELLS);
                ret = 2;
            }
        }
    }

    if (item->mId == dItem::ITEM_ID_NONE || ret == 3) {
        if (fn_800F45A4(item, 1, lbl_8059FF80, NULL, 0)) {
            ret = 0;
        }
    }
    return ret;
}

// 801254DC
BOOL isFossilRequestMatch(const dItem::Item *item, int mode, const dItem::Item *other) {
    if (item->mId == dItem::ITEM_ID_NONE) {
        return FALSE;
    }

    dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    if (bitm == NULL || bitm->getKind() != 0xB) {
        return FALSE;
    }

    BOOL ret = FALSE;
    switch (mode) {
    case 0:
        if (item->isSame(*other)) {
            ret = TRUE;
        }
        break;
    case 1:
        ret = TRUE;
        break;
    }
    return ret;
}

// 801255B8
u32 dAnimal_c::pickFossilReward(dItem::Item *item, int *price, dPrivateData_c *player, int mode, const dItem::Item *exclude) {
    if (item == NULL || price == NULL) {
        return 3;
    }
    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayer();
    }
    if (player == NULL) {
        return 3;
    }
    if (!mID.isValid()) {
        return 3;
    }

    int ret = 3;
    int base;
    switch (mode) {
    case 0: {
        base = 700;
        if (exclude->mId == dItem::ITEM_ID_NONE) {
            break;
        }
        dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*exclude);
        if (bitm == NULL || bitm->getKind() != 0xB) {
            break;
        }
        int fossil = getBITMFossil(bitm);
        if (fossil == 0 || fossil >= 0x1C) {
            break;
        }
        if (dItem::seeker_c::get()->searchFossil(fossil) == 1) {
            dItem::Item *p = pickPricedNewItem(exclude, 1, FALSE);
            if (p != NULL) {
                ret = 1;
                *item = *p;
            }
            base = 700;
        } else if (fn_800F45A4(item, 1, lbl_8059FF80, NULL, 0)) {
            ret = 0;
        } else if (fn_800F4668(item, 1, lbl_8059FF80, NULL, 0)) {
            ret = 0;
        }
        break;
    }
    case 1: {
        dItem::Item *p = pickNewItem(exclude, 1, FALSE);
        if (p != NULL) {
            ret = 1;
            *item = *p;
        }
        base = 500;
        break;
    }
    default:
        return 3;
    }

    if (item->mId == dItem::ITEM_ID_NONE || ret == 3) {
        f32 r = cM::rndF(100.0f);
        int owner = dSaveData_c::getTown()->mHomes.findOwner(player);
        if (r < 30.0f && (u32)owner < 4) {
            int rate = *fn_800AC28C();
            int room = player->getMoneyRoom(0);
            int amount = base + rate * 4;
            if (amount <= room) {
                *price = amount;
                item->setFromIndex(dItem::ITEM_IDX_100_BELLS);
                ret = 2;
            }
        }
    }

    if (item->mId == dItem::ITEM_ID_NONE || ret == 3) {
        if (fn_800F45A4(item, 1, lbl_8059FF80, NULL, 0)) {
            ret = 0;
        }
    }
    return ret;
}

// 80125878
BOOL dAnimal_c::isClothRequestMatch(const dItem::Item *item, int mode, const dItem::Item *other) {
    if (item->mId == dItem::ITEM_ID_NONE) {
        return FALSE;
    }

    dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    if (bitm == NULL || bitm->getKind() != 4) {
        return FALSE;
    }

    const dItem::Item &favorite = mCloth;
    BOOL ret = FALSE;
    switch (mode) {
    case 0:
        if (item->isSame(*other)) {
            ret = TRUE;
        }
        break;
    case 1: {
        int style = mTemplate.mDislikedStyle;
        BOOL other = !item->isSame(favorite);
        if (other && style != getBITMStyle(bitm)) {
            ret = TRUE;
        }
        break;
    }
    case 2: {
        int style = mTemplate.mLikedStyle;
        BOOL other = !item->isSame(favorite);
        if (other && style == getBITMStyle(bitm)) {
            ret = TRUE;
        }
        break;
    }
    }
    return ret;
}

// 801259DC
u32 dAnimal_c::pickClothReward(dItem::Item *item, int *price, dPrivateData_c *player, int mode, const dItem::Item *exclude) {
    if (item == NULL || price == NULL) {
        return 3;
    }
    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayer();
    }
    if (player == NULL) {
        return 3;
    }
    if (!mID.isValid()) {
        return 3;
    }

    int ret = 3;
    int base;
    switch (mode) {
    case 0: {
        dAnimalCatalogFilter_c filter(&player->mCatalog, 0x20);
        if (fn_800F45A4(item, 1, &filter, NULL, 0)) {
            ret = 0;
        } else if (fn_800F4668(item, 1, lbl_8059FF80, NULL, 0)) {
            ret = 0;
        }
        base = 700;
        break;
    }
    case 2: {
        dItem::Item *p = pickPricedNewItem(exclude, 1, FALSE);
        if (p != NULL) {
            ret = 1;
            *item = *p;
        }
        base = 700;
        break;
    }
    default: {
        dItem::Item *p = pickNewItem(exclude, 1, FALSE);
        if (p != NULL) {
            ret = 1;
            *item = *p;
        }
        base = 500;
        break;
    }
    }

    if (item->mId == dItem::ITEM_ID_NONE || ret == 3) {
        f32 r = cM::rndF(100.0f);
        int owner = dSaveData_c::getTown()->mHomes.findOwner(player);
        if (r < 30.0f && (u32)owner < 4) {
            int rate = *fn_800AC28C();
            int room = player->getMoneyRoom(0);
            int amount = base + rate * 4;
            if (amount <= room) {
                *price = amount;
                item->setFromIndex(dItem::ITEM_IDX_100_BELLS);
                ret = 2;
            }
        }
    }

    if (item->mId == dItem::ITEM_ID_NONE || ret == 3) {
        if (fn_800F45A4(item, 1, lbl_8059FF80, NULL, 0)) {
            ret = 0;
        }
    }
    return ret;
}

// 80125C38
// Checks a furniture item against a request: 0 match (new), 1/2 no match, 3 match (already owned), 4 invalid.
u32 dAnimal_c::checkFtrRequest(const dItem::Item *item, int mode, int value) {
    if (item->mId == dItem::ITEM_ID_NONE) {
        return 4;
    }

    dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    if (bitm == NULL || bitm->getKind() != 3) {
        return 4;
    }

    u32 ret = 4;
    switch (mode) {
    case 3:
        int category = getCategoryQ5(*item);
        if (category == value) {
            if (findFtr(item) == -1) {
                ret = 0;
            } else {
                ret = 3;
            }
        } else {
            ret = 1;
        }
        break;
    case 4: {
        int colorA;
        if (bitm->m_ftrColorA < 15) {
            colorA = bitm->resolveColor(bitm->m_ftrColorA, TRUE);
        } else {
            colorA = bitm->resolveColor(0, TRUE);
        }
        int colorB;
        if ((u32)(s8)bitm->m_ftrColorB < 15) {
            colorB = bitm->resolveColor((s8)bitm->m_ftrColorB, TRUE);
        } else {
            colorB = bitm->resolveColor(0, TRUE);
        }
        if (colorA == value || colorB == value) {
            if (findFtr(item) == -1) {
                ret = 0;
            } else {
                ret = 3;
            }
        } else {
            ret = 2;
        }
        break;
    }
    case 5:
        switch (value) {
        case 0:
            if (bitm->getAdultKiddy() == 1) {
                ret = 0;
            }
            break;
        case 1:
            if (bitm->getAdultKiddy() == 2) {
                ret = 0;
            }
            break;
        case 2:
            if (bitm->getNewOld() == 1) {
                ret = 0;
            }
            break;
        case 3:
            if (bitm->getNewOld() == 2) {
                ret = 0;
            }
            break;
        }
        if (ret == 0) {
            if (findFtr(item) != -1) {
                ret = 3;
            }
        } else {
            ret = 2;
        }
        break;
    case 6:
        int series = getBITMSeries(bitm);
        if (series == value) {
            if (findFtr(item) == -1) {
                ret = 0;
            } else {
                ret = 3;
            }
        } else {
            ret = 2;
        }
        break;
    }
    return ret;
}

// 80125ED8
u32 dAnimal_c::pickFtrReward(dItem::Item *item, int *price, dPrivateData_c *player, int mode, const dItem::Item *exclude) {
    if (item == NULL || price == NULL) {
        return 3;
    }
    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayer();
    }
    if (player == NULL) {
        return 3;
    }
    if (!mID.isValid()) {
        return 3;
    }

    int ret = 3;
    int base;
    switch (mode) {
    case 4:
    case 5:
    case 6: {
        dItem::Item *p = pickPricedNewItem(exclude, 1, FALSE);
        if (p != NULL) {
            ret = 1;
            *item = *p;
        }
        base = 700;
        break;
    }
    default: {
        dItem::Item *p = pickNewItem(exclude, 1, FALSE);
        if (p != NULL) {
            ret = 1;
            *item = *p;
        }
        base = 500;
        break;
    }
    }

    if (item->mId == dItem::ITEM_ID_NONE || ret == 3) {
        f32 r = cM::rndF(100.0f);
        int owner = dSaveData_c::getTown()->mHomes.findOwner(player);
        if (r < 30.0f && (u32)owner < 4) {
            int rate = *fn_800AC28C();
            int room = player->getMoneyRoom(0);
            int amount = base + rate * 4;
            if (amount <= room) {
                *price = amount;
                item->setFromIndex(dItem::ITEM_IDX_100_BELLS);
                ret = 2;
            }
        }
    }

    if (item->mId == dItem::ITEM_ID_NONE || ret == 3) {
        if (fn_800F45A4(item, 1, lbl_8059FF80, NULL, 0)) {
            ret = 0;
        }
    }
    return ret;
}

// 801260B0
u32 dAnimal_c::pickLostItemReward(dItem::Item *item, int *price, dPrivateData_c *player) {
    if (item == NULL || price == NULL) {
        return 3;
    }
    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayer();
    }
    if (player == NULL) {
        return 3;
    }
    if (!mID.isValid()) {
        return 3;
    }

    int ret = 3;
    f32 r = cM::rndF(100.0f);
    if (r < 40.0f) {
        dItem::Item *p = pickNewItem(NULL, 0, FALSE);
        if (p != NULL) {
            ret = 1;
            *item = *p;
        } else if (player->getMoneyRoom(0) >= 600) {
            *price = 600;
            item->setFromIndex(dItem::ITEM_IDX_100_BELLS);
            ret = 2;
        }
    } else if (r < 70.0f) {
        dItem::seeker_c::get()->search(0xC, 6, NULL);
        dItem::Item random = dItem::seeker_c::get()->getRandom();
        dItem::Item tmp = random;
        if (random.mId != dItem::ITEM_ID_NONE) {
            *item = tmp;
            ret = 0;
        }
    }

    if (item->mId == dItem::ITEM_ID_NONE || ret == 3) {
        if (fn_800F45A4(item, 1, lbl_8059FF80, NULL, 0)) {
            ret = 0;
        }
    }
    return ret;
}

// 80126238
int dAnimal_c::pickSickReward(dItem::Item *item, dPrivateData_c *player) {
    if (item == NULL) {
        return 3;
    }
    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayer();
    }
    if (player == NULL) {
        return 3;
    }
    if (!mID.isValid()) {
        return 3;
    }

    dAnimalCatalogFilter_c filter(&player->mCatalog, 0x20);
    if (fn_800F45A4(item, 1, &filter, NULL, 0)) {
        return 0;
    }

    static dAnimalPair_c sPairs[1] = {
        dAnimalPair_c(3, 0),
    };
    fn_800C60B4(item, 1, sPairs, 1, lbl_8059FF80, 0, 0, 0);
    return 0;
}

static const char *sSickThanksLabels[LOOKS_TYPE_NUM] = {
    "MAIL_BO_Q11", "MAIL_HA_Q11", "MAIL_KO_Q11", "MAIL_FU_Q11", "MAIL_GE_Q11", "MAIL_TA_Q11",
};

// 80126354
BOOL dAnimal_c::sendSickThanksLetter(const dPersonalID_c *to, const dItem::Item *present, BOOL tryDirect) {
    if (!mID.isValid()) {
        return FALSE;
    }

    dAnmPersonalID_c *id = &mID;
    if (!id->isValid()) {
        return FALSE;
    }

    u8 looks = id->getLooks(1);
    if (looks >= LOOKS_TYPE_NUM) {
        return FALSE;
    }

    const char *label = sSickThanksLabels[looks];
    u16 n;
    n = fn_800CBA14(label);
    u16 a = n > 1 ? (u16)(cM::rndInt(n - 1) + 1) : 1;
    n = fn_800CBA40(label);
    u16 b = n > 1 ? (u16)(cM::rndInt(n - 1) + 1) : 1;
    n = fn_800CBA6C(label);
    u16 c = n > 1 ? (u16)(cM::rndInt(n - 1) + 1) : 1;
    dItem::Item paper(fn_800FABF4(looks, dTime_c::getCurrent()->getSeason()));

    fn_800F46CC(&mID, 0);

    sMail.clear();
    sMail.setupFromAnimal(&a, &b, &c, (int)label, (int)label, (int)label, id, to, &paper);
    if (present->mId != dItem::ITEM_ID_NONE) {
        sMail.setPresent(present->mId, 0xFF);
    }

    if (tryDirect && fn_801029C0(&sMail)) {
        return TRUE;
    }
    if (fn_80102BBC(&sMail)) {
        return TRUE;
    }
    return FALSE;
}

// Temporary declarations (chunk 4)
extern "C" {
// Other chunks of this TU.

// Other TUs.
u16 fn_800CBA14(const char *label); // letter part counts
u16 fn_800CBA40(const char *label);
u16 fn_800CBA6C(const char *label);
void fn_800CBEB4(int slot, int value);
BOOL fn_801029C0(dMail_c *mail);
BOOL fn_80102BBC(dMail_c *mail);
int fn_800BA890(const dItem::Item *item);
u32 fn_800DCF30();
u32 fn_80169298();
int fn_8045243C(int value); // abs
}

struct dUnk8074EBE8_c {
    u8 _0000[0x5884];
    int _5884;
};
extern dUnk8074EBE8_c *lbl_8074EBE8;

// 80126544
BOOL dAnimal_c::sendSickReward(dItem::Item *item, dPrivateData_c *player, int arg) {
    dItem::Item tmp;
    if (item == NULL) {
        item = &tmp;
    }

    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayer();
    }

    if (player == NULL || !player->mPID.isValid()) {
        return FALSE;
    }

    if (!mID.isValid()) {
        return FALSE;
    }

    u8 state = mQuest.mQuest.mBase.mState;
    dQuestVillager_c *quest = &mQuest.mQuest;
    dPlayerID_c *requester = &quest->mRequester;
    if (!quest->mBase.isActive() || (int)quest->mBase.mKind != 5 || (state != 1 && state != 2) ||
        !requester->isValid() || !requester->isSame(&player->mPID.player)) {
        return FALSE;
    }

    if (pickSickReward(item, player) >= 3 || item->mId == dItem::ITEM_ID_NONE) {
        return FALSE;
    }

    if (sendSickThanksLetter(&player->mPID, item, arg)) {
        return TRUE;
    }
    return FALSE;
}

// 80126698
BOOL dAnimal_c::isRequestMatch(const dItem::Item *item, int kind, u8 a, u8 b, const dItem::Item *questItem) {
    if (item->mId == dItem::ITEM_ID_NONE) {
        return FALSE;
    }

    BOOL ret = FALSE;
    switch (kind) {
    case 0:
    case 1:
        if (item->isSame(*questItem)) {
            ret = TRUE;
        }
        break;
    case 2:
        ret = isFossilRequestMatch(item, a, questItem);
        break;
    case 3:
        ret = isClothRequestMatch(item, a, questItem);
        break;
    case 4:
        if (!checkFtrRequest(item, a, b)) {
            ret = TRUE;
        }
        break;
    }
    return ret;
}

// 80126764
BOOL dAnimal_c::isRequestedItem(const dItem::Item *item) {
    if (item->mId == dItem::ITEM_ID_NONE) {
        return FALSE;
    }

    if (!mID.isValid()) {
        return FALSE;
    }

    dQuestVillager_c *quest = &mQuest.mQuest;
    if (!quest->mBase.isActive()) {
        return FALSE;
    }

    switch (quest->mBase.mKind) {
    case 0:
        if (quest->mBase.mState == 1) {
            return FALSE;
        }
        break;
    case 1:
        if (quest->mBase.mState == 1) {
            return FALSE;
        }
        break;
    case 2:
        if (quest->mBase.mState >= 1) {
            return FALSE;
        }
        break;
    case 3:
        if (quest->mBase.mState >= 1) {
            return FALSE;
        }
        break;
    case 4:
        if (quest->mBase.mState >= 1) {
            return FALSE;
        }
        break;
    default:
        return FALSE;
    }

    return isRequestMatch(item, quest->mBase.mKind, quest->mMatchMode, quest->mMatchParam, &quest->mBase.mItem);
}

// 8012689C
BOOL dAnimal_c::updateQuests(dLostQuest_c *lost, dPrivateData_c *player, int arg2) {
    BOOL ret0 = expireErrandRequest(player, arg2);
    bool ret = ret0 | completeErrandRequest(-1, player, 1);
    ret |= completeErrandRequestFinal(-1, player, 1);
    ret |= clearExpiredRequest(lost, arg2);
    return ret;
}

static const char *sTunekichiLabels[LOOKS_TYPE_NUM] = {
    "MAIL_BO_Tunekichi", "MAIL_HA_Tunekichi", "MAIL_KO_Tunekichi",
    "MAIL_FU_Tunekichi", "MAIL_GE_Tunekichi", "MAIL_TA_Tunekichi",
};

// 80126950
BOOL dAnimal_c::sendTunekichiLetter(const dPersonalID_c *to, BOOL invite, BOOL flag) {
    if (!to->isValid()) {
        return FALSE;
    }

    dAnmPersonalID_c *id = &mID;
    if (!id->isValid()) {
        return FALSE;
    }

    u8 looks = id->getLooks(1);
    if (looks >= LOOKS_TYPE_NUM) {
        return FALSE;
    }

    u16 kind = invite ? 1 : (u16)(cM::rndInt(3) + 2);
    int season = dTime_c::getCurrent()->getSeason();
    dItem::Item paper;
    if (invite) {
        paper.setFromIndex(dItem::ITEM_IDX_INVITE_CARD);
    } else {
        paper.mId = fn_800FABF4(looks, season);
    }

    fn_800F46CC(&mID, 0);
    sMail.clear();
    sMail.setupFromAnimal(&kind, sTunekichiLabels[looks], id, to, &paper);
    if (invite) {
        sMail.flagInvite();
    }

    if (flag) {
        if (fn_80102BBC(&sMail)) {
            return TRUE;
        }
    } else if (fn_801029C0(&sMail)) {
        return TRUE;
    }
    return FALSE;
}

static inline u16 pickLetterPart(u16 num) {
    return num > 1 ? (u16)(cM::rndInt(num - 1) + 1) : (u16)1;
}

static const char *sBirthdayLabels[LOOKS_TYPE_NUM] = {
    "MAIL_BO_Birthday", "MAIL_HA_Birthday", "MAIL_KO_Birthday",
    "MAIL_FU_Birthday", "MAIL_GE_Birthday", "MAIL_TA_Birthday",
};

// 80126AD8
BOOL dAnimal_c::sendBirthdayLetter(const dPersonalID_c *to, const dItem::Item *present, int hi) {
    if (!to->isValid()) {
        return FALSE;
    }

    dAnmPersonalID_c *id = &mID;
    if (!id->isValid()) {
        return FALSE;
    }

    u8 looks = id->getLooks(1);
    if (looks >= LOOKS_TYPE_NUM) {
        return FALSE;
    }

    const char *label = sBirthdayLabels[looks];
    u16 a = pickLetterPart(fn_800CBA14(label));
    u16 b = pickLetterPart(fn_800CBA40(label));
    u16 c = pickLetterPart(fn_800CBA6C(label));
    dItem::Item paper(dItem::ITEM_IDX_BIRTHDAY_CARD);

    fn_800F46CC(&mID, 0);
    sMail.clear();
    sMail.setupFromAnimal(&a, &b, &c, (int)label, (int)label, (int)label, id, to, &paper);
    if (present->mId != dItem::ITEM_ID_NONE) {
        sMail.setPresent(present->mId, hi);
    }

    if (fn_801029C0(&sMail)) {
        return TRUE;
    }
    if (fn_80102BBC(&sMail)) {
        return TRUE;
    }
    return FALSE;
}

static const char *sValentineLabels[LOOKS_TYPE_NUM] = {
    "MAIL_BO_Valentine", "MAIL_HA_Valentine", "MAIL_KO_Valentine",
    "MAIL_FU_Valentine", "MAIL_GE_Valentine", "MAIL_TA_Valentine",
};

// 80126CAC
BOOL dAnimal_c::sendValentineLetter(const dPersonalID_c *to, const dItem::Item *present, int hi) {
    if (!to->isValid()) {
        return FALSE;
    }

    dAnmPersonalID_c *id = &mID;
    if (!id->isValid()) {
        return FALSE;
    }

    u8 looks = id->getLooks(1);
    if (looks >= LOOKS_TYPE_NUM) {
        return FALSE;
    }

    const char *label = sValentineLabels[looks];
    u16 a = fn_800CBA14(label) > 1 ? (u16)(cM::rndInt(3) + 1) : (u16)1;
    u16 b = fn_800CBA40(label) > 1 ? (u16)(cM::rndInt(3) + 1) : (u16)1;
    u16 c = fn_800CBA6C(label) > 1 ? (u16)(cM::rndInt(3) + 1) : (u16)1;
    if (present->mId == dItem::ITEM_ID_NONE) {
        a += 3;
        b += 3;
        c += 3;
    }
    dItem::Item paper(dItem::ITEM_IDX_ELEGANT_PAPER);

    fn_800F46CC(&mID, 0);
    sMail.clear();
    sMail.setupFromAnimal(&a, &b, &c, (int)label, (int)label, (int)label, id, to, &paper);
    if (present->mId != dItem::ITEM_ID_NONE) {
        sMail.setPresent(present->mId, hi);
    }

    if (fn_801029C0(&sMail)) {
        return TRUE;
    }
    if (fn_80102BBC(&sMail)) {
        return TRUE;
    }
    return FALSE;
}

static const char *sNewyearLabels[LOOKS_TYPE_NUM] = {
    "MAIL_BO_Newyear", "MAIL_HA_Newyear", "MAIL_KO_Newyear",
    "MAIL_FU_Newyear", "MAIL_GE_Newyear", "MAIL_TA_Newyear",
};

// 80126EAC
BOOL dAnimal_c::sendNewYearLetter(const dPersonalID_c *to, int year) {
    if (!to->isValid()) {
        return FALSE;
    }

    dAnmPersonalID_c *id = &mID;
    if (!id->isValid()) {
        return FALSE;
    }

    u8 looks = id->getLooks(1);
    if (looks >= LOOKS_TYPE_NUM) {
        return FALSE;
    }

    const char *label = sNewyearLabels[looks];
    u16 a = pickLetterPart(fn_800CBA14(label));
    u16 b = pickLetterPart(fn_800CBA40(label));
    u16 c = pickLetterPart(fn_800CBA6C(label));
    dItem::Item paper(dItem::ITEM_IDX_NEW_YEARS_CARDS);

    fn_800F46CC(&mID, 0);
    fn_800CBEB4(0, year);
    sMail.clear();
    sMail.setupFromAnimal(&a, &b, &c, (int)label, (int)label, (int)label, id, to, &paper);

    if (fn_801029C0(&sMail)) {
        return TRUE;
    }
    if (fn_80102BBC(&sMail)) {
        return TRUE;
    }
    return FALSE;
}

static const char *sMoveLabels[LOOKS_TYPE_NUM] = {
    "MAIL_BO_Move", "MAIL_HA_Move", "MAIL_KO_Move", "MAIL_FU_Move", "MAIL_GE_Move", "MAIL_TA_Move",
};

// 8012706C
BOOL dAnimal_c::sendMoveLetter(const dPersonalID_c *to) {
    if (!to->isValid()) {
        return FALSE;
    }

    if (!to->isFromTown()) {
        return FALSE;
    }

    dAnmPersonalID_c *id = &mID;
    if (!id->isValid()) {
        return FALSE;
    }

    dAnimalMemory_c *memory = findMemory2(to);
    if (memory == NULL) {
        return FALSE;
    }

    u8 looks = id->getLooks(1);
    if (looks >= LOOKS_TYPE_NUM) {
        return FALSE;
    }

    const char *label = sMoveLabels[looks];
    u16 a = cM::rndInt(2) + 1;
    u16 b = cM::rndInt(2) + 1;
    u16 c = cM::rndInt(2) + 1;
    if (memory->getFriendship() <= -1) {
        a = cM::rndInt(2) + 5;
        b = cM::rndInt(2) + 5;
        c = cM::rndInt(2) + 5;
    } else if (memory->getFriendship() <= 0x3F) {
        a = cM::rndInt(2) + 3;
        b = cM::rndInt(2) + 3;
        c = cM::rndInt(2) + 3;
    }
    dItem::Item paper(fn_800FABF4(looks, dTime_c::getCurrent()->getSeason()));

    fn_800F46CC(id, 0);
    sMail.clear();
    sMail.setupFromAnimal(&a, &b, &c, (int)label, (int)label, (int)label, id, to, &paper);

    if (fn_801029C0(&sMail)) {
        return TRUE;
    }
    if (fn_80102BBC(&sMail)) {
        return TRUE;
    }
    return FALSE;
}

// 80127290
BOOL dAnimal_c::sendMoveLetters() {
    dPrivateData_c *players = dSaveData_c::getRaw()->mPlayers;
    dHomeList_c *homes = &dSaveData_c::getRaw()->mHomes;
    for (int i = 0; i < PLAYER_NUM; i++) {
        dPrivateData_c *player = dPrivateData_c::getRaw(players, i);
        if (player != NULL && player->mPID.isValid() && homes->findOwner(player) != -1) {
            sendMoveLetter(&player->mPID);
        }
    }
    return FALSE;
}

// 80127330
wchar_t *dAnimal_c::getHabit(int language) {
    if (language == LANGUAGE_NUM) {
        language = getLanguage();
    }

    switch (language) {
    case LANGUAGE_US:
        return mTemplate.mHabits[LANGUAGE_US];
    case LANGUAGE_MX:
        return mTemplate.mHabits[LANGUAGE_MX];
    case LANGUAGE_QC:
        return mTemplate.mHabits[LANGUAGE_QC];
    case LANGUAGE_EN:
        return mTemplate.mHabits[LANGUAGE_EN];
    case LANGUAGE_ES:
        return mTemplate.mHabits[LANGUAGE_ES];
    case LANGUAGE_FR:
        return mTemplate.mHabits[LANGUAGE_FR];
    case LANGUAGE_IT:
        return mTemplate.mHabits[LANGUAGE_IT];
    case LANGUAGE_DE:
        return mTemplate.mHabits[LANGUAGE_DE];
    case LANGUAGE_KR:
        return mTemplate.mHabits[LANGUAGE_KR];
    case LANGUAGE_JP:
    default:
        return mTemplate.mHabits[LANGUAGE_JP];
    }
}

static inline int habitLen(const wchar_t *s) {
    int i;
    for (i = 0; i <= ANIMAL_HABIT_LEN; s++, i++) {
        if (*s == 0) {
            return i;
        }
    }
    return ANIMAL_HABIT_LEN;
}

// Clears dst and copies at most ANIMAL_HABIT_LEN characters of src.
static inline void copyHabit(wchar_t *dst, const wchar_t *src) {
    for (int i = 0; i < ANIMAL_HABIT_LEN + 1; i++) {
        dst[i] = 0;
    }

    int len = habitLen(src);

    for (int i = 0; i < len; i++) {
        dst[i] = src[i];
    }
}

// 801273D4
void dAnimal_c::setHabit(const wchar_t *habit, int language) {
    dAnimalTemplate_c *tmpl = &mTemplate;
    if (language == LANGUAGE_NUM) {
        language = getLanguage();
    }

    switch (language) {
    case LANGUAGE_US:
        copyHabit(tmpl->mHabits[LANGUAGE_US], habit);
        break;
    case LANGUAGE_MX:
        copyHabit(tmpl->mHabits[LANGUAGE_MX], habit);
        break;
    case LANGUAGE_QC:
        copyHabit(tmpl->mHabits[LANGUAGE_QC], habit);
        break;
    case LANGUAGE_EN:
        copyHabit(tmpl->mHabits[LANGUAGE_EN], habit);
        break;
    case LANGUAGE_ES:
        copyHabit(tmpl->mHabits[LANGUAGE_ES], habit);
        break;
    case LANGUAGE_FR:
        copyHabit(tmpl->mHabits[LANGUAGE_FR], habit);
        break;
    case LANGUAGE_IT:
        copyHabit(tmpl->mHabits[LANGUAGE_IT], habit);
        break;
    case LANGUAGE_DE:
        copyHabit(tmpl->mHabits[LANGUAGE_DE], habit);
        break;
    case LANGUAGE_KR:
        copyHabit(tmpl->mHabits[LANGUAGE_KR], habit);
        break;
    case LANGUAGE_JP:
    default:
        copyHabit(tmpl->mHabits[LANGUAGE_JP], habit);
        break;
    }
}

// 8012808C
u32 dAnimal_c::getSpecies() {
    return mTemplate.mSpecies;
}

// 80128094
u32 dAnimal_c::getBirthMonth() {
    return (u8)(mTemplate.mBirthMonth - 1);
}

// 801280A4
u32 dAnimal_c::getBirthDay() {
    return mTemplate.mBirthDay;
}

// 801280AC
dItem::Item dAnimal_c::getUmbrella() {
    return dItem::Item((u16)mTemplate.getUmbrella());
}

// 801280B8
void dAnimal_c::setUmbrella(const dItem::Item *item) {
    if (item->mId != dItem::ITEM_ID_NONE) {
        dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
        if (bitm != NULL) {
            switch (bitm->getKind()) {
            case dItem::KIND_UMBRELLA:
                mTemplate.mUmbrella = (u16)item->mId;
                break;
            }
        }
    }
}

// 8012813C
u8 dAnimal_c::fn_8012813C() {
    u8 ret = 0;
    if (mID.isValid()) {
        ret = mID.mNpcIdx % 5;
    }
    return ret;
}

// 801281AC
u8 dAnimal_c::fn_801281AC() {
    u8 ret = 0;
    if (mID.isValid()) {
        u16 idx = mID.mNpcIdx;
        ret = idx % fn_80169298();
    }
    return ret;
}

// 8012820C
BOOL dAnimal_c::isSleepTime(const dTime_c *now) {
    if (!mID.isValid()) {
        return FALSE;
    }
    BOOL ret = fn_800F4020(mID.getLooks(1), now);
    return ret;
}

// 80128270
BOOL dAnimal_c::canMoveOut(dAnimalBlock_c *block) {
    dAnmPersonalID_c *id = &mID;
    if (!id->isValid()) {
        return FALSE;
    }

    if (block->isStyleAnimal(id)) {
        return FALSE;
    }

    if (getDayPlace(0, block->isAppointmentAnimal(id), 0) != 3) {
        return FALSE;
    }

    if (dPrivateData_c::findErrandAll(dSaveData_c::getTown()->mPlayers, id, 2, 0)) {
        return FALSE;
    }

    dQuestVillager_c *quest = &mQuest.mQuest;
    if (quest->mBase.isActive()) {
        switch (quest->mBase.mKind) {
        case 5:
        case 6:
            return FALSE;
        }
    }
    return TRUE;
}

// 80128374
BOOL dAnimal_c::isLostItemRequestDone(dPrivateData_c *player, BOOL checkFlag) {
    if (!mID.isValid()) {
        return FALSE;
    }

    if (checkFlag) {
        if (player == NULL) {
            player = dPlayerMgr_c::getCurrentPlayerRaw();
        }
        if (player != NULL && player->isFlag0(0xD)) {
            return TRUE;
        }
    }

    dQuestVillager_c *quest = &mQuest.mQuest;
    if (quest->mBase.isActive() && (int)quest->mBase.mKind == 6 && quest->mBase.mState != 2) {
        return FALSE;
    }
    return TRUE;
}

// 80128440
BOOL dAnimal_c::fn_80128440() {
    if (!mID.isValid()) {
        return FALSE;
    }

    if ((int)mQuest.mQuest.mBase.mKind != 5) {
        return FALSE;
    }

    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayerRaw();
    if (player == NULL) {
        return FALSE;
    }

    if (fn_800DCF30() <= 1 && player->mPID.isFromTown()) {
        return FALSE;
    }
    return TRUE;
}

// 801284D4
s8 dAnimal_c::getMaxFriendship() {
    s8 best = -0x80;
    dAnimalMemory_c *memory = getMemory2(0);
    for (int i = 0; i < ANIMAL_MEMORY_NUM; i++, memory++) {
        if (memory->mPlayer.isValid()) {
            s8 value = memory->getFriendship();
            if (value > best) {
                best = value;
            }
        }
    }
    return best;
}

static inline bool isQuestTimeSet(dTimeStamp_c *time) {
    return !time->isNone();
}

// 8012855C
int dAnimal_c::getDaysSinceLastTalk(const dTime_c *now) {
    int best = -1;
    dAnimalMemory_c *memory = getMemory2(0);
    for (int i = 0; i < ANIMAL_MEMORY_NUM; i++, memory++) {
        if (memory->mPlayer.isValid() && isQuestTimeSet(&memory->mLastTalkTime)) {
            dTime_c time;
            time = memory->mLastTalkTime.get();
            int days = fn_8045243C(dTime_c::diffDays(now, &time, FALSE));
            if (best < 0 || days < best) {
                best = days;
            }
        }
    }
    return best;
}

// 8012865C
void dAnimal_c::validateItems() {
    clearInvalidPresents();
    validateHouse();
    validateNewItems();
}

// 80128698
void dAnimal_c::validateUmbrella() {
    dItem::Item umbrella = getUmbrella();
    dItem::Item tmp = umbrella;
    dItem::BITM *bitm = umbrella.mId != dItem::ITEM_ID_NONE ? dItem::infoBank_c::get()->getBITM(tmp) : NULL;
    dItem::Item item;
    if (bitm == NULL) {
        item = pickRandomUmbrella();
    }
    if (item.mId != dItem::ITEM_ID_NONE) {
        setUmbrella(&umbrella);
    }
}

// 80128728
BOOL dAnimal_c::isFtrBoxed(u32 idx) {
    if (idx < 10) {
        return (mBoxedFtrMask >> idx) & 1;
    }
    return FALSE;
}

// 80128748
void dAnimal_c::setFtrBoxed(u32 idx) {
    if (idx < 10) {
        mBoxedFtrMask |= 1 << idx;
    }
}

// 80128768
void dAnimal_c::clearFtrBoxed(u32 idx) {
    if (idx < 10) {
        mBoxedFtrMask &= ~(1 << idx);
    }
}

// 80128788
void dAnimal_c::pickBoxedFtr() {
    mBoxedFtrMask = 0;
    int num = cM::rndInt(2) + 3;
    while (num != 0) {
        u32 pick = -1;
        u32 count = 0;
        for (int i = 0; i < 10; i++) {
            if (!isFtrBoxed(i)) {
                dItem::Item item = getFtr(i);
                if (isHouseItem(&item)) {
                    f32 chance = 100.0f / (count + 1);
                    if (cM::rndF(100.0f) <= chance) {
                        pick = i;
                    }
                    count++;
                }
            }
        }
        if (pick >= 10) {
            break;
        }
        setFtrBoxed(pick);
        num--;
    }
}

// 801288AC
BOOL isNoneOrRoyalItem(const dItem::Item *item) {
    if (item->mId == dItem::ITEM_ID_NONE) {
        return TRUE;
    }

    static dItem::Item sItems[] = {dItem::Item(dItem::ITEM_IDX_THRONE), dItem::Item(dItem::ITEM_IDX_CROWN), dItem::Item(dItem::ITEM_IDX_ROYAL_CROWN)};
    const dItem::Item *p = sItems;
    for (u32 i = 0; i < 3; i++, p++) {
        if (item->isSame(*p)) {
            return TRUE;
        }
    }
    return FALSE;
}

// 801289B0
void dAnimal_c::pickBoxedFtrMoveOut() {
    if (!mID.isValid()) {
        return;
    }

    mBoxedFtrMask = 0;
    u32 num = cM::rndInt(2) + 3;
    int avail = countFtr() - 2;
    if (avail < 0) {
        num = 0;
    } else if (avail < num) {
        num = avail;
    }

    while (num != 0) {
        u32 pick = -1;
        u32 count = 0;
        for (int i = 0; i < 10; i++) {
            if (!isFtrBoxed(i)) {
                dItem::Item item = getFtr(i);
                if (isHouseItem(&item) && !isNoneOrRoyalItem(&item) && item.getPrice() > 0) {
                    f32 chance = 100.0f / (count + 1);
                    if (cM::rndF(100.0f) <= chance) {
                        pick = i;
                    }
                    count++;
                }
            }
        }
        if (pick >= 10) {
            break;
        }
        setFtrBoxed(pick);
        num--;
    }
}

// 80128B28
int dAnimal_c::countBoxedFtr() {
    int count = 0;
    for (int i = 0; i < 10; i++) {
        if (isFtrBoxed(i)) {
            count++;
        }
    }
    return count;
}

// 80128B90
BOOL isHoldableItem(const dItem::Item *item) {
    return fn_800BA890(item) != 0;
}

// 80128BBC
BOOL dAnimal_c::setHeldItem(const dItem::Item *item) {
    if (item->mId == dItem::ITEM_ID_NONE || isHoldableItem(item)) {
        mHeldItem = *item;
        return TRUE;
    }
    return FALSE;
}

// 80128C20
BOOL dAnimal_c::isHeldItemChangeMinute(u32 minute) {
    u8 cur = mHeldItemChangeMinute;
    if (cur >= 60 || minute >= 60) {
        return FALSE;
    }
    return cur == minute;
}

// 80128C4C
BOOL dAnimal_c::isHeldItemChangeDue(u32 minute) {
    u8 cur = mHeldItemChangeMinute;
    if (cur >= 60) {
        return TRUE;
    }
    if (minute >= 60) {
        return FALSE;
    }

    u8 diff = cur > minute ? cur - minute : cur + (60 - minute);
    if (cur != minute && diff > 20) {
        return TRUE;
    }
    return FALSE;
}

// 80128CA8
u8 calcHeldItemChangeMinute(int minute, BOOL soon) {
    int add = soon ? cM::rndInt(5) : cM::rndInt(5) + 15;
    return (minute + add) % 60;
}

// 80128D1C
BOOL dAnimal_c::wantsParasol() {
    dQuestWish_c *wish = &mQuest.mWish;
    int kind = wish->isValid() ? wish->mKind : 8;
    dUnk8074EBE8_c *p = lbl_8074EBE8;
    if (kind == 3 && p != NULL) {
        int mode = p->_5884;
        BOOL ok = (mode == 0 || mode == 1) ? TRUE : FALSE;
        if (ok && (u8)mID.getGender(1) == 1) {
            dTime_c *now = dTime_c::getCurrent();
            if (now->getSeason() == 1 && now->hour >= 7 && now->hour < 17) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

// Temporary declarations (chunk 5)
extern "C" {
// Other chunks of this TU.

// This chunk, used before their definitions.

// Other TUs.
BOOL fn_8014A098(void *fg, int idx);
BOOL fn_8014A4F4(void *fg, int idx);
BOOL fn_8014A544(void *fg, int *x, int *z, int idx);
BOOL fn_8014A5B8(void *fg, int idx);
void fn_8014EE50(void *obj, const dPersonalID_c *pid, const dAnmPersonalID_c *animal, u8 value);
extern u8 lbl_8059FF80[];
}

// Town save data at +0x640C8, used by recordImpression.
struct dTownUnk640E6_c {
    /* 0x000 */ u8 _000[0x16];
    /* 0x016 */ dPersonalID_c mPID;
    /* 0x042 */ u8 _042[0xC2];
    /* 0x104 */ u8 mFlag : 1;
};

struct dTownUnk640C8_c {
    /* 0x00 */ u8 _00[0x1E];
    /* 0x1E */ dTownUnk640E6_c _1E;
};

static inline int getWishKind(dQuestWish_c *wish) {
    return wish->isValid() ? wish->mKind : 8;
}

// 80128DF0
dItem::Item dAnimal_c::getWishTool() {
    dItem::Item item;
    dQuestWish_c *wish = &mQuest.mWish;

    switch (getWishKind(wish)) {
    case 1:
        item.setFromIndex(dItem::ITEM_IDX_FISHING_ROD);
        break;
    case 0:
        item.setFromIndex(dItem::ITEM_IDX_NET);
        break;
    case 2:
        item.setFromIndex(dItem::ITEM_IDX_SHOVEL);
        break;
    case 3: {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayerRaw();
        if (player == NULL || !player->isFlag0(0xD)) {
            if (wantsParasol()) {
                dItem::Item tmp = getUmbrella();
                if (isFancyUmbrella(&tmp)) {
                    item = tmp;
                }
            }
        }
        break;
    }
    case 6: {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayerRaw();
        if (player == NULL || !player->isFlag0(0xD)) {
            item.setFromIndex(dItem::ITEM_IDX_WATERING_CAN);
        }
        break;
    }
    }

    return item;
}

// 80128F34
BOOL dAnimal_c::recordImpression(const dPersonalID_c *pid) {
    dAnmPersonalID_c *id = &mID;
    if (!id->isValid()) {
        return FALSE;
    }

    if (!pid->isValid() || !pid->isFromTown()) {
        return FALSE;
    }

    dAnimalMemory_c *memory = findMemory2(pid);
    if (memory == NULL) {
        return FALSE;
    }

    dTownUnk640C8_c *obj;
    u32 value = memory->mImpression;
    if (value >= '1') {
        return FALSE;
    }

    obj = (dTownUnk640C8_c *)((u8 *)dSaveData_c::getTown() + 0x640C8);
    dTownUnk640E6_c *sub = &obj->_1E;
    dPersonalID_c *host = &sub->mPID;
    BOOL ok = FALSE;
    if (!host->isValid() || (sub->mFlag && *host == *pid)) {
        ok = TRUE;
    }

    if (!ok) {
        u32 num = dPrivateData_c::count(dSaveData_c::getRaw()->mPlayers);
        f32 chance;
        if (num != 0) {
            chance = 100.0f / num;
        } else {
            chance = 0.0f;
        }
        if (cM::rndF(100.0f) < chance) {
            ok = TRUE;
        }
    }

    if (!ok) {
        return FALSE;
    }

    fn_8014EE50(obj, pid, id, value);
    return TRUE;
}

// 80129100
BOOL dAnimal_c::setVisitorLetter(dPrivateData_c *player) {
    if (!mID.isValid()) {
        return FALSE;
    }

    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayer();
    }

    if (player == NULL || player->mPID.isFromTown()) {
        return FALSE;
    }

    dAnimalItem_c *animalItem = &player->mVisitorLetter;
    BOOL ok = FALSE;
    if (!animalItem->isValid()) {
        if (cM::rndF(100.0f) < 20.0f) {
            ok = TRUE;
        }
    } else if (cM::rndInt(dSaveData_c::getRaw()->mAnimals.mTown.getAnimalNum()) == 0) {
        ok = TRUE;
    }

    if (ok) {
        dItem::Item item;
        if (cM::rndF(100.0f) < 25.0f) {
            fn_800F45A4(&item, 1, lbl_8059FF80, NULL, 0);
        }
        animalItem->set(&mID, &item);
        return TRUE;
    }

    return FALSE;
}

static inline BOOL isSameBlock(int x0, int z0, int x1, int z1) {
    if (x0 == x1 && z0 == z1) {
        return TRUE;
    }
    return FALSE;
}

// 80129238
BOOL dAnimal_c::isItemInMyBlock(const dItem::Item *item) {
    if (!item->isExtId()) {
        return FALSE;
    }

    if (!mID.isValid()) {
        return FALSE;
    }

    if (mPlace != 0) {
        return FALSE;
    }

    dAnimalSpot_c *place = &mSpot;
    if (!place->isValid()) {
        return FALSE;
    }

    dSaveData_c *town = dSaveData_c::getTown();
    int x = -1;
    int z = -1;
    bool nf = !fn_8014B0F0(&town->_05EB04, &x, &z, *item, 1);
    if (nf) {
        return FALSE;
    }

    int px = place->mX >> 4;
    int pz = place->mZ >> 4;
    int bx = x >> 4;
    int bz = z >> 4;
    return isSameBlock(bx, bz, px, pz);
}

static const int sFancyUmbrellas[] = {dItem::ITEM_IDX_LACY_PARASOL, dItem::ITEM_IDX_LEAF_UMBRELLA, dItem::ITEM_IDX_RIBBON_UMBRELLA, dItem::ITEM_IDX_PETAL_PARASOL, dItem::ITEM_IDX_ELEGANT_UMBRELLA, 0};

// 80129364
dItem::Item pickFancyUmbrella() {
    return dItem::Item(sFancyUmbrellas[cM::rndInt(5)]);
}

// 801293AC
BOOL isFancyUmbrella(const dItem::Item *item) {
    for (u32 i = 0; i < 5; i++) {
        if (item->isSame(dItem::Item(sFancyUmbrellas[i]))) {
            return TRUE;
        }
    }
    return FALSE;
}

struct dItemRange_c {
    dItemRange_c(s32 kind, s32 n) : mKind(kind), mN(n) {}

    s32 mKind;
    s32 mN;
};

// 8012942C
dItem::Item pickRandomUmbrella() {
    static dItemRange_c range(0x38, 8);
    dItem::Item result;
    dItem::Item item;
    if (fn_800C60B4(&item, 1, &range.mKind, 1, lbl_8059FF80, 0, 0, 0)) {
        result = item;
    }
    return result;
}

// 801294C4
dDesign_c *pickTownDesign(int style, u32 sameStyle) {
    dDesign_c *designs = dSaveData_c::getTown()->_05EC80;
    dDesign_c *result = NULL;
    u32 num = 0;
    for (int i = 4; i < 8; i++) {
        dDesign_c *design = fn_80147F80(designs, i);
        bool same = design->getStyle() == style;
        if (design->getCreator()->isValid()) {
            if (style >= 11 || sameStyle == same) {
                f32 chance = 100.0f / (num + 1);
                if (cM::rndF(100.0f) <= chance) {
                    result = design;
                }
                num++;
            }
        }
    }
    return result;
}

// 801295D0
BOOL dAnimal_c::pickDesignToWear() {
    if (!mID.isValid()) {
        return FALSE;
    }

    int style;
    u32 same = TRUE;
    BOOL go = FALSE;
    style = mTemplate.mLikedStyle;
    if ((int)mQuest.mWish.mKind == 3) {
        if (cM::rndF(100.0f) < 20.0f) {
            go = TRUE;
            if (cM::rndF(100.0f) < 25.0f) {
                same = FALSE;
            }
        }
    } else if (cM::rndF(100.0f) < 10.0f) {
        go = TRUE;
    }

    if (go) {
        dDesign_c *design = pickTownDesign(style, same);
        if (design != NULL) {
            wearDesign(design);
            return TRUE;
        }
    }

    return FALSE;
}

static inline dItem::BITM *getItemBITM(const dItem::Item *item) {
    if (item->mId != dItem::ITEM_ID_NONE) {
        return dItem::infoBank_c::get()->getBITM(*item);
    }
    return NULL;
}

static inline BOOL isDifferentItem(const dItem::Item *a, const dItem::Item *b) {
    return !a->isSame(*b);
}

// 801296C4
BOOL dAnimal_c::pickUmbrella() {
    dAnmPersonalID_c *id = &mID;
    if (!id->isValid()) {
        return FALSE;
    }

    dItem::Item item;
    if ((int)mQuest.mWish.mKind == 3) {
        if ((u8)id->getGender(1) == GENDER_FEMALE) {
            dItem::Item tmp = getUmbrella();
            if (!isFancyUmbrella(&tmp)) {
                item = pickFancyUmbrella();
            }
        } else if (cM::rndF(100.0f) < 10.0f) {
            item = pickRandomUmbrella();
        }
    } else if (cM::rndF(100.0f) < 5.0f) {
        item = pickRandomUmbrella();
    }

    if (item.mId != dItem::ITEM_ID_NONE) {
        setUmbrella(&item);
        dItem::Item *cur = &mHeldItem;
        dItem::BITM *bitm = getItemBITM(cur);
        if (bitm != NULL && bitm->getKind() == dItem::KIND_UMBRELLA && isDifferentItem(cur, &item)) {
            setHeldItem(&item);
        }
        return TRUE;
    }
    return FALSE;
}

// 80129854
dAnimalHomeStay_c::dAnimalHomeStay_c() {}

// 80129858
void dAnimalHomeStay_c::clear() {
    mAnimalIdx = -1;
    mPeriod = 0;
}

// 8012986C
BOOL dAnimalHomeStay_c::isValid() {
    if ((u32)mAnimalIdx < ANIMAL_NUM && mPeriod < 4) {
        return TRUE;
    }
    return FALSE;
}

static const u8 sHomeStayPeriods[LOOKS_TYPE_NUM][2] = {
    {0, 1}, {0, 2}, {2, 3}, {0, 1}, {1, 2}, {1, 3},
};

// 80129898
void dAnimalHomeStay_c::set(u32 idx, u8 looks) {
    clear();
    if (looks < LOOKS_TYPE_NUM && idx < ANIMAL_NUM) {
        mAnimalIdx = idx;
        mPeriod = sHomeStayPeriods[looks][(u32)cM::rndF(100.0f) & 1];
    }
}

struct dHomeStayPeriod_c {
    u8 mValue;
    u8 mHourEnd;
};

static inline BOOL isUnk1E452Value(dAnimalHomeStay_c *obj, u8 value) {
    if (value == obj->mPeriod) {
        return TRUE;
    }
    return FALSE;
}

static inline u8 lookupHour(const dHomeStayPeriod_c *hours, int hour) {
    u8 value = 3;
    if (hour < hours[0].mHourEnd) {
        value = hours[0].mValue;
    } else if (hour < hours[1].mHourEnd) {
        value = hours[1].mValue;
    } else if (hour < hours[2].mHourEnd) {
        value = hours[2].mValue;
    } else if (hour < hours[3].mHourEnd) {
        value = hours[3].mValue;
    }
    return value;
}

// 80129918
BOOL dAnimalHomeStay_c::isPeriod(const dTime_c *time) {
    static dHomeStayPeriod_c sHours[4] = {{3, 6}, {0, 12}, {1, 17}, {2, 22}};

    if (!isValid()) {
        return FALSE;
    }

    if (time == NULL) {
        time = dTime_c::getCurrent();
    }

    u8 value = lookupHour(sHours, time->hour);

    if (isUnk1E452Value(this, value)) {
        return TRUE;
    }
    return FALSE;
}

// 801299E0
BOOL dAnimalHomeStay_c::isAnimal(int idx) {
    if (isValid() && mAnimalIdx == idx) {
        return TRUE;
    }
    return FALSE;
}

// 80129A3C
dAnimalBlock_c::dAnimalBlock_c() {}

// 80129AE4
void dAnimalBlock_c::clear() {
    memset(this, 0, sizeof(dAnimalBlock_c));

    for (int i = 0; i < ANIMAL_NUM; i++) {
        mAnimals[i].clear();
    }

    mSick.clear();
    mLostItem.clear();
    mAppointment.clear();
    mHideAndSeek.clear();
    mStyle.clear();
    mHomeStay.clear();
    clearIdxList(mOutdoorQueue, ANIMAL_NUM);
    mEventId = EVENT_NUM;
    mMoveOutIdx = -1;
    mMoveInIdx = -1;
    clearMovedOut();
}

// initNewTown fills the block for a new town.
static inline int getTemplateNpcIdx(const dAnimalTemplate_c *tmpl) {
    return tmpl->mNpcIdx;
}

// 80129BC4
void dAnimalBlock_c::initNewTown(void *a, u32 b, const dLandID_c *land) {
    clear();

    for (u32 i = 0; i < 6; i++) {
        u32 mask = 0;
        int num = getRarestLooksMask(&mask, mAnimals, ANIMAL_NUM, 0, FALSE);
        if (num == 0 || mask == 0) {
            mask = 0x3F;
            num = 6;
        }

        const dAnimalTemplate_c *tmpl =
            pickTemplate((dAnimalTemplate_c *)a, b, mask, num, 6, mAnimals, ANIMAL_NUM, mAppearedFlags, 0x1B, 0xD1, 1);
        if (tmpl != NULL) {
            dAnimal_c *animal = getFreeAnimal(mAnimals, ANIMAL_NUM, TRUE);
            if (animal != NULL) {
                u16 npcIdx = getTemplateNpcIdx(tmpl);
                animal->init(npcIdx, 0, land, tmpl);
                animal->pickUmbrella();
                setAppeared(npcIdx);
            }
        }
    }

    stampTalkCountTime();
    dItem::Item item;
    mLostItem.set(*dTime_c::getCurrent(), -1, &item);
}

// 80129D1C
u32 dAnimalBlock_c::getAnimalNum() {
    return countAnimals(mAnimals, ANIMAL_NUM);
}

// 80129D24
dItem::Item dAnimalBlock_c::getAnimalKey(const dAnmPersonalID_c *animal) {
    u32 idx = (u32)findAnimalIdx((dAnmPersonalID_c *)animal, mAnimals, ANIMAL_NUM);
    dItem::Item item;
    if (idx < ANIMAL_NUM) {
        item.mId = (idx & 0x3FF) + 0xE000;
    }
    return item;
}

// 80129D7C
u32 dAnimalBlock_c::getAnimalIdx(const dAnmPersonalID_c *animal) {
    return (u32)findAnimalIdx((dAnmPersonalID_c *)animal, mAnimals, ANIMAL_NUM);
}

// 80129D90
dAnimal_c *dAnimalBlock_c::getAnimal(int idx) {
    return fn_8011B910(mAnimals, ANIMAL_NUM, idx);
}

// 80129D9C
dAnimal_c *dAnimalBlock_c::getAnimalConst(int idx) {
    return fn_8011B92C(mAnimals, ANIMAL_NUM, idx);
}

// Villager key items: type 0xE ids without the 0x800 / 0x400 bits; the low 12 bits are the index.
static inline BOOL isAnimalKeyFlag(const u16 *id, u16 bit) {
    BOOL ret = FALSE;
    if (ITEM_NAME_TYPE(*id) == 0xE && (*id & bit)) {
        ret = TRUE;
    }
    return ret;
}

static inline BOOL isAnimalKey(const u16 *id) {
    BOOL type;
    BOOL valid;
    valid = FALSE;
    type = FALSE;
    if (ITEM_NAME_TYPE(*id) == 0xE && !isAnimalKeyFlag(id, 0x800)) {
        type = TRUE;
    }
    if (type && !isAnimalKeyFlag(id, 0x400)) {
        valid = TRUE;
    }
    return valid;
}

// 80129DA8
dAnimal_c *dAnimalBlock_c::getAnimalByKey(dItem::Item *key) {
    const u16 *id = &key->mId;
    if (isAnimalKey(id)) {
        return getAnimal(*id & 0xFFF);
    }
    return NULL;
}

// 80129E24
dAnimal_c *dAnimalBlock_c::getAnimalByKeyConst(const dItem::Item *key) {
    const u16 *id = &key->mId;
    if (isAnimalKey(id)) {
        return getAnimalConst(*id & 0xFFF);
    }
    return NULL;
}

// 80129EA0
dAnimal_c *dAnimalBlock_c::findAnimalByNpcIdx(u16 npcIdx) {
    return fn_8011BB0C(npcIdx, mAnimals, ANIMAL_NUM);
}

// 80129EB4
int dAnimalBlock_c::getFreeIdx() {
    return findFreeAnimalIdx(mAnimals, ANIMAL_NUM, TRUE);
}

// 80129EC0
// Villager whose fn_800F3F30 value against animal wins under cmp; ties are picked at random.
dAnimal_c *findAnimalByCompat(dAnimal_c *animals, dAnimal_c *animal, BOOL (*cmp)(u32, u32), u32 best) {
    dAnimal_c *result = NULL;

    if (animal != NULL && animal->mID.isValid()) {
        int self = (u32)findAnimalIdx((dAnmPersonalID_c *)&animal->mID, animals, ANIMAL_NUM);
        if ((u32)self < ANIMAL_NUM) {
            u32 value;
            u32 num = 0;
            for (int i = 0; i < ANIMAL_NUM; i++, animals++) {
                if (i == self || !animals->mID.isValid() || animals->isMoving()) {
                    continue;
                }

                value = fn_800F3F30(animal, animals);
                if (cmp(best, value)) {
                    result = animals;
                    best = value;
                    num = 1;
                } else if (best == value) {
                    if (result == NULL) {
                        result = animals;
                        num = 1;
                    } else {
                        f32 chance = 100.0f / (num + 1);
                        if (cM::rndF(100.0f) <= chance) {
                            result = animals;
                        }
                        num++;
                    }
                }
            }
        }
    }

    return result;
}

// 8012A040
BOOL isCompatHigher(u32 best, u32 value) {
    return best < value;
}

// 8012A054
BOOL isCompatLower(u32 best, u32 value) {
    return best > value;
}

// 8012A068
dAnimal_c *getBestCompatAnimal(dAnimal_c *animals, dAnimal_c *animal) {
    return findAnimalByCompat(animals, animal, isCompatHigher, 0);
}

// 8012A078
dAnimal_c *getWorstCompatAnimal(dAnimal_c *animals, dAnimal_c *animal) {
    return findAnimalByCompat(animals, animal, isCompatLower, 0xFFFFFFFF);
}

// 8012A088
dAnimal_c *dAnimalBlock_c::pickRandomAnimal(const dAnmPersonalID_c **exclude, u32 num, BOOL flag) {
    return getAnimal(pickAnimalIdx((dAnmPersonalID_c **)exclude, num, mAnimals, ANIMAL_NUM, flag));
}

// 8012A0D4
dAnimal_c *dAnimalBlock_c::pickRandomAnimalConst(const dAnmPersonalID_c **exclude, u32 num, BOOL flag) {
    return getAnimalConst(pickAnimalIdx((dAnmPersonalID_c **)exclude, num, mAnimals, ANIMAL_NUM, flag));
}

// 8012A120
int dAnimalBlock_c::getSickAnimalIdx() {
    dQuestBase_c *quest;
    dAnimal_c *animal = getAnimalConst(0);
    for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
        if (animal->mID.isValid()) {
            quest = &animal->mQuest.mQuest.mBase;
            if (quest->isActive() && (int)quest->mKind == QUEST_KIND_REQUEST_5) {
                return i;
            }
        }
    }
    return -1;
}

// 8012A1B0
BOOL dAnimalBlock_c::isSickAnimal(const dAnmPersonalID_c *animal) {
    u32 idx = (u32)findAnimalIdx((dAnmPersonalID_c *)animal, mAnimals, ANIMAL_NUM);
    if (idx >= ANIMAL_NUM) {
        return FALSE;
    }
    u32 cur = getSickAnimalIdx();
    return cur == idx;
}

// 8012A218
int dAnimalBlock_c::getStyleAnimalIdx() {
    dQuestPlayerPair_c *pair = &mStyle;
    dQuestBase_c *quest = &pair->mBase;
    if (quest->isActive() && (int)quest->mKind == QUEST_KIND_STYLE && (u32)pair->mAnimalIdx < ANIMAL_NUM) {
        return pair->mAnimalIdx;
    }
    return -1;
}

// 8012A280
BOOL dAnimalBlock_c::isStyleAnimal(const dAnmPersonalID_c *id) {
    if (!id->isValid()) {
        return FALSE;
    }

    dAnimal_c *animal = getAnimalConst(getStyleAnimalIdx());
    if (animal != NULL && animal->mID == *id) {
        return TRUE;
    }
    return FALSE;
}

// 8012A390
int dAnimalBlock_c::getAppointmentAnimalIdx() {
    dQuestPlayerItem_c *questItem = &mAppointment;
    dQuestBase_c *quest = &questItem->mBase;
    if (quest->isActive() && quest->getType() == QUEST_TYPE_APPOINTMENT &&
        (u32)questItem->mAnimalIdx < ANIMAL_NUM)
    {
        return questItem->mAnimalIdx;
    }
    return -1;
}

// 8012A3FC
BOOL dAnimalBlock_c::isAppointmentAnimal(const dAnmPersonalID_c *id) {
    if (!id->isValid()) {
        return FALSE;
    }

    dAnimal_c *animal = getAnimalConst(getAppointmentAnimalIdx());
    if (animal != NULL && animal->mID == *id) {
        return TRUE;
    }
    return FALSE;
}

// 8012A50C
BOOL dAnimalBlock_c::isAppointmentSoon(int idx) {
    dQuestPlayerItem_c *questItem = &mAppointment;
    dQuestBase_c *quest = &mAppointment.mBase;
    if (!quest->isActive() || (int)quest->mKind != QUEST_KIND_APPOINTMENT_0 || quest->mState == 2) {
        return FALSE;
    }

    if (idx != questItem->mAnimalIdx) {
        return FALSE;
    }

    dAnimal_c *animal = getAnimalConst(idx);
    if (animal == NULL || !animal->mID.isValid()) {
        return FALSE;
    }

    if ((int)animal->mPlace != 1) {
        return FALSE;
    }

    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayerRaw();
    if (player == NULL || !player->mPID.isFromTown() || !player->mPID.player.isSame(&questItem->mPlayer)) {
        return FALSE;
    }

    dTime_c *now = dTime_c::getCurrent();
    if (questItem->isPastMeetTime(*now, -60) && !questItem->isPastMeetTime(*now, -30)) {
        return TRUE;
    }
    return FALSE;
}

// 8012A668
int dAnimalBlock_c::getHiderIdx(u32 i) {
    if (i < 3) {
        dQuestPlayerAnimal_c *questAnimal = &mHideAndSeek;
        dQuestBase_c *quest = &mHideAndSeek.mBase;
        if (quest->isActive() && (int)quest->mKind == QUEST_KIND_HIDE_AND_SEEK) {
            return questAnimal->getHider(i);
        }
    }
    return -1;
}

// 8012A6E4
dAnimal_c *dAnimalBlock_c::pickRandomAnimalNotMoving(const dAnmPersonalID_c **exclude, int num) {
    int i;
    dAnimal_c *animals = getAnimal(0);
    u32 mask = 0;
    int count = 0;
    dAnimal_c *result = NULL;

    for (i = 0; i < ANIMAL_NUM; i++, animals++) {
        if (animals->mID.isValid() && !animals->isMoving()) {
            mask |= 1 << i;
            count++;
        }
    }
    animals -= ANIMAL_NUM;

    if (exclude != NULL) {
        for (u32 i = 0; i < num; exclude++, i++) {
            if (*exclude != NULL && (*exclude)->isValid()) {
                u32 idx = (u32)findAnimalIdx((dAnmPersonalID_c *)*exclude, animals, ANIMAL_NUM);
                if (idx < ANIMAL_NUM && ((mask >> idx) & 1)) {
                    mask &= ~(1 << idx);
                    count--;
                }
            }
        }
    }

    if (count > 0) {
        result = fn_8011B910(animals, ANIMAL_NUM, pickRandomBit(mask, count, ANIMAL_NUM));
    }
    return result;
}

// 8012A818
dAnimal_c *dAnimalBlock_c::pickRandomAvailableAnimal(const dAnmPersonalID_c **exclude, u32 num) {
    dAnimalHomeStay_c *unk;
    u32 j;
    dAnimal_c *animals;
    u32 mask;
    int count;
    int i;
    dAnimal_c *result;
    animals = getAnimal(0);
    unk = &mHomeStay;
    mask = 0;
    count = 0;
    result = NULL;

    for (i = 0; i < ANIMAL_NUM; i++, animals++) {
        if (animals->mID.isValid() && !animals->isMoving() && (int)animals->mPlace != 4 &&
            !unk->isAnimal(i))
        {
            mask |= 1 << i;
            count++;
        }
    }
    animals -= ANIMAL_NUM;

    if (exclude != NULL) {
        for (j = 0; j < num; exclude++, j++) {
            if (*exclude != NULL && (*exclude)->isValid()) {
                u32 idx = (u32)findAnimalIdx((dAnmPersonalID_c *)*exclude, animals, ANIMAL_NUM);
                if (idx < ANIMAL_NUM && ((mask >> idx) & 1)) {
                    mask &= ~(1 << idx);
                    count--;
                }
            }
        }
    }

    if (count > 0) {
        u32 idx = getSickAnimalIdx();
        if (idx < ANIMAL_NUM && ((mask >> idx) & 1)) {
            count--;
            mask &= ~(1 << idx);
        }
    }

    if (count > 0) {
        result = fn_8011B910(animals, ANIMAL_NUM, pickRandomBit(mask, count, ANIMAL_NUM));
    }
    return result;
}

// 8012A9AC
void clearIdxList(s8 *buf, u32 num) {
    for (int i = 0; i < num; i++) {
        buf[i] = -1;
    }
}

// 8012AA08
BOOL hasIdxInList(const s8 *buf, u32 num) {
    for (u32 i = 0; i < num; i++) {
        if (buf[i] != -1) {
            return TRUE;
        }
    }
    return FALSE;
}

// 8012AA3C
int findIdxInList(s8 idx, const s8 *buf, u32 num) {
    if ((u32)idx >= ANIMAL_NUM) {
        return -1;
    }

    for (u32 i = 0; i < num; i++) {
        s8 v = buf[i];
        if (v == idx) {
            return i;
        }
    }
    return -1;
}

// 8012AA84
void dAnimalBlock_c::initOutdoorQueue() {
    s8 *order = mOutdoorQueue;
    dAnimal_c *animal = getAnimal(0);
    clearIdxList(order, ANIMAL_NUM);
    for (int i = 0; i < ANIMAL_NUM; i++, animal++, order++) {
        if (animal->mID.isValid()) {
            *order = i;
        }
    }
}

// 8012AB04
void dAnimalBlock_c::compactOutdoorQueue() {
    s8 buf[ANIMAL_NUM];
    s8 *old = buf;
    memcpy(old, mOutdoorQueue, sizeof(buf));
    clearIdxList(mOutdoorQueue, ANIMAL_NUM);

    int num = 0;
    for (int i = 0; i < ANIMAL_NUM; i++, old++) {
        if ((u32)*old < ANIMAL_NUM) {
            dAnimal_c *animal = getAnimal(*old);
            if (animal != NULL && animal->mID.isValid() && findIdxInList(*old, mOutdoorQueue, ANIMAL_NUM) == -1) {
                mOutdoorQueue[num++] = *old;
            }
        }
    }
}

// 8012ABE8
BOOL dAnimalBlock_c::addToOutdoorQueue(u32 idx) {
    compactOutdoorQueue();
    if (idx >= ANIMAL_NUM) {
        return FALSE;
    }

    s8 *order = mOutdoorQueue;
    if (findIdxInList(idx, order, ANIMAL_NUM) != -1) {
        return TRUE;
    }

    // Unrolled in the target.
    if ((u32)order[0] >= ANIMAL_NUM) {
        order[0] = idx;
        return TRUE;
    }
    if ((u32)order[1] >= ANIMAL_NUM) {
        order[1] = idx;
        return TRUE;
    }
    if ((u32)order[2] >= ANIMAL_NUM) {
        order[2] = idx;
        return TRUE;
    }
    if ((u32)order[3] >= ANIMAL_NUM) {
        order[3] = idx;
        return TRUE;
    }
    if ((u32)order[4] >= ANIMAL_NUM) {
        order[4] = idx;
        return TRUE;
    }
    if ((u32)order[5] >= ANIMAL_NUM) {
        order[5] = idx;
        return TRUE;
    }
    if ((u32)order[6] >= ANIMAL_NUM) {
        order[6] = idx;
        return TRUE;
    }
    if ((u32)order[7] >= ANIMAL_NUM) {
        order[7] = idx;
        return TRUE;
    }
    if ((u32)order[8] >= ANIMAL_NUM) {
        order[8] = idx;
        return TRUE;
    }
    if ((u32)order[9] >= ANIMAL_NUM) {
        order[9] = idx;
        return TRUE;
    }
    return FALSE;
}

// 8012AD74
u32 halfRoundUp(u32 n) {
    return (n + 1) / 2;
}

// 8012AD80
u32 dAnimalBlock_c::getOutdoorNum() {
    return halfRoundUp(getAnimalNum());
}

// 8012ADA4
void dAnimalBlock_c::decideOutdoorAnimals(BOOL flag) {
    dAnimalHomeStay_c *unk;
    u32 count = getAnimalNum();
    u32 limit = halfRoundUp(count);
    u32 moved = 0;
    s8 order[ANIMAL_NUM];

    if (!hasIdxInList(mOutdoorQueue, ANIMAL_NUM)) {
        initOutdoorQueue();
    }

    clearIdxList(order, ANIMAL_NUM);

    {
        int i;
        int questIdx;
        dAnimal_c *animal;
        animal = getAnimal(0);
        questIdx = mAppointment.mAnimalIdx;
        for (i = 0; i < ANIMAL_NUM; i++, animal++) {
            int state = animal->getDayPlace(0, i == questIdx, 1);
            if (state == 0) {
                u32 pos = findIdxInList(i, mOutdoorQueue, ANIMAL_NUM);
                if (pos < ANIMAL_NUM) {
                    mOutdoorQueue[pos] = -1;
                }
                if (limit != 0) {
                    limit--;
                }
            }
            animal->mDayPlace = state;
        }
    }

    unk = &mHomeStay;
    if (limit != 0) {
        int idx = getBirthdayHostIdx(fn_801017B8(), 1, 0, 0);
        dAnimal_c *animal = getAnimal(idx);
        if (animal != NULL && animal->mID.isValid() && animal->mDayPlace >= 3) {
            animal->mDayPlace = 0;
            u32 pos = findIdxInList(idx, mOutdoorQueue, ANIMAL_NUM);
            if (pos < ANIMAL_NUM) {
                mOutdoorQueue[pos] = -1;
            }
            if (unk->isValid() && idx == unk->mAnimalIdx) {
                unk->clear();
            }
            limit--;
        }
    }

    if (!flag && unk->isValid()) {
        dAnimal_c *animal = getAnimal(unk->mAnimalIdx);
        if (animal != NULL) {
            animal->mDayPlace = 2;
        }
    }

    dAnimal_c *animals = getAnimal(0);
    for (int i = 0; i < ANIMAL_NUM; i++) {
        dAnimal_c *animal = getAnimal(mOutdoorQueue[i]);
        if (animal != NULL && animal->mID.isValid()) {
            if (moved < limit && animal->mDayPlace >= 3) {
                animal->mDayPlace = 0;
                moved++;
                mOutdoorQueue[i] = -1;
            }
        } else {
            mOutdoorQueue[i] = -1;
        }
    }

    if (moved < limit) {
        int i;
        dAnimal_c *animal = getAnimal(0);
        for (i = 0; i < ANIMAL_NUM; i++, animal++) {
            if (animal != NULL && animal->mID.isValid() && animal->mDayPlace >= 3) {
                moved++;
                animal->mDayPlace = 0;
                if (moved >= limit) {
                    break;
                }
            }
        }
    }

    if (flag) {
        dTime_c *now = dTime_c::getCurrent();
        unk->clear();
        if (dEvent::getTownEventOn(now) == -1) {
            u32 num = 0;
            dAnimal_c *animal;
            int i;
            u32 mask = 0;
            animal = getAnimal(0);
            for (i = 0; i < ANIMAL_NUM; i++, animal++) {
                if (animal->mID.isValid() && animal->mDayPlace >= 3 && animal->canMoveOut(this)) {
                    mask |= 1 << i;
                    num++;
                }
            }

            f32 chance = 100.0f * num / 12.0f;
            if (cM::rndF(100.0f) < chance) {
                int idx = pickRandomBit(mask, num, ANIMAL_NUM);
                dAnimal_c *chosen = getAnimal(idx);
                if (chosen != NULL && chosen->mID.isValid()) {
                    unk->set(idx, chosen->mID.getLooks(1));
                    chosen->mDayPlace = 2;
                }
            }
        }
    }

    {
        int i;
        dAnimal_c *animal = getAnimal(0);
        for (i = 0; i < ANIMAL_NUM; i++, animal++) {
            if (animal->mID.isValid() && animal->mDayPlace >= 3) {
                animal->mDayPlace = 1;
            }
        }
    }

    u32 num = 0;
    s8 *out = order;
    for (int i = 0; i < ANIMAL_NUM; i++) {
        int idx = mOutdoorQueue[i];
        dAnimal_c *animal = getAnimal(idx);
        if (animal != NULL && animal->mID.isValid()) {
            *out++ = idx;
            num++;
            mOutdoorQueue[i] = -1;
        }
    }

    {
        dAnimal_c *animal = getAnimal(0);
        s8 *out = order + num;
        for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
            if (animal->mID.isValid() && animal->mDayPlace < 3 && animal->mDayPlace != 0 &&
                findIdxInList(i, order, ANIMAL_NUM) == -1)
            {
                if (num >= count) {
                    break;
                }
                *out++ = i;
                num++;
            }
        }
    }

    {
        dAnimal_c *animal = getAnimal(0);
        s8 *out = order + num;
        for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
            if (animal->mID.isValid() && animal->mDayPlace == 0) {
                if (num >= count) {
                    break;
                }
                *out++ = i;
                num++;
            }
        }
    }

    memcpy(mOutdoorQueue, order, sizeof(order));
}

// 8012B35C
BOOL dAnimalBlock_c::shouldDecideOutdoor(BOOL flag) {
    if (flag) {
        return TRUE;
    }

    if (!hasIdxInList(mOutdoorQueue, ANIMAL_NUM)) {
        return TRUE;
    }

    if (hasBirthdayHost()) {
        return TRUE;
    }

    if ((u8)fn_80162548() != 0x3B) {
        return FALSE;
    }

    dSaveData_c *town = dSaveData_c::getTown();
    dTime_c time = town->_073522.get();
    dTime_c start;
    start.set(time.year, time.month, time.mday, time.hour - 1, 0, 0);
    start.normalize();
    dTime_c end;
    end.set(time.year, time.month, time.mday, time.hour + 1, 0, 0);
    end.normalize();

    dTime_c *now = dTime_c::getCurrent();
    BOOL ret = TRUE;
    if (dTime_c::isSameOrAfter(*now, start)) {
        if (!dTime_c::isSameOrAfter(*now, end)) {
            ret = FALSE;
        }
    }
    return ret;
}

// 8012B620
void dAnimalBlock_c::updateOutdoorAnimals(BOOL flag) {
    if (shouldDecideOutdoor(flag)) {
        if (!hasIdxInList(mOutdoorQueue, ANIMAL_NUM)) {
            flag = TRUE;
        }
        decideOutdoorAnimals(flag);
    }
}

// 8012B688
void dAnimalBlock_c::syncHouses() {
    dAnimal_c *animal = getAnimalConst(0);
    void *fg = &dSaveData_c::getTown()->_05EB04;
    for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
        if (animal->mID.isValid()) {
            if (!fn_8014A5B8(fg, i)) {
                fn_8014A098(fg, i);
            }
        } else if (fn_8014A5B8(fg, i)) {
            fn_8014A4F4(fg, i);
        }
    }
}

// 8012B73C
BOOL dAnimalBlock_c::removeHouse(u32 idx) {
    if (idx >= ANIMAL_NUM) {
        return FALSE;
    }

    void *fg = &dSaveData_c::getTown()->_05EB04;
    if (fn_8014A5B8(fg, idx)) {
        fn_8014A4F4(fg, idx);
        return TRUE;
    }
    return FALSE;
}

// 8012B7B4
BOOL dAnimalBlock_c::getHouseBlockPos(int *x, int *z, int idx) {
    void *fg = &dSaveData_c::getTown()->_05EB04;
    dAnimal_c *animal = getAnimalConst(idx);
    if (animal != NULL && animal->mID.isValid()) {
        int tmpX = 0;
        int tmpZ = 0;
        if (x == NULL) {
            x = &tmpX;
        }
        if (z == NULL) {
            z = &tmpZ;
        }
        if (fn_8014A544(fg, x, z, idx)) {
            return TRUE;
        }
    }
    return FALSE;
}

// 8012B86C
BOOL dAnimalBlock_c::getHouseBlockPosById(int *x, int *z, const dAnmPersonalID_c *animal) {
    return getHouseBlockPos(x, z, (u32)findAnimalIdx((dAnmPersonalID_c *)animal, mAnimals, ANIMAL_NUM));
}

// Temporary declarations (chunk 6)

extern "C" {
// This TU, other chunks.

// Event handlers in lbl_80475D38.

// Other TUs.
void fn_8008BED0(mVec3_c *out, int x, int z);
}

// The flag in dAnimalMemory_c+0x00 tested by sendTunekichiInvites.
struct dAnimalMemoryFlags6_c {
    u32 _0 : 9;
    u32 mTunekichiInvite : 1;
    u32 _10 : 22;
};

// 8012B8D0
BOOL dAnimalBlock_c::getHousePos(mVec3_c *out, int idx) {
    int x = 0;
    int z = 0;
    if (getHouseBlockPos(&x, &z, idx)) {
        fn_8008BED0(out, x, z);
        return TRUE;
    }
    return FALSE;
}

// 8012B938
BOOL dAnimalBlock_c::getHousePosById(mVec3_c *out, const dAnmPersonalID_c *id) {
    return getHousePos(out, findAnimalIdx((dAnmPersonalID_c *)id, mAnimals, ANIMAL_NUM));
}

// 8012B98C
BOOL dAnimalBlock_c::getHouseFrontPos(mVec3_c *out, int idx) {
    if (getHousePos(out, idx)) {
        out->z += 64.0f;
        return TRUE;
    }
    return FALSE;
}

// 8012B9DC
BOOL dAnimalBlock_c::isHomeStayTime(const dAnmPersonalID_c *id, dPrivateData_c *player) {
    dAnimal_c *animal;
    dAnimalHomeStay_c *unk;
    int idx;
    if (fn_800DCEDC()) {
        return FALSE;
    }

    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayerRaw();
    }

    if (player == NULL || !player->mPID.isValid() || player->isFlag0(0xD)) {
        return FALSE;
    }

    if (!player->mPID.isFromTown() && player->isFlag0(0x28)) {
        return FALSE;
    }

    if (!id->isValid()) {
        return FALSE;
    }

    idx = findAnimalIdx((dAnmPersonalID_c *)id, getAnimalConst(0), ANIMAL_NUM);
    animal = getAnimalConst(idx);
    if (animal == NULL || !animal->findMemory2(&player->mPID)) {
        return FALSE;
    }

    unk = &mHomeStay;
    if (!unk->isValid() || idx != unk->mAnimalIdx) {
        return FALSE;
    }

    return unk->isPeriod(NULL) != FALSE;
}

// 8012BB4C
void dAnimalBlock_c::updateAnimalPlaces() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    BOOL flag = player != NULL ? player->isFlag0(0xD) : FALSE;
    dAnimal_c *animal = getAnimal(0);
    BOOL event = dEvent::isActive(EVENT_TOY_DAY) != FALSE;

    for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
        if (!animal->mID.isValid()) {
            continue;
        }

        u8 kind = animal->mDayPlace;
        if (animal->getDayPlace(1, 0, 1) == 3 && (kind != 0 || !event) && animal->isSleepTime(0) && !flag) {
            animal->mPlace = 4;
            continue;
        }

        switch (kind) {
        case 0:
            if (!event && !flag && animal->isSleepTime(0)) {
                animal->mPlace = 4;
            } else {
                animal->mPlace = 0;
            }
            break;
        case 1:
        case 2:
            if (isHomeStayTime(&animal->mID, player)) {
                animal->mPlace = 3;
            } else {
                animal->mPlace = 1;
            }
            break;
        }
    }
}

// 8012BCBC
void dAnimalBlock_c::stampTalkCountTime() {
    mTalkCountTime.setNow();
}

// 8012BCC8
void dAnimalBlock_c::resetDailyTalkCounts() {
    dTime_c *now = dTime_c::getCurrent();
    dTime_c last = mTalkCountTime.get();
    BOOL newDay = FALSE;

    dTimeStamp_c today(now);
    today.toDayStart();
    dTime_c start = today.get();

    if (dTime_c::isSameOrAfter(start, last)) {
        newDay = TRUE;
    } else {
        start.add(1, 0, 0, 0);
        if (dTime_c::isSameOrAfter(last, start)) {
            newDay = TRUE;
        }
    }

    if (newDay) {
        dAnimal_c *animal = getAnimal(0);
        for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
            animal->clearTalkCounts();
        }
        stampTalkCountTime();
    }
}

// 80475D38
static const dAnimalEvent_c sEventHandlers[] = {
    {EVENT_HARVEST_FESTIVAL, (dAnimalEventFunc)initHarvestFestival}, {EVENT_HALLOWEEN, (dAnimalEventFunc)initHalloween}, {EVENT_FISHING_TOURNEY, (dAnimalEventFunc)initFishingTourney}, {EVENT_BUG_OFF, (dAnimalEventFunc)initBugOff}, {EVENT_COUNTDOWN, (dAnimalEventFunc)initCountdown},
    {EVENT_FIREWORKS, (dAnimalEventFunc)initFireworks}, {EVENT_FLEA_MARKET, (dAnimalEventFunc)initFleaMarket}, {EVENT_FESTIVALE, (dAnimalEventFunc)initFestivale}, {EVENT_TOY_DAY, (dAnimalEventFunc)initToyDay},
};

// 8012BF70
const dAnimalEvent_c *findTodayEvent() {
    const dAnimalEvent_c *event = sEventHandlers;
    for (u32 i = 0; i < ARRAY_SIZE(sEventHandlers); i++, event++) {
        if (dEvent::isActive((dQuestEvent_e)event->mEvent)) {
            return event;
        }
    }
    return NULL;
}

// 8012BFDC
const dAnimalEvent_c *findEventOnDay(const dTime_c *time, u32 arg) {
    const dAnimalEvent_c *event = sEventHandlers;
    dTime_c t = *time;
    t.add(0, -6, 0, 0);

    for (u32 i = 0; i < ARRAY_SIZE(sEventHandlers); i++, event++) {
        if (dEvent::isEventWithin((dQuestEvent_e)event->mEvent, t, 0, arg)) {
            return event;
        }
    }
    return NULL;
}

// 8012C0C4
void dAnimalBlock_c::updateEvent(BOOL force) {
    BOOL update = FALSE;
    if (mEventId >= EVENT_NUM || force) {
        update = TRUE;
    }

    if (update) {
        dAnimal_c *animal = getAnimal(0);
        mEventId = EVENT_NUM;
        for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
            animal->mEvent.clear();
        }

        const dAnimalEvent_c *event = findTodayEvent();
        if (event != NULL) {
            mEventId = event->mEvent;
            if (event->mFunc != NULL) {
                int arg = getOutdoorNum();
                event->mFunc(getAnimal(0), ANIMAL_NUM, mOutdoorQueue, arg, mAppointment.mAnimalIdx);
            }
        }
    } else if (mEventId < EVENT_NUM) {
        dAnimal_c *animal = getAnimal(0);
        dTime_c *now = dTime_c::getCurrent();

        switch (mEventId) {
        case 0x0E:
        case 0x0F:
        case 0x10:
        case 0x11:
        case 0x15:
        case 0x16:
            if (dEvent::isNotStarted((dQuestEvent_e)mEventId)) {
                for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
                    animal->clearMemoryFlag24();
                    animal->clearEventFlags();
                }
            } else if (dEvent::isOngoing((dQuestEvent_e)mEventId)) {
                for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
                    animal->clearMemoryFlag24();
                }
            }
            break;
        case 0x13:
            if (dEvent::isNotStarted((dQuestEvent_e)mEventId)) {
                for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
                    animal->clearMemoryFlag24();
                    animal->clearEventFlags();
                }
            } else {
                if (dEvent::isOngoing((dQuestEvent_e)mEventId) && now->hour > 6) {
                    for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
                        animal->clearMemoryFlag24();
                    }
                }
            }
            break;
        }
    }
}

// 8012C330
void dAnimalBlock_c::updateQuestsDaily(int arg) {
    dPrivateData_c *players;
    dAnimal_c *animal = getAnimal(0);
    players = dSaveData_c::getTown()->mPlayers;
    dLostQuest_c *lost = &mLostItem;

    updateSick(arg);
    updateStyleDaily(arg);
    updateAppointmentDaily(arg);
    updateHideAndSeekDaily(arg);

    for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
        if (animal->mID.isValid()) {
            for (int j = 0; j < PLAYER_NUM; j++) {
                dPrivateData_c *player = dPrivateData_c::getChecked(players, j);
                if (player != NULL && player->mPID.isValid()) {
                    animal->updateQuests(lost, player, arg);
                }
            }
        }
    }
}

// 8012C420
// Wish of the most similar villager (ties broken at random).
int dAnimalBlock_c::pickWishOfBestMatch(dAnimal_c *animal) {
    int idx;
    dQuestWish_c *wish;
    int result = 8;

    if (animal->mID.isValid()) {
        idx = findAnimalIdx((dAnmPersonalID_c *)&animal->mID, mAnimals, ANIMAL_NUM);
        if ((u32)idx < ANIMAL_NUM) {
            int best;
            int kind = animal->mQuest.mWish.mKind;
            best = 0;
            dAnimal_c *other = getAnimalConst(0);
            u32 num = 0;

            for (int i = 0; i < ANIMAL_NUM; i++, other++) {
                if (i == idx || !other->mID.isValid()) {
                    continue;
                }

                wish = &other->mQuest.mWish;
                if (!wish->isValid() || kind == wish->mKind) {
                    continue;
                }

                int score = fn_800F3F30(animal, other);
                if (best < score) {
                    result = wish->mKind;
                    best = score;
                    num = 1;
                } else if (best == score) {
                    if (result == 8) {
                        result = wish->mKind;
                        num = 1;
                    } else {
                        f32 rate = 100.0f / (num + 1);
                        if (cM::rndF(100.0f) <= rate) {
                            result = wish->mKind;
                        }
                        num++;
                    }
                }
            }
        }
    }

    return result;
}

// 8012C59C
// A wish none of the least similar villagers has (picked at random).
int dAnimalBlock_c::pickWishUnlikeWorstMatch(dAnimal_c *animal) {
    int result;
    int idx;
    dQuestWish_c *wish;
    u32 best;
    dAnimal_c *other;
    u16 mask;

    result = 8;
    if (animal->mID.isValid()) {
        idx = findAnimalIdx((dAnmPersonalID_c *)&animal->mID, mAnimals, ANIMAL_NUM);
        if ((u32)idx < ANIMAL_NUM) {
            best = 0xFFFFFFFF;
            other = getAnimalConst(0);
            mask = 0;

            for (int i = 0; i < ANIMAL_NUM; i++, other++) {
                if (i == idx || !other->mID.isValid()) {
                    continue;
                }

                wish = &other->mQuest.mWish;
                if (!wish->isValid()) {
                    continue;
                }

                u8 kind = wish->mKind;
                u32 score = fn_800F3F30(animal, other);
                if (best > score) {
                    best = score;
                    mask = 1 << kind;
                } else if (best == score) {
                    if (!((mask >> kind) & 1)) {
                        mask |= 1 << kind;
                    }
                }
            }

            int own = animal->mQuest.mWish.mKind;
            if (own < 8 && !((mask >> own) & 1)) {
                mask |= 1 << own;
            }

            u32 num = 0;
            for (int i = 0; i < 8; i++) {
                if (!((mask >> i) & 1)) {
                    f32 rate = 100.0f / (num + 1);
                    if (cM::rndF(100.0f) <= rate) {
                        result = i;
                    }
                    num++;
                }
            }
        }
    }

    return result;
}

// 8012C75C
int pickRandomWish(int exclude) {
    int result = 8;
    u32 num = 0;
    for (int i = 0; i < 8; i++) {
        if (i != exclude) {
            f32 rate = 100.0f / (num + 1);
            if (cM::rndF(100.0f) <= rate) {
                result = i;
            }
        }
    }
    return result;
}

// 8012C818
BOOL dAnimalBlock_c::changeWish(dAnimal_c *animal) {
    if (animal == NULL || !animal->mID.isValid()) {
        return FALSE;
    }

    if ((u32)findAnimalIdx((dAnmPersonalID_c *)&animal->mID, getAnimal(0), ANIMAL_NUM) >= ANIMAL_NUM) {
        return FALSE;
    }

    getAnimal(0);
    dQuestWish_c *wish = &animal->mQuest.mWish;
    int kind = 8;
    int old = wish->mKind;

    if (cM::rndF(100.0f) < 70.0f) {
        kind = pickWishOfBestMatch(animal);
    }
    if (kind == 8) {
        kind = pickWishUnlikeWorstMatch(animal);
    }
    if (kind == 8) {
        kind = pickRandomWish(wish->mKind);
    }

    wish->set(kind);
    if ((wish->isValid() && kind == 2 && old != 2) || (kind != 2 && old == 2)) {
        animal->clearFossilRoomFtr();
    }
    return TRUE;
}

// 8012C948
void dAnimalBlock_c::updateWishes(BOOL arg) {
    dQuestVillagerWish_c *quest;
    if (!arg) {
        return;
    }

    dAnimal_c *animal = getAnimal(0);
    for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
        if (!animal->mID.isValid() || animal->isMoving()) {
            continue;
        }

        animal->growWish();
        quest = &animal->mQuest;
        if (quest->mWish.isValid()) {
            if (!quest->mQuest.mBase.isActive()) {
                u8 value = quest->mWish.mValue;
                if ((u32)cM::rndF(100.0f) < value) {
                    changeWish(animal);
                }
            }
        } else {
            quest->mWish.setRandom();
            if (quest->mWish.isValid() && (int)quest->mWish.mKind == 2) {
                animal->clearFossilRoomFtr();
            }
        }
    }
}

// 8012CA50
void dAnimalBlock_c::expireLostItemQuests() {
    dAnimal_c *animal = getAnimal(0);
    for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
        animal->updateLostItemRequest();
    }
}

// 8012CAA4
void dAnimalBlock_c::sendTunekichiInvites() {
    dPrivateData_c *players;
    dHomeList_c *homes;
    dAnimal_c *animal = getAnimal(0);
    players = dSaveData_c::getRaw()->mPlayers;
    homes = &dSaveData_c::getRaw()->mHomes;

    for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
        if (!animal->mID.isValid()) {
            continue;
        }

        for (int j = 0; j < PLAYER_NUM; j++) {
            dPrivateData_c *player = dPrivateData_c::getRaw(players, j);
            if (player == NULL || !player->mPID.isValid() || (u32)homes->findOwner(player) >= 4) {
                continue;
            }

            dAnimalMemoryFlags6_c *memory = (dAnimalMemoryFlags6_c *)animal->findMemory(&player->mPID);
            if (memory != NULL && memory->mTunekichiInvite) {
                if (animal->sendTunekichiLetter(&player->mPID, 1, 1)) {
                    memory->mTunekichiInvite = 0;
                }
            }
        }
    }
}

// 8012CBAC
void dAnimalBlock_c::updateDaily(int arg) {
    if (arg) {
        updateQuestsDaily(arg);
        updateWishes(arg);
        sendTunekichiInvites();
        updateClothes(arg);

        dAnimal_c *animal = getAnimal(0);
        for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
            animal->updateOwnItems();
            animal->clearMemoryFlag24();
            animal->clearEventFlags();
            animal->fn_801236DC();
            animal->fn_801237C0();
            animal->fn_801238A4();
            animal->fn_80123988();
            animal->decNicknameWait(arg);
            animal->decGreetingWait(arg);
            animal->decHabitCooldown(arg);
        }
    }

    clearBirthdayHosts();
    decideBirthdayHosts();
    sendBirthdayLetters();
    sendValentineLetters();
    sendNewYearLettersCurrent();
    updateEvent(arg);
}

// 8012CCD0
int dAnimalBlock_c::receiveLetter(dMail_c *mail) {
    int result = 0;

    if (mail != NULL && !mail->isEmpty()) {
        dAnmPersonalID_c *to = mail->getToAnimal();
        if (to != NULL && to->isValid()) {
            int idx = findAnimalIdx((dAnmPersonalID_c *)to, getAnimal(0), ANIMAL_NUM);
            dAnimal_c *animal = getAnimal(idx);
            if (animal != NULL) {
                dPrivateData_c *player;
                dPersonalID_c *from = mail->getFromPlayer();
                if (from != NULL) {
                    player = dPlayerMgr_c::getPlayer(from);
                } else {
                    player = NULL;
                }

                if (player != NULL) {
                    player->mErrand.checkLetter(mail);
                    if (mail->isFlaggedInvite()) {
                        animal->sendTunekichiLetter(&player->mPID, 0, 1);
                    } else {
                        animal->writeReplyLetter(mail, player);
                    }
                }

                result = animal->receiveLetter(mail, idx);
            }
        }
    }

    return result;
}

// 8012CE0C
// Kind of the quest the villager is busy with, QUEST_KIND_NONE if none.
int dAnimalBlock_c::getBusyQuestKind(const dAnmPersonalID_c *id, int which, BOOL current) {
    dAnmPersonalID_c *animalId;
    dAnimal_c *animal;
    dQuestBase_c *quest;
    int idx;
    idx = findAnimalIdx((dAnmPersonalID_c *)id, getAnimalConst(0), ANIMAL_NUM);
    animal = getAnimalConst(idx);
    if (animal == NULL) {
        return QUEST_KIND_NONE;
    }

    animalId = &animal->mID;
    if (!animalId->isValid()) {
        return QUEST_KIND_NONE;
    }

    if (current) {
        dPlayerMgr_c::getCurrentPlayer();
    } else {
        dPrivateData_c::findErrandAll(dSaveData_c::getTown()->mPlayers, animalId, which, 0);
    }

    int styleIdx = getStyleAnimalIdx();
    if (styleIdx == idx) {
        return QUEST_KIND_STYLE;
    }

    int itemIdx = getAppointmentAnimalIdx();
    if (itemIdx == idx) {
        return mAppointment.mBase.mKind;
    }

    if (getHiderSlot(id) != 3) {
        return mHideAndSeek.mBase.mKind;
    }

    quest = &animal->mQuest.mQuest.mBase;
    if (quest->isActive()) {
        return quest->mKind;
    }

    return QUEST_KIND_NONE;
}

static const u8 sErrandKinds[] = {QUEST_KIND_ERRAND_REQUEST, QUEST_KIND_ERRAND_REQUEST_FINAL};
static const u8 sAppointmentKinds[] = {QUEST_KIND_APPOINTMENT_1, QUEST_KIND_APPOINTMENT_0};

// 8012CF3C
// Hands out the day's quests to the villagers' NPC entries.
void dAnimalBlock_c::assignQuestCandidates() {
    dAnimal_c *animal;
    dQuestWish_c *wish;
    u8 questKind;
    for (int i = 0; i < ANIMAL_NUM; i++) {
        dNpcEntry_c *entry = fn_800F3E54(i);
        if (entry != NULL) {
            entry->mQuest.clear();
        }
    }

    f32 chance = 10.0f * mHideAndSeek._E9;
    if (!mHideAndSeek.mBase.isActive() && cM::rndF(100.0f) < chance) {
        int sel = -1;
        u32 num = 0;
        animal = getAnimal(0);
        for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
            if (!animal->mID.isValid()) {
                continue;
            }
            dNpcEntry_c *entry = fn_800F3E54(i);
            if (entry == NULL || entry->mQuest.isActive()) {
                continue;
            }
            if (getBusyQuestKind(&animal->mID, 2, 1) != QUEST_KIND_NONE || !canHide(i, 0)) {
                continue;
            }
            f32 rate = 100.0f / (num + 1);
            if (cM::rndF(100.0f) <= rate) {
                sel = i;
            }
            num++;
        }

        dNpcEntry_c *entry = fn_800F3E54(sel);
        if (entry != NULL) {
            dItem::Item item;
            entry->mQuest.set(QUEST_KIND_HIDE_AND_SEEK, &item, NULL, QUEST_DEADLINE_LIMIT, 0);
        }
    }

    const u8 *kind = sErrandKinds;
    for (u32 j = 0; j < ARRAY_SIZE(sErrandKinds); j++, kind++) {
        int sel = -1;
        u32 num = 0;
        animal = getAnimal(0);
        for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
            if (!animal->mID.isValid()) {
                continue;
            }
            dNpcEntry_c *entry = fn_800F3E54(i);
            if (entry == NULL || entry->mQuest.isActive()) {
                continue;
            }
            int busy = getBusyQuestKind(&animal->mID, 0, 1);
            if (busy == QUEST_KIND_NONE) {
                f32 rate = 100.0f / (num + 1);
                if (cM::rndF(100.0f) <= rate) {
                    sel = i;
                }
                num++;
            } else if (busy == *kind) {
                break;
            }
        }

        dNpcEntry_c *entry = fn_800F3E54(sel);
        if (entry != NULL) {
            dItem::Item item;
            entry->mQuest.set(*kind, &item, NULL, QUEST_DEADLINE_LIMIT, 0);
        }
    }

    if ((u32)getStyleAnimalIdx() >= ANIMAL_NUM) {
        int sel = -1;
        u32 num = 0;
        animal = getAnimal(0);
        for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
            if (!animal->mID.isValid()) {
                continue;
            }
            dNpcEntry_c *entry = fn_800F3E54(i);
            if (entry == NULL || entry->mQuest.isActive()) {
                continue;
            }
            if (getBusyQuestKind(&animal->mID, 2, 1) != QUEST_KIND_NONE) {
                continue;
            }
            f32 rate = 100.0f / (num + 1);
            if (cM::rndF(100.0f) <= rate) {
                sel = i;
            }
            num++;
        }

        dNpcEntry_c *entry = fn_800F3E54(sel);
        if (entry != NULL) {
            dItem::Item item;
            entry->mQuest.set(QUEST_KIND_STYLE, &item, NULL, QUEST_DEADLINE_LIMIT, 0);
        }
    }

    if ((u32)getAppointmentAnimalIdx() >= ANIMAL_NUM) {
        int pick = cM::rndInt(2);
        int sel = -1;
        u32 num = 0;
        animal = getAnimal(0);
        for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
            if (!animal->mID.isValid()) {
                continue;
            }
            dNpcEntry_c *entry = fn_800F3E54(i);
            if (entry == NULL || entry->mQuest.isActive()) {
                continue;
            }
            if (getBusyQuestKind(&animal->mID, 2, 1) != QUEST_KIND_NONE) {
                continue;
            }
            wish = &animal->mQuest.mWish;
            if (!wish->isValid() || ((int)wish->mKind != 7 && (int)wish->mKind != 5)) {
                continue;
            }
            f32 rate = 100.0f / (num + 1);
            if (cM::rndF(100.0f) <= rate) {
                sel = i;
            }
            num++;
        }

        dNpcEntry_c *entry = fn_800F3E54(sel);
        if (entry != NULL) {
            dItem::Item item;
            entry->mQuest.set(sAppointmentKinds[pick], &item, NULL, QUEST_DEADLINE_LIMIT, 0);
        }
    }

    animal = getAnimal(0);
    for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
        if (!animal->mID.isValid()) {
            continue;
        }
        dNpcEntry_c *entry = fn_800F3E54(i);
        if (entry == NULL || entry->mQuest.isActive()) {
            continue;
        }
        if (getBusyQuestKind(&animal->mID, 2, 1) != QUEST_KIND_NONE) {
            continue;
        }
        int questKind = animal->mQuest.mWish.getQuestKind();
        if (dQuestBase_c::getKindType(questKind) == QUEST_TYPE_REQUEST) {
            dItem::Item item;
            entry->mQuest.set(questKind, &item, NULL, QUEST_DEADLINE_LIMIT, 0);
        }
    }

    fn_800F3E48();
}

// 8012D508
void dAnimalBlock_c::updateQuestCandidates() {
    dTime_c *now = dTime_c::getCurrent();
    dTime_c *last = fn_800F3E38();
    dTime_c next;
    next.set(last->year, last->month, last->mday, last->hour + 1, last->min, last->sec);
    next.normalize();

    if (!dTime_c::isSameOrAfter(*now, next)) {
        return;
    }

    dAnmPersonalID_c *id;
    u32 total;
    u32 free;
    dAnimal_c *animal;

    fn_800F3E48();
    total = getAnimalNum();
    free = 0;
    animal = getAnimal(0);
    for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
        id = &animal->mID;
        if (!id->isValid()) {
            continue;
        }
        if (animal->isMoving()) {
            total--;
            continue;
        }
        dNpcEntry_c *entry = fn_800F3EE8(id);
        if (entry != NULL && !entry->mQuest.isActive() && getBusyQuestKind(id, 0, 1) == QUEST_KIND_NONE) {
            free++;
        }
    }

    if (total != 0 && free >= total / 2) {
        assignQuestCandidates();
    }
}

// 8012D6E8
int dAnimalBlock_c::pickEvent16Animal() {
    dAnimalEventState_c *unk;
    int sel = -1;

    if ((int)mEventId == 0x16) {
        dAnimal_c *animal = getAnimalConst(0);
        u32 num = 0;
        for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
            if (!animal->mID.isValid()) {
                continue;
            }
            unk = &animal->mEvent;
            if (i == getSickAnimalIdx() || !unk->isInEvent() || unk->isFlag(0)) {
                continue;
            }
            f32 rate = 100.0f / (num + 1);
            if (cM::rndF(100.0f) <= rate) {
                sel = i;
            }
            num++;
        }
    }

    return sel;
}

// 8012D80C
void dAnimalBlock_c::setEvent16Done(int idx) {
    dAnimal_c *animal = getAnimal(idx);
    if (animal != NULL) {
        animal->mEvent.setFlag(0);
        fn_800F0108(idx, animal->mEvent.mFlags);
    }
}

// 8012D864
// Villager with an active lost-item request, -1 if none.
int dAnimalBlock_c::getLostItemAnimalIdx() {
    dQuestBase_c *quest;
    dAnimal_c *animal = getAnimalConst(0);
    for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
        if (animal->mID.isValid()) {
            quest = &animal->mQuest.mQuest.mBase;
            if (quest->isActive() && (int)quest->mKind == QUEST_KIND_REQUEST_6) {
                return i;
            }
        }
    }
    return -1;
}

// 8012D8F4
BOOL dAnimalBlock_c::isLostItemAnimal(const dAnmPersonalID_c *id) {
    u32 idx = findAnimalIdx((dAnmPersonalID_c *)id, mAnimals, ANIMAL_NUM);
    if (idx >= ANIMAL_NUM) {
        return FALSE;
    }
    u32 cur = getLostItemAnimalIdx();
    return cur == idx;
}

// 8012D95C
BOOL dAnimalBlock_c::tryStartLostItem(int arg) {
    if ((u32)getLostItemAnimalIdx() < ANIMAL_NUM) {
        return FALSE;
    }

    if (!shouldDecideOutdoor(arg)) {
        return FALSE;
    }

    dLostQuest_c *lost;
    dTime_c *now = dTime_c::getCurrent();
    if (!dQuestBase_c::checkEventSchedule(QUEST_KIND_REQUEST_6, now)) {
        return FALSE;
    }

    if (!dQuestBase_c::checkTodayEvents(QUEST_KIND_REQUEST_6)) {
        return FALSE;
    }

    lost = &mLostItem;
    if (!lost->fn_801424BC(now)) {
        return FALSE;
    }

    int skip = lost->_08;
    dAnimal_c *animal = getAnimal(0);
    u32 num = 0;
    int sel = -1;
    for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
        if (i == skip || !animal->mID.isValid() || animal->isMoving()) {
            continue;
        }
        if (getBusyQuestKind(&animal->mID, 2, 1) != QUEST_KIND_NONE) {
            continue;
        }
        f32 rate = 100.0f / (num + 1);
        if (cM::rndF(100.0f) <= rate) {
            sel = i;
        }
        num++;
    }

    dAnimal_c *target = getAnimal(sel);
    if (target != NULL) {
        dItem::Item key = lost->fn_801423D8();
        target->mQuest.mQuest.start(QUEST_KIND_REQUEST_6, NULL, &key, now, QUEST_DEADLINE_12_HOURS, 0);
        lost->set(*now, sel, &key);
        return TRUE;
    }
    return FALSE;
}

// 8012DB6C
int dAnimalBlock_c::getLostItemLikeIdx(dPrivateData_c *player) {
    dAnimal_c *animal = getAnimalConst(getLostItemAnimalIdx());
    if (animal == NULL) {
        return -1;
    }

    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayer();
    }
    if (player == NULL) {
        return -1;
    }

    dQuestVillager_c *quest = &animal->mQuest.mQuest;
    int result = -1;
    if (quest->mBase.mState == 0 && quest->getPlayerFlag(&player->mPID.player)) {
        dItem::Item item = quest->mBase.mItem;
        if (item.mId != dItem::ITEM_ID_NONE) {
            dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(item);
            if (bitm != NULL) {
                int kind = 0x29;
                int k = bitm->m_kind;
                if ((u32)k < 0x57) {
                    kind = k;
                }
                if (kind == 0x35) {
                    result = dItem::seeker_c::get()->findLike(item);
                }
            }
        }
    }
    return result;
}

// 8012DC70
void dAnimalBlock_c::setLostItemState1() {
    dAnimal_c *animal = getAnimal(getLostItemAnimalIdx());
    if (animal != NULL && animal->mQuest.mQuest.mBase.mState == 0) {
        animal->mQuest.mQuest.mBase.mState = 1;
    }
}

// 8012DCC4
// Advances the town's sickness by days; may make a villager sick.
BOOL dAnimalBlock_c::tryStartSick(int days, dTime_c *now) {
    if (days == 0) {
        return FALSE;
    }

    int sickness;
    dQuestSick_c *sick;
    dAnmPersonalID_c *id;
    dQuestVillager_c *quest;
    int sel;
    sick = &mSick;
    sickness = sick->mSickness;
    dTime_c time;
    time.set(now->year, now->month, now->mday + days, 0, 0, 0);
    time.normalize();

    if (sickness < 0) {
        sick->addSickness(1, (u8)time.month);
        return FALSE;
    }

    if ((u32)getSickAnimalIdx() < ANIMAL_NUM) {
        return FALSE;
    }

    if (getAnimalNum() <= 6) {
        return FALSE;
    }

    if (!dQuestBase_c::checkTodayEvents(QUEST_KIND_REQUEST_5)) {
        return FALSE;
    }

    if ((int)cM::rndF(100.0f) < sickness) {
        int skip0 = sick->mAnimalIdx;
        int skip1 = mMoveOutIdx;
        sel = -1;
        dAnimal_c *animal = getAnimal(0);
        u32 num = 0;
        for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
            if (i == skip0) {
                continue;
            }
            id = &animal->mID;
            if (i == skip1) {
                continue;
            }
            if (!id->isValid() || animal->isMoving()) {
                continue;
            }
            if (getBusyQuestKind(id, 2, 1) != QUEST_KIND_NONE) {
                continue;
            }
            f32 rate = 100.0f / (num + 1);
            if (cM::rndF(100.0f) <= rate) {
                sel = i;
            }
            num++;
        }

        dAnimal_c *target = getAnimal(sel);
        if (target != NULL) {
            quest = &target->mQuest.mQuest;
            dItem::Item medicine = dQuestSick_c::getMedicine();
            quest->start(QUEST_KIND_REQUEST_5, NULL, &medicine, now, 5, 0);
            sick->start(sel);
            quest->mMatchParam = (u32)cM::rndF(2.0f) + 1;
            return TRUE;
        }
    }

    sick->addSickness(1, (u8)time.month);
    return FALSE;
}

// 8012DF50
BOOL dAnimalBlock_c::updateSickAnimal() {
    u8 state;
    dQuestSick_c *sick;
    dQuestVillager_c *quest;
    dPlayerID_c *requester;
    dAnimal_c *animal;

    u32 idx = getSickAnimalIdx();
    if (idx >= ANIMAL_NUM) {
        return FALSE;
    }

    animal = getAnimal(idx);
    if (animal == NULL) {
        return FALSE;
    }

    quest = &animal->mQuest.mQuest;
    sick = &mSick;
    requester = &quest->mRequester;
    state = animal->mQuest.mQuest.mBase.mState;

    if (requester->isValid() && (state == 2 || (state == 1 && quest->findPlayer(requester) != -1))) {
        for (int i = 0; i < PLAYER_NUM; i++) {
            dPrivateData_c *player = dPlayerMgr_c::getPlayer(i);
            if (player != NULL && player->mPID.player.isSame(requester)) {
                animal->sendSickReward(0, player, 1);
                break;
            }
        }
    }

    if (state == 1 || state == 2) {
        quest->clear();
        return TRUE;
    }

    int visit = quest->mMatchParam;
    if (visit >= 3) {
        return FALSE;
    }

    int next;
    if (sick->_43) {
        next = visit - 1;
    } else {
        dPlayerID_c *player = sick->getPlayer(visit);
        if (player == NULL) {
            return FALSE;
        }
        if (player->isValid()) {
            next = visit - 1;
        } else {
            next = visit + 1;
            if (next >= 3) {
                next = 2;
            }
        }
    }

    if (next < 0) {
        quest->mBase.mState = 1;
        quest->mMatchParam = 3;
        dPlayerID_c *best = NULL;
        u32 bestNum = 0;
        for (int i = 0; i < 3; i++) {
            dPlayerID_c *player = sick->getPlayer(i);
            u32 num = 0;
            if (player != NULL && player->isValid()) {
                for (int j = i; j < 3; j++) {
                    dPlayerID_c *other = sick->getPlayer(j);
                    if (other != NULL && other->isValid() && player->isSame(other)) {
                        num++;
                    }
                }
                if ((num != 0 && num == bestNum) || num > bestNum) {
                    best = player;
                    bestNum = num;
                }
            }
        }
        if (best != NULL) {
            quest->setRequester(best);
        }
    } else {
        quest->mMatchParam = next;
        sick->clearPlayer(next);
        sick->addVisit();
    }

    return TRUE;
}

// 8012E1CC
void dAnimalBlock_c::updateSick(int days) {
    while (days != 0) {
        if (!updateSickAnimal()) {
            break;
        }
        days = days > 0 ? days - 1 : days + 1;
    }

    dTime_c *now = dTime_c::getCurrent();
    while (days != 0) {
        if (tryStartSick(days, now)) {
            break;
        }
        days = days > 0 ? days - 1 : days + 1;
    }
}

// Temporary declarations (chunk 7)

// {kind, sub} range for the item picker fn_800C60B4.
struct dAnmItemRange_c {
    dAnmItemRange_c(int kind, int sub) : mKind(kind), mSub(sub) {}

    /* 0x0 */ int mKind;
    /* 0x4 */ int mSub;
};

// Filter argument of fn_800C60B4. _0 is a catalog (dPrivateData_c::mCatalog) or NULL.
struct dAnmItemFilter_c {
    dAnmItemFilter_c(const void *p, int a, int b) : _0(p), _4(a), _8(b) {}

    /* 0x0 */ const void *_0;
    /* 0x4 */ int _4;
    /* 0x8 */ int _8;
};

extern "C" {
// This TU, other chunks.

// Other TUs.
u16 fn_800CBA14(const char *label); // header variants of a mail label
u16 fn_800CBA40(const char *label); // body variants
u16 fn_800CBA6C(const char *label); // footer variants
void fn_800CBCD4(int slot, dScript::Word_c *word);
BOOL fn_801029C0(dMail_c *mail);
BOOL fn_80102BBC(dMail_c *mail);
extern u8 lbl_8059FF80[];
}

// 804EED68
static const char *sQ13MailLabels[LOOKS_TYPE_NUM] = {
    "MAIL_BO_Q13", "MAIL_HA_Q13", "MAIL_KO_Q13", "MAIL_FU_Q13", "MAIL_GE_Q13", "MAIL_TA_Q13",
};

// 8012E278
// Random other town player the villager knows, or NULL.
dPlayerID_c *dAnimalBlock_c::pickStyleOtherPlayer(dAnimal_c *animal, dPrivateData_c *players, dPrivateData_c *self) {
    if (!animal->mID.isValid()) {
        return NULL;
    }
    if (self == NULL) {
        return NULL;
    }

    dPersonalID_c *pid;
    const dPersonalID_c *selfPid = &self->mPID;
    if (!selfPid->isValid() || self->isFlag0(0xD)) {
        return NULL;
    }

    dPlayerID_c *result = NULL;
    u32 count = 0;
    for (int i = 0; i < 4; i++) {
        dPrivateData_c *player = dPrivateData_c::getRaw(players, i);
        if (player != NULL) {
            pid = &player->mPID;
            if (pid->isValid() && !player->isFlag0(0xD) && animal->findMemory2(pid)) {
                if (!(*pid == *selfPid)) {
                    f32 chance = 100.0f / (count + 1);
                    if (cM::rndF(100.0f) <= chance) {
                        result = &pid->player;
                    }
                    count++;
                }
            }
        }
    }
    return result;
}

// 8012E444
BOOL dAnimalBlock_c::canStartStyle(dAnimal_c *animal, dPrivateData_c *players, dPrivateData_c *self) {
    if (!animal->mID.isValid()) {
        return FALSE;
    }
    if (self == NULL || !self->mPID.isValid() || self->isFlag0(0xD) || !self->mPID.isFromTown()) {
        return FALSE;
    }
    if ((u32)getStyleAnimalIdx() < ANIMAL_NUM) {
        return FALSE;
    }
    if (!dQuestBase_c::checkEventSchedule(0x13, NULL)) {
        return FALSE;
    }
    if (!dQuestBase_c::checkTodayEvents(0x13)) {
        return FALSE;
    }
    BOOL ret = pickStyleOtherPlayer(animal, players, self) != NULL;
    return ret;
}

// 8012E558
BOOL dAnimalBlock_c::startStyle(const dAnmPersonalID_c *animal, const dPlayerID_c *player0, const dPlayerID_c *player1, dTime_c *limit) {
    if (!animal->isValid()) {
        return FALSE;
    }
    if (!player0->isValid() || !player1->isValid()) {
        return FALSE;
    }
    u32 idx = findAnimalIdx((dAnmPersonalID_c *)animal, mAnimals, ANIMAL_NUM);
    if (idx >= ANIMAL_NUM) {
        return FALSE;
    }
    mStyle.start(idx, player0, player1, limit);
    return TRUE;
}

// 8012E618
// Clothing for the style quest's topic.
dItem::Item dAnimalBlock_c::pickStylePresent(dPrivateData_c *player) {
    dItem::Item result;
    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayer();
    }
    if (player == NULL) {
        return result;
    }

    switch (mStyle.mTopic) {
    case 0: {
        dAnmItemRange_c range(4, 0);
        dAnmItemFilter_c filter(NULL, 0, 1);
        u16 id = player->mEquipment.mShirt.mId;
        if (id != dItem::ITEM_ID_NONE) {
            dItem::Item exclude(id);
            fn_800C60B4(&result, 1, &range, 1, &filter, &exclude, 1, 0);
        } else {
            fn_800C60B4(&result, 1, &range, 1, &filter, NULL, 0, 0);
        }
        break;
    }
    case 1: {
        dAnmItemRange_c range(6, 8);
        dAnmItemFilter_c filter(NULL, 0, 1);
        u16 id = player->mEquipment.mAcc.mId;
        if (id != dItem::ITEM_ID_NONE) {
            dItem::Item exclude(id);
            fn_800C60B4(&result, 1, &range, 1, &filter, &exclude, 1, 0);
        } else {
            fn_800C60B4(&result, 1, &range, 1, &filter, NULL, 0, 0);
        }
        break;
    }
    case 2: {
        dAnmItemRange_c range(3, 0);
        dAnmItemFilter_c filter(NULL, 0, 1);
        fn_800C60B4(&result, 1, &range, 1, &filter, NULL, 0, 0);
        break;
    }
    case 3: {
        dAnmItemRange_c range(1, 0);
        dAnmItemFilter_c filter(NULL, 0, 1);
        fn_800C60B4(&result, 1, &range, 1, &filter, NULL, 0, 0);
        break;
    }
    case 4: {
        dAnmItemRange_c range(2, 0);
        dAnmItemFilter_c filter(NULL, 0, 1);
        fn_800C60B4(&result, 1, &range, 1, &filter, NULL, 0, 0);
        break;
    }
    }
    return result;
}

// 8012E87C
// Sends the style quest letter to one of the two players.
BOOL dAnimalBlock_c::sendStyleLetter(const dPersonalID_c *pid0, const dPersonalID_c *pid1, dAnimal_c *animal, const wchar_t *topic, const dItem::Item *present, u32 which, BOOL flag) {
    static const u8 sPartBase[2] = {4, 1};

    dAnmPersonalID_c *id = &animal->mID;
    if (!id->isValid()) {
        return FALSE;
    }
    if (which >= 2) {
        return FALSE;
    }

    const dPersonalID_c *to;
    if (which == 0) {
        to = pid0;
    } else {
        to = pid1;
    }
    if (!to->isValid()) {
        return FALSE;
    }

    u8 looks = id->getLooks(1);
    if (looks >= LOOKS_TYPE_NUM) {
        return FALSE;
    }

    const char *label = sQ13MailLabels[looks];
    u16 header = fn_800CBA14(label) > 1 ? (u16)(sPartBase[which] + cM::rndInt(3)) : 1;
    u16 body = fn_800CBA40(label) > 1 ? (u16)(sPartBase[which] + cM::rndInt(3)) : 1;
    u16 footer = fn_800CBA6C(label) > 1 ? (u16)(sPartBase[which] + cM::rndInt(3)) : 1;
    dItem::Item paper(fn_800FABF4(looks, dTime_c::getCurrent()->getSeason()));

    fn_800F46CC(id, 0);
    fn_800F4774(present, 1);
    dString::Word_c word(topic);
    fn_800CBCD4(2, &word);
    fn_800F4718(pid0, 3);
    u8 gender0 = pid0->player.mGender;
    fn_800F4794(gender0, id->getLooks(1), 4);
    fn_800F4718(pid1, 5);
    u8 gender1 = pid1->player.mGender;
    fn_800F4794(gender1, id->getLooks(1), 6);

    sMail.clear();
    sMail.setupFromAnimal(&header, &body, &footer, (int)label, (int)label, (int)label, id, to, &paper);
    if (present->mId != dItem::ITEM_ID_NONE) {
        sMail.setPresent(present->mId, 0xFF);
    }
    if (flag && fn_801029C0(&sMail)) {
        return TRUE;
    }
    if (fn_80102BBC(&sMail)) {
        return TRUE;
    }
    return FALSE;
}

// 8012EB58
dPrivateData_c *findPlayerByID(dPrivateData_c *players, const dPlayerID_c *pid) {
    if (!pid->isValid()) {
        return NULL;
    }
    dPlayerID_c *id;
    for (int i = 0; i < 4; i++) {
        dPrivateData_c *player = dPrivateData_c::getRaw(players, i);
        if (player != NULL) {
            id = &player->mPID.player;
            if (id->isValid() && id->isSame(pid)) {
                return player;
            }
        }
    }
    return NULL;
}

// 8012EC00
dPrivateData_c *dAnimalBlock_c::getStylePlayer(dPrivateData_c *players, u32 i) {
    if (i >= 2) {
        return NULL;
    }
    return findPlayerByID(players, mStyle.getPlayer(i));
}

// 8012EC54
BOOL dAnimalBlock_c::sendStyleLetterTo(u32 which, BOOL flag) {
    if (which >= 2) {
        return FALSE;
    }

    dQuestPlayerPair_c *pair;
    dPrivateData_c *players;
    dAnimal_c *animal = getAnimalConst(getStyleAnimalIdx());
    if (animal == NULL || !animal->mID.isValid()) {
        return FALSE;
    }

    pair = &mStyle;
    players = dSaveData_c::getRaw()->mPlayers;
    dPrivateData_c *player0 = getStylePlayer(players, 0);
    if (player0 == NULL) {
        return FALSE;
    }
    dPrivateData_c *player1 = getStylePlayer(players, 1);
    if (player1 == NULL) {
        return FALSE;
    }

    dItem::Item item = pickStylePresent(which == 0 ? player0 : player1);
    if (item.mId == dItem::ITEM_ID_NONE) {
        return FALSE;
    }
    if (!sendStyleLetter(&player0->mPID, &player1->mPID, animal, pair->mTopicText, &item, which, flag)) {
        return FALSE;
    }
    return TRUE;
}

// 8012ED90
void dAnimalBlock_c::checkStylePlayers() {
    dQuestPlayerPair_c *pair = &mStyle;
    dQuestBase_c *base = &pair->mBase;
    if (base->isActive() && (int)base->mKind == 0x13) {
        dPrivateData_c *players = dSaveData_c::getRaw()->mPlayers;
        if (getStylePlayer(players, 0) == NULL || getStylePlayer(players, 1) == NULL) {
            pair->clearInfo();
        }
    }
}

// 8012EE30
BOOL dAnimalBlock_c::updateStyleDaily(BOOL arg) {
    if (!arg) {
        return FALSE;
    }

    dQuestPlayerPair_c *pair = &mStyle;
    dQuestBase_c *base = &pair->mBase;
    if (!base->isActive() || (int)base->mKind != 0x13) {
        return FALSE;
    }
    if (base->isExpired(NULL)) {
        for (int i = 0; i < 2; i++) {
            if ((pair->_5E >> i) & 1) {
                sendStyleLetterTo(i, TRUE);
            }
        }
        pair->clearInfo();
        return TRUE;
    }
    return FALSE;
}

// 8012EF00
BOOL dAnimalBlock_c::canStartAppointment(dPrivateData_c *player, dTime_c *time, int kind) {
    if (player == NULL || !player->mPID.isValid() || player->isFlag0(0xD) || !player->mPID.isFromTown()) {
        return FALSE;
    }
    if (fn_800DCEDC()) {
        return FALSE;
    }
    if (!dQuestBase_c::checkEventSchedule(kind, time)) {
        return FALSE;
    }
    if (!dQuestBase_c::checkTodayEvents(kind)) {
        return FALSE;
    }
    if ((u32)getAppointmentAnimalIdx() < ANIMAL_NUM) {
        return FALSE;
    }
    if ((u32)time->hour < 6 || (u32)time->hour >= 18) {
        return FALSE;
    }
    return TRUE;
}

// 8012F010
BOOL dAnimalBlock_c::startAppointment(u8 kind, const dPlayerID_c *player, const dAnmPersonalID_c *animal, u8 hour, u8 min, dTime_c *limit) {
    if (dQuestBase_c::getKindType(kind) != 2) {
        return FALSE;
    }
    if (!animal->isValid()) {
        return FALSE;
    }
    u32 idx = findAnimalIdx((dAnmPersonalID_c *)animal, mAnimals, ANIMAL_NUM);
    if (idx >= ANIMAL_NUM) {
        return FALSE;
    }
    mAppointment.start(kind, player, idx, hour, min, limit);
    return TRUE;
}

// 8012F0D0
// Copies the items of both layers of the player's main room into a new heap buffer.
dItem::Item *allocRoomItems(u32 *outNum, dPrivateData_c *player, EGG::Heap *heap) {
    u32 dummy = 0;
    if (outNum == NULL) {
        outNum = &dummy;
    }
    *outNum = 0;

    if (player == NULL) {
        return NULL;
    }
    if (heap == NULL) {
        return NULL;
    }

    dHomeList_c *homes = &dSaveData_c::getTown()->mHomes;
    u32 h = homes->findOwner(player);
    if (h >= 4) {
        return NULL;
    }
    dHome_c *home = homes->getHome(h);
    if (home == NULL) {
        return NULL;
    }
    dHomeRoom_c *room = home->getRoom(0);
    if (room == NULL) {
        return NULL;
    }

    dItem::Item *buf = (dItem::Item *)heap->alloc(0x400, 4);
    if (buf == NULL) {
        return buf;
    }
    dItem::Item *p = buf;
    for (int i = 0; i < 0x200; i++, p++) {
        if (p != NULL) {
            p->mId = dItem::ITEM_ID_NONE;
        }
    }

    dHomeLayer_c *layer = room->getLayer(0);
    if (layer == NULL) {
        heap->free(buf);
        return NULL;
    }
    memcpy(buf, layer, 0x200);

    layer = room->getLayer(1);
    if (layer == NULL) {
        heap->free(buf);
        return NULL;
    }
    memcpy(buf + 0x100, layer, 0x200);

    *outNum = 0x200;
    return buf;
}

// 8012F2F8
dItem::Item pickAppointmentPresent(dPrivateData_c *player, u32 kind, BOOL flag) {
    dItem::Item result;
    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayer();
    }
    if (player == NULL) {
        return result;
    }
    if (kind == 0 || kind > 5) {
        return result;
    }

    EGG::Heap *heap = EGG::Heap::getCurrentHeap();
    switch (kind) {
    case 1:
        if (flag) {
            dAnmItemRange_c range(3, 1);
            fn_800C60B4(&result, 1, &range, 1, lbl_8059FF80, NULL, 0, 0);
        } else {
            u32 num = 0;
            dItem::Item *items = allocRoomItems(&num, player, heap);
            if (items != NULL) {
                dAnmItemRange_c range(3, 2);
                dAnmItemFilter_c filter(NULL, 0, 0x11);
                fn_800C60B4(&result, 1, &range, 1, &filter, items, num, 0);
                heap->free(items);
            }
        }
        break;
    case 2:
        if (flag) {
            fn_800F45A4(&result, 1, lbl_8059FF80, NULL, 0);
        } else {
            u32 num = 0;
            dItem::Item *items = allocRoomItems(&num, player, heap);
            if (items != NULL) {
                dAnmItemRange_c range(3, 2);
                dAnmItemFilter_c filter(NULL, 0, 0x11);
                fn_800C60B4(&result, 1, &range, 1, &filter, items, num, 0);
                heap->free(items);
            }
        }
        break;
    case 3:
        if (flag) {
            dAnmItemRange_c range(3, 1);
            dAnmItemFilter_c filter(NULL, 0, 1);
            fn_800C60B4(&result, 1, &range, 1, &filter, NULL, 0, 0);
        } else {
            u32 num = 0;
            dItem::Item *items = allocRoomItems(&num, player, heap);
            if (items != NULL) {
                dAnmItemRange_c range(3, 2);
                dAnmItemFilter_c filter(&player->mCatalog, 0, 0x11);
                fn_800C60B4(&result, 1, &range, 1, &filter, items, num, 0);
                heap->free(items);
            }
        }
        break;
    case 4: {
        static dAnmItemRange_c sRanges[3] = {
            dAnmItemRange_c(3, 1),
            dAnmItemRange_c(1, 1),
            dAnmItemRange_c(2, 1),
        };
        dAnmItemFilter_c filter(NULL, 0, 1);
        fn_800C60B4(&result, 1, sRanges, 3, &filter, NULL, 0, 0);
        break;
    }
    case 5: {
        static dAnmItemRange_c sRanges[3] = {
            dAnmItemRange_c(3, 1),
            dAnmItemRange_c(1, 1),
            dAnmItemRange_c(2, 1),
        };
        dAnmItemFilter_c filter(&player->mCatalog, 0, 1);
        fn_800C60B4(&result, 1, sRanges, 3, &filter, NULL, 0, 0);
        break;
    }
    }
    return result;
}

// 8012F6EC
dPrivateData_c *dAnimalBlock_c::getAppointmentPlayer(dPrivateData_c *players) {
    return findPlayerByID(players, &mAppointment.mPlayer);
}

static inline u16 pickMailPart(u16 num) {
    return num > 1 ? (u16)(cM::rndInt(num - 1) + 1) : 1;
}

// 8012F700
// Sends a villager letter with a present, text parts picked at random.
BOOL sendPresentLetter(const char *label, const dPersonalID_c *to, dAnmPersonalID_c *id,
                            const dItem::Item *present) {
    if (label == NULL) {
        return FALSE;
    }
    if (!id->isValid()) {
        return FALSE;
    }
    if (!to->isValid()) {
        return FALSE;
    }
    if (present->mId == dItem::ITEM_ID_NONE) {
        return FALSE;
    }

    u8 looks = id->getLooks(1);
    if (looks >= LOOKS_TYPE_NUM) {
        return FALSE;
    }

    u16 header = pickMailPart(fn_800CBA14(label));
    u16 body = pickMailPart(fn_800CBA40(label));
    u16 footer = pickMailPart(fn_800CBA6C(label));
    dItem::Item paper(fn_800FABF4(looks, dTime_c::getCurrent()->getSeason()));

    fn_800F46CC(id, 0);
    sMail.clear();
    sMail.setupFromAnimal(&header, &body, &footer, (int)label, (int)label, (int)label, id, to, &paper);
    dItem::Item item = *present;
    sMail.setPresent(item.mId, 0xFF);
    if (fn_801029C0(&sMail)) {
        return TRUE;
    }
    if (fn_80102BBC(&sMail)) {
        return TRUE;
    }
    return FALSE;
}

// 804EEDC8
static const char *sQ8MailLabels[LOOKS_TYPE_NUM] = {
    "MAIL_BO_Q8", "MAIL_HA_Q8", "MAIL_KO_Q8", "MAIL_FU_Q8", "MAIL_GE_Q8", "MAIL_TA_Q8",
};

// 8012F8E8
BOOL sendQ8Letter(const dPersonalID_c *to, dAnmPersonalID_c *id, const dItem::Item *present) {
    if (!id->isValid()) {
        return FALSE;
    }
    if (!to->isValid()) {
        return FALSE;
    }
    if (present->mId == dItem::ITEM_ID_NONE) {
        return FALSE;
    }
    u8 looks = id->getLooks(1);
    if (looks >= LOOKS_TYPE_NUM) {
        return FALSE;
    }
    return sendPresentLetter(sQ8MailLabels[looks], to, id, present);
}

// 8012F9AC
BOOL dAnimalBlock_c::sendAppointment1Letter() {
    dAnimal_c *animal = getAnimalConst(getAppointmentAnimalIdx());
    if (animal == NULL || !animal->mID.isValid()) {
        return FALSE;
    }
    dPrivateData_c *player = getAppointmentPlayer(dSaveData_c::getRaw()->mPlayers);
    if (player == NULL) {
        return FALSE;
    }
    dItem::Item *item = &mAppointment.mItem;
    if (item->mId == dItem::ITEM_ID_NONE) {
        return FALSE;
    }
    return sendQ8Letter(&player->mPID, &animal->mID, item) != FALSE;
}

// 8012FA64
int pickAppointmentPresent2(dItem::Item *out, dPrivateData_c *player, dAnimal_c *animal, BOOL flag) {
    if (out == NULL) {
        return 2;
    }
    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayer();
    }
    if (player == NULL) {
        return 2;
    }
    if (animal == NULL || !animal->mID.isValid()) {
        return 2;
    }

    int result = 2;
    if (flag) {
        dAnmItemFilter_c filter(&player->mCatalog, 0, 0x21);
        fn_800F45A4(out, 1, &filter, NULL, 0);
        if (out->mId == dItem::ITEM_ID_NONE) {
            dAnmItemFilter_c filter2(NULL, 0, 1);
            fn_800F4668(out, 1, &filter2, NULL, 0);
        }
        if (out->mId != dItem::ITEM_ID_NONE) {
            result = 0;
        }
    } else {
        dItem::Item *item = animal->pickNewItem(0, 0, 0);
        if (item != NULL) {
            *out = *item;
            result = 1;
        } else {
            out->setFromIndex(dItem::ITEM_IDX_500_BELLS);
            result = 0;
        }
    }
    return result;
}

// 804EEE28
static const char *sQ9MailLabels[LOOKS_TYPE_NUM] = {
    "MAIL_BO_Q9", "MAIL_HA_Q9", "MAIL_KO_Q9", "MAIL_FU_Q9", "MAIL_GE_Q9", "MAIL_TA_Q9",
};

// 8012FBC8
BOOL sendQ9Letter(const dPersonalID_c *to, dAnmPersonalID_c *id, const dItem::Item *present) {
    if (!id->isValid()) {
        return FALSE;
    }
    if (!to->isValid()) {
        return FALSE;
    }
    if (present->mId == dItem::ITEM_ID_NONE) {
        return FALSE;
    }
    u8 looks = id->getLooks(1);
    if (looks >= LOOKS_TYPE_NUM) {
        return FALSE;
    }
    return sendPresentLetter(sQ9MailLabels[looks], to, id, present);
}

// 8012FC8C
BOOL dAnimalBlock_c::sendAppointment0Letter() {
    dAnimal_c *animal = getAnimalConst(getAppointmentAnimalIdx());
    if (animal == NULL || !animal->mID.isValid()) {
        return FALSE;
    }
    dPrivateData_c *player = getAppointmentPlayer(dSaveData_c::getRaw()->mPlayers);
    if (player == NULL) {
        return FALSE;
    }
    dItem::Item *item = &mAppointment.mItem;
    if (item->mId == dItem::ITEM_ID_NONE) {
        return FALSE;
    }
    return sendQ9Letter(&player->mPID, &animal->mID, item) != FALSE;
}

// 8012FD44
void dAnimalBlock_c::updateAppointmentFlags() {
    dQuestPlayerItem_c *quest = &mAppointment;
    dQuestBase_c *base = &quest->mBase;
    if (!base->isActive()) {
        return;
    }
    if (getAppointmentPlayer(dSaveData_c::getRaw()->mPlayers) == NULL) {
        quest->clear();
        return;
    }

    switch (base->mKind) {
    case 0x12:
        if (base->mState == 0 && (quest->isFlag(0) || quest->isFlag(1))) {
            base->mState = 2;
        }
        break;
    case 0x11:
        if (base->mState == 0 && quest->isFlag(0)) {
            base->mState = 2;
        }
        break;
    }
}

// 8012FE44
BOOL dAnimalBlock_c::updateAppointmentDaily(BOOL arg) {
    if (!arg) {
        return FALSE;
    }

    dQuestPlayerItem_c *quest = &mAppointment;
    dQuestBase_c *base = &quest->mBase;
    if (!base->isActive()) {
        return FALSE;
    }

    if (quest->mItem.mId != dItem::ITEM_ID_NONE) {
        switch (base->mKind) {
        case 0x12:
            if (sendAppointment1Letter()) {
                quest->clear();
                return TRUE;
            }
            break;
        case 0x11:
            if (sendAppointment0Letter()) {
                quest->clear();
                return TRUE;
            }
            break;
        }
    }

    dTime_c *now = dTime_c::getCurrent();
    dTime_c limit = base->getTimeLimit();
    if (!dTime_c::isSameOrAfter(*now, limit)) {
        quest->clear();
        return TRUE;
    }

    dTime_c meet = quest->getMeetTime();
    if (dTime_c::isSameOrAfter(*now, meet)) {
        quest->clear();
        return TRUE;
    }

    if (findEventOnDay(now, 0) && now->year == meet.year && now->month == meet.month && now->mday == meet.mday) {
        quest->clear();
        return TRUE;
    }
    return FALSE;
}

// 80130160
BOOL dAnimalBlock_c::canAskHideAndSeek(u32 idx) {
    dAnimal_c *animal = getAnimalConst(idx);
    if (animal == NULL || !animal->mID.isValid()) {
        return FALSE;
    }
    if (animal->isMoving()) {
        return FALSE;
    }
    if (animal->mPlace != 0) {
        return FALSE;
    }
    return getBusyQuestKind(&animal->mID, 2, 1) == 0x15;
}

// 80130204
BOOL dAnimalBlock_c::canAskHideAndSeekById(const dAnmPersonalID_c *id) {
    return canAskHideAndSeek(findAnimalIdx((dAnmPersonalID_c *)id, mAnimals, ANIMAL_NUM));
}

static inline BOOL isAppointmentAnimal(dAnimalBlock_c *block, int idx) {
    if (block->getAppointmentAnimalIdx() == idx) {
        return TRUE;
    }
    return FALSE;
}

static inline BOOL isQuestActive(dAnimal_c *animal) {
    if (animal->mQuest.mQuest.mBase.isActive()) {
        return TRUE;
    }
    return FALSE;
}

// 80130248
static inline BOOL isAppointmentIdx(dAnimalBlock_c *blk, int idx) {
    int v = blk->getAppointmentAnimalIdx();
    if (v == idx) {
        return TRUE;
    }
    return FALSE;
}

// Whether villager idx can hide in a hide and seek game with the player.
BOOL dAnimalBlock_c::canHide(int idx, dPrivateData_c *player) {
    dPersonalID_c *pid;
    dAnimal_c *animal = getAnimalConst(idx);
    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayerRaw();
    }
    if (player == NULL) {
        return FALSE;
    }

    pid = &player->mPID;
    if (!pid->isValid()) {
        return FALSE;
    }
    if (animal == NULL || !animal->mID.isValid()) {
        return FALSE;
    }
    if (animal->isMoving()) {
        return FALSE;
    }
    if (animal->mPlace != 0) {
        return FALSE;
    }
    if (!animal->findMemory2(pid)) {
        return FALSE;
    }
    BOOL active = animal->mQuest.mQuest.mBase.isActive();
    if (active) {
        return FALSE;
    }
    if (active) {
        return active;
    }
    if (isAppointmentIdx(this, idx)) {
        return FALSE;
    }
    return TRUE;
}

static inline BOOL isNotInList(const int *list, u32 num, int value) {
    BOOL ok = TRUE;
    for (u32 i = 0; i < num; i++, list++) {
        if (value == *list) {
            ok = FALSE;
            break;
        }
    }
    return ok;
}

// 80130378
// Random villager that can hide, not in exclude. -1 if none.
int dAnimalBlock_c::pickHider(const int *exclude, u32 num) {
    u32 count = 0;
    int result = -1;
    for (int i = 0; i < ANIMAL_NUM; i++) {
        if (isNotInList(exclude, num, i) && canHide(i, NULL)) {
            f32 chance = 100.0f / (count + 1);
            if (cM::rndF(100.0f) <= chance) {
                result = i;
            }
            count++;
        }
    }
    return result;
}

// 8013048C
BOOL dAnimalBlock_c::canStartHideAndSeek(dPrivateData_c *player) {
    if (player == NULL || !player->mPID.isValid() || player->isFlag0(0xD) || !player->mPID.isFromTown()) {
        return FALSE;
    }
    if (!dQuestBase_c::checkEventSchedule(0x14, NULL)) {
        return FALSE;
    }
    if (!dQuestBase_c::checkTodayEvents(0x14)) {
        return FALSE;
    }
    BOOL ret = (u32)getHiderIdx(0) >= ANIMAL_NUM;
    return ret;
}

// 8013055C
int dAnimalBlock_c::countHiders(const dAnmPersonalID_c *exclude) {
    int self = getAnimalIdx(exclude);
    int count = 0;
    for (int i = 0; i < ANIMAL_NUM; i++) {
        if (i != self && canHide(i, NULL)) {
            count++;
        }
    }
    return count;
}

// 801305E0
BOOL dAnimalBlock_c::startHideAndSeek(const dPlayerID_c *player, const dAnmPersonalID_c *id, dTime_c *limit) {
    if (!id->isValid()) {
        return FALSE;
    }
    u32 idx = findAnimalIdx((dAnmPersonalID_c *)id, mAnimals, ANIMAL_NUM);
    if (idx >= ANIMAL_NUM) {
        return FALSE;
    }

    dQuestPlayerAnimal_c *quest = &mHideAndSeek;
    int hiders[3] = {-1, -1, -1};
    hiders[0] = idx;
    int *p = &hiders[1];
    for (int i = 1; i < 3; i++, p++) {
        u32 hider = pickHider(hiders, i);
        if (hider >= ANIMAL_NUM) {
            break;
        }
        *p = hider;
    }
    quest->start(hiders[0], hiders[1], hiders[2], player, limit);
    return TRUE;
}

// 801306D0
// Hider slot of the villager, 3 if it is not hiding.
int dAnimalBlock_c::getHiderSlot(const dAnmPersonalID_c *id) {
    if (!id->isValid()) {
        return 3;
    }
    int idx = findAnimalIdx((dAnmPersonalID_c *)id, mAnimals, ANIMAL_NUM);
    if ((u32)idx >= ANIMAL_NUM) {
        return 3;
    }
    for (int i = 0; i < 3; i++) {
        if (idx == getHiderIdx(i)) {
            return i;
        }
    }
    return 3;
}

static inline u16 itemId(const dItem::Item &item) {
    return item.mId;
}

// 80130778
int dAnimalBlock_c::pickHideAndSeekPresent(dItem::Item *out, dAnimal_c *animal) {
    if (out == NULL || animal == NULL || !animal->mID.isValid()) {
        return 2;
    }

    u32 r = cM::rndInt(100);
    int result = 2;
    dItem::Item item;
    if (r < 20) {
        dAnmItemFilter_c filter(NULL, 0, 0x21);
        fn_800F45A4(&item, 1, &filter, NULL, 0);
        result = 0;
    } else if (r < 40) {
        dItem::Item *wish = animal->pickNewItem(0, 0, 0);
        if (wish != NULL) {
            item = *wish;
            result = 1;
        }
    }

    if (itemId(item) == dItem::ITEM_ID_NONE) {
        dAnmItemRange_c range(0xC, 0xA);
        dAnmItemFilter_c filter(NULL, 0, 0x21);
        fn_800C60B4(&item, 1, &range, 1, &filter, NULL, 0, 0);
        if (itemId(item) != dItem::ITEM_ID_NONE) {
            result = 0;
        }
    }
    if (itemId(item) != dItem::ITEM_ID_NONE) {
        out->mId = itemId(item);
    }
    return result;
}

// 801308DC
dPrivateData_c *dAnimalBlock_c::getHideAndSeekPlayer(dPrivateData_c *players) {
    return findPlayerByID(players, &mHideAndSeek.mPlayer);
}

// 804EEE88
static const char *sQ10MailLabels[LOOKS_TYPE_NUM] = {
    "MAIL_BO_Q10", "MAIL_HA_Q10", "MAIL_KO_Q10", "MAIL_FU_Q10", "MAIL_GE_Q10", "MAIL_TA_Q10",
};

// 801308F0
BOOL sendQ10Letter(const dPersonalID_c *to, dAnmPersonalID_c *id, const dItem::Item *present, BOOL flag) {
    if (!id->isValid()) {
        return FALSE;
    }
    if (!to->isValid()) {
        return FALSE;
    }
    if (present->mId == dItem::ITEM_ID_NONE) {
        return FALSE;
    }

    u8 looks = id->getLooks(1);
    if (looks >= LOOKS_TYPE_NUM) {
        return FALSE;
    }

    const char *label = sQ10MailLabels[looks];
    u16 header = pickMailPart(fn_800CBA14(label));
    u16 body = pickMailPart(fn_800CBA40(label));
    u16 footer = pickMailPart(fn_800CBA6C(label));
    dItem::Item paper(fn_800FABF4(looks, dTime_c::getCurrent()->getSeason()));

    fn_800F46CC(id, 0);
    sMail.clear();
    sMail.setupFromAnimal(&header, &body, &footer, (int)label, (int)label, (int)label, id, to, &paper);
    dItem::Item item = *present;
    sMail.setPresent(item.mId, 0xFF);
    if (flag && fn_801029C0(&sMail)) {
        return TRUE;
    }
    if (fn_80102BBC(&sMail)) {
        return TRUE;
    }
    return FALSE;
}

// 80130AE4
BOOL dAnimalBlock_c::sendHideAndSeekLetter(BOOL flag) {
    dQuestPlayerAnimal_c *quest = &mHideAndSeek;
    dAnmPersonalID_c *id = &quest->mAnimal;
    if (!id->isValid()) {
        return FALSE;
    }
    dPrivateData_c *player = getHideAndSeekPlayer(dSaveData_c::getRaw()->mPlayers);
    if (player == NULL) {
        return FALSE;
    }
    dItem::Item *item = &quest->mItem;
    if (item->mId == dItem::ITEM_ID_NONE) {
        return FALSE;
    }
    return sendQ10Letter(&player->mPID, id, item, flag) != FALSE;
}

// Temporary declarations (chunk 8)
// Event id passed by reference to the event checks at 80089F0C.. (other TU).

extern "C" {
// This TU, other chunks.

// Other TUs.
int fn_800BA890(const dItem::Item *item);
void *fn_800F9F64(u32 *num);
u32 fn_80162550();

extern dUnk8074EBE8_c *lbl_8074EBE8;
}

// Event ids (.sdata). Plain ints so they are not constructed by __sinit.
int lbl_8074B060 = EVENT_FLEA_MARKET;
int lbl_8074B064 = EVENT_VALENTINES_DAY;
int lbl_8074B068 = EVENT_VALENTINES_DAY;

static inline BOOL isValidPlayerNo(int i) {
    return i >= 0 && i < PLAYER_NUM;
}

static inline dAnmPersonalID_c *getAnimalID(dAnimal_c *animal) {
    return &animal->mID;
}

static inline int getTmplNpcIdx(const dAnimalTemplate_c *tmpl) {
    return tmpl->mNpcIdx;
}

static inline dPersonalID_c *getPlayerPID(dPrivateData_c *player) {
    return &player->mPID;
}

static inline BOOL isScene3B() {
    return (u8)fn_80162548() == 0x3B;
}

// Flag word at dAnimalMemory_c+0x00 (declared there as u8[4]).
struct dAnimalMemoryFlags8_c {
    u32 _hi : 19;
    u32 mBirthdayLetterSent : 1; // 0x1000
    u32 mBirthdayDone : 1;    // 0x800
    u32 _lo : 11;
};

static inline dAnimalMemoryFlags8_c *memFlags(dAnimalMemory_c *mem) {
    return (dAnimalMemoryFlags8_c *)&mem->mFlags;
}

// 80130BA0
void dAnimalBlock_c::checkHideAndSeek() {
    dQuestPlayerAnimal_c *quest = &mHideAndSeek;
    dQuestBase_c *base = &quest->mBase;
    if (base->isActive()) {
        if (!getHideAndSeekPlayer(dSaveData_c::getRaw()->mPlayers)) {
            quest->clearInfo();
        } else if (base->mState != 4 || quest->mItem.mId == dItem::ITEM_ID_NONE) {
            quest->clearInfo();
        }
    }
}

// 80130C3C
BOOL dAnimalBlock_c::updateHideAndSeekDaily(int delta) {
    if (delta == 0) {
        return FALSE;
    }

    dQuestPlayerAnimal_c *quest = &mHideAndSeek;
    if (delta > 0) {
        quest->fn_80142F90(delta);
    }

    dQuestBase_c *base = &quest->mBase;
    if (!base->isActive()) {
        return FALSE;
    }

    if (base->mState == 4 && quest->mItem.mId != dItem::ITEM_ID_NONE) {
        sendHideAndSeekLetter(1);
    }
    quest->clearInfo();
    return TRUE;
}

// 80130CE4
BOOL dAnimalBlock_c::isHideAndSeekRunning() {
    dQuestPlayerAnimal_c *quest = &mHideAndSeek;
    dQuestBase_c *base = &quest->mBase;
    if (!base->isActive()) {
        return FALSE;
    }
    if (base->mState != 1) {
        return FALSE;
    }
    if (quest->countUnfound() == 0) {
        return FALSE;
    }
    return !quest->isTimeUp(*dTime_c::getCurrent(), TRUE);
}

// 80130D7C
BOOL dAnimalBlock_c::isErrandSenderResident(dPrivateData_c *player) {
    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayer();
    }
    if (player == NULL) {
        return FALSE;
    }

    const dAnmPersonalID_c *sender = player->mErrand.getSender();
    if (sender == NULL || !sender->isValid()) {
        return FALSE;
    }
    return (u32)findAnimalIdx((dAnmPersonalID_c *)sender, mAnimals, ANIMAL_NUM) < ANIMAL_NUM;
}

// 80130E18
BOOL dAnimalBlock_c::hasErrandFromResident(dPrivateData_c *player) {
    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayer();
    }
    if (player == NULL) {
        return FALSE;
    }

    dQuestErrandList_c *errand = &player->mErrand;
    if (!errand->isActive()) {
        return FALSE;
    }

    dAnimal_c *animal = getAnimalConst(0);
    int i;
    for (i = 0; i < ANIMAL_NUM; i++, animal++) {
        dAnmPersonalID_c *id = getAnimalID(animal);
        if (id->isValid() && errand->isSender(id)) {
            return TRUE;
        }
    }
    return FALSE;
}

// 80130EEC
void dAnimalBlock_c::setAppeared(u16 npcIdx) {
    if (npcIdx > 0xD1) {
        return;
    }
    mAppearedFlags[npcIdx >> 3] |= 1 << (npcIdx & 7);
}

// 80130F1C
void dAnimalBlock_c::resetAppeared() {
    memset(mAppearedFlags, 0, sizeof(mAppearedFlags));

    dAnimal_c *animal = getAnimal(0);
    for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
        dAnmPersonalID_c *id = getAnimalID(animal);
        if (id->isValid()) {
            setAppeared(id->mNpcIdx);
        }
    }
}

static inline BOOL isMoveInCertain(u32 num, u8 value) {
    return num < ANIMAL_NUM - 1 || value >= 8;
}

// 80130FB4
BOOL dAnimalBlock_c::shouldMoveIn() {
    u8 value = mMoveDays;
    if (value == 0) {
        return FALSE;
    }

    u32 num = getAnimalNum();
    if (num == ANIMAL_NUM) {
        return FALSE;
    }

    if (isMoveInCertain(num, value)) {
        return TRUE;
    }
    return (u32)cM::rndInt(100) < (u32)(value * 100) / 8;
}

// 8013104C
int dAnimalBlock_c::moveInNewAnimal() {
    int idx = getFreeIdx();
    dAnimal_c *animal = getAnimal(idx);
    if (animal == NULL) {
        return -1;
    }

    u32 listNum = 0;
    void *list = fn_800F9F64(&listNum);
    u32 skip = 0;
    u32 mask = 0;
    u32 total = 0;
    int num;
    const dAnimalTemplate_c *found = NULL;
    const dAnimalTemplate_c *tmpl;

    while (skip != 0x3F && total < 6) {
        mask = 0;
        num = getRarestLooksMask(&mask, getAnimal(0), ANIMAL_NUM, skip, total);
        tmpl = pickTemplate((dAnimalTemplate_c *)list, listNum, mask, num, 6, getAnimal(0), ANIMAL_NUM, mAppearedFlags,
                           sizeof(mAppearedFlags), 0xD1, 0);
        if (tmpl != NULL) {
            found = tmpl;
            break;
        }
        total += num;
        skip |= mask;
    }

    if (found == NULL) {
        resetAppeared();
    }

    mask = 0;
    num = getRarestLooksMask(&mask, getAnimal(0), ANIMAL_NUM, skip, total);
    tmpl = pickTemplate((dAnimalTemplate_c *)list, listNum, mask, num, 6, getAnimal(0), ANIMAL_NUM, mAppearedFlags,
                       sizeof(mAppearedFlags), 0xD1, 0);
    if (tmpl == NULL) {
        return -1;
    }

    dSaveData_c *save = dSaveData_c::getRaw();
    u16 npcIdx = getTmplNpcIdx(tmpl);
    animal->init(npcIdx, 1, &save->mLandID, tmpl);
    setAppeared(npcIdx);
    addToOutdoorQueue(idx);
    animal->mDayPlace = 1;
    animal->setMovingIn();
    animal->mPlace = 1;
    animal->pickBoxedFtr();
    return idx;
}

// 80131268
int dAnimalBlock_c::moveInAnimal(const dAnimal_c *src) {
    if (!src->mID.isValid()) {
        return -1;
    }

    int idx = getFreeIdx();
    dAnimal_c *animal = getAnimal(idx);
    if (animal == NULL) {
        return -1;
    }

    animal->copy(src);
    animal->validateItems();
    setAppeared(animal->mID.mNpcIdx);
    addToOutdoorQueue(idx);
    animal->mDayPlace = 1;
    animal->setMovingIn();
    animal->mPlace = 1;
    dSaveData_c *save = dSaveData_c::getRaw();
    animal->mID.mLand.copy(&save->mLandID);
    animal->pickBoxedFtr();
    animal->mQuest.mWish.setValue(0);
    return idx;
}

// 8013135C
void dAnimalBlock_c::addMoveDays(int delta) {
    u32 value = mMoveDays + delta;
    if (value > 0xFF) {
        value = 0xFF;
    }
    mMoveDays = value;
}

// 80131380
void dAnimalBlock_c::finishMovingIn() {
    dAnimal_c *animal = getAnimal(0);
    for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
        if (animal->mID.isValid() && animal->isMovingIn()) {
            animal->clearMoving();
        }
    }
}

// 801313F4
int dAnimalBlock_c::getMovingOutIdx() {
    dAnimal_c *animal = getAnimalConst(0);
    for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
        if (animal->mID.isValid() && animal->isMovingOut()) {
            return i;
        }
    }
    return -1;
}

// 8013146C
BOOL dAnimalBlock_c::shouldPickMoveOut() {
    if ((u32)mMoveOutIdx < ANIMAL_NUM) {
        return FALSE;
    }

    u8 value = mMoveDays;
    if (value >= 8) {
        return TRUE;
    }
    return (u32)cM::rndInt(100) < (u32)(value * 100) / 8;
}

// 801314E0
void dAnimalBlock_c::pickMoveOutAnimal() {
    dAnimal_c *animal = getAnimal(0);
    int sel = -1;
    u32 num = 0;
    int skip = mMoveInIdx;

    for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
        if (i == skip) {
            continue;
        }
        dAnmPersonalID_c *id = getAnimalID(animal);
        if (id->isValid() && getBusyQuestKind(id, 2, 0) == 0x15) {
            f32 ratio = 100.0f / (num + 1);
            if (cM::rndF(100.0f) <= ratio) {
                sel = i;
            }
            num++;
        }
    }

    mMoveOutIdx = sel;
}

// 801315F4
BOOL dAnimalBlock_c::isMoveOutAnimal(const dAnmPersonalID_c *id) {
    if (!id->isValid()) {
        return FALSE;
    }

    u32 sel = mMoveOutIdx;
    if (sel >= ANIMAL_NUM) {
        return FALSE;
    }
    u32 idx = (u32)findAnimalIdx((dAnmPersonalID_c *)id, getAnimalConst(0), ANIMAL_NUM);
    return idx == sel;
}

// 8013168C
BOOL isAnimalIdx(u32 idx) {
    return idx < ANIMAL_NUM;
}

// 801316A4
void clearNpcIdxList(u16 *buf, u32 num) {
    for (int i = 0; i < num; i++) {
        *buf++ = 0xFFFF;
    }
}

// 80131704
void dAnimalBlock_c::clearMovedOut() {
    clearNpcIdxList(mMovedOutNpcIdx, ANIMAL_NUM);
}

// 80131714
int dAnimalBlock_c::findMovedOut(u16 item) {
    if (item == 0xFFFF) {
        return -1;
    }
    u16 *list = mMovedOutNpcIdx;
    for (int i = 0; i < ANIMAL_NUM; i++) {
        u16 value = list[i];
        if (value == item) {
            return i;
        }
    }
    return -1;
}

// 801317F8
void dAnimalBlock_c::packMovedOut() {
    u16 *dst = mMovedOutNpcIdx;
    u16 tmp[ANIMAL_NUM];
    memcpy(tmp, dst, sizeof(tmp));
    clearNpcIdxList(dst, ANIMAL_NUM);
    for (int i = 0; i < ANIMAL_NUM; i++) {
        if (tmp[i] != 0xFFFF) {
            *dst++ = tmp[i];
        }
    }
}

// 80131904
void dAnimalBlock_c::removeMovedOut(u16 item) {
    int idx = findMovedOut(item);
    if (isAnimalIdx(idx)) {
        mMovedOutNpcIdx[idx] = 0xFFFF;
        packMovedOut();
    }
}

// 80131968
void dAnimalBlock_c::pushMovedOut(u16 item) {
    if (item == 0xFFFF) {
        return;
    }
    removeMovedOut(item);
    for (int i = ANIMAL_NUM - 1; i > 0; i--) {
        mMovedOutNpcIdx[i] = mMovedOutNpcIdx[i - 1];
    }
    mMovedOutNpcIdx[0] = item;
}

static inline BOOL isItemChanged(const dItem::Item &give, const dItem::Item *cur) {
    return give.isSame(*cur) == FALSE;
}

static inline BOOL isHalloweenOngoing(int kind) {
    return kind == EVENT_HALLOWEEN && isEventOngoing(kind);
}

static inline BOOL isOngoingInt(int k) {
    return isEventOngoing(k);
}
static inline BOOL isOverInt(int k) {
    return isEventOver(k);
}
static inline BOOL isActiveInt(int k) {
    return isEventActive(k);
}
static inline BOOL isStarted(int kind) {
    return isOngoingInt(kind) || isEventOver(kind);
}

static inline BOOL isActiveAt22(int kind, const dTime_c *now) {
    return isEventActive(kind) && now->hour == 22;
}

static inline BOOL isActiveAfter2230(int kind, const dTime_c *now) {
    return isActiveAt22(kind, now) && now->min >= 30;
}

static inline BOOL isStartedOrAfter2230(int kind, const dTime_c *now) {
    return isStarted(kind) || isActiveAfter2230(kind, now);
}

static inline BOOL isCountdownTime(int kind, const dTime_c *now) {
    return kind == EVENT_COUNTDOWN && isStartedOrAfter2230(kind, now);
}

static inline BOOL isActiveNotOver(int kind) {
    return isActiveInt(kind) && !isEventOver(kind);
}

static inline BOOL isActiveNotOverAt18(int kind, const dTime_c *now) {
    return isActiveNotOver(kind) && now->hour == 18;
}

static inline BOOL isActiveNotOverAfter1830(int kind, const dTime_c *now) {
    return isActiveNotOverAt18(kind, now) && now->min >= 30;
}

static inline BOOL isOngoingOrAfter1830(int kind, const dTime_c *now) {
    return isEventOngoing(kind) || isActiveNotOverAfter1830(kind, now);
}

static inline BOOL isFireworksTime(int kind, const dTime_c *now) {
    return kind == EVENT_FIREWORKS && isOngoingOrAfter1830(kind, now);
}

static inline BOOL isWantedActive(int kind, int want) {
    return kind == want && isEventActive(kind);
}

static inline BOOL isWantedOngoing(int kind, int want) {
    return kind == want && isEventOngoing(kind);
}

static inline BOOL isInWantedActive(dAnimal_c *animal, int kind, int want) {
    return isWantedActive(kind, want) && animal->mEvent.isInEvent();
}

static inline BOOL isInWantedOngoing(dAnimal_c *animal, int kind, int want) {
    return isWantedOngoing(kind, want) && animal->mEvent.isInEvent();
}

static inline BOOL isRequestDoneActive(dAnimal_c *animal, int kind, int want) {
    return isInWantedActive(animal, kind, want) && animal->isLostItemRequestDone(0, 1);
}

static inline BOOL isRequestDoneOngoing(dAnimal_c *animal, int kind, int want) {
    return isInWantedOngoing(animal, kind, want) && animal->isLostItemRequestDone(0, 1);
}

static inline BOOL isWeatherIn(const dUnk8074EBE8_c *p, int want) {
    return p->_5884 == want;
}

static inline BOOL isItemDiff(const dItem::Item *a, const dItem::Item &b) {
    return a->isSame(b) == FALSE;
}

static inline const dItem::Item *getCurItem(const dAnimal_c *a) {
    return &a->mHeldItem;
}

static inline dQuestPlayerAnimal_c *getQuestAnimal(dAnimalBlock_c *b) {
    return &b->mHideAndSeek;
}

static inline BOOL isRainWeatherIn(const dUnk8074EBE8_c *unk) {
    BOOL special = FALSE;
    if (unk != NULL) {
        BOOL is = FALSE;
        if (isWeatherIn(unk, 4) || isWeatherIn(unk, 3)) {
            is = TRUE;
        }
        if (is) {
            special = TRUE;
        }
    }
    return special;
}

// 80131A10
BOOL dAnimalBlock_c::updateHeldItem(int idx) {
    const dItem::Item *cur;
    BOOL special;
    dQuestPlayerAnimal_c *quest;
    dAnimal_c *animal = getAnimal(idx);
    if (animal == NULL || !animal->mID.isValid()) {
        return FALSE;
    }

    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayerRaw();
    cur = getCurItem(animal);
    dItem::BITM *bitm = cur->mId != dItem::ITEM_ID_NONE ? dItem::infoBank_c::get()->getBITM(*cur) : NULL;
    bool isKind43;
    if (bitm != NULL) {
        isKind43 = bitm->getKind() == 0x43;
    } else {
        isKind43 = false;
    }

    dUnk8074EBE8_c *unk = lbl_8074EBE8;
    u8 min = dTime_c::getCurrent()->min;
    dItem::Item give;
    BOOL set = FALSE;
    BOOL changed = FALSE;
    BOOL flag = FALSE;
    special = isRainWeatherIn(unk);

    dItem::Item fav = animal->getWishTool();
    int kind = mEventId;
    quest = getQuestAnimal(this);
    dTime_c *now = dTime_c::getCurrent();
    dItem::Item item99E(dItem::ITEM_IDX_NET);
    dItem::Item item99F(dItem::ITEM_IDX_FISHING_ROD);

    if (!fn_800DCEDC() && (u8)fn_80162550() == 0x3B &&
        isBirthdayHost(&animal->mID, fn_801017B8(), TRUE, TRUE, TRUE) && special) {
        give = animal->getUmbrella();
        set = TRUE;
        if (isItemChanged(give, cur)) {
            changed = TRUE;
        }
    } else if (isHalloweenOngoing(kind)) {
        give = dItem::ITEM_ID_NONE;
        set = TRUE;
        if (isItemChanged(give, cur)) {
            changed = TRUE;
        }
    } else if (isCountdownTime(kind, now)) {
        give = dItem::ITEM_ID_NONE;
        set = TRUE;
        if (isItemChanged(give, cur)) {
            changed = TRUE;
        }
    } else if (isFireworksTime(kind, now)) {
        give = dItem::ITEM_ID_NONE;
        set = TRUE;
        if (isItemChanged(give, cur)) {
            changed = TRUE;
        }
    } else if (quest->mBase.isActive() && quest->mBase.mState != 4 && quest->countUnfound() &&
               getHiderSlot(&animal->mID) != 3 && !quest->isTimeUp(*now, FALSE)) {
        give = dItem::ITEM_ID_NONE;
        set = TRUE;
        if (isItemChanged(give, cur)) {
            changed = TRUE;
        }
    } else if (special) {
        give = animal->getUmbrella();
        set = TRUE;
        if (isItemChanged(give, cur)) {
            changed = TRUE;
        }
    } else if (!animal->isLostItemRequestDone(0, 1)) {
        give = dItem::ITEM_ID_NONE;
        set = TRUE;
        if (isItemChanged(give, cur)) {
            changed = TRUE;
        }
    } else if (isRequestDoneActive(animal, kind, EVENT_FESTIVALE)) {
        give = dItem::ITEM_ID_NONE;
        set = TRUE;
        if (isItemChanged(give, cur)) {
            changed = TRUE;
        }
    } else if (isRequestDoneOngoing(animal, kind, EVENT_FISHING_TOURNEY)) {
        give = item99F;
        set = TRUE;
        if (isItemChanged(give, cur)) {
            changed = TRUE;
        }
    } else if (isRequestDoneOngoing(animal, kind, EVENT_BUG_OFF)) {
        give = item99E;
        set = TRUE;
        if (isItemChanged(give, cur)) {
            changed = TRUE;
        }
    } else if (player != NULL && player->isFlag0(0xD) && isKind43) {
        give = dItem::ITEM_ID_NONE;
        set = TRUE;
        if (isItemChanged(give, cur)) {
            changed = TRUE;
        }
    } else if (animal->isHeldItemChangeDue(min)) {
        changed = TRUE;
        flag = TRUE;
    } else if (animal->isHeldItemChangeMinute(min) || (cur->mId != dItem::ITEM_ID_NONE && isItemDiff(cur, fav)) ||
               (!special && !animal->wantsParasol() && fn_800BA890(cur) == 10)) {
        changed = TRUE;
        give = (cM::rndInt(4) & 1) ? fav.mId : (u16)dItem::ITEM_ID_NONE;
        set = TRUE;
    }

    BOOL ret = FALSE;
    if (set && isItemDiff(cur, give)) {
        animal->setHeldItem(&give);
        fn_800EFFA0(idx, &give);
        ret = TRUE;
    }

    if (changed) {
        u8 value = calcHeldItemChangeMinute(min, flag);
        animal->mHeldItemChangeMinute = value;
        fn_800F0054(idx, value);
    }

    return ret;
}

// 80132268
BOOL dAnimalBlock_c::updateEvent15Flag(int idx) {
    dAnimalEventState_c *obj;
    dAnimal_c *animal = getAnimal(idx);
    if (animal == NULL || !animal->mID.isValid() || !animal->mEvent.isInEvent() || animal->isMoving()) {
        return FALSE;
    }

    int kind = mEventId;
    if (kind != 0x15) {
        return FALSE;
    }

    obj = &animal->mEvent;
    u32 event = isEventOngoing(kind) != 0;
    u32 cur = obj->isFlag(0);
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player != NULL && player->isFlag0(0xD) && event) {
        event = 0;
    }

    if (event != cur) {
        if (!cur) {
            obj->setFlag(0);
        } else {
            obj->clearFlag(0);
        }
        fn_800F0108(idx, obj->mFlags);
        return TRUE;
    }
    return FALSE;
}

// 801323A4
int dAnimalBlock_c::pickEvent11Animal(BOOL any, const dPersonalID_c *pid, const dLandID_c *land) {
    if (!isEventOngoing(reinterpret_cast<const dEventId_c &>(lbl_8074B060))) {
        return -1;
    }
    int kind = mEventId;
    if (kind != 0x11) {
        return -1;
    }

    if (pid == NULL) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        if (player != NULL) {
            pid = &player->mPID;
        }
    }
    if (pid == NULL || !pid->isValid()) {
        return -1;
    }

    if (land == NULL) {
        land = &dSaveData_c::getRaw()->mLandID;
    }
    if (land == NULL || !land->isValid()) {
        return -1;
    }

    dAnimal_c *animal = getAnimalConst(0);
    int sel = -1;
    u32 found = 0;
    int skip = getLostItemAnimalIdx();
    u32 num = 0;

    for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
        if (i == skip || animal == NULL || !animal->mID.isValid() || !animal->mEvent.isInEvent()) {
            continue;
        }

        dAnimalMemory_c *mem = animal->findMemory2(pid);
        if (mem == NULL) {
            continue;
        }
        if (!(mem->mLand == *land)) {
            continue;
        }

        if (mem->isEventFlag(0)) {
            if (!any && !mem->isEventFlag(1)) {
                sel = i;
                break;
            }
            if (++found >= 5) {
                sel = -1;
                break;
            }
        } else if (animal->mPlace == 0) {
            f32 ratio = 100.0f / (num + 1);
            if (cM::rndF(100.0f) <= ratio) {
                sel = i;
            }
            num++;
        }
    }

    return sel;
}

// 80132614
int dAnimalBlock_c::pickBirthdayHost(dPrivateData_c *player) {
    dPersonalID_c *pid = &player->mPID;
    if (!pid->isValid() || !pid->isFromTown()) {
        return -1;
    }
    if (player->isFlag0(0xD)) {
        return -1;
    }

    dAnimal_c *animal = getAnimalConst(0);
    int sel = -1;
    u32 num = 0;
    s8 best = 30;
    int skip = getSickAnimalIdx();

    for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
        if (!animal->mID.isValid() || animal->isMoving() || i == skip) {
            continue;
        }

        dAnimalMemory_c *mem = animal->findMemory2(pid);
        if (mem == NULL) {
            continue;
        }

        s8 value = mem->getFriendship();
        if (value > best) {
            sel = i;
            best = value;
            num = 1;
        } else if (value == best) {
            f32 ratio = 100.0f / (num + 1);
            if (cM::rndF(100.0f) <= ratio) {
                sel = i;
            }
            num++;
        }
    }

    return sel;
}

// 801327A4
BOOL dAnimalBlock_c::decideBirthdayHost(int playerNo) {
    if (!isValidPlayerNo(playerNo)) {
        return FALSE;
    }

    dPrivateData_c *player = dPlayerMgr_c::getPlayer(playerNo);
    if (player == NULL) {
        return FALSE;
    }
    if (!player->mPID.isValid()) {
        return FALSE;
    }
    if (!isEventActive(playerNo + EVENT_PLAYER_BIRTHDAY_0)) {
        return FALSE;
    }

    s32 year = dTime_c::getCurrent()->year;
    if (year != player->mBirthdayHost._00) {
        s8 sel = pickBirthdayHost(player);
        player->mBirthdayHost._00 = year;
        player->mBirthdayHost._08 = sel;
        return TRUE;
    }
    return FALSE;
}

// 80132890
void dAnimalBlock_c::decideBirthdayHosts() {
    dSaveData_c::getTown();
    for (int i = 0; i < PLAYER_NUM; i++) {
        decideBirthdayHost(i);
    }
}

// 801328E0
void dAnimalBlock_c::clearBirthdayHost(int playerNo) {
    if (isValidPlayerNo(playerNo)) {
        dPrivateData_c *player = dPlayerMgr_c::getPlayer(playerNo);
        if (player != NULL && player->mPID.isValid()) {
            dUnk55FC_c *unk = &player->mBirthdayHost;
            s32 year = dTime_c::getCurrent()->year;
            if (year == unk->_00 && (u32)unk->_08 < ANIMAL_NUM) {
                if (!isEventActive(playerNo + EVENT_PLAYER_BIRTHDAY_0)) {
                    if (year == unk->_04) {
                        unk->_08 = -1;
                    } else {
                        unk->clear();
                    }
                }
            }
        }
    }
}

// 801329BC
void dAnimalBlock_c::clearBirthdayHosts() {
    dSaveData_c::getTown();
    for (int i = 0; i < PLAYER_NUM; i++) {
        clearBirthdayHost(i);
    }
}

// 80132A0C
void dAnimalBlock_c::sendBirthdayLetters() {
    int playerNo = fn_801017B8();
    if (!isValidPlayerNo(playerNo)) {
        return;
    }

    dPrivateData_c *player = dPlayerMgr_c::getPlayer(playerNo);
    if (player == NULL) {
        return;
    }
    dPersonalID_c *pid = &player->mPID;
    if (!pid->isValid()) {
        return;
    }
    if (!isEventOngoing(playerNo + EVENT_PLAYER_BIRTHDAY_0)) {
        return;
    }

    s32 year = dTime_c::getCurrent()->year;
    BOOL inScene = isScene3B();
    if (player->isFlag0(0xD)) {
        player->mBirthdayHost._04 = year;
        return;
    }

    if (year != player->mBirthdayHost._04) {
        dAnimal_c *animal = getAnimal(0);
        int skip = getSickAnimalIdx();
        for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
            if (!animal->mID.isValid()) {
                continue;
            }
            dAnimalMemory_c *mem = animal->findMemory(pid);
            if (mem == NULL) {
                continue;
            }
            memFlags(mem)->mBirthdayLetterSent = 0;
            memFlags(mem)->mBirthdayDone = 0;
            if (mem->getFriendship() < 30 || animal->isMoving() || i == skip || i == player->mBirthdayHost._08) {
                continue;
            }
            dItem::Item none;
            BOOL ok = animal->sendBirthdayLetter(pid, &none, 0) != 0;
            if (ok) {
                memFlags(mem)->mBirthdayLetterSent = 1;
            }
        }
        player->mBirthdayHost._04 = year;
    }

    if (inScene) {
        dAnimal_c *animal = getAnimal(0);
        for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
            if (animal == NULL || !animal->mID.isValid()) {
                continue;
            }
            dAnimalMemory_c *mem = animal->findMemory(pid);
            if (mem == NULL) {
                continue;
            }
            if (memFlags(mem)->mBirthdayLetterSent) {
                memFlags(mem)->mBirthdayDone = 1;
                memFlags(mem)->mBirthdayLetterSent = 0;
            }
        }
    }
}

// 80132C38
BOOL dAnimalBlock_c::hasBirthdayHost() {
    int playerNo = fn_801017B8();
    if (!isValidPlayerNo(playerNo)) {
        return FALSE;
    }

    dPrivateData_c *player = dPlayerMgr_c::getPlayerRaw(playerNo);
    if (player == NULL) {
        return FALSE;
    }
    if (!player->mPID.isValid()) {
        return FALSE;
    }
    if (!isEventOngoing(playerNo + EVENT_PLAYER_BIRTHDAY_0)) {
        return FALSE;
    }
    s32 year = dTime_c::getCurrent()->year;
    if (year != player->mBirthdayHost._00) {
        return FALSE;
    }

    dAnimal_c *animal = getAnimalConst(player->mBirthdayHost._08);
    if (animal == NULL || !animal->mID.isValid()) {
        return FALSE;
    }
    return TRUE;
}

// 80132D44
void dAnimalBlock_c::sendBirthdayHostPresent() {
    int playerNo = fn_801017B8();
    if (isValidPlayerNo(playerNo)) {
        dPrivateData_c *player = dPlayerMgr_c::getPlayer(playerNo);
        if (player != NULL) {
            dPersonalID_c *pid = getPlayerPID(player);
            if (pid->isValid()) {
                if ((u8)fn_80162548() == 0x3B) {
                    if (isEventOngoing(playerNo + EVENT_PLAYER_BIRTHDAY_0)) {
                        s32 year = dTime_c::getCurrent()->year;
                        if (year == player->mBirthdayHost._00) {
                            if (year == player->mBirthdayHost._04) {
                                dAnimal_c *animal = getAnimal(player->mBirthdayHost._08);
                                if (animal != NULL && animal->mID.isValid()) {
                                    if (animal->mDayPlace != 0) {
                                        dItem::Item item(dItem::ITEM_IDX_BIRTHDAY_CAKE);
                                        animal->sendBirthdayLetter(pid, &item, 1);
                                        dAnimalMemory_c *mem = animal->findMemory(pid);
                                        if (mem != NULL) {
                                            memFlags(mem)->mBirthdayDone = 1;
                                            memFlags(mem)->mBirthdayLetterSent = 0;
                                        }
                                        player->mBirthdayHost._08 = -1;
                                    }
                                } else {
                                    player->mBirthdayHost._08 = -1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// 80132EAC
int getBirthdayHostIdxIn(dAnimal_c *animals, u32 num, int playerNo, BOOL checkEvent, BOOL checkFlag,
                           BOOL checkPocket) {
    u32 sel;
    if (!isValidPlayerNo(playerNo)) {
        return -1;
    }

    dPrivateData_c *player = dPlayerMgr_c::getPlayer(playerNo);
    if (player == NULL || !player->mPID.isValid()) {
        return -1;
    }
    if (checkPocket && player->findEmptyPocket(0) == -1) {
        return -1;
    }
    s32 year = dTime_c::getCurrent()->year;
    if (year != player->mBirthdayHost._00) {
        return -1;
    }

    sel = player->mBirthdayHost._08;
    if (sel >= num) {
        return -1;
    }

    int event = playerNo + 10;
    if (!isEventActive(event)) {
        return -1;
    }
    checkEvent = checkEvent && !isEventOngoing(event);
    if (checkEvent) {
        return -1;
    }

    dAnimal_c *animal = fn_8011B92C(animals, num, sel);
    if (animal == NULL || !animal->mID.isValid()) {
        return -1;
    }
    if (checkFlag && animal->mDayPlace != 0) {
        return -1;
    }
    return sel;
}

// 80133044
int dAnimalBlock_c::getBirthdayHostIdx(int playerNo, BOOL checkEvent, BOOL checkFlag, BOOL checkPocket) {
    return getBirthdayHostIdxIn(getAnimalConst(0), ANIMAL_NUM, playerNo, checkEvent, checkFlag, checkPocket);
}

// 801330B0
BOOL isBirthdayHostIn(const dAnmPersonalID_c *id, dAnimal_c *animals, u32 num, int playerNo,
                            BOOL checkEvent, BOOL checkFlag, BOOL checkPocket) {
    if (!id->isValid()) {
        return FALSE;
    }

    u32 idx = (u32)findAnimalIdx((dAnmPersonalID_c *)id, animals, num);
    if (idx >= num) {
        return FALSE;
    }
    return getBirthdayHostIdxIn(animals, num, playerNo, checkEvent, checkFlag, checkPocket) == idx;
}

// 80133158
BOOL dAnimalBlock_c::isBirthdayHost(const dAnmPersonalID_c *id, int playerNo, BOOL checkEvent, BOOL checkFlag, BOOL checkPocket) {
    return isBirthdayHostIn(id, getAnimalConst(0), ANIMAL_NUM, playerNo, checkEvent, checkFlag, checkPocket);
}

// 801331C0
void dAnimalBlock_c::sendValentineLetter(dPrivateData_c *player) {
    dUnk8634_c *last;
    dPersonalID_c *pid;
    int skip;
    u8 gender;
    dAnimal_c *animal;
    if (player == NULL) {
        return;
    }
    pid = getPlayerPID(player);
    if (!pid->isValid()) {
        return;
    }
    if (!isEventOngoing(reinterpret_cast<const dEventId_c &>(lbl_8074B064))) {
        return;
    }

    last = &player->mValentineYear;
    s32 year = dTime_c::getCurrent()->year;
    if (player->isFlag0(0xD)) {
        last->mValue = year;
        return;
    }
    if (year == last->mValue) {
        return;
    }

    animal = getAnimal(0);
    skip = getSickAnimalIdx();
    gender = pid->player.mGender;
    u32 num = 0;
    s8 best = 20;
    int sel = -1;

    for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
        dAnmPersonalID_c *id = getAnimalID(animal);
        if (i == skip || !id->isValid() || gender == (u8)id->getGender(1) || animal->isMoving()) {
            continue;
        }

        dAnimalMemory_c *mem = animal->findMemory(pid);
        if (mem == NULL) {
            continue;
        }

        s8 value = mem->getFriendship();
        if (value == best) {
            num++;
            f32 ratio = 100.0f / num;
            if (cM::rndF(100.0f) <= ratio) {
                sel = i;
            }
        } else if (value > best) {
            sel = i;
            best = value;
            num = 1;
        }
    }

    animal = getAnimal(sel);
    if (animal != NULL && animal->mID.isValid()) {
        static dItem::Item sItem(dItem::ITEM_IDX_CHOCOLATE_HEART);
        dItem::Item item;
        if (best >= 0x40) {
            item = sItem;
        }
        animal->sendValentineLetter(pid, &item, 1);
    }
    last->mValue = year;
}

// 80133418
void dAnimalBlock_c::sendValentineLetters() {
    int playerNo = fn_801017B8();
    if (isValidPlayerNo(playerNo)) {
        dPrivateData_c *player = dPlayerMgr_c::getPlayer(playerNo);
        if (player != NULL && player->mPID.isValid() && isEventOngoing(reinterpret_cast<const dEventId_c &>(lbl_8074B068))) {
            sendValentineLetter(player);
        }
    }
}

// Temporary declarations (chunk 9)

extern "C" {
// This TU, other chunks.

// Other TUs.
extern dUnk8074EBE8_c *lbl_8074EBE8;
void fn_801192C0(dUnk300C_c *dst, const dUnk300C_c *src); // copy
u16 fn_800CBA98(const char *label);
u16 fn_800CBAC4(const char *label);
BOOL fn_80102BBC(dMail_c *mail);

// This chunk, used before their definitions.
}

static int s_801334A8_event = EVENT_NEW_YEARS_DAY; // 8074B06C

// 801334A8
// Sends the New Year letters (once a year, from villagers who like the player).
void dAnimalBlock_c::sendNewYearLetters(dPrivateData_c *player) {
    if (player != NULL) {
        dUnk8634_c *last;
        dPersonalID_c *pid = &player->mPID;
        if (pid->isValid() && dEvent::isOngoing(reinterpret_cast<const dQuestEvent_e &>(s_801334A8_event))) {
            last = &player->mNewYearYear;
            int year = dTime_c::getCurrent()->year;
            if (player->isFlag0(0xD)) {
                last->mValue = year;
            } else if (year != last->mValue) {
                dAnimal_c *animal = getAnimal(0);
                int skip = getSickAnimalIdx();
                for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
                    dAnmPersonalID_c *id = &animal->mID;
                    if (i != skip && id->isValid() && !animal->isMoving()) {
                        dAnimalMemory_c *memory = animal->findMemory(pid);
                        if (memory != NULL && memory->getFriendship() >= 0x40) {
                            animal->sendNewYearLetter(pid, year);
                        }
                    }
                }
                last->mValue = year;
            }
        }
    }
}

static int s_801335CC_event = EVENT_NEW_YEARS_DAY; // 8074B070

static inline BOOL isValidPlayerIdx(int idx) {
    BOOL valid = FALSE;
    if (idx >= 0 && idx < 4) {
        valid = TRUE;
    }
    return valid;
}

// 801335CC
void dAnimalBlock_c::sendNewYearLettersCurrent() {
    int idx = fn_801017B8();
    if (isValidPlayerIdx(idx)) {
        dPrivateData_c *player = dPlayerMgr_c::getPlayer(idx);
        if (player != NULL && player->mPID.isValid() && dEvent::isOngoing(reinterpret_cast<const dQuestEvent_e &>(s_801335CC_event))) {
            sendNewYearLetters(player);
        }
    }
}

// 8013365C
void dAnimalBlock_c::clearSpots() {
    dAnimal_c *animal = getAnimal(0);
    for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
        animal->mSpot.clear();
    }
}

// 801336B0
void dAnimalBlock_c::fn_801336B0() {
    dAnimal_c *animal = getAnimal(0);
    for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
        animal->clearQuestStarted();
    }
}

// 80133704
void dAnimalBlock_c::fn_80133704() {
    dAnimal_c *animal = getAnimal(0);
    for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
        animal->clearQuestStarted();
    }
}

// 80133758
void dAnimalBlock_c::clearMemoryFlag200() {
    dAnimal_c *animal = getAnimal(0);
    for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
        animal->fn_80123A6C();
    }
}

// 801337AC
void dAnimalBlock_c::updateTalkFlags(dPrivateData_c *player) {
    dAnmPersonalID_c *id;
    dAnimal_c *animal = getAnimalConst(0);
    for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
        id = &animal->mID;
        if (id->isValid()) {
            dNpcEntry_c *entry = fn_800F3EE8(id);
            if (entry != NULL) {
                entry->_288.fn_800ED6E0(animal, player);
            }
        }
    }
}

// 80133840
void dAnimalBlock_c::halveTalkFlags(int skip) {
    dAnmPersonalID_c *id;
    dAnimal_c *animal = getAnimalConst(0);
    for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
        if (i != skip) {
            id = &animal->mID;
            if (id->isValid()) {
                dNpcEntry_c *entry = fn_800F3EE8(id);
                if (entry != NULL) {
                    entry->_288.half();
                }
            }
        }
    }
}

// 801338D4
// Number of villagers that have the item (in the given group, or any if 2).
int dAnimalBlock_c::countAnimalsNearItem(const dItem::Item *item, int group) {
    if (!item->isExtId()) {
        return 0;
    }

    dAnimal_c *animal = getAnimalConst(0);
    int count = 0;
    for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
        if (animal->isItemInMyBlock(item)) {
            if (group == 2 || group == animal->mSpot.mGroup) {
                count++;
            }
        }
    }
    return count;
}

// 80133988
int dAnimalBlock_c::countByOutdoorState(int value) {
    if ((u32)value >= 3) {
        return 0;
    }

    dAnimal_c *animal = getAnimalConst(0);
    int count = 0;
    for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
        if (animal->mID.isValid() && value == animal->mDayPlace) {
            count++;
        }
    }
    return count;
}

// 80133A20
void dAnimalBlock_c::updateClothes(BOOL flag) {
    if (flag) {
        dAnimal_c *animal = getAnimal(0);
        int skip = getSickAnimalIdx();
        for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
            if (i != skip && animal->mID.isValid() && !animal->isMoving()) {
                animal->pickDesignToWear();
                animal->pickUmbrella();
            }
        }
    }
}

// 80133AC4
dMovedAnimalList_c::dMovedAnimalList_c() {}

// 80133B0C
void dMovedAnimalList_c::clear() {
    memset(this, 0, sizeof(dMovedAnimalList_c));
    for (int i = 0; i < ANIMAL_NUM; i++) {
        mAnimals[i].clear();
    }
}

// 80133B68
// Key of a villager in the list: 0xE800 | index.
dItem::Item dMovedAnimalList_c::getAnimalKey(const dAnmPersonalID_c *id) {
    u32 idx = findAnimalIdx((dAnmPersonalID_c *)id, mAnimals, ANIMAL_NUM);
    dItem::Item key;
    if (idx < ANIMAL_NUM) {
        key.mId = 0xE000 + ((idx & 0x3FF) | 0x800);
    }
    return key;
}

// 80133BC4
int dMovedAnimalList_c::getAnimalIdx(const dAnmPersonalID_c *id) {
    return findAnimalIdx((dAnmPersonalID_c *)id, mAnimals, ANIMAL_NUM);
}

// 80133BD8
dAnimal_c *dMovedAnimalList_c::getAnimalByKey(const dItem::Item *key) {
    if (isAnimalKeyFlag(&key->mId, 0x800)) {
        return getAnimal(key->mId - 0xE800);
    }
    return NULL;
}

// 80133C14
dAnimal_c *dMovedAnimalList_c::getAnimal(int idx) {
    return fn_8011B910(mAnimals, ANIMAL_NUM, idx);
}

// 80133C20
dAnimal_c *dMovedAnimalList_c::getAnimalConst(int idx) {
    return fn_8011B92C(mAnimals, ANIMAL_NUM, idx);
}

// 80133C2C
u32 dMovedAnimalList_c::getAnimalNum() {
    return countAnimals(mAnimals, ANIMAL_NUM);
}

// 80133C34
// Picks a list villager to move in: one with the town's rarest personality,
// from another town, not already living here. Returns its index or -1.
int dMovedAnimalList_c::pickMoveInIdx(dAnimalBlock_c *block, const dLandID_c *land) {
    if (!land->isValid()) {
        return -1;
    }

    dAnmPersonalID_c *id;
    u32 skip = 0;
    u32 tries = 0;
    u32 mask = 0;
    int num;
    dAnimal_c *town = block->getAnimalConst(0);
    while (skip != 0x3F && tries < LOOKS_TYPE_NUM) {
        num = getRarestLooksMask(&mask, town, ANIMAL_NUM, skip, tries);
        if (mask == 0 || num == 0) {
            break;
        }

        while (mask != 0 && num != 0) {
            u8 looks = pickRandomBit(mask, num, LOOKS_TYPE_NUM);
            if (looks < LOOKS_TYPE_NUM) {
                dAnimal_c *animal = getAnimalConst(0);
                u32 count = 0;
                int found = -1;
                for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
                    id = &animal->mID;
                    if (id->isValid() && looks == id->getLooks(1)) {
                        if (animal->mPrevLand != *land && animal->mJoinType != 3) {
                            u16 npcIdx = id->mNpcIdx;
                            if (!block->findAnimalByNpcIdx(npcIdx) && !isAnimalIdx(block->findMovedOut(npcIdx))) {
                                count++;
                                f32 chance = 100.0f / count;
                                if (cM::rndF(100.0f) < chance) {
                                    found = i;
                                }
                            }
                        }
                    }
                }

                if (found != -1) {
                    return found;
                }

                u32 bit = 1 << looks;
                skip |= bit;
                tries++;
                num--;
                mask &= ~bit;
            } else {
                return -1;
            }
        }
    }
    return -1;
}

// 80133E98
// Like pickMoveInIdx, but only with a chance of a third per villager in the list.
int dMovedAnimalList_c::tryPickMoveInIdx(dAnimalBlock_c *block, const dLandID_c *land) {
    if (!land->isValid()) {
        return -1;
    }

    u32 num = getAnimalNum();
    BOOL go;
    if (num >= 3) {
        go = TRUE;
    } else {
        go = (u32)cM::rndInt(100) < num * 100 / 3;
    }

    if (!go) {
        return -1;
    }
    return pickMoveInIdx(block, land);
}

// 80133F60
// Slot of the list to store the animal in: a free one, or the one to replace.
u32 dMovedAnimalList_c::getReplaceIdx(dAnimal_c *animal) {
    if (!animal->mID.isValid()) {
        return -1;
    }

    u32 idx = findFreeAnimalIdx(mAnimals, ANIMAL_NUM, TRUE);
    if (idx < ANIMAL_NUM) {
        return idx;
    }

    dAnimal_c *cur = getAnimalConst(0);
    u32 count = 0;
    dAnimal_c *best = NULL;
    dTime_c *now = dTime_c::getCurrent();
    if (animal->mID.mNpcIdx <= 0xD1 || animal->getMemoryNum()) {
        best = animal;
        count = 1;
        idx = -1;
    }

    for (int i = 0; i < ANIMAL_NUM; i++, cur++) {
        u32 a = cur->getMemoryNum();
        if ((!cur->mID.isValid() || cur->mID.mNpcIdx > 0xD1) && a == 0) {
            continue;
        }

        u32 b = best != NULL ? best->getMemoryNum() : 0x11;
        if (a < b) {
            best = cur;
            idx = i;
            count = 1;
            continue;
        }
        if (a != b) {
            continue;
        }

        a = cur->hasAnyLetter();
        b = best->hasAnyLetter();
        if (a == 0 && b != 0) {
            best = cur;
            idx = i;
            count = 1;
            continue;
        }
        if (a != b) {
            continue;
        }

        s8 c = cur->getMaxFriendship();
        s8 d = best->getMaxFriendship();
        if (c < d) {
            best = cur;
            idx = i;
            count = 1;
            continue;
        }
        if (c != d) {
            continue;
        }

        int e = cur->getDaysSinceLastTalk(now);
        int f = best->getDaysSinceLastTalk(now);
        if (f >= 0 && e > f) {
            best = cur;
            idx = i;
            count = 1;
            continue;
        }
        if (e != f) {
            continue;
        }

        if (best->mID == animal->mID) {
            best = cur;
            idx = i;
            count = 1;
            continue;
        }

        f32 chance = 100.0f / (count + 1);
        if (cM::rndF(100.0f) <= chance) {
            best = cur;
            idx = i;
        }
        count++;
    }

    if (best == NULL) {
        dAnimal_c *cur = getAnimalConst(0);
        u32 n = 1;
        idx = -1;
        for (int i = 0; i < ANIMAL_NUM; i++, cur++) {
            if (cur->mID.isValid()) {
                f32 chance = 100.0f / (n + 1);
                if (cM::rndF(100.0f) <= chance) {
                    idx = i;
                }
                n++;
            }
        }
    }
    return idx;
}

// 801342E0
// Stores a copy of the villager in the list. Returns its slot, or -1.
u32 dMovedAnimalList_c::addAnimal(dAnimal_c *animal, const dLandID_c *land, const dUnk300C_c *arg) {
    dAnmPersonalID_c *id = &animal->mID;
    if (!id->isValid()) {
        return -1;
    }

    u16 npcIdx = id->mNpcIdx;
    if (fn_8011BA8C(npcIdx, getAnimal(0), ANIMAL_NUM)) {
        return -1;
    }

    u32 idx = findFreeAnimalIdx(getAnimal(0), ANIMAL_NUM, TRUE);
    dAnimal_c *dst = getAnimal(idx);
    if (dst == NULL) {
        idx = getReplaceIdx(animal);
        dst = getAnimal(idx);
    }

    if (dst != NULL) {
        dst->copy(animal);
        if (land != NULL && land->isValid()) {
            dst->mPrevLand.copy(land);
        }
        if (arg != NULL) {
            fn_801192C0(&dst->_300C, arg);
        }
        dItem::Item none;
        dst->setHeldItem(&none);
        dst->validateUmbrella();
        dst->clearMemoryFlag24();
        dst->clearEventFlags();
    }
    return idx;
}

// 8013443C
// A random list villager, preferring those from the given town.
u32 dMovedAnimalList_c::pickGiveAwayIdx(const dLandID_c *land) {
    if (!land->isValid()) {
        return -1;
    }

    dAnimal_c *animal = getAnimalConst(0);
    u32 sameIdx = -1;
    u32 sameNum = 0;
    u32 otherIdx = -1;
    u32 otherNum = 0;
    for (int i = 0; i < ANIMAL_NUM; i++, animal++) {
        if (animal->mID.isValid() && animal->mJoinType != 3) {
            if (animal->mPrevLand == *land) {
                f32 chance = 100.0f / (sameNum + 1);
                if (cM::rndF(100.0f) <= chance) {
                    sameIdx = i;
                }
                sameNum++;
            } else {
                f32 chance = 100.0f / (otherNum + 1);
                if (cM::rndF(100.0f) <= chance) {
                    otherIdx = i;
                }
                otherNum++;
            }
        }
    }

    if (sameIdx < ANIMAL_NUM) {
        otherIdx = sameIdx;
    }
    return otherIdx;
}

// 801345D8
dAnimalSave_c::dAnimalSave_c() {}

// 80134614
void dAnimalSave_c::updateChecksum() {
    mChecksum = calcChecksum();
}

static inline BOOL checkCrc(dAnimalSave_c *save) {
    u32 checksum = save->calcChecksum();
    if (checksum == save->mChecksum) {
        return TRUE;
    }
    return FALSE;
}

// 80134648
BOOL dAnimalSave_c::isChecksumValid() {
    if (checkCrc(this)) {
        return TRUE;
    }
    return FALSE;
}

// 80134690
u32 dAnimalSave_c::calcChecksum() const {
    const u8 *base = (const u8 *)this;
    return sCrc::calcCRC32((const u8 *)&mChecksum + sizeof(mChecksum),
                           sizeof(dAnimalSave_c) - sizeof(mChecksum) - ((const u8 *)&mChecksum - base),
                           -1, -1);
}

// 801346B8
void dAnimalSave_c::clear() {
    memset(this, 0, sizeof(dAnimalSave_c));
    mTown.clear();
    mMoved.clear();
}

// 80134704
dItem::Item dAnimalSave_c::getAnimalKey(const dAnmPersonalID_c *animal) {
    dItem::Item key;
    if (animal->isValid()) {
        key = mTown.getAnimalKey(animal);
        if (!isAnimalKey(&key.mId)) {
            key = mMoved.getAnimalKey(animal);
        }
    }
    return key;
}

// 801347FC
dAnimal_c *dAnimalSave_c::getAnimalByKey(const dItem::Item *key) {
    if (isAnimalKey(&key->mId)) {
        return mTown.getAnimalByKey((dItem::Item *)key);
    }
    if (isAnimalKeyFlag(&key->mId, 0x800)) {
        return mMoved.getAnimalByKey(key);
    }
    return NULL;
}

// 801348A0
// Daily move-out / move-in step. Returns TRUE if something happened.
BOOL dAnimalSave_c::updateMoves(int days) {
    if (days <= 0) {
        return FALSE;
    }

    dMovedAnimalList_c *list = &mMoved;
    dLandID_c *land = &dSaveData_c::getRaw()->mLandID;
    dUnk300C_c *arg = &dSaveData_c::getRaw()->_07359E;
    dAnimalBlock_c *block = &mTown;
    block->finishMovingIn();
    block->addMoveDays(days);
    int idx = block->getMovingOutIdx();
    dAnimal_c *animal = block->getAnimal(idx);
    if (animal != NULL) {
        if (block->mMoveDays >= 2) {
            animal->clearMemoryFlag23();
            animal->fn_801235F8();
            list->addAnimal(animal, land, arg);
            block->pushMovedOut(animal->mID.mNpcIdx);
            animal->sendMoveLetters();
            animal->clear();
            block->removeHouse(idx);
            block->mMoveDays = 0;
            block->mMoveInIdx = -1;
            return TRUE;
        }
        return FALSE;
    }

    if (block->getAnimalNum() < ANIMAL_NUM) {
        if (block->shouldMoveIn()) {
            dAnimal_c *newcomer = list->getAnimal(list->tryPickMoveInIdx(block, land));
            if (newcomer != NULL && newcomer->mID.isValid()) {
                int slot = block->moveInAnimal(newcomer);
                if (slot != -1) {
                    block->mMoveInIdx = slot;
                    newcomer->clear();
                    block->mMoveDays = 0;
                    return TRUE;
                }
            } else {
                u32 slot = block->moveInNewAnimal();
                if (slot < ANIMAL_NUM) {
                    block->mMoveInIdx = slot;
                    block->mMoveDays = 0;
                    return TRUE;
                }
            }
        }
        return FALSE;
    }

    dAnimal_c *mover = block->getAnimal(block->mMoveOutIdx);
    if (mover != NULL) {
        if (block->mMoveDays >= 7) {
            if (mover->mID.isValid()) {
                mover->setMovingOut();
                mover->mDayPlace = 1;
                mover->mPlace = 1;
                mover->pickBoxedFtr();
            }
            block->mMoveOutIdx = -1;
            block->mMoveDays = 0;
            return TRUE;
        }
        return FALSE;
    }

    if (block->shouldPickMoveOut()) {
        block->pickMoveOutAnimal();
        block->mMoveDays = 0;
        return TRUE;
    }
    return FALSE;
}

// 80134B28
BOOL dAnimalSave_c::addMovedAnimal(dAnimal_c *animal) {
    dAnmPersonalID_c *id = &animal->mID;
    if (!id->isValid()) {
        return FALSE;
    }
    if (mTown.findAnimalByNpcIdx(id->mNpcIdx)) {
        return FALSE;
    }
    return mMoved.addAnimal(animal, NULL, NULL) < ANIMAL_NUM;
}

// 80134BC8
dAnimal_c *dAnimalSave_c::pickMovedAnimal() {
    dMovedAnimalList_c *list = &mMoved;
    dSaveData_c *data = dSaveData_c::getRaw();
    dAnimal_c *animal = list->getAnimal(list->pickGiveAwayIdx(&data->mLandID));
    if (animal == NULL || !animal->mID.isValid()) {
        return NULL;
    }
    return animal;
}

// 80134C40
BOOL dAnimalSave_c::takeMovedAnimal(dAnimal_c *out, BOOL remove) {
    if (out == NULL) {
        return FALSE;
    }

    dAnimal_c *animal = pickMovedAnimal();
    if (animal == NULL || !animal->mID.isValid()) {
        return FALSE;
    }

    out->copy(animal);
    if (remove) {
        animal->clear();
    }
    return TRUE;
}

// 80134CD4
// Index of a random set bit among the first max bits of mask (num of them set).
u32 pickRandomBit(u32 mask, int num, u32 max) {
    if (num != 0) {
        int n = cM::rndInt(num);
        for (u32 i = 0; i < max; i++) {
            if ((mask >> i) & 1) {
                if (n == 0) {
                    return i;
                }
                n--;
            }
        }
    }
    return -1;
}

// Letter conditions, one per entry of sImpressionTable.

// 80134D54
BOOL fn_80134D54(const dPrivateData_c *player) {
    if (player->isFlag0(2) && player->_83F5 == 0) {
        return TRUE;
    }
    return FALSE;
}

// 80134DA8
BOOL fn_80134DA8(const dPrivateData_c *player) {
    return dEvent::getActiveOfKind(EVENT_KIND_TOWN) < EVENT_NUM;
}

// 80134DE0
BOOL isHoldingNet(const dPrivateData_c *player) {
    dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(player->mEquipment.mHeld);
    if (bitm != NULL && bitm->isNet()) {
        return TRUE;
    }
    return FALSE;
}

// 80134E30
BOOL isHoldingFishingrod(const dPrivateData_c *player) {
    dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(player->mEquipment.mHeld);
    if (bitm != NULL && bitm->isFishingrod()) {
        return TRUE;
    }
    return FALSE;
}

// 80134E80
BOOL isHoldingWatering(const dPrivateData_c *player) {
    dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(player->mEquipment.mHeld);
    if (bitm != NULL && bitm->isWatering()) {
        return TRUE;
    }
    return FALSE;
}

// 80134ED0
BOOL isHoldingAxe(const dPrivateData_c *player) {
    dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(player->mEquipment.mHeld);
    if (bitm != NULL && bitm->isAxe()) {
        return TRUE;
    }
    return FALSE;
}

// 80134F20
BOOL isWearingKingOutfit(const dPrivateData_c *player) {
    dItem::Item hat(dItem::ITEM_IDX_ROYAL_CROWN);
    BOOL diff = !player->mEquipment.mHat.isSame(hat);
    if (diff) {
        return FALSE;
    }
    dItem::Item acc(dItem::ITEM_IDX_KINGS_BEARD);
    diff = !player->mEquipment.mAcc.isSame(acc);
    if (diff) {
        return FALSE;
    }
    return TRUE;
}

// 80134FA4
BOOL isWearingRoyalCrown(const dPrivateData_c *player) {
    dItem::Item hat(dItem::ITEM_IDX_ROYAL_CROWN);
    BOOL diff = !player->mEquipment.mHat.isSame(hat);
    if (diff) {
        return FALSE;
    }
    dItem::Item acc(dItem::ITEM_IDX_KINGS_BEARD);
    if (player->mEquipment.mAcc.isSame(acc)) {
        return FALSE;
    }
    return TRUE;
}

// 80135024
BOOL isWearingCrown(const dPrivateData_c *player) {
    dItem::Item hat(dItem::ITEM_IDX_CROWN);
    if (player->mEquipment.mHat.isSame(hat)) {
        return TRUE;
    }
    return FALSE;
}

// 80135078
BOOL isWearingChefsHat(const dPrivateData_c *player) {
    dItem::Item hat(dItem::ITEM_IDX_CHEFS_HAT);
    if (player->mEquipment.mHat.isSame(hat)) {
        return TRUE;
    }
    return FALSE;
}

// 801350CC
BOOL isWearingBridalVeil(const dPrivateData_c *player) {
    dItem::Item hat(dItem::ITEM_IDX_BRIDAL_VEIL);
    if (player->mEquipment.mHat.isSame(hat)) {
        return TRUE;
    }
    return FALSE;
}

// 80135120
BOOL isWearingSwimCapGoggles(const dPrivateData_c *player) {
    dItem::Item hat(dItem::ITEM_IDX_SWIMMING_CAP);
    BOOL diff = !player->mEquipment.mHat.isSame(hat);
    if (diff) {
        return FALSE;
    }
    dItem::Item acc(dItem::ITEM_IDX_GOGGLES);
    diff = !player->mEquipment.mAcc.isSame(acc);
    if (diff) {
        return FALSE;
    }
    return TRUE;
}

// 801351A4
BOOL isWearingOutbackHat(const dPrivateData_c *player) {
    dItem::Item hat(dItem::ITEM_IDX_OUTBACK_HAT);
    if (player->mEquipment.mHat.isSame(hat)) {
        return TRUE;
    }
    return FALSE;
}

// 801351F8
BOOL isWearingHalo(const dPrivateData_c *player) {
    dItem::Item hat(dItem::ITEM_IDX_HALO);
    if (player->mEquipment.mHat.isSame(hat)) {
        return TRUE;
    }
    return FALSE;
}

// 8013524C
BOOL isWearingJesterCap(const dPrivateData_c *player) {
    dItem::Item hat(dItem::ITEM_IDX_JESTERS_CAP);
    if (player->mEquipment.mHat.isSame(hat)) {
        return TRUE;
    }
    return FALSE;
}

// 801352A0
BOOL isWearingWitchHat(const dPrivateData_c *player) {
    dItem::Item hat(dItem::ITEM_IDX_WITCHS_HAT);
    if (player->mEquipment.mHat.isSame(hat)) {
        return TRUE;
    }
    return FALSE;
}

// 801352F4
BOOL isWearingBabyOutfit(const dPrivateData_c *player) {
    dItem::Item hat(dItem::ITEM_IDX_BABYS_HAT);
    BOOL diff = !player->mEquipment.mHat.isSame(hat);
    if (diff) {
        return FALSE;
    }
    dItem::Item acc(dItem::ITEM_IDX_PACIFIER);
    diff = !player->mEquipment.mAcc.isSame(acc);
    if (diff) {
        return FALSE;
    }
    return TRUE;
}

// 80135378
BOOL isWearingAfroWig(const dPrivateData_c *player) {
    dItem::Item hat(dItem::ITEM_IDX_AFRO_WIG);
    if (player->mEquipment.mHat.isSame(hat)) {
        return TRUE;
    }
    return FALSE;
}

// 801353CC
BOOL isWearingBunnyHood(const dPrivateData_c *player) {
    dItem::Item hat(dItem::ITEM_IDX_BUNNY_HOOD);
    if (player->mEquipment.mHat.isSame(hat)) {
        return TRUE;
    }
    return FALSE;
}

// 80135420
BOOL isWearingGeishaWig(const dPrivateData_c *player) {
    dItem::Item hat(dItem::ITEM_IDX_GEISHA_WIG);
    if (player->mEquipment.mHat.isSame(hat)) {
        return TRUE;
    }
    return FALSE;
}

// 80135474
BOOL isWearingSamuraiWig(const dPrivateData_c *player) {
    dItem::Item hat(dItem::ITEM_IDX_SAMURAI_WIG);
    if (player->mEquipment.mHat.isSame(hat)) {
        return TRUE;
    }
    return FALSE;
}

// 801354C8
BOOL isWearingKingTutMask(const dPrivateData_c *player) {
    dItem::Item hat(dItem::ITEM_IDX_KING_TUT_MASK);
    if (player->mEquipment.mHat.isSame(hat)) {
        return TRUE;
    }
    return FALSE;
}

// 8013551C
BOOL isWearingNinjaHood(const dPrivateData_c *player) {
    dItem::Item hat(dItem::ITEM_IDX_NINJA_HOOD);
    if (player->mEquipment.mHat.isSame(hat)) {
        return TRUE;
    }
    return FALSE;
}

// 80135570
BOOL isWearingWrestlingMask(const dPrivateData_c *player) {
    dItem::Item hat(dItem::ITEM_IDX_WRESTLING_MASK);
    if (player->mEquipment.mHat.isSame(hat)) {
        return TRUE;
    }
    return FALSE;
}

// 801355C4
BOOL isWearingDressing(const dPrivateData_c *player) {
    dItem::Item hat(dItem::ITEM_IDX_DRESSING);
    if (player->mEquipment.mHat.isSame(hat)) {
        return TRUE;
    }
    return FALSE;
}

// 80135618
BOOL isWearingMohawkWig(const dPrivateData_c *player) {
    dItem::Item hat(dItem::ITEM_IDX_MOHAWK_WIG);
    if (player->mEquipment.mHat.isSame(hat)) {
        return TRUE;
    }
    return FALSE;
}

// 8013566C
BOOL isWearingRegentWig(const dPrivateData_c *player) {
    dItem::Item hat(dItem::ITEM_IDX_REGENT_WIG);
    if (player->mEquipment.mHat.isSame(hat)) {
        return TRUE;
    }
    return FALSE;
}

static inline BOOL isRainWeather() {
    if (lbl_8074EBE8 != NULL) {
        BOOL result = FALSE;
        if (isWeatherIn(lbl_8074EBE8, 4) || isWeatherIn(lbl_8074EBE8, 3)) {
            result = TRUE;
        }
        return result;
    }
    return FALSE;
}

static inline BOOL isSnowWeather() {
    if (lbl_8074EBE8 != NULL) {
        BOOL result = FALSE;
        if (isWeatherIn(lbl_8074EBE8, 6) || isWeatherIn(lbl_8074EBE8, 5)) {
            result = TRUE;
        }
        return result;
    }
    return FALSE;
}

// 801356C0
BOOL isRaining(const dPrivateData_c *player) {
    if (isRainWeather()) {
        return TRUE;
    }
    return FALSE;
}

// 80135708
BOOL isSnowing(const dPrivateData_c *player) {
    if (isSnowWeather()) {
        return TRUE;
    }
    return FALSE;
}

// 80135750
BOOL isMorning(const dPrivateData_c *player) {
    int hour = dTime_c::getCurrent()->hour;
    if (hour >= 5 && hour < 10) {
        return TRUE;
    }
    return FALSE;
}

// 80135790
BOOL isLateNight(const dPrivateData_c *player) {
    int hour = dTime_c::getCurrent()->hour;
    if (hour >= 0 && hour < 5) {
        return TRUE;
    }
    return FALSE;
}

// 801357D0
BOOL isInsectCatalogComplete(const dPrivateData_c *player) {
    return const_cast<dCatalog_c &>(player->mCatalog).isComplete(7);
}

// 801357E0
BOOL isFishCatalogComplete(const dPrivateData_c *player) {
    return const_cast<dCatalog_c &>(player->mCatalog).isComplete(8);
}

// 801357F0
BOOL isFullyTanned(const dPrivateData_c *player) {
    return (player->mTan >> 1) == 7;
}

// 8013580C
BOOL hasNoPocketMoney(const dPrivateData_c *player) {
    return player->getPocketMoney() == 0;
}

// 80135834
BOOL hasSavings100k(const dPrivateData_c *player) {
    return player->mSavings >= 100000;
}

// 80135854
BOOL isNever(const dPrivateData_c *player) {
    return FALSE;
}

// 8013585C
BOOL fn_8013585C(const dPrivateData_c *player) {
    dItem::Item acc = player->mEquipment.mAcc;
    if (acc.mId == dItem::ITEM_ID_NONE) {
        return FALSE;
    }
    dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(acc);
    if (!bitm->_187_3) {
        return FALSE;
    }
    if (player->_83F5 != 0) {
        return FALSE;
    }
    if (player->mEquipment.mHat.mId == dItem::ITEM_ID_NONE &&
        (player->mHair == 3 || player->mHair == 5 || player->mHair == 8)) {
        return TRUE;
    }
    return FALSE;
}

// 80135908
BOOL fn_80135908(const dPrivateData_c *player) {
    if (player->_83F5 != 0) {
        return FALSE;
    }
    if (player->mEquipment.mHat.mId == dItem::ITEM_ID_NONE && player->mHair == 9) {
        return TRUE;
    }
    return FALSE;
}

// 80135948
BOOL fn_80135948(const dPrivateData_c *player) {
    if (player->_83F5 != 0) {
        return FALSE;
    }
    if (player->mEquipment.mHat.mId == dItem::ITEM_ID_NONE && player->mEquipment.mAcc.mId == dItem::ITEM_ID_NONE &&
        player->mHair == 6) {
        return TRUE;
    }
    return FALSE;
}

// 80135994
BOOL hasFemaleHairNoHat(const dPrivateData_c *player) {
    if (player->_83F5 != 0) {
        return FALSE;
    }
    if (player->mEquipment.mHat.mId == dItem::ITEM_ID_NONE && player->mHair >= 0xD && player->mHair < 0x1A) {
        return TRUE;
    }
    return FALSE;
}

// 801359DC
BOOL fn_801359DC(const dPrivateData_c *player) {
    if (player->_83F5 != 0) {
        return FALSE;
    }
    if (player->mEquipment.mHat.mId == dItem::ITEM_ID_NONE) {
        switch (player->mHair) {
        case 0x11:
        case 0x13:
        case 0x14:
        case 0x16:
            return TRUE;
        }
    }
    return FALSE;
}

// 80135A40
BOOL fn_80135A40(const dPrivateData_c *player) {
    dItem::Item acc = player->mEquipment.mAcc;
    if (acc.mId == dItem::ITEM_ID_NONE) {
        return FALSE;
    }
    return dItem::infoBank_c::get()->getBITM(acc)->_187_3;
}

// 80135A88
BOOL hasMaleHairNoHat(const dPrivateData_c *player) {
    if (player->_83F5 != 0) {
        return FALSE;
    }
    if (player->mEquipment.mHat.mId == dItem::ITEM_ID_NONE && player->mHair < 0xD) {
        return TRUE;
    }
    return FALSE;
}

// Letter conditions take the player as const (fn_8013585C only matches that way).
struct dImpressionCond_c {
    u8 mGender; // 2 = any
    BOOL (*mFunc)(const dPrivateData_c *player);
};

#define IMPRESSION_NUM 50

// 80475F48
static const dImpressionCond_c sImpressionTable[IMPRESSION_NUM] = {
    {2, fn_80134D54}, {0, fn_80134DA8}, {1, fn_80134DA8}, {2, isHoldingNet}, {2, isHoldingFishingrod},
    {2, isHoldingWatering}, {2, isHoldingAxe}, {0, isWearingKingOutfit}, {0, isWearingRoyalCrown}, {1, isWearingCrown},
    {2, isWearingChefsHat}, {1, isWearingBridalVeil}, {0, isWearingSwimCapGoggles}, {2, isWearingOutbackHat}, {2, isWearingHalo},
    {2, isWearingJesterCap}, {1, isWearingWitchHat}, {2, isWearingBabyOutfit}, {2, isWearingAfroWig}, {0, isWearingBunnyHood},
    {1, isWearingBunnyHood}, {1, isWearingGeishaWig}, {0, isWearingSamuraiWig}, {2, isWearingKingTutMask}, {0, isWearingNinjaHood},
    {1, isWearingNinjaHood}, {2, isWearingWrestlingMask}, {2, isWearingDressing}, {0, isWearingMohawkWig}, {0, isWearingRegentWig},
    {0, isRaining}, {1, isRaining}, {0, isSnowing}, {1, isSnowing}, {2, isMorning},
    {2, isLateNight}, {2, isInsectCatalogComplete}, {2, isFishCatalogComplete}, {2, isFullyTanned}, {2, hasNoPocketMoney},
    {2, hasSavings100k}, {2, isNever}, {0, fn_8013585C}, {0, fn_80135908}, {0, fn_80135948},
    {0, hasFemaleHairNoHat}, {1, fn_801359DC}, {1, fn_80135A40}, {1, hasMaleHairNoHat}, {2, NULL},
};

// 80135AC8
// First letter condition the player meets, or IMPRESSION_NUM.
int getImpression(dPrivateData_c *player) {
    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayer();
    }
    if (player == NULL) {
        return IMPRESSION_NUM;
    }

    u8 gender = player->mPID.player.mGender;
    const dImpressionCond_c *cond = sImpressionTable;
    for (int i = 0; i < IMPRESSION_NUM; i++, cond++) {
        if (cond->mGender == 2 || cond->mGender == gender) {
            if (cond->mFunc == NULL || cond->mFunc(player)) {
                return i;
            }
        }
    }
    return IMPRESSION_NUM;
}

// 804EEF00
static const char *sVisitorReplyLabels[LOOKS_TYPE_NUM] = {
    "MAIL_BO_ReplyV", "MAIL_HA_ReplyV", "MAIL_KO_ReplyV", "MAIL_FU_ReplyV", "MAIL_GE_ReplyV", "MAIL_TA_ReplyV",
};

// 804EEF90
static const char *sVisitorPresentLabels[LOOKS_TYPE_NUM] = {
    "MAIL_BO_PresentV", "MAIL_HA_PresentV", "MAIL_KO_PresentV",
    "MAIL_FU_PresentV", "MAIL_GE_PresentV", "MAIL_TA_PresentV",
};

static inline u16 rndPart(u16 num) {
    return num > 1 ? (u16)(cM::rndInt(num - 1) + 1) : 1;
}

// 80135B88
// Sends the visitor letter (MAIL_xx_ReplyV/PresentV) recorded by setVisitorLetter while the
// player visited another town, once the player is home.
BOOL sendVisitorLetter(dPrivateData_c *player) {
    u8 looks;
    u16 present;
    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayer();
    }
    if (player == NULL) {
        return FALSE;
    }

    dPersonalID_c *pid = &player->mPID;
    if (!pid->isValid() || !pid->isFromTown()) {
        return FALSE;
    }

    dAnimalItem_c *gift = &player->mVisitorLetter;
    if (!gift->isValid()) {
        return FALSE;
    }
    if (!gift->mAnimal.isValid()) {
        return FALSE;
    }

    present = gift->mItem.mId;
    if (present != dItem::ITEM_ID_NONE) {
        dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(dItem::Item(present));
        if (bitm == NULL) {
            present = dItem::ITEM_ID_NONE;
        }
    }

    looks = gift->mAnimal.getLooks(1);
    if (looks >= LOOKS_TYPE_NUM) {
        return FALSE;
    }

    const char *bodyLabel;
    const char *replyLabel = sVisitorReplyLabels[looks];
    u16 num = fn_800CBAC4(replyLabel);
    u16 header = rndPart(num);
    u16 body = rndPart(num);

    if (present != dItem::ITEM_ID_NONE) {
        bodyLabel = sVisitorPresentLabels[looks];
    } else {
        bodyLabel = sVisitorReplyLabels[looks];
    }
    num = fn_800CBA98(bodyLabel);
    u16 part0 = rndPart(num);
    u16 part1 = rndPart(num);
    u16 part2 = rndPart(num);

    dItem::Item paper(fn_800FABF4(looks, dTime_c::getCurrent()->getSeason()));
    fn_800F46CC(&gift->mAnimal, 0);
    fn_800F4764(&gift->mAnimal, 8);
    sMail.clear();
    sMail.setupFromAnimal(&body, &part0, &part1, &part2, &header, (int)bodyLabel, (int)replyLabel, &gift->mAnimal,
                          pid, &paper);
    if (present != dItem::ITEM_ID_NONE) {
        sMail.setPresent(present, 0xFF);
    }

    if (fn_80102BBC(&sMail)) {
        return TRUE;
    }
    return FALSE;
}
