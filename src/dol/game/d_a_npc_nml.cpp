// The normal (villager) npc actor: dAcNpcNml_c and its talk_c / clothMng_c / resMng_c.
// .text 8002ED4C..80036324. See include/game/game/d_a_npc_nml.hpp and notes/d_a_npc_nml.txt.
// These three first: weak RTTI data order in .data (dAcNpc_c group, dDemoActor_c, dMsg::Rcpt_c,
// the dItem::name*_c vtables, dActor_c, dString::Word_c, WordBase_c, dScript::Word_c) needs
// dScript/dString words, then dActor_c, then the dItem names completed before d_a_npc.hpp's classes.
#include <game/game/d_string.hpp>
#include <game/game/d_actor.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_a_npc_nml.hpp>
#include <game/cLib/c_math.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_fg_data.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_npc_list.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_save_data.hpp>
#include <lib/egg/core/eggHeap.h>
#include <game/game/d_net.hpp>
#include <game/game/d_quest.hpp>
#include <game/game/d_string.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_npc_talk_approach.hpp>
#include <game/game/d_npc_talk_arbeit.hpp>
#include <game/game/d_npc_talk_birthday.hpp>
#include <game/game/d_npc_talk_bug.hpp>
#include <game/game/d_npc_talk_carnival.hpp>
#include <game/game/d_npc_talk_countdown.hpp>
#include <game/game/d_npc_talk_fireworks.hpp>
#include <game/game/d_npc_talk_fishing.hpp>
#include <game/game/d_npc_talk_free.hpp>
#include <game/game/d_npc_talk_halloween.hpp>
#include <game/game/d_npc_talk_harvest.hpp>
#include <game/game/d_npc_talk_quest_q01.hpp>
#include <game/game/d_npc_talk_quest_q02.hpp>
#include <game/game/d_npc_talk_quest_q03.hpp>
#include <game/game/d_npc_talk_quest_q04.hpp>
#include <game/game/d_npc_talk_quest_q05.hpp>
#include <game/game/d_npc_talk_quest_q06.hpp>
#include <game/game/d_npc_talk_quest_q07.hpp>
#include <game/game/d_npc_talk_quest_q08.hpp>
#include <game/game/d_npc_talk_quest_q09.hpp>
#include <game/game/d_npc_talk_quest_q10.hpp>
#include <game/game/d_npc_talk_quest_q11.hpp>
#include <game/game/d_npc_talk_quest_q12.hpp>
#include <game/game/d_npc_talk_quest_q13.hpp>
#include <game/game/d_npc_talk_reaction.hpp>
#include <game/game/d_npc_talk_town.hpp>
#include <game/game/d_npc_talk_fmarket.hpp>       // procedure-set tables
#include <game/game/d_npc_talk_quest_delivery.hpp>
#include <game/game/d_npc_talk_rollan.hpp>
#include <game/game/d_mail.hpp>                   // static dMail_c (.bss)
#include <revolution/CX/CXUncompression.h>        // resMng_c
#include <cstdio>                                 // sprintf
#include <game/game/d_event.hpp>
#include <game/game/d_msg.hpp>
#include <game/game/d_npc_mdl_mng.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_weather.hpp>
#include <game/mLib/m_heap.hpp>


// ---- file-top data (before the first function; order = .rodata 8046BCD0.., .sdata2 8074FF30..) ----
// 8046BCD0 (the ear-type table l_speciesEarType) must be defined right here, ABOVE the labels: the labels
// follow it in .rodata, while in .sdata2 they precede the 0.0f literal of fn_8002EF94.
// 8046BCD0: ear type of each species (dAcNpcNml_c::getSpeciesEarType; 60 = no ears)
static const int l_speciesEarType[33] = {
    0, 8, 16, 3, 7, 6, 1, 60, 19, 14, 10, 20, 60, 22, 23, 60, 60,
    60, 21, 5, 25, 9, 13, 60, 17, 4, 24, 60, 15, 2, 18, 11, 12,
};
const char l_Ai_Quest[] = "Ai_Quest";       // 8046BD54
const char l_Q_Cancel[] = "Q_Cancel";       // 8046BD60
const char l_Q_ItemFull[] = "Q_ItemFull";   // 8046BD6C
const char l_Q_Timeover[] = "Q_Timeover";   // 8046BD78
const char l_Q_Timeover2[] = "Q_Timeover2"; // 8046BD84
const char l_Q_Payback[] = "Q_Payback";     // 8046BD90
const char l_Ev_Arbeit[] = "Ev_Arbeit";     // 8046BD9C
const char l_Q_Yes[] = "Q_Yes";             // 8074FF30
const char l_Q_No[] = "Q_No";               // 8074FF38
const char l_Q_Item[] = "Q_Item";           // 8074FF40
const char l_Q_Lost[] = "Q_Lost";           // 8074FF48

// ---- dAcNpcNml_c (8002ED4C..80030194) ----

// 8002ED4C
bool dAcNpcNml_c::canInteract(dDemoActor_c *actor) {
    if (!dAcNpc_c::canInteract(actor)) {
        return false;
    }
    if (mBusy) {
        return false;
    }
    if (actor == NULL) {
        return false;
    }
    if (isInUse(NULL)) {
        return false;
    }
    return !mTalk.isBusy();
}

// 8002EDE4
int dAcNpcNml_c::demoHook60(dDemoActor_c *actor) {
    if (actor == NULL) {
        return 0;
    }
    if (isInUse(NULL)) {
        return 0;
    }
    return !mTalk.isBusy();
}

// 8002EE44
u32 dAcNpcNml_c::getHeapSize() {
    return calcHeapSize(FALSE);
}

// 8002EE4C
u32 dAcNpcNml_c::addToNpcList() {
    return fn_800F981C(this, &mNpcItem.mId);
}

// 8002EE54
void dAcNpcNml_c::removeFromNpcList() {
    fn_800F9834(this);
}

// 8002EE58
int dAcNpcNml_c::getFaceType() {
    return _DC + 4;
}

// 8002EE64
void dAcNpcNml_c::setAnimal() {
    mpAnimal = dSaveData_c::getTown()->mAnimals.getAnimalByKey(&mNpcItem);
    if (mpAnimal != NULL) {
        mpEntry = fn_800F3EE8(&mpAnimal->mID);
    }
}

// 8002EEB8
int dAcNpcNml_c::getNpcIdx() const {
if (isVillagerItem(mNpcItem)) {
return mNpcItem.mId & 0xFFF;
}
if (isSpNpcItem(mNpcItem)) {
return mNpcItem.mId - 0xE800;
}
return -1;
}

// 8002EF5C
void dAcNpcNml_c::getName(dHmnName::Word_c *name, int len) {
    if (mpAnimal != NULL) {
        mpAnimal->mID.setWord(name, len);
    }
}

// 8002EF74
u8 dAcNpcNml_c::getNameKind() {
    if (mpAnimal != NULL) {
        return mpAnimal->mID.getGender(1);
    }
    return 2;
}

// 8002EF94
f32 dAcNpcNml_c::vt90() const {
    if (mpAnimal != NULL) {
        return fn_800F4D84(mpAnimal->getSpecies(), 0);
    }
    return 0.0f;
}

// 8002EFD4
f32 dAcNpcNml_c::vt94() const {
    if (mpAnimal != NULL) {
        return fn_800F4D84(mpAnimal->getSpecies(), 1);
    }
    return 0.0f;
}

// 8002F014
int dAcNpcNml_c::getSpeciesEarType(u8 species) {
    if (species < 33) {
        return l_speciesEarType[species];
    }
    return 60;
}

// 8002F038
int dAcNpcNml_c::getEarType() {
    if (mpAnimal != NULL) {
        u8 species = mpAnimal->getSpecies();
        if (species < 33) {
            return getSpeciesEarType(species);
        }
    }
    return 60;
}

// 8002F090
int dAcNpcNml_c::getVoiceType() const {
    dAnimal_c *animal = mpAnimal;
    int res = 0;
    if (animal != NULL && animal->mID.isValid()) {
        res = fn_800F3FFC(animal->mID.getLooks(1));
    }
    return res;
}

// 8002F0F8
BOOL dAcNpcNml_c::isTradeFtr(u32 idx) {
    if (idx < 10) {
        return (mTradeFtrFlags >> idx) & 1;
    }
    return FALSE;
}

// 8002F118
void dAcNpcNml_c::clearTradeFtr(u32 idx) {
    if (idx < 10) {
        mTradeFtrFlags &= ~(1 << idx);
    }
}

// 8002F138
int dAcNpcNml_c::getTradeFtrNum() {
    int count = 0;
    for (int i = 0; i < 10; i++) {
        if (isTradeFtr(i)) {
            count++;
        }
    }
    return count;
}

// 8002F1A0
BOOL dAcNpcNml_c::isSellableFtr(const dItem::Item *item) {
    if (!item->isValid()) {
        return FALSE;
    }
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(*item);
    if (bitm == NULL) {
        return FALSE;
    }
    int kind = bitm->getKind();
    if ((kind == dItem::KIND_FTR || kind == dItem::KIND_FOSSIL) && !bitm->m_noPurchase) {
        int price = item->getPrice();
        if (price > 0 && price < 10000) {
            return TRUE;
        }
    }
    return FALSE;
}

// 8002F254
void dAcNpcNml_c::selectTradeFtrs() {
    dAnimal_c *animal = mpAnimal;
    mTradeFtrFlags = 0;
    if (animal == NULL) {
        return;
    }
    for (int i = 0; i < 10; i++) {
        dItem::Item ftr = animal->getFtr(i);
        if (isSellableFtr(&ftr)) {
            mTradeFtrFlags |= 1 << i;
        }
    }
    u32 count = getTradeFtrNum();
    if (count <= 3) {
        mTradeFtrFlags = 0;
        return;
    }
    u32 keep = cM::rndInt(3) + 3;
    u32 remove = count > keep ? count - keep : 0;
    for (; remove != 0; remove--) {
        u32 num = 0;
        int sel = -1;
        for (int i = 0; i < 10; i++) {
            if (isTradeFtr(i)) {
                f32 rate = 100.0f / (num + 1);
                if (cM::rndF(100.0f) <= rate) {
                    sel = i;
                }
                num++;
            }
        }
        clearTradeFtr(sel);
    }
}

// 8002F3D0
int dAcNpcNml_c::getRoomFtrIdx(u32 x, u32 z) {
    dAnimal_c *animal = mpAnimal;
    if (animal == NULL) {
        return 10;
    }
    int type = 1;
    int layout = animal->getRoomLayout(&type);
    if (layout == -1) {
        return 10;
    }
    EGG::Heap *heap = EGG::Heap::getCurrentHeap();
    if (heap == NULL) {
        return 10;
    }
    dItem::Item *items = (dItem::Item *)heap->alloc(0x100 * sizeof(dItem::Item), 4);
    if (items == NULL) {
        return 10;
    }
    for (int i = 0; i < 0x100; i++) {
        new (&items[i]) dItem::Item;
    }
    BOOL ok;
    if (type == 1) {
        ok = dFgData_getLayout1((u16 *)items, layout);
    } else {
        ok = dFgData_getLayout2((u16 *)items, layout);
    }
    int slot = 10;
    if (ok) {
        slot = getRoomFtrSlot(items, x, z);
    }
    heap->free(items);
    return slot;
}

// 8002F568
dItem::Item dAcNpcNml_c::getRoomFtrAt(int *outX, int *outZ, int x, int z) {
    return fn_800F1BE4(outX, outZ, x, z, NULL, 0x44);
}

// 8002F574
dItem::Item dAcNpcNml_c::getRoomFtrAtPos(int *outX, int *outZ, const mVec3_c *pos) {
    f32 x = pos->x;
    f32 z = pos->z;
    return getRoomFtrAt(outX, outZ, (int)x >> 5, (int)z >> 5);
}

// 8002F5BC
int dAcNpcNml_c::removeRoomFtr(int obj, int x, int z) {
    return 0;
}

// 8002F5C4
void dAcNpcNml_c::onSessionFlag(u32 bit) {
    if (bit < 8) {
        mSessionFlags |= 1 << bit;
    }
}

// 8002F5E4
BOOL dAcNpcNml_c::isSessionFlag(u32 bit) {
    if (bit < 8) {
        return (mSessionFlags >> bit) & 1;
    }
    return FALSE;
}

// 8002F604
int dAcNpcNml_c::getSoundId() {
    dAnimal_c *animal = mpAnimal;
    int id = 0xFFFF;
    if (animal != NULL && animal->mID.isValid() && mpEntry != NULL && !mpEntry->_28C.mTalked) {
        id = (u16)animal->getSoundId();
    }
    return id;
}

// 8002F684
const Vec *dAcNpcNml_c::searchPosTable(const posTable33_s *table, int num, int key, u8 idx) {
    if (table == NULL || num == 0) {
        return NULL;
    }
    if (idx >= 33) {
        return NULL;
    }
    for (u32 i = 0; i < num; i++, table++) {
        if (table->mKey == key) {
            return &table->mPos[idx];
        }
    }
    return NULL;
}

// 8002F6E4
void dAcNpcNml_c::getManpuOfs(mVec3_c *ofs, mVec3_c *ofsL, mVec3_c *ofsR, u8 type) {
    if (ofs != NULL && ofsL != NULL && ofsR != NULL && type < 0x4E && mpAnimal != NULL) {
        u32 species = mpAnimal->getSpecies();
        if ((u8)species < 33) {
            const Vec *pos = searchPosTable(l_80466730, l_8074FE38, type, species);
            if (pos != NULL) {
                ofs->x = pos->x;
                ofs->y = pos->y;
                ofs->z = pos->z;
            }
            pos = searchPosTable(l_80467090, l_8074FE3C, type, species);
            if (pos != NULL) {
                ofsL->x = pos->x;
                ofsL->y = pos->y;
                ofsL->z = pos->z;
            }
            pos = searchPosTable(l_80467540, l_8074FE40, type, species);
            if (pos != NULL) {
                ofsR->x = pos->x;
                ofsR->y = pos->y;
                ofsR->z = pos->z;
            }
        }
    }
}

// 8002F804
int dAcNpcNml_c::startNpcChat(dAcNpc_c *partner) {
    return 0;
}

// 8002F80C
int dAcNpcNml_c::endNpcChat() {
    return 0;
}

// 8002F814
int dAcNpcNml_c::preCreate() {
    if (dAcNpc_c::preCreate() == NOT_READY) {
        return NOT_READY;
    }
    mpChatPartner = NULL;
    mSessionFlags = 0;
    setAnimal();
    mpRes = &mRes;
    setRecept(&mNmlTalk);
    mCloth.init(this);
    mBusy = 0;
    return SUCCEEDED;
}

// 8002F8A0
int dAcNpcNml_c::create() {
    if (dAcNpc_c::create() == NOT_READY) {
        return NOT_READY;
    }
    dNpcEntry_c *entry = mpEntry;
    if (entry != NULL) {
        entry->_28C.mTalked = 0;
        entry->_28C.mEventTalked = 0;
        entry->mMsg2.clear();
        entry->mTimer.update();
    }
    return SUCCEEDED;
}

// 8002F914
int dAcNpcNml_c::doDelete() {
    if (!mCloth.syncLoad()) {
        return NOT_READY;
    }
    if (dAcNpc_c::doDelete() == NOT_READY) {
        return NOT_READY;
    }
    return SUCCEEDED;
}

// 8002F968
BOOL dAcNpcNml_c::setDaubToolChange(BOOL on) {
    if (!isLocalOwner()) {
        return FALSE;
    }
    dNpcDaub_c *daub = fn_800EFD04(&mNpcItem.mId);
    if (daub == NULL) {
        return FALSE;
    }
    daubData_s data;
    daub->get11(&data, sizeof(data));
    if (on) {
        data.mToolChange = 1;
    } else {
        data.mToolChange = 0;
    }
    setDaubData(&data, sizeof(data));
    return TRUE;
}

// 8002FA0C
void dAcNpcNml_c::onDaubToolChange() {
    setDaubToolChange(TRUE);
}

// 8002FA14
void dAcNpcNml_c::offDaubToolChange() {
    setDaubToolChange(FALSE);
}

// 8002FA1C
BOOL dAcNpcNml_c::isDaubToolChange() {
    daubData_s data;
    if (getDaubData(&data, sizeof(data)) && data.mToolChange) {
        return TRUE;
    }
    return FALSE;
}

// 8002FA64
BOOL dAcNpcNml_c::setDaubToolChanged(BOOL on) {
    if (!isLocalOwner()) {
        return FALSE;
    }
    dNpcDaub_c *daub = fn_800EFD04(&mNpcItem.mId);
    if (daub == NULL) {
        return FALSE;
    }
    daubData_s data;
    daub->get11(&data, sizeof(data));
    if (on) {
        data.mToolChanged = 1;
    } else {
        data.mToolChanged = 0;
    }
    setDaubData(&data, sizeof(data));
    return TRUE;
}

// 8002FB08
void dAcNpcNml_c::onDaubToolChanged() {
    setDaubToolChanged(TRUE);
}

// 8002FB10
void dAcNpcNml_c::offDaubToolChanged() {
    setDaubToolChanged(FALSE);
}

// 8002FB18
BOOL dAcNpcNml_c::isDaubToolChanged() {
    daubData_s data;
    if (getDaubData(&data, sizeof(data)) && data.mToolChanged) {
        return TRUE;
    }
    return FALSE;
}

// 8002FB60
BOOL dAcNpcNml_c::setDaubMaskChange(BOOL on) {
    if (!isLocalOwner()) {
        return FALSE;
    }
    dNpcDaub_c *daub = fn_800EFD04(&mNpcItem.mId);
    if (daub == NULL) {
        return FALSE;
    }
    daubData_s data;
    daub->get11(&data, sizeof(data));
    if (on) {
        data.mMaskChange = 1;
    } else {
        data.mMaskChange = 0;
    }
    setDaubData(&data, sizeof(data));
    return TRUE;
}

// 8002FC04
void dAcNpcNml_c::onDaubMaskChange() {
    setDaubMaskChange(TRUE);
}

// 8002FC0C
void dAcNpcNml_c::offDaubMaskChange() {
    setDaubMaskChange(FALSE);
}

// 8002FC14
BOOL dAcNpcNml_c::isDaubMaskChange() {
    daubData_s data;
    if (getDaubData(&data, sizeof(data)) && data.mMaskChange) {
        return TRUE;
    }
    return FALSE;
}

// 8002FC5C
BOOL dAcNpcNml_c::setDaubInHouse(BOOL on) {
    if (!isLocalOwner()) {
        return FALSE;
    }
    dNpcDaub_c *daub = fn_800EFD04(&mNpcItem.mId);
    if (daub == NULL) {
        return FALSE;
    }
    daubData_s data;
    daub->get11(&data, sizeof(data));
    if (on) {
        data.mInHouse = 1;
    } else {
        data.mInHouse = 0;
    }
    setDaubData(&data, sizeof(data));
    return TRUE;
}

// 8002FD00
void dAcNpcNml_c::onDaubInHouse() {
    setDaubInHouse(TRUE);
}

// 8002FD08
void dAcNpcNml_c::offDaubInHouse() {
    setDaubInHouse(FALSE);
}

// 8002FD10
BOOL dAcNpcNml_c::isDaubInHouse() {
    daubData_s data;
    if (getDaubData(&data, sizeof(data)) && data.mInHouse) {
        return TRUE;
    }
    return FALSE;
}

// 8002FD58
BOOL dAcNpcNml_c::setDaubWatchFireworks(BOOL on) {
    if (!isLocalOwner()) {
        return FALSE;
    }
    dNpcDaub_c *daub = fn_800EFD04(&mNpcItem.mId);
    if (daub == NULL) {
        return FALSE;
    }
    daubData_s data;
    daub->get11(&data, sizeof(data));
    if (on) {
        data.mWatchFireworks = 1;
    } else {
        data.mWatchFireworks = 0;
    }
    setDaubData(&data, sizeof(data));
    return TRUE;
}

// 8002FDFC
void dAcNpcNml_c::onDaubWatchFireworks() {
    setDaubWatchFireworks(TRUE);
}

// 8002FE04
void dAcNpcNml_c::offDaubWatchFireworks() {
    setDaubWatchFireworks(FALSE);
}

// 8002FE0C
BOOL dAcNpcNml_c::isDaubWatchFireworks() {
    daubData_s data;
    if (getDaubData(&data, sizeof(data)) && data.mWatchFireworks) {
        return TRUE;
    }
    return FALSE;
}

// 8002FE54
BOOL dAcNpcNml_c::setDaubClothChange(BOOL on) {
    if (!isLocalOwner()) {
        return FALSE;
    }
    dNpcDaub_c *daub = fn_800EFD04(&mNpcItem.mId);
    if (daub == NULL) {
        return FALSE;
    }
    daubData_s data;
    daub->get11(&data, sizeof(data));
    if (on) {
        data.mClothChange = 1;
    } else {
        data.mClothChange = 0;
    }
    setDaubData(&data, sizeof(data));
    return TRUE;
}

// 8002FEF8
void dAcNpcNml_c::onDaubClothChange() {
    setDaubClothChange(TRUE);
}

// 8002FF00
void dAcNpcNml_c::offDaubClothChange() {
    setDaubClothChange(FALSE);
}

// 8002FF08
BOOL dAcNpcNml_c::isDaubClothChange() {
    daubData_s data;
    if (getDaubData(&data, sizeof(data)) && data.mClothChange) {
        return TRUE;
    }
    return FALSE;
}

// 8002FF50
BOOL dAcNpcNml_c::setDaubClothChanged(BOOL on) {
    if (!isLocalOwner()) {
        return FALSE;
    }
    dNpcDaub_c *daub = fn_800EFD04(&mNpcItem.mId);
    if (daub == NULL) {
        return FALSE;
    }
    daubData_s data;
    daub->get11(&data, sizeof(data));
    if (on) {
        data.mClothChanged = 1;
    } else {
        data.mClothChanged = 0;
    }
    setDaubData(&data, sizeof(data));
    return TRUE;
}

// 8002FFF4
void dAcNpcNml_c::onDaubClothChanged() {
    setDaubClothChanged(TRUE);
}

// 8002FFFC
void dAcNpcNml_c::offDaubClothChanged() {
    setDaubClothChanged(FALSE);
}

// 80030004
BOOL dAcNpcNml_c::isDaubClothChanged() {
    daubData_s data;
    if (getDaubData(&data, sizeof(data)) && data.mClothChanged) {
        return TRUE;
    }
    return FALSE;
}

// 8003004C
BOOL dAcNpcNml_c::isClothChangeReady() const {
    if (mpAnimal == NULL) {
        return FALSE;
    }
    const clothMng_c *cloth = &mCloth;
    if (mpAnimal->mCloth.isSame(cloth->mCurCloth)) {
        return FALSE;
    }
    return cloth->isLoaded(this) != FALSE;
}

// 800300D4
void dAcNpcNml_c::changeClothModel() {
    if (isClothChangeReady()) {
        nw4r::g3d::ResFile file(getMdlResFile());
        mCloth.swapCloth(file.GetResMdl(0));
    }
}

// 80030148
dAcNpcNml_c::resMng_c::resMng_c() : mpMdl(NULL), mpTex(NULL), mpSetup(NULL) {}

// ---- dAcNpcNml_c::resMng_c, dAcNpcNml_c::clothMng_c, talk_c ctor (80030194..800309A0) ----

// 80030194
dAcNpcNml_c::resMng_c::~resMng_c() {}

// 800301EC
BOOL dAcNpcNml_c::resMng_c::create(dAcNpc_c *npc) {
    dAnimal_c *animal = static_cast<dAcNpcNml_c *>(npc)->mpAnimal;
    if (animal == NULL) {
        return TRUE;
    }

    EGG::Heap *heap = npc->m_heap_p;
    if (mpMdl == NULL) {
        mpMdl = heap->alloc(fn_800F9D8C(), 0x20);
        if (mpMdl != NULL) {
            u32 size = 0;
            int species = animal->getSpecies();
            fn_800F9D94(mpMdl, &size, species);
        } else {
            return FALSE;
        }
    }

    if (!animal->mSetupLoaded) {
        char path[32];
        sprintf(path, "/Npc/Normal/Setup/%d.bin", animal->mID.mNpcIdx);
        if (npc->loadRes(&mpSetup, path)) {
            animal->setSetupData(mpSetup);
        } else {
            return FALSE;
        }
    }

    if (mpTex == NULL) {
        mpTex = heap->alloc(0x8480, 0x20);
        if (mpTex != NULL) {
            memset(mpTex, 0, 4);
            CXUncompressLZ(animal, mpTex);
            DCFlushRange(mpTex, 0x8480);
            nw4r::g3d::ResFile mdl(getMdlRes());
            nw4r::g3d::ResFile tex(getTexRes());
            tex.Init();
            mdl.Bind(tex);
        }
    }
    return TRUE;
}

// 800303B0
void *dAcNpcNml_c::resMng_c::getMdlRes() {
    return mpMdl;
}

// 800303B8
void *dAcNpcNml_c::resMng_c::getTexRes() {
    return mpTex;
}

// 800303C0
dAcNpcNml_c::clothMng_c::clothMng_c() {}

// 8003041C
dAcNpcNml_c::clothMng_c::~clothMng_c() {}

// 80030484
void dAcNpcNml_c::clothMng_c::init(dAcNpcNml_c *npc) {
    mHmnCloth.setSlot(npc->_DC + 4);
    if (!mCloth.isValid()) {
        dItem::Item cloth(dItem::ITEM_IDX_ONE_BALL_SHIRT);
        if (npc->mpAnimal != NULL) {
            cloth = npc->mpAnimal->mCloth;
        }
        mCloth = cloth;
    }
}

// 800304FC
BOOL dAcNpcNml_c::clothMng_c::isSlotValid() {
    int slot = mHmnCloth.mSlot;
    return slot < dHmnClothMng_c::SLOT_NUM;
}

// 80030518
BOOL dAcNpcNml_c::clothMng_c::requestCloth(const dItem::Item *item, dDesign_c *design) {
    if (mHmnCloth.mSlot >= dHmnClothMng_c::SLOT_NUM) {
        return TRUE;
    }

    dItem::Item cloth(*item);
    if (!dHmnClothMng_c::isCloth(&cloth) && (!dHmnClothMng_c::isOrgCloth(NULL, &cloth) || design == NULL)) {
        cloth.setFromIndex(dItem::ITEM_IDX_ONE_BALL_SHIRT);
    }
    mReqCloth = cloth;
    if (mHmnCloth.request(&cloth, NULL, design)) {
        mLoadedCloth = cloth;
        mReqCloth = dItem::ITEM_ID_NONE;
        return TRUE;
    }
    mLoadedCloth = dItem::ITEM_ID_NONE;
    return FALSE;
}

// 800305F0
BOOL dAcNpcNml_c::clothMng_c::requestNpcCloth(dAcNpc_c *npc) {
    dAnimal_c *animal = static_cast<dAcNpcNml_c *>(npc)->mpAnimal;
    if (animal != NULL) {
        return requestCloth(&animal->mCloth, &animal->mClothDesign);
    }
    return TRUE;
}

// 8003061C
BOOL dAcNpcNml_c::clothMng_c::create(dAcNpc_c *npc) {
    dAnimal_c *animal = static_cast<dAcNpcNml_c *>(npc)->mpAnimal;
    dItem::Item cloth(mCloth);
    if (!cloth.isValid() && animal != NULL) {
        cloth = animal->mCloth;
    }
    if (cloth.isValid() && animal != NULL) {
        return requestCloth(&cloth, &animal->mClothDesign);
    }
    return TRUE;
}

// 80030690
void dAcNpcNml_c::clothMng_c::bindCloth(nw4r::g3d::ResMdl mdl) {
    if (mdl.IsValid()) {
        void *res = mHmnCloth.getResFile();
        if (res != NULL) {
            mdl.Bind(nw4r::g3d::ResFile(res));
            mHmnCloth.lock();
            if (mLoadedCloth.isValid()) {
                mCurCloth = mLoadedCloth;
            }
        }
    }
}

// 80030708
void dAcNpcNml_c::clothMng_c::swapCloth(nw4r::g3d::ResMdl mdl) {
    static const char cTexName[] = "cloth";  // 8074FF60
    static const char cPlttName[] = "cloth"; // 8074FF68
    if (mdl.IsValid()) {
        mdl.ReleasePlttByName(cPlttName);
        mdl.ReleaseTexByName(cTexName);
        bindCloth(mdl);
    }
}

// 80030780
BOOL dAcNpcNml_c::clothMng_c::syncLoad() {
    return mHmnCloth.syncLoad();
}

// 80030788
BOOL dAcNpcNml_c::clothMng_c::execute(dAcNpc_c *npc) {
    if (!isSlotValid()) {
        return FALSE;
    }
    dAnimal_c *animal = static_cast<dAcNpcNml_c *>(npc)->mpAnimal;
    if (animal == NULL) {
        return FALSE;
    }
    if (!animal->isWearingOrgCloth() && !animal->isWearingCloth()) {
        return FALSE;
    }
    if (animal->mCloth.isNotSame(mLoadedCloth)) {
        requestNpcCloth(npc);
    }
    return TRUE;
}

// 80030848
BOOL dAcNpcNml_c::clothMng_c::isLoaded(const dAcNpc_c *npc) const {
    dAnimal_c *animal = static_cast<const dAcNpcNml_c *>(npc)->mpAnimal;
    if (animal == NULL) {
        return FALSE;
    }
    return !animal->mCloth.isNotSame(mLoadedCloth);
}

// 80030898
void dAcNpcNml_c::clothMng_c::clearLoaded() {
    mLoadedCloth = dItem::ITEM_ID_NONE;
}

// ---- data defined between resMng_c (.data 804A0768 literal) and talk_c's code (804A0C28..) ----
// .bss 80565060..80565428 in this order (__sinit registers them in definition order).
static dMail_c l_80565070;
mVec3_c l_walkTargetPos(256.0f, 0.0f, 464.0f);

// 804A0784: procedure sets of the talk TUs (NULL slots: __ptmf_null copies in __sinit)
dAcNpcNml_c::talk_c::procSet_s l_talkProcSets[dAcNpcNml_c::talk_c::TALK_PROC_NUM] = {
    {&dAcNpcNml_c::talk_c::msgEntry, NULL, NULL}, // 0 talk
    {&dAcNpcNml_c::talk_c::msgVisitCall, NULL, NULL}, // 1 QuestQ08
    {&dAcNpcNml_c::talk_c::msgVisitFirst, NULL, NULL}, // 2 QuestQ08
    {&dAcNpcNml_c::talk_c::msgVisitWait, NULL, NULL}, // 3 QuestQ08
    {&dAcNpcNml_c::talk_c::msgVisitBack, NULL, NULL}, // 4 QuestQ08
    {&dAcNpcNml_c::talk_c::msgInviteWelcome, NULL, NULL}, // 5 QuestQ09
    {&dAcNpcNml_c::talk_c::msgInviteFirst, NULL, NULL}, // 6 QuestQ09
    {&dAcNpcNml_c::talk_c::msgInviteTrade, NULL, NULL}, // 7 QuestQ09
    {&dAcNpcNml_c::talk_c::msgInviteWait, NULL, NULL}, // 8 QuestQ09
    {&dAcNpcNml_c::talk_c::msgInviteAnalog, NULL, NULL}, // 9 QuestQ09
    {&dAcNpcNml_c::talk_c::msgHideExplain, NULL, NULL}, // 10 QuestQ10
    {&dAcNpcNml_c::talk_c::msgHideHider, NULL, NULL}, // 11 QuestQ10
    {&dAcNpcNml_c::talk_c::msgHideReward, NULL, NULL}, // 12 QuestQ10
    {&dAcNpcNml_c::talk_c::msgHideLose, NULL, NULL}, // 13 QuestQ10
    {&dAcNpcNml_c::talk_c::msgCandyAsk, NULL, NULL}, // 14 Halloween
    {&dAcNpcNml_c::talk_c::msgHalloweenCostume, NULL, NULL}, // 15 Halloween
    {&dAcNpcNml_c::talk_c::msgApproach, &dAcNpcNml_c::talk_c::endApproach, NULL}, // 16 Approach
    {&dAcNpcNml_c::talk_c::msgTown, NULL, NULL}, // 17 Town
    {&dAcNpcNml_c::talk_c::msgTownTheater, NULL, NULL}, // 18 Town
    {&dAcNpcNml_c::talk_c::msgTownGrace, NULL, NULL}, // 19 Town
    {&dAcNpcNml_c::talk_c::msgTown3P, NULL, NULL}, // 20 Town
    {&dAcNpcNml_c::talk_c::msgFmarket, &dAcNpcNml_c::talk_c::endFmarket, NULL}, // 21 Fmarket
    {&dAcNpcNml_c::talk_c::msgSale, &dAcNpcNml_c::talk_c::endFmarket, NULL}, // 22 Fmarket
    {&dAcNpcNml_c::talk_c::msgStallCall, NULL, NULL}, // 23 Fmarket
    {&dAcNpcNml_c::talk_c::msgStallWait, NULL, NULL}, // 24 Fmarket
    {&dAcNpcNml_c::talk_c::msgStallBack, NULL, NULL}, // 25 Fmarket
    {&dAcNpcNml_c::talk_c::msgStallNoItem, &dAcNpcNml_c::talk_c::endStallBuy, NULL}, // 26 Fmarket
    {&dAcNpcNml_c::talk_c::msgStallItem, &dAcNpcNml_c::talk_c::endStallBuy, &dAcNpcNml_c::talk_c::stepStallItem}, // 27 Fmarket
    {&dAcNpcNml_c::talk_c::msgPlayerBirthday, &dAcNpcNml_c::talk_c::endPlayerBirthday, &dAcNpcNml_c::talk_c::stepPlayerBirthday}, // 28 Birthday
    {&dAcNpcNml_c::talk_c::msgNpcBirthday, &dAcNpcNml_c::talk_c::endNpcBirthday, NULL}, // 29 Birthday
    {&dAcNpcNml_c::talk_c::msgEtcHit, &dAcNpcNml_c::talk_c::endEtc, NULL}, // 30 Free
    {&dAcNpcNml_c::talk_c::msgEtcPush, &dAcNpcNml_c::talk_c::endEtc, NULL}, // 31 Free
    {&dAcNpcNml_c::talk_c::msgEtcFlea, &dAcNpcNml_c::talk_c::endEtc, NULL}, // 32 Free
};

// 800308A8
dAcNpcNml_c::talk_c::talk_c() {
    mMsgProc = NULL;
    mHookProc = NULL;
    hookFunc noProc = NULL;
    mActProc = noProc;
    mTalkEndProc = noProc;
    mResultProc = noProc;
    mNextResultProc = noProc;
    mpMemory = NULL;
    mMemoryIdx = -1;
}

// ---- dAcNpcNml_c::talk_c 800309A0..80031C00 ----

// 800309A0
dAcNpcNml_c::talk_c::~talk_c() {}

// 80030A14
void dAcNpcNml_c::talk_c::init() {
    recept_c::init();
    mMsgProc = NULL;
    mHookProc = NULL;
    mStepProc = NULL;
    hookFunc nullHook = NULL;
    mActProc = nullHook;
    mTalkEndProc = nullHook;
    mResultProc = nullHook;
    mNextResultProc = nullHook;
    mpMemory = NULL;
    mMemoryIdx = -1;
    mPickCode = 0xFFFF;
    mJinxTown = -1;
    mRumorTown = -1;
    reqFunc nullReq = NULL;
    for (int i = 0; i < 5; i++) {
        mReqProc[i] = nullReq;
    }
    clearChoice();
    clearNpcChoice();
    mNoRecordTalk = 0;
    mItem0 = dItem::ITEM_ID_NONE;
    mItem1 = dItem::ITEM_ID_NONE;
    mItem2 = dItem::ITEM_ID_NONE;
    mErrandDeadline = QUEST_DEADLINE_NEXT_HOUR;
    mAnswer = 0xFF;
    mFtrPosX = -1;
    mFtrPosZ = -1;
    mFtrHandle = -1;
    mName.clear();
    mHadNickname = 0;
    mIsAnimalItem = 0;
    mGameCount[0] = 0;
    mGameCount[1] = 0;
    mMsgPersonal.clear();
    mMsgPlayer.clear();
    mMsgLand.clear();
    mGameCount[2] = 0;
    mItem3 = dItem::ITEM_ID_NONE;
    mItem4 = dItem::ITEM_ID_NONE;
    mItem5 = dItem::ITEM_ID_NONE;
    mGameCount[3] = 0;
    mGameShown[0] = 0;
    mGameShown[1] = 0;
    mGiveFlag = 0;
    memset(mMemoryText, 0, sizeof(mMemoryText));
    mMsgYear = -1;
    mImpression = 50;
    mStartDay = dTime_c::getCurrent()->mday;
}

// 80030C1C
void dAcNpcNml_c::talk_c::setActProc(hookFunc proc) {
    mActProc = proc;
}

// 80030C38
void dAcNpcNml_c::talk_c::clearActProc() {
    setActProc(NULL);
}

// 80030C78
void dAcNpcNml_c::talk_c::setProcSet(const procSet_s *set) {
    if (set != NULL) {
        mMsgProc = set->mMsg;
        mHookProc = set->mHook;
        mStepProc = set->mStep;
    }
}

// 80030CCC
void dAcNpcNml_c::talk_c::setProcs(msgFunc msg, endFunc hook, stepFunc step) {
    mMsgProc = msg;
    mHookProc = hook;
    mStepProc = step;
}

// 80030D18
void dAcNpcNml_c::talk_c::setMsgProc(msgFunc proc) {
    mMsgProc = proc;
}

// 80030D34
void dAcNpcNml_c::talk_c::setHookProc(endFunc proc) {
    mHookProc = proc;
}

// 80030D50
void dAcNpcNml_c::talk_c::setStepProc(stepFunc proc) {
    mStepProc = proc;
}

// 80030D6C
dAnimal_c *dAcNpcNml_c::talk_c::getAnimal() {
    dAcNpcNml_c *npc = static_cast<dAcNpcNml_c *>(mpNpc);
    if (npc != NULL) {
        return npc->mpAnimal;
    }
    return NULL;
}

// 80030D88
dAnimal_c *dAcNpcNml_c::talk_c::getAnimal() const {
    dAcNpcNml_c *npc = static_cast<dAcNpcNml_c *>(mpNpc);
    if (npc != NULL) {
        return npc->mpAnimal;
    }
    return NULL;
}

// 80030DA4
int dAcNpcNml_c::talk_c::getNpcIdx() {
    dAcNpcNml_c *npc = static_cast<dAcNpcNml_c *>(mpNpc);
    if (npc != NULL) {
        return npc->getNpcIdx();
    }
    return 0;
}

// 80030DBC
dNpcEntry_c *dAcNpcNml_c::talk_c::getEntry() {
    dAcNpcNml_c *npc = static_cast<dAcNpcNml_c *>(mpNpc);
    if (npc != NULL) {
        return npc->mpEntry;
    }
    return NULL;
}

// 80030DD8
dNpcEntry_c *dAcNpcNml_c::talk_c::getEntry() const {
    dAcNpcNml_c *npc = static_cast<dAcNpcNml_c *>(mpNpc);
    if (npc != NULL) {
        return npc->mpEntry;
    }
    return NULL;
}

// 80030DF4
BOOL dAcNpcNml_c::talk_c::isHoldingTool(int toolType) {
    if (mpNpc == NULL) {
        return FALSE;
    }
    dAcNpc_c::toolBase_c *tool = mpNpc->getTool();
    if (tool == NULL || !tool->isEnable()) {
        return FALSE;
    }
    return dHmnToolBank_c::getItemToolType(&tool->mItem) == toolType;
}

// 80030E7C
BOOL dAcNpcNml_c::talk_c::startMsg() {
    msgInfo_s info;
    info.mLabel = NULL;
    info.mCode = 0;
    if (mMsgProc) {
        (this->*mMsgProc)(&info);
        if (mpController != NULL) {
            fixMsgCode(&info.mCode);
            fn_801A4E44(mpController, info.mCode);
            if (info.mLabel != NULL) {
                fn_801A4E34(mpController, info.mLabel);
            }
            return TRUE;
        }
    }
    return FALSE;
}

// 80030F18
void dAcNpcNml_c::talk_c::recordMemoryTalk(dAnimalMemory_c **memory, u32 *memoryIdx, dAnimal_c *animal, int npcIdx,
                                      const dLandID_c *land) {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    const dPersonalID_c *pid = player != NULL ? &player->mPID : NULL;
    if (pid != NULL && animal != NULL) {
        if (*memory == NULL) {
            *memoryIdx = animal->getMemoryIdx(pid);
            *memory = animal->getMemory(*memoryIdx);
        }
        if (*memory == NULL) {
            *memoryIdx = animal->getNewMemoryIdx();
            *memory = animal->getMemory(*memoryIdx);
            if (*memory != NULL) {
                (*memory)->init(pid, land, NULL);
                (*memory)->updateLetterCond(0);
                fn_800F0208(npcIdx, *memoryIdx, (*memory)->getImpression(), (*memory)->getTalkDays(),
                            (*memory)->getFriendship());
                animal->recordImpression(pid);
            }
        } else {
            (*memory)->updateLetterCond(0);
            (*memory)->updateTalk(pid, land, NULL);
            fn_800F02A8(npcIdx, *memoryIdx, (*memory)->getImpression(), (*memory)->getTalkDays(),
                        (*memory)->getFriendship());
            animal->recordImpression(pid);
        }
        animal->setVisitorLetter(player);
    }
}

// 800310A0
void dAcNpcNml_c::talk_c::recordTalk(const dLandID_c *land) {
    if (!mNoRecordTalk) {
        recordMemoryTalk(&mpMemory, &mMemoryIdx, getAnimal(), getNpcIdx(), land);
    }
}

// 80031114
void dAcNpcNml_c::talk_c::fixMsgCode(u16 *code) {
    if (*code == 0) {
        mPickCode = 0;
        *code = 1;
    }
}

// 80031134
void dAcNpcNml_c::talk_c::setTopic(u32 kind, u8 group, u8 idx) {
    mTopicKind = kind;
    mTopicGroup = group;
    mTopicIdx = idx;
}

// 80031144
void dAcNpcNml_c::talk_c::setTopicFrom(const dNpc::msgMemory_c *mem) {
    mTopicKind = mem->mKind;
    mTopicGroup = mem->mTopicGroup;
    mTopicIdx = mem->mTopicIdx;
}

// 80031160
void dAcNpcNml_c::talk_c::start(dAcNpc_c *npc, dActor_c *actor, const procSet_s *set) {
    init();
    mpActActor = actor;
    setProcSet(set);
    mpMemory = NULL;
    mMemoryIdx = -1;
    setTopic(dNpc::msgMemory_c::KIND_ANY, 0, 0);
    mCountTalk = 0;
    mStylePlayer = NULL;
    mEquip.clear();
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    const dPersonalID_c *pid = player != NULL ? &player->mPID : NULL;
    if (pid != NULL && pid->isValid()) {
        dAnimal_c *animal = getAnimal();
        if (animal != NULL) {
            mMemoryIdx = animal->getMemoryIdx(pid);
            mpMemory = animal->getMemory(mMemoryIdx);
        }
    }
}

// 80031254
void dAcNpcNml_c::talk_c::preExecute() {
    if (mActProc) {
        (this->*mActProc)();
    }
}

// 8003129C
void dAcNpcNml_c::talk_c::onRequestEnd(int kind) {
    if (mResultProc) {
        (this->*mResultProc)();
        hookFunc nullHook = NULL;
        mResultProc = nullHook;
        if (mNextResultProc) {
            mResultProc = mNextResultProc;
        }
        mNextResultProc = nullHook;
    }
}

// 8003134C
void dAcNpcNml_c::talk_c::getMsgInfo(msgInfo_s *info) {
    if (mMsgProc) {
        (this->*mMsgProc)(info);
        fixMsgCode(&info->mCode);
    }
}

// 800313B0
void dAcNpcNml_c::talk_c::onMessageStart(int arg) {
    if (!execReq(arg) && mHookProc) {
        endFunc proc = mHookProc;
        mHookProc = NULL;
        (this->*proc)(arg);
    }
}

// 80031448
void dAcNpcNml_c::talk_c::resSell() {
    if (mItem0.isValid()) {
        requestItemAct(&mItem0, 0, 0);
    }
}

// 80031468
BOOL dAcNpcNml_c::talk_c::reqSell() {
    if (!mItem0.isValid()) {
        return FALSE;
    }
    if (mPrice == 0) {
        return FALSE;
    }
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL) {
        return FALSE;
    }
    if (mPrice > player->getPocketMoney()) {
        return FALSE;
    }
    if (mIsAnimalItem) {
        dAnimal_c *animal = getAnimal();
        if (animal == NULL) {
            return FALSE;
        }
        if (animal->removeNewItem(&mItem0)) {
            fn_800F0FE4(getNpcIdx(), &mItem0);
        }
    }
    if (player->pickUp(&mItem0, FALSE)) {
        player->payMoney(mPrice, FALSE);
    }
    dItem::Item money(dItem::ITEM_IDX_100_BELLS);
    requestItemActEx(6, &money, 0, 0, 0, 2);
    mResultProc = &talk_c::resSell;
    return TRUE;
}

// 800315A8
void dAcNpcNml_c::talk_c::resBuy() {
    if (mItem0.isValid()) {
        requestItemActEx(6, &mItem0, 0, 0, 0, 2);
    }
}

// 800315D4
BOOL dAcNpcNml_c::talk_c::reqBuy() {
    if (!mItem0.isValid()) {
        return FALSE;
    }
    if (mPrice == 0) {
        return FALSE;
    }
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL) {
        return FALSE;
    }
    dAnimal_c *animal = getAnimal();
    if (animal == NULL) {
        return FALSE;
    }
    int room = player->getMoneyRoom(1);
    if (mPrice > room) {
        mPrice = room;
    }
    for (int i = 0; i < PLAYER_POCKETS_COUNT; i++) {
        if (!player->getPocketFlag(i) && player->mPockets[i].isSame(mItem0)) {
            player->clearPocket(i);
            player->addMoney(mPrice);
            break;
        }
    }
    if (animal->addNewItem(&mItem0)) {
        fn_800F0E9C(getNpcIdx(), &mItem0);
    }
    if (mpMemory != NULL && mMemoryIdx < 16) {
        if (mpMemory->setPresent(&mItem0)) {
            fn_800F1514(getNpcIdx(), mMemoryIdx, &mItem0);
        }
    }
    dItem::Item money(dItem::ITEM_IDX_100_BELLS);
    requestItemAct(&money, 0, 0);
    mResultProc = &talk_c::resBuy;
    return TRUE;
}

// 8003178C
void dAcNpcNml_c::talk_c::resTrade() {
    if (mItem2.isValid()) {
        requestItemActEx(6, &mItem2, 0, 0, 0, 2);
    }
}

// 800317B8
BOOL dAcNpcNml_c::talk_c::reqTrade() {
    if (!mItem0.isValid()) {
        return FALSE;
    }
    if (!mItem2.isValid()) {
        return FALSE;
    }
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL) {
        return FALSE;
    }
    dAnimal_c *animal = getAnimal();
    if (animal == NULL) {
        return FALSE;
    }
    for (int i = 0; i < PLAYER_POCKETS_COUNT; i++) {
        if (!player->getPocketFlag(i) && player->mPockets[i].isSame(mItem2)) {
            player->setPocket(&mItem0, i, FALSE);
            break;
        }
    }
    if (mIsAnimalItem) {
        if (animal->removeNewItem(&mItem0)) {
            fn_800F0FE4(getNpcIdx(), &mItem0);
        }
    }
    if (animal->addNewItem(&mItem2)) {
        fn_800F0E9C(getNpcIdx(), &mItem2);
    }
    if (mpMemory != NULL && mMemoryIdx < 16) {
        if (mpMemory->setPresent(&mItem2)) {
            fn_800F1514(getNpcIdx(), mMemoryIdx, &mItem2);
        }
    }
    requestItemAct(&mItem0, 0, 0);
    mResultProc = &talk_c::resTrade;
    return TRUE;
}

// 80031960
BOOL dAcNpcNml_c::talk_c::reqGive() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL) {
        return FALSE;
    }
    if (player->findEmptyPocket(0) == -1 || !mItem0.isValid()) {
        return TRUE;
    }
    player->pickUp(&mItem0, mGiveFlag);
    requestItemAct(&mItem0, mGiveFlag, 0);
    return TRUE;
}

// 800319F4
BOOL dAcNpcNml_c::talk_c::reqTake() {
    if (!mItem0.isValid()) {
        return FALSE;
    }
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL) {
        return FALSE;
    }
    for (int i = 0; i < PLAYER_POCKETS_COUNT; i++) {
        if (!player->getPocketFlag(i) && player->mPockets[i].isSame(mItem0)) {
            player->clearPocket(i);
            break;
        }
    }
    requestItemActEx(6, &mItem0, 0, 0, 0, 2);
    return TRUE;
}

// 80031AE8
BOOL dAcNpcNml_c::talk_c::reqLetter() {
    dAnimal_c *animal = getAnimal();
    if (animal != NULL) {
        reqMailMenu(&animal->mLetter, 0);
        return TRUE;
    }
    return FALSE;
}

// 80031B3C
void dAcNpcNml_c::talk_c::resHabit() {
    if (!isMenuInvalid()) {
        const wchar_t *habit = static_cast<const wchar_t *>(getMenuWork());
        if (habit != NULL) {
            dAnimal_c *animal = getAnimal();
            if (animal != NULL) {
                animal->setHabit(habit, 10);
                animal->mHabitCooldown = 6;
            }
        }
    }
    reqMsgClose();
}

// 80031BC0
BOOL dAcNpcNml_c::talk_c::reqHabit() {
    dAnimal_c *animal = getAnimal();
    if (animal == NULL) {
        return FALSE;
    }
    wchar_t *habit = animal->getHabit(10);
    if (habit == NULL) {
        return FALSE;
    }
    reqTextInput2F(reinterpret_cast<u16 *>(habit), 1);
    mResultProc = &talk_c::resHabit;
    return TRUE;
}

// talk_c 80031C00..80032D00

// 80031C44
void dAcNpcNml_c::talk_c::resNickname() {
    if (!isMenuInvalid()) {
        void *work = getMenuWork();
        if (work != NULL && mpMemory != NULL) {
            mpMemory->setNickname((const wchar_t *)work, 8);
            mpMemory->mFlags.mNicknameWait = 14;
        }
    }
    reqMsgClose();
}

// 80031CB8
BOOL dAcNpcNml_c::talk_c::reqNickname() {
    if (mpMemory == NULL) {
        return FALSE;
    }
    u16 *buf = (u16 *)mpMemory->mNickname;
    if (buf == NULL) {
        return FALSE;
    }
    reqTextInput33(buf, 1);
    mResultProc = &talk_c::resNickname;
    return TRUE;
}

// 80031D2C
void dAcNpcNml_c::talk_c::resGreeting() {
    if (!isMenuInvalid()) {
        void *work = getMenuWork();
        if (work != NULL && mpMemory != NULL) {
            mpMemory->setGreeting((const wchar_t *)work, 16);
            mpMemory->mFlags.mGreetingWait = 6;
        }
    }
    reqMsgClose();
}

// 80031DA0
BOOL dAcNpcNml_c::talk_c::reqGreeting() {
    if (mpMemory == NULL) {
        return FALSE;
    }
    u16 *buf = (u16 *)mpMemory->mGreeting;
    if (buf == NULL) {
        return FALSE;
    }
    reqTextInput34(buf, 1);
    mResultProc = &talk_c::resGreeting;
    return TRUE;
}

// 80031E14
BOOL dAcNpcNml_c::talk_c::reqProc0() {
    if (mReqProc[0]) {
        return (this->*mReqProc[0])();
    }
    return FALSE;
}

// 80031E64
BOOL dAcNpcNml_c::talk_c::reqProc1() {
    if (mReqProc[1]) {
        return (this->*mReqProc[1])();
    }
    return FALSE;
}

// 80031EB4
BOOL dAcNpcNml_c::talk_c::reqProc2() {
    if (mReqProc[2]) {
        return (this->*mReqProc[2])();
    }
    return FALSE;
}

// 80031F04
BOOL dAcNpcNml_c::talk_c::reqProc3() {
    if (mReqProc[3]) {
        return (this->*mReqProc[3])();
    }
    return FALSE;
}

// 80031F54
BOOL dAcNpcNml_c::talk_c::reqProc4() {
    if (mReqProc[4]) {
        return (this->*mReqProc[4])();
    }
    return FALSE;
}

// 80031FA4
BOOL dAcNpcNml_c::talk_c::execReq(int kind) {
    static reqFunc l_reqTable[] = {
        NULL,
        &talk_c::reqSell,
        &talk_c::reqBuy,
        &talk_c::reqTrade,
        &talk_c::reqGive,
        &talk_c::reqTake,
        &talk_c::reqLetter,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        &talk_c::reqHabit,
        &talk_c::reqNickname,
        NULL,
        &talk_c::reqGreeting,
        &talk_c::reqProc0,
        &talk_c::reqProc1,
        &talk_c::reqProc2,
        &talk_c::reqProc3,
        &talk_c::reqProc4,
    };
    if (kind < ARRAY_SIZE(l_reqTable)) {
        reqFunc *proc = &l_reqTable[kind];
        if (*proc) {
            return (this->*(*proc))();
        }
    }
    return FALSE;
}

// 8003209C
void dAcNpcNml_c::talk_c::onMessageEnd(int kind) {
    if (!execReq(kind) && mStepProc) {
        stepFunc proc = mStepProc;
        if ((this->*proc)(kind) && mStepProc == proc) {
            mStepProc = NULL;
        }
    }
    dDemo_c *ctrl = getController();
    if (ctrl != NULL && ctrl->_6C86 != 0) {
        rememberMsg(mMessageLabel, ctrl->_6C86);
    }
}

// 80032174
void dAcNpcNml_c::talk_c::choice_c::clear() {
    memset(this, 0, sizeof(choice_c));
}

// 80032180
void dAcNpcNml_c::talk_c::clearChoice() {
    mChoice.clear();
}

// 80032188
void dAcNpcNml_c::talk_c::choice_c::setAnswer(int idx, u16 code, u8 range, hookFunc proc) {
    if (idx < ARRAY_SIZE(mEntry)) {
        mEntry[idx].mCode = code;
        mEntry[idx].mRange = range;
        mProc[idx] = proc;
    }
}

// 800321C4
void dAcNpcNml_c::talk_c::setChoice(int idx, u16 code, u8 range, hookFunc proc) {
    mChoice.setAnswer(idx, code, range, proc);
}

// 80032204
void dAcNpcNml_c::talk_c::setChoiceProc(int idx, hookFunc proc) {
    mChoice.setAnswer(idx, 0, 0, proc);
}

// 8003224C
void dAcNpcNml_c::talk_c::choice_c::setNum(u32 num) {
    mNum = num;
}

// 80032254
void dAcNpcNml_c::talk_c::setChoiceNum(u32 num) {
    mChoice.setNum(num);
}

// 8003225C
void dAcNpcNml_c::talk_c::choice_c::setCancel(int cancel) {
    mCancel = cancel;
}

// 80032264
void dAcNpcNml_c::talk_c::setChoiceCancel(int cancel) {
    mChoice.setCancel(cancel);
}

// 8003226C
void dAcNpcNml_c::talk_c::showChoice() const {
    dMsgSelect_c *sel = getSelect();
    if (sel != NULL) {
        const choice_c::entry_s *entry = mChoice.mEntry;
        for (u32 i = 0; i < mChoice.mNum; i++, entry++) {
            sel->fn_8015BF9C(i, entry->mCode + (int)cM::rndF(entry->mRange), 0, 0, 0, 0);
        }
        sel->mNum = mChoice.mNum;
        sel->mCancel = mChoice.mCancel;
        fn_801A5A00(getController());
    }
}

// 80032354
BOOL dAcNpcNml_c::talk_c::choice_c::hasAnswers() const {
    return mNum != 0;
}

// 80032368
BOOL dAcNpcNml_c::talk_c::hasChoice() const {
    return mChoice.hasAnswers();
}

// 80032370
void dAcNpcNml_c::talk_c::npcChoice_c::clear() {
    memset(this, 0, sizeof(npcChoice_c));
}

// 8003237C
void dAcNpcNml_c::talk_c::clearNpcChoice() {
    mNpcChoice.clear();
}

// 80032384
void dAcNpcNml_c::talk_c::npcChoice_c::set(u16 code0, u8 range0, u16 code1, u8 range1, hookFunc proc) {
    mEntry[0].mCode = code0;
    mEntry[0].mRange = range0;
    mEntry[1].mCode = code1;
    mEntry[1].mRange = range1;
    mProc = proc;
}

// 800323B0
void dAcNpcNml_c::talk_c::setNpcChoice(u16 code0, u8 range0, u16 code1, u8 range1, hookFunc proc) {
    mNpcChoice.set(code0, range0, code1, range1, proc);
}

// 800323F0
void dAcNpcNml_c::talk_c::showNpcChoice() const {
    dMsgAnalogSelect_c *sel = getAnalogSelect();
    if (sel != NULL) {
        sel->fn_8000D340(0, mNpcChoice.mEntry[0].mCode + (u16)cM::rndInt(mNpcChoice.mEntry[0].mRange),
                    "sys_SELECT/SYS_SelectNPC");
        sel->fn_8000D340(1, mNpcChoice.mEntry[1].mCode + (u16)cM::rndInt(mNpcChoice.mEntry[1].mRange),
                    "sys_SELECT/SYS_SelectNPC");
        fn_801A5A4C(getController());
    }
}

// 800324A4
bool dAcNpcNml_c::talk_c::npcChoice_c::isSet() const {
    return mProc != NULL;
}

// 800324D4
bool dAcNpcNml_c::talk_c::hasNpcChoice() const {
    return mNpcChoice.isSet();
}

// 800324DC
void dAcNpcNml_c::talk_c::onAnswer(int) {
    dDemo_c *ctrl = getController();
    if (ctrl == NULL) {
        return;
    }
    if (ctrl->mpCurSelect == &ctrl->mAnalogSelect) {
        if (hasNpcChoice()) {
            mAnswer = ctrl->getAnalogSelect() != NULL ? ctrl->getAnalogSelect()->mCursor : -1;
            if (mAnswer < 8 && mNpcChoice.mProc) {
                (this->*mNpcChoice.mProc)();
            }
            clearNpcChoice();
        }
    } else if (hasChoice()) {
        mAnswer = ctrl->getSelect() != NULL ? ctrl->getSelect()->mCursor : -1;
        if (mAnswer < mChoice.mNum && mChoice.mNum <= ARRAY_SIZE(mChoice.mProc) && mChoice.mProc[mAnswer]) {
            (this->*mChoice.mProc[mAnswer])();
        }
        clearChoice();
    }
}

// 800325FC
void dAcNpcNml_c::talk_c::onTalkEnd() {
    if (mTalkEndProc) {
        (this->*mTalkEndProc)();
        mTalkEndProc = NULL;
    }
    dNpcEntry_c *entry = getEntry();
    if (entry != NULL) {
        entry->_28C.mTalked = 1;
    }
}

// 8003267C
const dAcNpcNml_c::talk_c::procSet_s *dAcNpcNml_c::talk_c::getProcSet(int idx) {
    if (idx < TALK_PROC_NUM) {
        return &l_talkProcSets[idx];
    }
    return NULL;
}

// 800326A0
dNpc::msgMemorySecond_c *dAcNpcNml_c::talk_c::getRememberedMsg() {
    dNpcEntry_c *entry = getEntry();
    return entry != NULL ? &entry->mMsg2 : NULL;
}

// 800326D4
dNpc::msgMemorySecond_c *dAcNpcNml_c::talk_c::getRememberedMsg() const {
    dNpcEntry_c *entry = getEntry();
    return entry != NULL ? &entry->mMsg2 : NULL;
}

// 80032708
const char *dAcNpcNml_c::talk_c::getRememberedLabel(u16 *code, int kind) const {
    dNpc::msgMemorySecond_c *mem = getRememberedMsg();
    const char *label = NULL;
    if (mem != NULL && mem->isValid()) {
        if (kind == dNpc::msgMemory_c::KIND_ANY || kind == mem->mKind) {
            if (code != NULL) {
                *code = mem->mMsgId;
            }
            label = mem->mGroup;
        }
    }
    return label;
}

// 80032798
BOOL dAcNpcNml_c::talk_c::rememberMsg(const char *label, u16 code) {
    const wchar_t *text;
    dNpc::msgMemorySecond_c *mem = getRememberedMsg();
    if (mem != NULL) {
        mem->set(label, code, mTopicKind, mTopicGroup, mTopicIdx);
        mem->setA0(mJinxTown);
        mem->setA4(mRumorTown);
        if (mMsgPersonal.isValid()) {
            mem->setPersonal(&mMsgPersonal);
        }
        if (mMsgPlayer.isValid()) {
            mem->setPlayer(&mMsgPlayer);
        }
        if (mMsgLand.isValid()) {
            mem->setLand(&mMsgLand);
        }
        if (mItem4 != dItem::ITEM_ID_NONE) {
            mem->mItem0 = mItem4;
        }
        if (mItem5 != dItem::ITEM_ID_NONE) {
            mem->mItem1 = mItem5;
        }
        text = mMemoryText;
        if (dScript::getStringLength(text, 16, 0) != 0) {
            mem->setText(text);
        }
        if (mImpression < 50) {
            mem->setAC(mImpression);
        }
        if (mMsgYear >= 0) {
            mem->_A8 = mMsgYear;
        }
        return TRUE;
    }
    return FALSE;
}

// 800328EC
void dAcNpcNml_c::talk_c::setLooksMsg(msgInfo_s *info, const char *label, u16 code) {
    dAnimal_c *animal = getAnimal();
    if (animal != NULL && label != NULL) {
        u8 looks = animal->mID.getLooks(1);
        dAnmPersonalID_c::makeResName(mLabel, sizeof(mLabel), label, looks);
        info->mLabel = mLabel;
    } else {
        info->mLabel = NULL;
    }
    info->mCode = code;
}

// 80032988
void dAcNpcNml_c::talk_c::selectMessage(const void *bmg) {
    if (mPickCode != 0) {
        return;
    }
    dScript::Res_c res(bmg);
    u16 count = res.getCount();
    dTime_c *now = dTime_c::getCurrent();
    int timeOfDay = now->getTimeOfDay();
    int season = dTime_c::getCurrentSeason();
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    dPersonalID_c *pid = player != NULL ? &player->mPID : NULL;
    dAnimal_c *animal = getAnimal();
    dAnimalMemory_c *memory = animal != NULL && pid != NULL ? animal->findMemory2(pid) : NULL;
    u32 num = 0;
    u16 code = 0;
    for (int i = 0; i < count; i++) {
        const msgAttr_s *attr = (const msgAttr_s *)res.getEntry((u16)i);
        if (attr->mFixed != 0) {
            continue;
        }
        BOOL ok = FALSE;
        if ((attr->mTimeOfDay == 0 || attr->mTimeOfDay - 1 == timeOfDay) &&
            (attr->mGender == 0 || (pid != NULL && attr->mGender - 1 == pid->player.mGender)) &&
            (attr->mSeason == 0 || attr->mSeason - 1 == season) &&
            (attr->mFriendship == 0 || (memory != NULL && attr->mFriendship - 1 == memory->getFriendshipLevel())) &&
            (attr->mMonth == 0 || attr->mMonth - 1 == now->month) &&
            (attr->mDay == 0 || attr->mDay == now->mday)) {
            ok = TRUE;
        }
        if (ok) {
            f32 rate = 100.0f / (num + 1);
            if (cM::rndF(100.0f) <= rate) {
                code = i;
            }
            num++;
        }
    }
    setMessageCode(code);
    mPickCode = 0xFFFF;
}

// 80032BD8
dAnimal_c *dAcNpcNml_c::talk_c::getSpeakerAnimal() {
    dAnimal_c *animal = NULL;
    switch (getTalkIdx()) {
    case 1: {
        dAcNpc_c *npc = getTalkNpc();
        if (npc != NULL) {
            animal = dSaveData_c::getTown()->mAnimals.getAnimalByKey(&npc->mNpcItem);
        }
        break;
    }
    default:
        animal = getAnimal();
        break;
    }
    return animal;
}

// 80032C50
dAnimal_c *dAcNpcNml_c::talk_c::getListenerAnimal() {
    dAnimal_c *animal = NULL;
    switch (getTalkIdx()) {
    case 0: {
        dAcNpc_c *npc = getPartnerNpc();
        if (npc != NULL) {
            animal = dSaveData_c::getTown()->mAnimals.getAnimalByKey(&npc->mNpcItem);
        }
        break;
    }
    default:
        animal = getAnimal();
        break;
    }
    return animal;
}

// 80032CCC
int dAcNpcNml_c::talk_c::isSpeakerItchy() {
    dAcNpcNml_c *npc = (dAcNpcNml_c *)getTalkNpc();
    if (npc != NULL && npc->isItchy()) {
        return TRUE;
    }
    return FALSE;
}

// ---- dAcNpcNml_c::talk_c 80032D00..80033E00 ----

// 80032D1C
int dAcNpcNml_c::talk_c::getVoiceMood() {
    static const int l_moodByTimer[5] = {0, 4, 1, 2, 1};
    int mood = 0;
    dNpcEntry_c *entry = getEntry();
    int mode = entry != NULL ? entry->mTimer.getMode() : 5;
    if ((u32)mode < 5) {
        mood = l_moodByTimer[mode];
    }
    return mood;
}

// 80032D7C
BOOL dAcNpcNml_c::talk_c::startMood(int mood, int hours) {
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        return FALSE;
    }
    dAcNpcNml_c *npc = static_cast<dAcNpcNml_c *>(getNpc(mSpeaker));
    if (npc != NULL) {
        npc->mMood.startTimer(mood, hours);
    }
    return TRUE;
}

// 80032E04
void dAcNpcNml_c::talk_c::resetMood() {
    startMood(0, 0);
}

// 80032E10
void dAcNpcNml_c::talk_c::setMood1(int hours) {
    startMood(1, hours);
}

// 80032E1C
void dAcNpcNml_c::talk_c::setMood2(int hours) {
    startMood(2, hours);
}

// 80032E28
void dAcNpcNml_c::talk_c::setMood3(int hours) {
    startMood(3, hours);
}

// 80032E34
void dAcNpcNml_c::talk_c::setMood4(int hours) {
    startMood(4, hours);
}

// 80032E40
void dAcNpcNml_c::talk_c::playTownTune() {
    dDemo_c *ctrl = getController();
    if (ctrl != NULL) {
        ctrl->mLock = 1;
    }
    setActProc(&talk_c::actPlayTune);
}

// 80032E94
void dAcNpcNml_c::talk_c::actPlayTune() {
    if (!fn_8000FBF4()) {
        const recept_c *self = this; // the target calls the const getNpc overload (800292B4)
        dAcNpcNml_c *npc = static_cast<dAcNpcNml_c *>(self->getNpc(mTalkIdx));
        BOOL started = FALSE;
        if (npc != NULL && npc->mpAnimal != NULL) {
            u8 *notes = npc->mpAnimal->mMelody.getNotes();
            fn_8000FC98(npc->getSoundId(), notes);
            setActProc(&talk_c::actWaitTuneStart);
            started = TRUE;
        }
        if (!started) {
            dDemo_c *ctrl = getController();
            if (ctrl != NULL) {
                ctrl->mLock = 0;
            }
            clearActProc();
        }
    }
}

// 80032F7C
void dAcNpcNml_c::talk_c::actWaitTuneStart() {
    if (fn_8000FBF4()) {
        setActProc(&talk_c::actWaitTuneEnd);
    }
}

// 80032FD8
void dAcNpcNml_c::talk_c::actWaitTuneEnd() {
    if (!fn_8000FBF4()) {
        dDemo_c *ctrl = getController();
        if (ctrl != NULL) {
            ctrl->mLock = 0;
        }
        clearActProc();
    }
}

// 80033028
const char *dAcNpcNml_c::talk_c::getLabelByTable(int table, u32 kind, u32 idx, u32 sub) {
    const char *label = NULL;
    switch (table) {
    case 0x0:
        label = getMsgLabel(kind, idx, sub);
        break;
    case 0x10:
        label = getApproachLabel(kind, idx, sub);
        break;
    case 0x14:
        label = get3PLabel(kind);
        break;
    }
    return label;
}

// 800330AC
const char *dAcNpcNml_c::talk_c::getMsgLabel(u32 kind, u32 idx, u32 sub) {
    const char *label = NULL;
    switch (kind) {
    case TALK_REACTION:
        if (idx < 28) {
            label = l_reactionLabels[idx];
        }
        break;
    case TALK_ARBEIT:
        if (idx < 6) {
            label = l_arbeitLabels[idx];
        }
        break;
    case TALK_HARVEST:
        if (idx < 3) {
            label = l_harvestLabels[idx];
        }
        break;
    case TALK_HALLOWEEN:
        label = l_Ev_Halloween;
        break;
    case TALK_COUNTDOWN:
        if (idx < 7) {
            label = l_countdownLabels[idx];
        }
        break;
    case TALK_CARNIVAL:
        if (idx < 25) {
            label = l_carnivalLabels[idx];
        }
        break;
    case TALK_FISHING:
        if (idx < 7) {
            label = l_fishingLabels[idx];
        }
        break;
    case TALK_BUG:
        if (idx < 6) {
            label = l_bugLabels[idx];
        }
        break;
    case TALK_FIREWORKS:
        if (idx < 4) {
            label = l_fireworksLabels[idx];
        }
        break;
    case TALK_QUEST_DELIVERY:
        if (idx < 2) {
            label = l_Ai_Quest;
        }
        break;
    case TALK_QUEST:
        switch (idx) {
        case QUEST_TALK_ERRAND:
            if (sub < 6) {
                label = l_q06Labels[sub];
            }
            break;
        case QUEST_TALK_ERRAND_FINAL:
            if (sub < 6) {
                label = l_q07Labels[sub];
            }
            break;
        case QUEST_TALK_INSECT:
            if (sub < 8) {
                label = l_q01Labels[sub];
            }
            break;
        case QUEST_TALK_FISH:
            if (sub < 8) {
                label = l_q02Labels[sub];
            }
            break;
        case QUEST_TALK_FOSSIL:
            if (sub < 10) {
                label = l_q03Labels[sub];
            }
            break;
        case QUEST_TALK_CLOTH:
            if (sub < 16) {
                label = l_q04Labels[sub];
            }
            break;
        case QUEST_TALK_FTR:
            if (sub < 13) {
                label = l_q05Labels[sub];
            }
            break;
        case QUEST_TALK_LOST_KEY:
            if (sub < 7) {
                label = l_q12Labels[sub];
            }
            break;
        case QUEST_TALK_SICK:
            if (sub < 13) {
                label = l_q11Labels[sub];
            }
            break;
        case QUEST_TALK_STYLE:
            if (sub < 7) {
                label = l_q13Labels[sub];
            }
            break;
        case QUEST_TALK_HOUSE_VISIT:
            if (sub < 3) {
                label = l_q08Labels[sub];
            }
            break;
        case QUEST_TALK_INVITATION:
            if (sub < 4) {
                label = l_q09Labels[sub];
            }
            break;
        case QUEST_TALK_HIDE_AND_SEEK:
            if (sub < 1) {
                label = l_q10Labels[sub];
            }
            break;
        }
        break;
    case TALK_ROLLAN:
        label = l_Ai_Quest;
        break;
    case TALK_QUEST_OFFER:
        if (sub < 3) {
            label = l_aiqLabels[sub];
        }
        break;
    case TALK_GREETING:
        if (idx < 16) {
            label = l_aiGreetLabels[idx];
        }
        break;
    case TALK_FREE:
        switch (idx) {
        case 0:
            if (sub < 7) {
                label = l_freeALabels[sub];
            }
            break;
        case 1:
            if (sub < 7) {
                label = l_freeASeasonLabels[sub];
            }
            break;
        case 3:
            if (sub < 8) {
                label = l_freeBLabels[sub];
            }
            break;
        case 4:
            if (sub < 4) {
                label = l_apcLabels[sub];
            }
            break;
        case 5:
            if (sub < 1) {
                label = l_freeDLabels[sub];
            }
            break;
        case 6:
            if (sub < 2) {
                label = l_freeELabels[sub];
            }
            break;
        case 7:
            if (sub < 4) {
                label = l_freeFLabels[sub];
            }
            break;
        case 8:
            if (sub < 3) {
                label = l_freeGLabels[sub];
            }
            break;
        case 2:
            if (sub < 6) {
                label = l_freeHLabels[sub];
            }
            break;
        case 9:
            if (sub < 2) {
                label = l_freeILabels[sub];
            }
            break;
        }
        break;
    }
    return label;
}

static dItem::Item l_filterItem; // the item filterItem looks for
static int l_filterFlag;         // its flag argument

// 800334CC
BOOL dAcNpcNml_c::talk_c::filterItem(const dItem::Item *item, int flag) {
    if (flag == l_filterFlag && item->isSame(l_filterItem)) {
        return TRUE;
    }
    return FALSE;
}

// 80033510
int dAcNpcNml_c::talk_c::countPocketsItem(const dItem::Item *item, int flag, u16 *mask, dPrivateData_c *player) {
    int count = 0;
    if (mask != NULL) {
        *mask = 0;
    }
    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayer();
    }
    if (player != NULL) {
        l_filterFlag = flag;
        l_filterItem = *item;
        u32 num = player->countPocketsFlag(filterItem, mask);
        if (num <= 15) {
            count = num;
        }
    }
    return count;
}

// 800335B4
int dAcNpcNml_c::talk_c::countPocketsKind(u16 *mask, int kind, dPrivateData_c *player, BOOL noSaleToo) {
    int count = 0;
    u16 dummy = 0;
    if (mask == NULL) {
        mask = &dummy;
    }
    *mask = 0;
    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayer();
    }
    if (player != NULL) {
        dItem::Item *pockets = player->mPockets;
        for (int i = 0; i < 15; i++) {
            if (pockets[i].isValid() && !player->getPocketFlag(i)) {
                const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(pockets[i]);
                if (bitm != NULL && kind == bitm->getKind() && (noSaleToo || !bitm->m_noPurchase)) {
                    *mask |= 1 << i;
                    count++;
                }
            }
        }
    }
    return count;
}

// 800336C8
BOOL dAcNpcNml_c::talk_c::filterUsed(const dItem::Item *item, int flag) {
    if (item->isValid() && flag == 0) {
        return TRUE;
    }
    return FALSE;
}

// 800336EC
int dAcNpcNml_c::talk_c::countPocketsUsed(u16 *mask, dPrivateData_c *player) {
    int count = 0;
    if (mask != NULL) {
        *mask = 0;
    }
    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayer();
    }
    if (player != NULL) {
        u32 num = player->countPocketsFlag(filterUsed, mask);
        if (num < 15) {
            count = num;
        }
    }
    return count;
}

// 80033770
BOOL dAcNpcNml_c::talk_c::setMemoryText(const wchar_t *name) {
    memset(mMemoryText, 0, sizeof(mMemoryText));
    if (name != NULL) {
        u32 len = dScript::getStringLength(name, 16, 0);
        if (len > 16) {
            len = 16;
        }
        memcpy(mMemoryText, name, len * sizeof(wchar_t));
        return TRUE;
    }
    return FALSE;
}

// 804A0E48: entry procedure sets ([15] = free talk, the default). Defined here (after getMsgLabel's
// jumptables and the static l_filterItem, before setQ5Word) for the .data / __sinit order.
const dAcNpcNml_c::talk_c::procSet_s l_talkEntrySets[dAcNpcNml_c::talk_c::TALK_NUM] = {
    {&dAcNpcNml_c::talk_c::msgReaction, &dAcNpcNml_c::talk_c::endReaction, NULL}, // 0 talk
    {&dAcNpcNml_c::talk_c::msgArbeit, &dAcNpcNml_c::talk_c::endArbeit, NULL}, // 1 Arbeit
    {&dAcNpcNml_c::talk_c::msgHalloween, &dAcNpcNml_c::talk_c::endHalloween, NULL}, // 2 Halloween
    {&dAcNpcNml_c::talk_c::msgCountdown, &dAcNpcNml_c::talk_c::endCountdown, NULL}, // 3 Countdown
    {&dAcNpcNml_c::talk_c::msgCarnival, &dAcNpcNml_c::talk_c::endCarnival, NULL}, // 4 Carnival
    {&dAcNpcNml_c::talk_c::msgDelivery, &dAcNpcNml_c::talk_c::endQuestCommon, NULL}, // 5 QuestDelivery
    {&dAcNpcNml_c::talk_c::msgQuestTalk, &dAcNpcNml_c::talk_c::endQuestTalk, NULL}, // 6 talk
    {&dAcNpcNml_c::talk_c::msgRollan, &dAcNpcNml_c::talk_c::endRollan, &dAcNpcNml_c::talk_c::stepRollan}, // 7 Rollan
    {&dAcNpcNml_c::talk_c::msgQuestOffer, &dAcNpcNml_c::talk_c::endQuestOffer, NULL}, // 8 talk
    {&dAcNpcNml_c::talk_c::msgRecall, &dAcNpcNml_c::talk_c::endRecall, NULL}, // 9 talk
    {&dAcNpcNml_c::talk_c::msgHarvest, &dAcNpcNml_c::talk_c::endHarvest, &dAcNpcNml_c::talk_c::stepHarvest}, // 10 Harvest
    {&dAcNpcNml_c::talk_c::msgFishing, &dAcNpcNml_c::talk_c::endFishing, NULL}, // 11 Fishing
    {&dAcNpcNml_c::talk_c::msgBugOff, &dAcNpcNml_c::talk_c::endBugOff, NULL}, // 12 Bug
    {&dAcNpcNml_c::talk_c::msgFireworks, &dAcNpcNml_c::talk_c::endFireworks, NULL}, // 13 Fireworks
    {&dAcNpcNml_c::talk_c::msgGreeting, &dAcNpcNml_c::talk_c::endGreeting, &dAcNpcNml_c::talk_c::stepGreeting}, // 14 talk
    {&dAcNpcNml_c::talk_c::msgFree, &dAcNpcNml_c::talk_c::endFree, NULL}, // 15 Free
};

// 800337F8
int dAcNpcNml_c::talk_c::msgEntry(msgInfo_s *info) {
    const procSet_s *set = l_talkEntrySets;
    int ret = 0;
    for (int i = 0; i < TALK_NUM; i++, set++) {
        if (set->mMsg != NULL) {
            ret = (this->*set->mMsg)(info);
            if (ret != 0) {
                break;
            }
        }
    }
    return ret;
}

// 80033880
const dAcNpcNml_c::talk_c::procSet_s *dAcNpcNml_c::talk_c::getEventProcSet() {
    static setFunc const l_setFuncs[3] = {
        &talk_c::getFishingProcSet,
        &talk_c::getBugOffProcSet,
        &talk_c::getFireworksProcSet,
    };
    for (u32 i = 0; i < 3; i++) {
        if (l_setFuncs[i] != NULL) {
            const procSet_s *set = (this->*l_setFuncs[i])();
            if (set != NULL) {
                return set;
            }
        }
    }
    return NULL;
}

// 80033904
const dAcNpcNml_c::talk_c::msgFunc dAcNpcNml_c::talk_c::getReactionMsg(int idx) {
    static const msgFunc l_reactions[28] = {
        &talk_c::msgReMoveout,
        &talk_c::msgReCafe,
        &talk_c::msgReFishing,
        &talk_c::msgReFall,
        &talk_c::msgReRun,
        &talk_c::msgEvFirst,
        &talk_c::msgReFirstA1,
        &talk_c::msgReFirstA2,
        &talk_c::msgReFirstB1,
        &talk_c::msgReFirstB2,
        &talk_c::msgReFirstC1,
        &talk_c::msgReFirstC2,
        &talk_c::msgReFirstV,
        &talk_c::msgReMovein,
        &talk_c::msgReBirthday,
        &talk_c::msgRe30days,
        &talk_c::msgRe7days,
        &talk_c::msgReTire,
        &talk_c::msgReAnger,
        &talk_c::msgReSad,
        &talk_c::msgReBeeFace,
        &talk_c::msgRePoison,
        &talk_c::msgReXmas,
        &talk_c::msgReNewyear,
        &talk_c::msgReHarvest,
        &talk_c::msgReHalloween,
        &talk_c::msgReFireworks,
        &talk_c::msgReDownload,
    };

    if ((u32)idx < 28) {
        return l_reactions[idx];
    }
    return NULL;
}

// 80033958
int dAcNpcNml_c::talk_c::msgReaction(msgInfo_s *info) {
    int found = 28;
    setProcSet(&l_talkEntrySets[TALK_REACTION]);
    for (int i = 0; i < 28; i++) {
        msgFunc func = getReactionMsg(i);
        if (func != NULL && (this->*func)(info)) {
            found = i;
            break;
        }
    }
    if (found < 28) {
        dNpc::msgMemorySecond_c *mem = getRememberedMsg();
        if (mem != NULL) {
            mem->clear();
        }
        setTopic(dNpc::msgMemory_c::KIND_REACTION, found, 0);
        return TRUE;
    }
    return FALSE;
}

// 80033A60
void dAcNpcNml_c::talk_c::endReaction(int arg) {
    recordTalk(0);
    if (mCountTalk) {
        if (mpMemory != NULL) {
            mpMemory->mTalkCount.inc(0x44);
        }
        mCountTalk = 0;
    }
    dNpc::msgMemorySecond_c *mem = getRememberedMsg();
    if (mem != NULL) {
        mem->clear();
    }
}

// 80033ADC
BOOL dAcNpcNml_c::talk_c::stepReaction(int kind) {
    dDemo_c *ctrl = getController();
    if (ctrl != NULL && ctrl->mNextMsgCode == 0) {
        dNpcEntry_c *entry = getEntry();
        dQuestBase_c *quest = entry != NULL ? &entry->mQuest : NULL;
        if (quest != NULL) {
            quest->clear();
        }
        stepBeeFaceSeen(kind);
    }
    return FALSE;
}

// 80033B5C
int dAcNpcNml_c::talk_c::countPocketsFossil(u16 *mask, dQuestVillager_c *quest, dPrivateData_c *player) {
    int count = 0;
    if (player == NULL) {
        player = dPlayerMgr_c::getCurrentPlayer();
    }
    u16 dummy = 0;
    if (mask == NULL) {
        mask = &dummy;
    }
    if (player != NULL && quest->mBase.isActive() && (int)quest->mBase.mKind == QUEST_KIND_REQUEST_FOSSIL) {
        int mode = quest->mMatchMode;
        const dItem::Item *want = &quest->mBase.mItem;
        for (int i = 0; i < 15; i++) {
            dItem::Item *item = &player->mPockets[i];
            if (item->isValid() && !player->getPocketFlag(i) && isFossilRequestMatch(item, mode, want)) {
                *mask |= 1 << i;
                count++;
            }
        }
    }
    return count;
}

// 80033C54
void dAcNpcNml_c::talk_c::setQ4Word(int mode, int idx) {
    dAnimal_c *animal = getAnimal();
    if (animal != NULL) {
        switch (mode) {
        case QUEST_MATCH_LIKED_STYLE: {
            s8 look = animal->mTemplate.getLikedStyle();
            dItem::nameLookQ4_c name;
            setQ4LookName(&name, look);
            getController()->setWord(idx, &name);
            break;
        }
        }
    }
}

// 80033CF0
void dAcNpcNml_c::talk_c::setQ5Word(int mode, u32 value, int idx) {
    switch (mode) {
    case QUEST_MATCH_FTR_CATEGORY: {
        dItem::nameCategoryQ5_c name;
        setQ5PartName(&name, value);
        getController()->setWord(idx, &name);
        break;
    }
    case QUEST_MATCH_FTR_COLOR: {
        u32 color = 1;
        if (value != 0 && value < 15) {
            color = value;
        }
        dString::Word_c name(color, "sys_STRING/STR_Q05_Color");
        getController()->setWord(idx, &name);
        break;
    }
    case QUEST_MATCH_FTR_IMAGE: {
        u32 image = 0;
        if (value < 4) {
            image = value;
        }
        dString::Word_c name(image + 1, "sys_STRING/STR_Q05_Image");
        getController()->setWord(idx, &name);
        break;
    }
    case QUEST_MATCH_FTR_SERIES: {
        dItem::nameSeriesQ5_c name;
        setSeriesName(&name, value);
        getController()->setWord(idx, &name);
        break;
    }
    }
}

// ---- talk_c 80033E68..80035C08 ----
typedef dAcNpcNml_c::talk_c talk_c;

// 80033E68
int dAcNpcNml_c::talk_c::msgQuestTalk(msgInfo_s *info) {
    static const msgFunc l_questTalk[QUEST_TALK_NUM] = {
        &talk_c::msgErrandQuest, &talk_c::msgErrandFinalQuest,
        &talk_c::msgInsectQuest, &talk_c::msgFishQuest,
        &talk_c::msgFossilQuest, &talk_c::msgClothQuest,
        &talk_c::msgFtrQuest, &talk_c::msgLostKeyQuest,
        &talk_c::msgSickQuest, &talk_c::msgStyleQuest,
        &talk_c::msgVisitQuest, &talk_c::msgInviteQuest,
        &talk_c::msgHideQuest,
    };
    for (int i = 0; i < QUEST_TALK_NUM; i++) {
        if (l_questTalk[i] && (this->*l_questTalk[i])(info)) {
            return TRUE;
        }
    }
    return FALSE;
}

// 80033F00
void dAcNpcNml_c::talk_c::endQuestTalk(int arg) {
    recordTalk(NULL);
    dNpc::msgMemorySecond_c *mem = getRememberedMsg();
    if (mem != NULL) {
        mem->clear();
    }
    dNpcEntry_c *entry = getEntry();
    if (entry != NULL) {
        entry->mMsg.set(mMessageLabel, mMessageCode, mTopicKind, mTopicGroup, mTopicIdx);
    }
    if (mCountTalk) {
        if (mpMemory != NULL) {
            mpMemory->mTalkCount.inc(0x44);
        }
        mCountTalk = 0;
    }
}

// 80033FA4
BOOL dAcNpcNml_c::talk_c::stepErrandOver(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        dQuestErrand_c *errand = player != NULL ? player->mErrand.get(0) : NULL;
        dQuestBase_c *quest = errand != NULL ? &errand->mBase : NULL;
        u16 mask = 0;
        if (quest != NULL && quest->mItem.isValid() && countPocketsItem(&quest->mItem, 2, &mask, player)) {
            for (int i = 0; i < 15; i++) {
                if ((mask >> i) & 1) {
                    player->clearPocket(i);
                    break;
                }
            }
            requestItemActEx(6, &quest->mItem, 2, 0, 0, 2);
            setMsgProc(&talk_c::msgPayback);
            addFriendship(-3);
        } else {
            setMsgProc(&talk_c::msgLost);
            addFriendship(-5);
        }
        startMsg();
        if (errand != NULL) {
            errand->clear();
        }
        return TRUE;
    }
    return FALSE;
}

// 80034164
int dAcNpcNml_c::talk_c::msgPayback(msgInfo_s *info) {
    setLooksMsg(info, l_Q_Payback, 0);
    return TRUE;
}

// 80034194
int dAcNpcNml_c::talk_c::msgLost(msgInfo_s *info) {
    setLooksMsg(info, l_Q_Lost, 0);
    return TRUE;
}

// 800341C0
int dAcNpcNml_c::talk_c::msgTimeover2(msgInfo_s *info) {
    setLooksMsg(info, l_Q_Timeover2, 0);
    setStepProc(&talk_c::stepClearErrand);
    return TRUE;
}

// 80034224
BOOL dAcNpcNml_c::talk_c::stepClearErrand(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
        dQuestErrand_c *errand = player != NULL ? player->mErrand.get(0) : NULL;
        if (errand != NULL) {
            errand->clear();
        }
        return TRUE;
    }
    return FALSE;
}

// 80034290
void dAcNpcNml_c::talk_c::selResumeTalk() {
    const procSet_s *set = getEventProcSet();
    if (set == NULL) {
        set = &l_talkEntrySets[TALK_FREE];
    }
    setProcSet(set);
    startMsg();
}

// 800342E4
int dAcNpcNml_c::talk_c::msgCancel(msgInfo_s *info) {
    setLooksMsg(info, l_Q_Cancel, 0);
    return TRUE;
}

// Labels of the quest offer topic (getMsgLabel(8, ..) of startQuestOffer: approach, first, again).
const char *l_aiqLabels[3] = {"AiQ_Approach", "AiQ_First", "AiQ_Again"};

// 80034314
int dAcNpcNml_c::talk_c::startQuestOffer(msgInfo_s *info, int labelIdx, endFunc hook, stepFunc step, BOOL checkTalk) {
    int sub = 3;
    BOOL busy = FALSE;
    if (mpNpc != NULL && static_cast<dAcNpcNml_c *>(mpNpc)->vtCC()) {
        busy = TRUE;
    }
    dNpcEntry_c *entry = getEntry();
    if (busy) {
        sub = 0;
    } else if (checkTalk) {
        if (mpMemory != NULL && mpMemory->mTalkCount.mCount == 0) {
            sub = 1;
        } else {
            BOOL flag = entry != NULL ? entry->_28C.mTalked != 0 : TRUE;
            if (flag) {
                sub = 2;
            }
        }
    }
    setProcSet(&l_talkEntrySets[TALK_QUEST_OFFER]);
    switch (sub) {
    case 0:
    case 1:
    case 2: {
        const char *label = getMsgLabel(TALK_QUEST_OFFER, labelIdx, sub);
        if (label != NULL) {
            setLooksMsg(info, label, 0);
            setStepProc(step);
            setTopic(dNpc::msgMemory_c::KIND_QUEST, labelIdx, 0);
            return TRUE;
        }
        break;
    }
    default:
        if (entry != NULL) {
            entry->_28C.mTalked = 0;
        }
        msgGreeting(info);
        setTopic(dNpc::msgMemory_c::KIND_QUEST, labelIdx, 0);
        setHookProc(hook);
        setStepProc(step);
        return TRUE;
    }
    return FALSE;
}

// 8003451C
BOOL dAcNpcNml_c::talk_c::isEventOngoing() {
    static const int l_events[3] = {EVENT_HALLOWEEN, EVENT_COUNTDOWN, EVENT_FESTIVALE};
    for (u32 i = 0; i < 3; i++) {
        if (dEvent::isOngoing((dQuestEvent_e)l_events[i])) {
            return TRUE;
        }
    }
    return FALSE;
}

// 80034588
// The player's errand, through the const accessor (dQuestErrandList_c::get const).
static inline const dQuestErrand_c *getErrand(const dPrivateData_c *player) {
    return player->mErrand.get(0);
}

int dAcNpcNml_c::talk_c::startErrandOffer(msgInfo_s *info, int questKind, int labelIdx, stepFunc step) {
    if (dAcNpc_c::isMultiPlay()) {
        return FALSE;
    }
    dAnimal_c *animal = getAnimal();
    if (animal == NULL) {
        return FALSE;
    }
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL || player->isFlag0(0xD)) {
        return FALSE;
    }
    if (!dQuestBase_c::checkEventSchedule(questKind, NULL)) {
        return FALSE;
    }
    if (!dQuestBase_c::checkTodayEvents(questKind)) {
        return FALSE;
    }
    if (!player->mPID.isFromTown()) {
        return FALSE;
    }
    const dQuestErrand_c *errand = player != NULL ? getErrand(player) : NULL;
    if (errand == NULL || errand->mBase.isActive()) {
        return FALSE;
    }
    if (player->findEmptyPocket(0) == -1) {
        return FALSE;
    }
    const dAnmPersonalID_c *exclude = &animal->mID;
    dSaveTown_c *town = dSaveData_c::getTown();
    dAnimal_c *other = town->mAnimals.mTown.pickRandomAvailableAnimal(&exclude, 1);
    if (other != NULL && other->mID.isValid()) {
        return startQuestOffer(info, labelIdx, &talk_c::endQuestCommon, step, TRUE);
    }
    return FALSE;
}

// 80034750
int dAcNpcNml_c::talk_c::startRequestOffer(msgInfo_s *info, int labelIdx, int questKind, stepFunc step) {
    if (dAcNpc_c::isMultiPlay()) {
        return FALSE;
    }
    dAnimal_c *animal = getAnimal();
    if (animal == NULL) {
        return FALSE;
    }
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL || player->isFlag0(0xD)) {
        return FALSE;
    }
    if (!player->mPID.isFromTown()) {
        return FALSE;
    }
    if (animal->mQuest.mQuest.mBase.isActive()) {
        return FALSE;
    }
    int pick = animal->pickRequestItem(NULL, NULL, questKind);
    if (pick >= 7) {
        return FALSE;
    }
    if (!dQuestBase_c::checkEventSchedule(questKind, NULL)) {
        return FALSE;
    }
    if (!dQuestBase_c::checkTodayEvents(questKind)) {
        return FALSE;
    }
    return startQuestOffer(info, labelIdx, &talk_c::endQuestCommon, step, TRUE);
}

// 800348C0
int dAcNpcNml_c::talk_c::msgPendingQuest(msgInfo_s *info, f32 chance) {
    struct questTalk_s {
        msgFunc mProc;
        u8 mKind;
    };
    static const questTalk_s l_questOffer[11] = {
        {&talk_c::msgErrandOffer, 0x07}, {&talk_c::msgErrandFinalOffer, 0x08},
        {&talk_c::msgInsectOffer, 0x00}, {&talk_c::msgFishOffer, 0x01},
        {&talk_c::msgFossilOffer, 0x02}, {&talk_c::msgClothOffer, 0x03},
        {&talk_c::msgFtrOffer, 0x04}, {&talk_c::msgStyleOffer, 0x13},
        {&talk_c::msgVisitOffer, 0x12}, {&talk_c::msgInviteOffer, 0x11},
        {&talk_c::msgHideOffer, 0x14},
    };
    if (cM::rndF(100.0f) < chance) {
        dNpcEntry_c *entry = getEntry();
        dQuestBase_c *quest = entry != NULL ? &entry->mQuest : NULL;
        if (quest != NULL && quest->isActive()) {
            for (int i = 0; i < 11; i++) {
                if (l_questOffer[i].mKind == quest->getKind()) {
                    if (l_questOffer[i].mProc) {
                        return (this->*l_questOffer[i].mProc)(info);
                    }
                    break;
                }
            }
        }
    }
    return FALSE;
}

// 800349C0
int dAcNpcNml_c::talk_c::msgQuestOffer(msgInfo_s *info) {
    dNpcEntry_c *entry = getEntry();
    f32 chance = (entry != NULL ? entry->_28C.mTalked != 0 : TRUE) ? 10.0f : 20.0f;
    return msgPendingQuest(info, chance);
}

// 80034A3C
void dAcNpcNml_c::talk_c::endQuestOffer(int arg) {
    recordTalk(NULL);
    dNpc::msgMemorySecond_c *mem = getRememberedMsg();
    if (mem != NULL) {
        mem->clear();
    }
    dNpcEntry_c *entry = getEntry();
    if (entry != NULL) {
        entry->mMsg.set(mMessageLabel, mMessageCode, mTopicKind, mTopicGroup, mTopicIdx);
    }
    if (mpMemory != NULL) {
        mpMemory->mTalkCount.inc(0x44);
    }
}

// 80034ACC
int dAcNpcNml_c::talk_c::msgYes(msgInfo_s *info) {
    u16 code = 4;
    switch (mErrandDeadline) {
    case QUEST_DEADLINE_NEXT_PERIOD:
        code = 5;
        break;
    case QUEST_DEADLINE_MIDNIGHT:
        code = 6;
        break;
    }
    setLooksMsg(info, l_Q_Yes, code);
    return TRUE;
}

// 80034B20
void dAcNpcNml_c::talk_c::selNo() {
    setMsgProc(&talk_c::msgNo);
    startMsg();
}

// 80034B74
int dAcNpcNml_c::talk_c::msgNo(msgInfo_s *info) {
    setLooksMsg(info, l_Q_No, 0);
    return TRUE;
}

// 80034BA0
void dAcNpcNml_c::talk_c::selRequestNo() {
    dAnimal_c *animal = getAnimal();
    if (animal != NULL) {
        dQuestVillager_c *quest = &animal->mQuest.mQuest;
        u32 num = quest->countPlayers(TRUE);
        if (num == 1) {
            quest->clear();
        } else {
            dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
            if (player != NULL) {
                quest->clearPlayerFlag(&player->mPID.player);
            }
        }
    }
    setMsgProc(&talk_c::msgRequestNo);
    startMsg();
}

// 80034C4C
int dAcNpcNml_c::talk_c::msgRequestNo(msgInfo_s *info) {
    setLooksMsg(info, l_Q_No, 0);
    return TRUE;
}

// 80034C78
BOOL dAcNpcNml_c::talk_c::stepRequestChoice(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        clearChoice();
        setChoice(0, 0x15, 10, &talk_c::selRequestYes);
        setChoice(1, 0x1F, 10, &talk_c::selRequestNo);
        setChoiceNum(2);
        setChoiceCancel(1);
        showChoice();
        dNpcEntry_c *entry = getEntry();
        dQuestBase_c *quest = entry != NULL ? &entry->mQuest : NULL;
        if (quest != NULL) {
            quest->clear();
        }
        return TRUE;
    }
    return FALSE;
}

// 80034D78
void dAcNpcNml_c::talk_c::selRequestYes() {
    dAnimal_c *animal = getAnimal();
    if (animal != NULL) {
        u32 num = animal->mQuest.mQuest.countPlayers(TRUE);
        if (num == 1) {
            dTime_c *now = dTime_c::getCurrent();
            dQuestBase_c *quest = &animal->mQuest.mQuest.mBase;
            quest->setTimeLimit(*now);
            quest->mDeadline = 3;
        }
    }
    setMsgProc(&talk_c::msgRequestYes);
    startMsg();
}

// 80034E18
int dAcNpcNml_c::talk_c::msgRequestYes(msgInfo_s *info) {
    static const char *l_yesLabels[4] = {"Q_Yes1", "Q_Yes2", "Q_Yes34", "Q_Yes34"};
    dAnimal_c *animal = getAnimal();
    u32 num = animal != NULL ? animal->mQuest.mQuest.countPlayers(FALSE) : 0;
    if (num == 0 || num > 4) {
        num = 1;
    }
    setLooksMsg(info, l_yesLabels[num - 1], 0);
    setStepProc(&talk_c::stepRequestYes);
    return TRUE;
}

// 80034ED0
BOOL dAcNpcNml_c::talk_c::stepRequestYes(int kind) {
    mpNpc->mAudioObj.startSound(0x171E);
    return TRUE;
}

inline dSaveTownListEntry_c *dAcNpcNml_c::talk_c::getJinxTown() {
    if ((u32)mJinxTown < SAVE_TOWN_LIST_NUM) {
        dSaveTown_c *town = dSaveData_c::getTown();
        return town->mTownList.getTown(mJinxTown);
    }
    return NULL;
}

static inline const dSaveJinx_c *getJinx(dSaveTownListEntry_c *town) {
    return town != NULL ? &town->mJinx : &dSaveData_c::getTown()->mJinx;
}

// 80034F08
int dAcNpcNml_c::talk_c::msgRecall(msgInfo_s *info) {
    u16 code = 0;
    const char *label = getRememberedLabel(&code, dNpc::msgMemory_c::KIND_ANY);
    dAnimal_c *animal = getAnimal();
    if (label != NULL && code != 0) {
        info->mLabel = label;
        info->mCode = code;
        setProcSet(&l_talkEntrySets[TALK_RECALL]);
        dSaveOtherTown_c *otherTown;
        u32 fashion;
        const wchar_t *design;
        dPersonalID_c *creator;
        dPersonalID_c *personal;
        dItem::Item *item0;
        dPlayerID_c *playerID;
        u8 topic;
        dNpc::msgMemory_c::kind_e kind;
        dNpc::msgMemorySecond_c *mem = getRememberedMsg();
        if (mem != NULL) {
            setTopicFrom(mem);
            topic = mem->mTopicGroup;
            u8 sub = mem->mTopicIdx;
            kind = (dNpc::msgMemory_c::kind_e)mem->mKind;
            switch (kind) {
            case dNpc::msgMemory_c::KIND_REACTION:
                switch (topic) {
                case 8:
                case 9:
                case 10:
                case 11: {
                    dLandID_c *land = &mem->mLand;
                    if (land->isValid()) {
                        setLandName(land, 0);
                        mMsgLand.copy(land);
                    }
                    break;
                }
                case 12:
                    setPlayerLandWord();
                    break;
                }
                break;
            case dNpc::msgMemory_c::KIND_FREE:
                switch (topic) {
                case 0:
                    switch (sub) {
                    case 4:
                        setMemoryWords(&mem->mPersonal, &mem->mLand, mem->_AC, &mem->mItem0, animal);
                        break;
                    case 6: {
                        mJinxTown = mem->_A0;
                        dSaveTownListEntry_c *town = getJinxTown();
                        const dSaveJinx_c *jinx = getJinx(town);
                        const dLandID_c *land = town != NULL ? &town->mLand : NULL;
                        if (jinx != NULL) {
                            setJinxWords(jinx, land);
                        }
                        setStepProc(&talk_c::stepJinx);
                        break;
                    }
                    case 5: {
                        mRumorTown = mem->_A4;
                        u32 idx = mRumorTown;
                        if (idx < 4) {
                            otherTown = &dSaveData_c::getRaw()->mOtherTowns[idx];
                            if (otherTown->mLand.isValid()) {
                                setLandName(&otherTown->mLand, 0);
                                dPersonalID_c *player = &otherTown->mPlayer;
                                if (player->isValid()) {
                                    setPersonalName(player, 1);
                                    if (animal != NULL) {
                                        u8 gender = player->player.mGender;
                                        u16 unit = fn_800F3F38(gender, animal->mID.getLooks(1));
                                        if (unit == 0) {
                                            clearWord(2);
                                        } else {
                                            getController()->fn_801A5874(2, unit, "sys_STRING/STR_Unit");
                                        }
                                    }
                                }
                                if (otherTown->mImpression < 0x31) {
                                    getController()->fn_801A5874(3, otherTown->mImpression + 1, "sys_STRING/STR_Impress");
                                }
                                dAnmPersonalID_c *villager = (dAnmPersonalID_c *)otherTown->mVillager;
                                if (villager->isValid()) {
                                    setAnmPersonalName(villager, 4);
                                    if (animal != NULL) {
                                        u16 unit = fn_800F3F38(villager->getGender(1), animal->mID.getLooks(1));
                                        if (unit == 0) {
                                            clearWord(5);
                                        } else {
                                            getController()->fn_801A5874(5, unit, "sys_STRING/STR_Unit");
                                        }
                                    }
                                }
                            }
                        }
                        break;
                    }
                    case 0: {
                        dAnimalMemory_c *memory = mpMemory;
                        if (memory != NULL) {
                            const dItem::Item *item = &memory->mLastClothes;
                            if (item->isValid()) {
                                setItemName(item, 0);
                                fashion = item->getFashion();
                                dItem::nameFashion_c fashionName;
                                setFashionName(&fashionName, fashion);
                                getController()->setWord(1, &fashionName);
                                int look = item->getStyle();
                                dItem::nameLook_c lookName;
                                setLookName(&lookName, look);
                                getController()->setWord(2, &lookName);
                            }
                        }
                        break;
                    }
                    case 2:
                        if (animal != NULL) {
                            creator = &mem->mPersonal;
                            design = mem->mText;
                            if (creator->isValid()) {
                                setDesignWords(creator, design, animal);
                            }
                        }
                        break;
                    }
                    break;
                case 8:
                    switch (sub) {
                    case 2: {
                        const dSaveJinx_c *jinx = getJinx(NULL);
                        if (jinx != NULL) {
                            setJinxWords(jinx, NULL);
                        }
                        setStepProc(&talk_c::stepJinx);
                        break;
                    }
                    case 1:
                        setHostWords();
                        break;
                    }
                    break;
                case 7:
                    switch (sub) {
                    case 2: {
                        personal = &mem->mPersonal;
                        if (personal->isValid()) {
                            setPersonalName(personal, 0);
                            if (animal != NULL) {
                                u8 gender = personal->player.mGender;
                                u16 unit = fn_800F3F38(gender, animal->mID.getLooks(1));
                                if (unit == 0) {
                                    clearWord(1);
                                } else {
                                    getController()->fn_801A5874(1, unit, "sys_STRING/STR_Unit");
                                }
                            }
                            mMsgPersonal.copy(personal);
                        }
                        break;
                    }
                    }
                    break;
                case 3:
                    switch (sub) {
                    case 0: {
                        item0 = &mem->mItem0;
                        if (item0->isValid()) {
                            setItemName(item0, 0);
                            mItem4 = *item0;
                        }
                        dItem::Item *item1 = &mem->mItem1;
                        if (item1->isValid()) {
                            setItemName(item1, 1);
                            mItem5 = *item1;
                        }
                        break;
                    }
                    case 2: {
                        item0 = &mem->mItem0;
                        if (item0->isValid()) {
                            setItemName(item0, 0);
                            mItem4 = *item0;
                        }
                        dItem::Item *item1 = &mem->mItem1;
                        if (item1->isValid()) {
                            setItemName(item1, 1);
                            mItem5 = *item1;
                        }
                        break;
                    }
                    case 3: {
                        item0 = &mem->mItem0;
                        if (item0->isValid()) {
                            setItemName(item0, 0);
                            mItem4 = *item0;
                        }
                        break;
                    }
                    }
                    break;
                case 2:
                    switch (sub) {
                    case 2: {
                        int year = mem->_A8;
                        if (year >= 0) {
                            setNumber4(year, 0);
                            mMsgYear = year;
                        }
                        break;
                    }
                    }
                    break;
                }
                break;
            case dNpc::msgMemory_c::KIND_QUEST:
                switch (topic) {
                case QUEST_TALK_STYLE: {
                    playerID = &mem->mPlayer;
                    if (playerID->isValid()) {
                        setPlayerName(playerID, 2);
                        if (animal != NULL) {
                            u8 gender = playerID->mGender;
                            u16 unit = fn_800F3F38(gender, animal->mID.getLooks(1));
                            if (unit == 0) {
                                clearWord(3);
                            } else {
                                getController()->fn_801A5874(3, unit, "sys_STRING/STR_Unit");
                            }
                        }
                        mMsgPlayer.copy(playerID);
                    }
                    break;
                }
                case QUEST_TALK_ERRAND:
                case QUEST_TALK_ERRAND_FINAL: {
                    const dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
                    const dQuestErrand_c *errand = player != NULL ? player->mErrand.get(0) : NULL;
                    const dQuestBase_c *quest = errand != NULL ? &errand->mBase : NULL;
                    if (quest != NULL && quest->isActive()) {
                        dAnmPersonalID_c *sender = errand->getAnimal(1);
                        if (animal != NULL && sender->isValid()) {
                            if (*sender == animal->mID) {
                                dAnmPersonalID_c *recipient = errand->getAnimal(0);
                                if (recipient->isValid()) {
                                    setAnmPersonalName(recipient, 2);
                                    u16 unit = fn_800F3F38(recipient->getGender(1), animal->mID.getLooks(1));
                                    if (unit == 0) {
                                        clearWord(3);
                                    } else {
                                        getController()->fn_801A5874(3, unit, "sys_STRING/STR_Unit");
                                    }
                                }
                            }
                        }
                    }
                    break;
                }
                }
                break;
            }
        }
        return TRUE;
    }
    return FALSE;
}

// 800357DC
void dAcNpcNml_c::talk_c::endRecall(int arg) {
    recordTalk(NULL);
    dNpcEntry_c *entry = getEntry();
    if (entry != NULL) {
        entry->mMsg.set(mMessageLabel, mMessageCode, mTopicKind, mTopicGroup, mTopicIdx);
    }
}

// Greeting labels of msgGreeting (getMsgLabel(TALK_GREETING, idx, 0)): snow / rain / fine x first talk / again,
// field, house x3, then the same for a visitor.
const char *l_aiGreetLabels[16] = {
    "Ai_Snow1",  "Ai_Rain1",  "Ai_Fine1",  "Ai_Snow2",  "Ai_Rain2",  "Ai_Fine2",  "Ai_Field3", "Ai_House1",
    "Ai_House2", "Ai_House3", "AiV_Snow1", "AiV_Rain1", "AiV_Fine1", "AiV_Snow2", "AiV_Rain2", "AiV_Fine2",
};

// 80035834
static inline BOOL isWeatherIn(int mode, int want) {
    return mode == want;
}

int dAcNpcNml_c::talk_c::msgGreeting(msgInfo_s *info) {
    dNpcEntry_c *entry = getEntry();
    BOOL flag = entry != NULL ? entry->_28C.mTalked != 0 : TRUE;
    if (!flag) {
        dAnimalMemory_c *memory = mpMemory;
        if (memory != NULL) {
            u8 scene = getCurrentScene();
            BOOL inTown = isSceneAttr(scene, SCENE_ATTR_TOWN);
            u8 count = memory->mTalkCount.get(scene);
            dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayerRaw();
            BOOL fromTown = player != NULL ? player->mPID.isFromTown() : FALSE;
            int idx;
            if (inTown) {
                dUnk8074EBE8_c *weatherMgr = lbl_8074EBE8;
                if (fromTown && count >= 5) {
                    idx = 6;
                } else {
                    int weather = 2;
                    if (weatherMgr != NULL) {
                        BOOL snow = FALSE;
                        int mode = weatherMgr->_5884;
                        if (isWeatherIn(mode, 6) || isWeatherIn(mode, 5)) {
                            snow = TRUE;
                        }
                        if (snow) {
                            weather = 0;
                        } else {
                            BOOL rain = FALSE;
                            if (isWeatherIn(mode, 4) || isWeatherIn(mode, 3)) {
                                rain = TRUE;
                            }
                            if (rain) {
                                weather = 1;
                            }
                        }
                    }
                    int base;
                    if (count == 0) {
                        if (fromTown) {
                            base = 0;
                        } else {
                            base = 10;
                        }
                    } else {
                        base = 3;
                        if (!fromTown) {
                            base = 13;
                        }
                    }
                    idx = base + weather;
                }
            } else if (fromTown && count >= 3) {
                idx = 9;
            } else {
                idx = 8;
                if (count == 0) {
                    idx = 7;
                }
            }
            setProcSet(&l_talkEntrySets[TALK_GREETING]);
            switch (idx) {
            case 7:
            case 8:
            case 9: {
                const stepFunc noStep = NULL;
                setStepProc(noStep);
                break;
            }
            }
            const char *label = getMsgLabel(TALK_GREETING, idx, 0);
            if (label != NULL) {
                setLooksMsg(info, label, 0);
                return TRUE;
            }
        }
    }
    return FALSE;
}

// 80035A58
void dAcNpcNml_c::talk_c::endGreeting(int arg) {
    recordTalk(NULL);
    if (mpMemory != NULL) {
        mpMemory->mTalkCount.inc(0x44);
    }
}

// 80035AA0
BOOL dAcNpcNml_c::talk_c::stepGreeting(int kind) {
    if (getController() != NULL && getController()->mNextMsgCode == 0) {
        setProcSet(&l_talkEntrySets[TALK_FREE]);
        startMsg();
        return TRUE;
    }
    return FALSE;
}

// 80035B04
void dAcNpcNml_c::talk_c::addFriendship(int delta) {
    dAnimal_c *animal = getAnimal();
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (animal != NULL && player != NULL) {
        dAnimalMemory_c *memory = animal->findMemory(&player->mPID);
        if (memory != NULL && memory->mPlayer.isValid()) {
            memory->addFriendship(delta);
        }
    }
}

// 80035B88
u32 dAcNpcNml_c::calcHeapSize(BOOL withFrm) {
    u32 size = fn_800F9D8C();
    size += ROUND_UP(isCurrentSceneAttr(SCENE_ATTR_TOWN) ? 0x5000 : 0x2800, 4);
    size += 0x9CC0;
    if (withFrm) {
        size += mHeap::frmHeapCost(0, 0x20);
    }
    return size;
}
