#include <game/game/d_menu_map.hpp>
#include <cstdio>
#include <game/game/d_animal.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_home.hpp>
#include <game/game/d_map.hpp>
#include <game/game/d_obj_cursor.hpp>
#include <game/game/d_obj_tutorial.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_save_building.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_string.hpp>
#include <game/snd/snd_id.hpp>

// The map picture shows the usable blocks: unit positions lose the border
// block, and the picture's centre is the origin of the icon panes.
enum {
    MAP_UNIT_CENTER = FG_BLOCK_X_NUM * UT_BASE_NUM / 2,
};

static const char *l_btnPushPane = "mapBtn_Psh";
static const char *l_allPanes[] = {"N_all", nullptr};
static const char *l_iconPushPane = "mapIcn_00_InPsh";
static const char *l_iconPane = "mapIcn_00";
static const char *l_playerPane = "player_00";

// Shows an icon's normal look, or its highlighted (pushed) one.
static inline void setIconNormal(m2d::SimpleEx_c *icon, bool normal) {
    icon->setVisible(l_iconPushPane, !normal);
    icon->setVisible(l_iconPane, normal);
}

// .text:0x5C
int dMenuMap_c::preCreate() {
    return dMenuBase_c::preCreate() != NOT_READY;
}

// .text:0x88
void dMenuMap_c::postCreate(MAIN_STATE_e state) {
    if (state == SUCCESS) {
        mCursor = SLOT_NONE;
        mMapColumn = 0;
    }
    dMenuBase_c::postCreate(state);
}

// .text:0xA4
void dMenuMap_c::postDelete(MAIN_STATE_e state) {
    dMenuBase_c::postDelete(state);
}

// .text:0xA8
int dMenuMap_c::execute() {
    if (!makeMapTex()) {
        return SUCCEEDED;
    }
    hideTutorial();
    int i;
    bool anm = isState(STATE_ANM);
    int prev = mCursor;
    int result = dMenuBase_c::execute();
    int cur = mCursor;
    if (cur != SLOT_NONE && cur != prev && !isTab(cur)) {
        playSeCursor();
    }
    if (anm) {
        for (i = 0; i < ENTRY_NUM; i++) {
            mWinLyt.copyPos(&mIcons[i], "map_00", l_allPanes[0]);
        }
        mWinLyt.copyPos(&mPlayerIcon, "map_00", l_allPanes[0]);
        onFlag(FLAG_CALC_ICONS);
    }
    calcButtonAnms();
    calcScroll();
    calcBlink();
    if (isFlag(FLAG_CALC_WINDOW)) {
        offFlag(FLAG_CALC_WINDOW);
        mWinLyt.calc();
    }
    if (isFlag(FLAG_CALC_ICONS)) {
        offFlag(FLAG_CALC_ICONS);
        for (i = 0; i < ENTRY_NUM; i++) {
            mIcons[i].calc();
        }
        mPlayerIcon.calc();
    }
    return result;
}

// .text:0x260
int dMenuMap_c::draw() {
    if (isFlag(FLAG_DRAW)) {
        mWinLyt.entry();
        for (int i = 0; i < ENTRY_NUM; i++) {
            mIcons[i].entry();
        }
        mPlayerIcon.entry();
    }
    return SUCCEEDED;
}

// .text:0x2DC
bool dMenuMap_c::load() {
    return mRes.load("Layout/map.arc");
}

// .text:0x2EC
void dMenuMap_c::createLayouts() {
    mSelected = -1;
    mWinLyt.build(&mRes, "map_win_00.brlyt");
    mWinLyt.setVisible("B_msg_05", false);
    mWinLyt.setAnm(&mScrollIconAnm, "map_win_00_scrlIcon_00.brlan", nullptr);
    mScrollIconAnm.mFrameCtrl.mFlags = 0;
    initLists();
    createIcons();
    initAnms();
    initRows();
    setScrollAnm(SCROLL_NEXT);
    setListAnm();
    refreshList();
    updateScrollButtons();
    setSlotAnm(SLOT_ROW, FALSE);
}

// .text:0x3BC
void dMenuMap_c::createIcons() {
    // The building of each icon (ENTRY_e).
    static u16 buildings[ENTRY_NUM] = {
        BUILDING_PLAYER_HOUSE_0, BUILDING_PLAYER_HOUSE_1,
        BUILDING_PLAYER_HOUSE_2, BUILDING_PLAYER_HOUSE_3,
        BUILDING_NPC_HOUSE_0,    BUILDING_NPC_HOUSE_1,
        BUILDING_NPC_HOUSE_2,    BUILDING_NPC_HOUSE_3,
        BUILDING_NPC_HOUSE_4,    BUILDING_NPC_HOUSE_5,
        BUILDING_NPC_HOUSE_6,    BUILDING_NPC_HOUSE_7,
        BUILDING_NPC_HOUSE_8,    BUILDING_NPC_HOUSE_9,
        BUILDING_SHOP,           BUILDING_TAILOR,
        BUILDING_TOWN_HALL,      BUILDING_MUSEUM,
        BUILDING_GATE,           BUILDING_BUS_STOP,
    };
    static const f32 scaleX = 3.0f;
    static const f32 scaleY = 3.0f;
    int x;
    int z;
    mVec3_c pos;
    for (int i = 0; i < ENTRY_NUM; i++) {
        mIcons[i].build(&mRes, "map_icon_00.brlyt");
        mIcons[i].setVisible(l_iconPushPane, false);
        mIcons[i].setVisible(l_playerPane, false);
        dItem::Item item = buildings[i];
        if (dSaveBuildingList_c::get()->getPos(&x, &z, &item, 1)) {
            x -= UT_BASE_NUM;
            z -= UT_BASE_NUM;
            pos.x = scaleX * (x - MAP_UNIT_CENTER);
            pos.y = -scaleY * (z - MAP_UNIT_CENTER);
            if (i == ENTRY_TOWN_HALL || i == ENTRY_GATE) {
                pos.y -= 8.0f;
            }
            if (i == ENTRY_BUS_STOP) {
                pos.y -= 11.0f;
            }
            mIcons[i].findPane("N_mapIcn_all")->SetTranslate(pos);
            f32 frame = getIconFrame(i);
            mIcons[i].setAnm(&mIconAnms[i], "map_icon_00_mapIcon_TxPtrn.brlan",
                             nullptr);
            mIconAnms[i].setFrame(frame);
        } else {
            mIcons[i].setVisible(l_allPanes[0], false);
        }
        mIcons[i].calc();
        mIcons[i].setAnm(&mIconAnms[i], "map_icon_00_mapIcon_grn.brlan",
                         nullptr);
        mIconAnms[i].setToEnd();
        mIcons[i].calc();
    }
    mPlayerIcon.build(&mRes, "map_icon_00.brlyt");
    mPlayerIcon.setVisible("N_mapIcn_all", false);
    mVec3_c *playerPos =
        dPlayerMgr::getSoundPosition(dPlayerMgr::PLAYER_INDEX_CURRENT);
    if (!dScene::isSceneAttr(dScene::getCurrentScene(),
                             dScene::SCENE_ATTR_OUTDOOR |
                                 dScene::SCENE_ATTR_MY_TOWN)) {
        playerPos = nullptr;
    }
    if (playerPos != nullptr) {
        x = worldToUnit(playerPos->x);
        z = worldToUnit(playerPos->z);
        x -= UT_BASE_NUM;
        z -= UT_BASE_NUM;
        pos.x = scaleX * (x - MAP_UNIT_CENTER);
        pos.y = -scaleY * (z - MAP_UNIT_CENTER);
        mPlayerIcon.findPane(l_playerPane)->SetTranslate(pos);
        mPlayerIcon.calc();
    } else {
        mPlayerIcon.setVisible(l_playerPane, false);
    }
}

// .text:0x744
BOOL dMenuMap_c::remove() {
    return mRes.remove();
}

// .text:0x74C
int dMenuMap_c::hitCheck(const mVec2_c *pos) {
    int slot;
    if (!dObjCursor_c::isVisible()) {
        return SLOT_NONE;
    }
    for (int i = 0; i < ENTRY_NUM; i++) {
        mVec2_c p = *pos;
        if (m2d::hitButton(&p, &mIcons[i], 0, 0, -1, -1, 1.0f) != -1) {
            return i + SLOT_ICON;
        }
    }
    mVec2_c p = *pos;
    slot = m2d::hitButton(&p, &mWinLyt, 0, SLOT_FACILITY_BUTTON, -1, -1, 1.0f);
    if (slot == SLOT_SCROLL_UP && !canScrollPrev()) {
        slot = SLOT_NONE;
    }
    if (slot == SLOT_SCROLL_DOWN && !canScrollNext()) {
        slot = SLOT_NONE;
    }
    return slot;
}

// .text:0x864
BOOL dMenuMap_c::pushSlot(int slot) {
    switch (slot) {
    case SLOT_HOUSE_BUTTON:
        if (!isFlag(FLAG_FACILITY)) {
            return FALSE;
        }
        mHouseBtnAnm.setNext(BTN_ANM_PUSH);
        mWinLyt.setPosFrom("N_mapBtn_00", l_btnPushPane, 0.0f, 0.0f);
        playSeModeChange();
        return TRUE;
    case SLOT_FACILITY_BUTTON:
        BOOL facility = isFlag(FLAG_FACILITY);
        if (facility == TRUE) {
            return FALSE;
        }
        mFacilityBtnAnm.setNext(BTN_ANM_PUSH);
        mWinLyt.setPosFrom("N_mapBtn_01", l_btnPushPane, 0.0f, 0.0f);
        playSeModeChange();
        return TRUE;
    case SLOT_SCROLL_UP:
        if (!canScrollPrev()) {
            return FALSE;
        }
        mScrollPrevBtnAnm.setNext(BTN_ANM_PUSH);
        playSePageTurn();
        return TRUE;
    case SLOT_SCROLL_DOWN:
        if (!canScrollNext()) {
            return FALSE;
        }
        mScrollNextBtnAnm.setNext(BTN_ANM_PUSH);
        playSePageTurn();
        return TRUE;
    }
    if (isRow(slot)) {
        int idx = slot + mScroll;
        if (idx >= getListNum()) {
            return FALSE;
        }
        if (select(getList()[idx])) {
            playSeListChoice();
        }
        setRowCursor(slot);
        return FALSE;
    }
    if (isIcon(slot)) {
        int entry = slot - SLOT_ICON;
        if (select(entry)) {
            playSeListChoice();
        }
        if (showEntry(entry)) {
            setListAnm();
            refreshList();
            updateScrollButtons();
        } else {
            setRowCursor(-1);
        }
        return TRUE;
    }
    return FALSE;
}

// .text:0xAC8
void dMenuMap_c::initAnms() {
    static const char *scrollPrevBtnAnms[] = {
        "map_win_00_scrlBtn_00_Out.brlan",
        "map_win_00_scrlBtn_00_In.brlan",
        "map_win_00_scrlBtn_00_Psh.brlan",
        nullptr,
        "map_win_00_scrlBtn_00_NoPsh.brlan",
        "map_win_00_scrlBtn_00_CanPsh.brlan",
    };
    static const char *scrollNextBtnAnms[] = {
        "map_win_00_scrlBtn_01_Out.brlan",
        "map_win_00_scrlBtn_01_In.brlan",
        "map_win_00_scrlBtn_01_Psh.brlan",
        nullptr,
        "map_win_00_scrlBtn_01_NoPsh.brlan",
        "map_win_00_scrlBtn_01_CanPsh.brlan",
    };
    static const char *houseBtnAnms[] = {
        "map_win_00_mapBtn_00_Out.brlan",
        "map_win_00_mapBtn_00_In.brlan",
        "map_win_00_mapBtn_00_Psh.brlan",
        nullptr,
    };
    static const char *facilityBtnAnms[] = {
        "map_win_00_mapBtn_01_Out.brlan",
        "map_win_00_mapBtn_01_In.brlan",
        "map_win_00_mapBtn_00_Psh.brlan",
        nullptr,
    };
    static const char *rowMsgAnms[] = {
        "map_win_00_T_msg_Out.brlan",
        "map_win_00_T_msg_In.brlan",
        "map_win_00_T_msg_psh.brlan",
        "map_win_00_T_msg_Out.brlan",
    };
    mScrollPrevBtnAnm.init(&mWinLyt, nullptr, scrollPrevBtnAnms,
                           SCROLL_BTN_ANM_NUM);
    mScrollNextBtnAnm.init(&mWinLyt, nullptr, scrollNextBtnAnms,
                           SCROLL_BTN_ANM_NUM);
    mHouseBtnAnm.init(&mWinLyt, nullptr, houseBtnAnms, MAP_BTN_ANM_NUM);
    mFacilityBtnAnm.init(&mWinLyt, nullptr, facilityBtnAnms, MAP_BTN_ANM_NUM);
    for (int i = 0; i < ROW_NUM; i++) {
        mRowMsgAnms[i].init(&mWinLyt, &mRes, rowMsgAnms, ROW_ANM_NUM);
        mRowMsgAnms[i].setNumber(i);
    }
}

// .text:0xBAC
void dMenuMap_c::calcButtonAnms() {
    int playing = mScrollPrevBtnAnm.play();
    playing |= mScrollNextBtnAnm.play();
    playing |= mHouseBtnAnm.play();
    playing |= mFacilityBtnAnm.play();
    for (int i = 0; i < ROW_NUM; i++) {
        playing |= mRowMsgAnms[i].play();
    }
    if (playing) {
        onFlag(FLAG_BUTTON_ANM);
        onFlag(FLAG_CALC_WINDOW);
    } else {
        offFlag(FLAG_BUTTON_ANM);
    }
}

// .text:0xC70
BOOL dMenuMap_c::makeMapTex() {
    if (mMapColumn >= dMap::MAP_COLUMN_NUM) {
        return TRUE;
    }
    dMap::makeMap(m2d::getTexMaps(mWinLyt.findPane("map_00"))->GetImage(),
                  mMapColumn);
    if (++mMapColumn >= dMap::MAP_COLUMN_NUM) {
        onFlag(FLAG_DRAW);
        return TRUE;
    }
    return FALSE;
}

// .text:0xCF8
void dMenuMap_c::initRows() {
    static const char *rowIconAnms[] = {
        "map_win_00_mapIcon_TxPtrn.brlan",
        "map_win_00_Icn_BrueGrn.brlan",
        nullptr,
        "map_win_00_Icn_BruePnk.brlan",
    };
    char name[40];
    m2d::stringWord_c title;
    dString::loadString(&title, MSG_TOWN_MAP, "sys_2D/SYS_2D_menu");
    wchar_t *text = title.getText();
    nw4r::lyt::TextBox *textBox = mWinLyt.findTextBox("T_name_00");
    textBox->AllocStringBuffer(title.getCapacity());
    textBox->SetString(text, 0);
    for (int i = 0; i < ROW_NUM; i++) {
        sprintf(name, "T_msg_%02d", i);
        mRowWords[i].setTextBox(mWinLyt.findTextBox(name));
    }
    for (int i = 0; i < ROW_NUM; i++) {
        mRowIconAnms[i].init(&mWinLyt, &mRes, rowIconAnms, ROW_ANM_NUM);
        mRowIconAnms[i].setNumber(i);
    }
}

// .text:0xE48
void dMenuMap_c::setSlotAnm(int slot, BOOL on) {
    int entry;
    m2d::AnmEx_c *btnAnm = nullptr;
    switch (slot) {
    case SLOT_SCROLL_UP:
        btnAnm = &mScrollPrevBtnAnm;
        break;
    case SLOT_SCROLL_DOWN:
        btnAnm = &mScrollNextBtnAnm;
        break;
    case SLOT_HOUSE_BUTTON:
        btnAnm = &mHouseBtnAnm;
        break;
    case SLOT_FACILITY_BUTTON:
        btnAnm = &mFacilityBtnAnm;
        break;
    }
    if (isRow(slot)) {
        int anm;
        if (on) {
            anm = ROW_ANM_CURSOR;
        } else {
            int idx = slot + mScroll;
            if (idx < getListNum() && mSelected == getList()[idx]) {
                anm = ROW_ANM_SELECTED;
            } else {
                anm = ROW_ANM_NORMAL;
            }
        }
        setRowAnm(slot, anm);
    } else if (isIcon(slot)) {
        entry = slot - SLOT_ICON;
        if (on) {
            mIcons[entry].setAnm(&mIconAnms[entry],
                                 "map_icon_00_mapIcon_grn.brlan", nullptr);
            mIconAnms[entry].setToEnd();
        } else if (entry == mSelected) {
            on = TRUE;
            mIcons[entry].setAnm(&mIconAnms[entry],
                                 "map_icon_00_mapIcon_pnk.brlan", nullptr);
            mIconAnms[entry].setToEnd();
        }
        setIconNormal(&mIcons[entry], !on);
        onFlag(FLAG_CALC_ICONS);
    } else if (btnAnm != nullptr) {
        btnAnm->setNext(on != FALSE);
    }
}

// .text:0x1050
void dMenuMap_c::setScrollAnm(int dir) {
    static const char *scrollAnms[] = {
        "map_win_00_scrlUp.brlan",
        "map_win_00_scrlDown.brlan",
    };
    mWinLyt.setAnm(&mScrollAnm, scrollAnms[dir], nullptr);
}

// .text:0x1070
void dMenuMap_c::setListAnm() {
    static const char *listAnms[] = {
        "map_win_00_toHouse.brlan",
        "map_win_00_toFacility.brlan",
    };
    bool facility = isFlag(FLAG_FACILITY);
    mWinLyt.setAnm(&mListAnm, listAnms[facility], nullptr);
    mListAnm.setToEnd();
}

// .text:0x10C8
void dMenuMap_c::calcScroll() {
    if (isFlag(FLAG_SCROLL)) {
        if (mScrollAnm.mFrameCtrl.isStop()) {
            setScrollAnm(SCROLL_NEXT);
            refreshList();
            offFlag(FLAG_SCROLL);
        } else {
            mScrollAnm.play();
        }
        onFlag(FLAG_CALC_WINDOW);
    }
    mScrollIconAnm.play();
    onFlag(FLAG_CALC_WINDOW);
}

// .text:0x115C
u8 *dMenuMap_c::getList() {
    if (isFlag(FLAG_FACILITY)) {
        return mFacilities;
    }
    return mResidents;
}

// .text:0x11A0
int dMenuMap_c::getListNum() {
    if (isFlag(FLAG_FACILITY)) {
        return mFacilityNum;
    }
    return mResidentNum;
}

// .text:0x11E4
void dMenuMap_c::initLists() {
    mFacilityNum = FACILITY_NUM;
    for (int i = 0; i < FACILITY_NUM; i++) {
        mFacilities[i] = ENTRY_FACILITY + i;
    }
    mResidentNum = 0;
    for (int i = 0; i < RESIDENT_NUM; i++) {
        mResidents[i] = ENTRY_NUM;
    }
    mOwnHome = -1;
    dPrivateData_c *me = dPlayerMgr::Mgr_c::getCurrentPlayer();
    int j;
    for (int i = 0; i < PLAYER_NUM; i++) {
        dPrivateData_c *player =
            dSaveData_c::getTown()->mHomes.getHomePlayer(i);
        if (player != nullptr && player->mPID.isValid()) {
            mResidents[mResidentNum] = ENTRY_PLAYER + i;
            mResidentNum++;
            if (*player->getPersonalID() == *me->getPersonalID()) {
                mOwnHome = i;
            }
        }
    }
    for (j = 0; j < ANIMAL_NUM; j++) {
        dAnimal_c *animal = dSaveData_c::getTown()->mAnimals.mTown.getAnimal(j);
        if (animal != nullptr && animal->mID.isValid()) {
            mResidents[mResidentNum] = ENTRY_ANIMAL + j;
            mResidentNum++;
        }
    }
    mScroll = 0;
}

// .text:0x13C0
void dMenuMap_c::updateRowWords() {
    static const u16 facilityMsgs[FACILITY_NUM] = {MSG_SHOP,      MSG_TAILOR,
                                                   MSG_TOWN_HALL, MSG_MUSEUM,
                                                   MSG_GATE,      MSG_BUS_STOP};
    u8 *list = getList();
    int num = getListNum();
    dHmnName::Word_c name;
    for (int i = 0; i < ROW_NUM; i++) {
        int idx = i + mScroll;
        name.clear();
        mRowWords[i].clear();
        if (idx < num) {
            u8 entry = list[idx];
            if (entry <= ENTRY_PLAYER_LAST) {
                dSaveData_c::getTown()
                    ->mHomes.getHomePlayer(entry)
                    ->mPID.setWord(&name);
                mRowWords[i].copy(&name, 0);
            } else if (entry >= ENTRY_ANIMAL && entry <= ENTRY_ANIMAL_LAST) {
                dSaveData_c::getTown()
                    ->mAnimals.mTown.getAnimal(entry - ENTRY_ANIMAL)
                    ->mID.setWord(&name, LANGUAGE_NUM);
                mRowWords[i].copy(&name, 0);
            } else if (entry >= ENTRY_FACILITY &&
                       entry <= ENTRY_FACILITY_LAST) {
                dString::loadString(&mRowWords[i],
                                    facilityMsgs[entry - ENTRY_FACILITY],
                                    "sys_2D/SYS_2D_menu");
            }
        }
        mRowWords[i].update();
    }
    onFlag(FLAG_CALC_WINDOW);
    mShownScroll = mScroll;
}

// .text:0x1528
f32 dMenuMap_c::getIconFrame(u8 entry) {
    f32 frame = 0.0f;
    if (entry <= ENTRY_PLAYER_LAST) {
        if (mOwnHome == entry) {
            frame = 7.0f;
        } else {
            frame = 6.0f;
        }
    } else if (entry >= ENTRY_ANIMAL && entry <= ENTRY_ANIMAL_LAST) {
        frame = 8.0f;
    } else if (entry >= ENTRY_FACILITY && entry <= ENTRY_FACILITY_LAST) {
        frame = entry - ENTRY_FACILITY;
    }
    return frame;
}

// .text:0x15AC
void dMenuMap_c::updateRowIcons() {
    u8 *list = getList();
    int num = getListNum();
    for (int i = 0; i < ROW_NUM; i++) {
        int idx = i + mScroll;
        f32 frame = 0.0f;
        if (idx < num) {
            frame = getIconFrame(list[idx]);
        }
        mRowIconAnms[i].start(ROW_ANM_NORMAL);
        mRowIconAnms[i].setFrame(frame);
    }
    mWinLyt.calc();
    offFlag(FLAG_CALC_WINDOW);
}

// .text:0x1678
bool dMenuMap_c::showEntry(int entry) {
    bool changed = setFacilityMode(entry >= ENTRY_FACILITY &&
                                   entry <= ENTRY_FACILITY_LAST);
    if (changed) {
        int idx = findEntry(entry);
        if (idx != -1) {
            int row = idx - mScroll;
            if (row >= SCROLL_ROW_NUM) {
                mScroll = row - (SCROLL_ROW_NUM - 1);
            }
        }
    }
    return changed;
}

// .text:0x1710
bool dMenuMap_c::setFacilityMode(BOOL facility) {
    if (facility == isFlag(FLAG_FACILITY)) {
        return false;
    }
    if (facility) {
        onFlag(FLAG_FACILITY);
    } else {
        offFlag(FLAG_FACILITY);
    }
    mScroll = 0;
    onFlag(FLAG_CALC_WINDOW);
    return true;
}

// .text:0x1798
int dMenuMap_c::findEntry(int entry) {
    u8 *list = getList();
    int num = getListNum();
    for (int i = 0; i < num; i++) {
        if (entry == list[i]) {
            return i;
        }
    }
    return -1;
}

// .text:0x1818
bool dMenuMap_c::select(int entry) {
    int prev = mSelected;
    if (prev == entry) {
        return false;
    }
    mSelected = entry;
    if (prev != -1) {
        setSlotAnm(prev + SLOT_ICON, FALSE);
    }
    setSlotAnm(mSelected + SLOT_ICON, FALSE);
    onFlag(FLAG_CALC_ICONS);
    mBlinkTimer = 0;
    updateRowSelect();
    return true;
}

// .text:0x18A0
void dMenuMap_c::changeList(BOOL facility) {
    setFacilityMode(facility);
    setListAnm();
    refreshList();
    updateScrollButtons();
}

// .text:0x18E4
int dMenuMap_c::getIconScrollDir() {
    int idx = findEntry(mCursor - SLOT_ICON);
    if (idx != -1) {
        int row = idx - mScroll;
        if (row < 0) {
            return SCROLL_PREV;
        }
        if (row >= SCROLL_ROW_NUM) {
            return SCROLL_NEXT;
        }
    }
    return SCROLL_NONE;
}

// .text:0x1948
void dMenuMap_c::refreshList() {
    updateRowWords();
    updateRowIcons();
    setRowCursor(-1);
    updateRowSelect();
}

// .text:0x1990
void dMenuMap_c::setRowCursor(int row) {
    for (int i = 0; i <= SLOT_ROW_LAST; i++) {
        if (row == i) {
            setSlotAnm(i, TRUE);
        } else {
            setSlotAnm(i, FALSE);
        }
    }
    onFlag(FLAG_CALC_WINDOW);
}

// .text:0x1A14
void dMenuMap_c::updateRowSelect() {
    for (int i = 0; i < ROW_NUM; i++) {
        char name[32];
        sprintf(name, "slctBase_%02d", i);
        mWinLyt.setVisible(name, isSelectedRow(i));
    }
}

// .text:0x1AD8
void dMenuMap_c::startScroll(int dir) {
    setScrollAnm(dir);
    onFlag(FLAG_SCROLL);
    if (dir == SCROLL_PREV) {
        mScroll--;
        refreshList();
    } else {
        mScroll++;
    }
}

// .text:0x1B48
void dMenuMap_c::updateScrollButtons() {
    if (!canScrollPrev()) {
        mScrollPrevBtnAnm.start(BTN_ANM_NO_PUSH);
    } else {
        mScrollPrevBtnAnm.start(BTN_ANM_CAN_PUSH);
    }
    mScrollPrevBtnAnm.setToEnd();
    if (!canScrollNext()) {
        mScrollNextBtnAnm.start(BTN_ANM_NO_PUSH);
    } else {
        mScrollNextBtnAnm.start(BTN_ANM_CAN_PUSH);
    }
    mScrollNextBtnAnm.setToEnd();
    onFlag(FLAG_CALC_WINDOW);
}

// .text:0x1BE0
BOOL dMenuMap_c::canScrollPrev() {
    return mScroll > 0;
}

// .text:0x1BF4
BOOL dMenuMap_c::canScrollNext() {
    return mScroll + SCROLL_ROW_NUM < getListNum();
}

// .text:0x1C3C
void dMenuMap_c::setRowAnm(int row, int anm) {
    m2d::AnmEx2_c *msgAnm = &mRowMsgAnms[row];
    msgAnm->start(anm);
    msgAnm->setToEnd();
    m2d::AnmEx2_c *iconAnm = &mRowIconAnms[row];
    iconAnm->start(anm);
    if (anm == ROW_ANM_NORMAL) {
        int idx = row + mShownScroll;
        f32 frame = 0.0f;
        if (idx < getListNum()) {
            frame = getIconFrame(getList()[idx]);
        }
        iconAnm->setFrame(frame);
    } else {
        iconAnm->setToEnd();
    }
    onFlag(FLAG_CALC_WINDOW);
}

// .text:0x1D20
void dMenuMap_c::calcBlink() {
    static const int blinkOn = 45;
    static const int blinkOff = 35;
    if (mSelected == -1) {
        return;
    }
    if (++mBlinkTimer >= blinkOn + blinkOff) {
        mBlinkTimer = 0;
    }
    bool lit = mBlinkTimer < blinkOn;
    if (isIcon(mCursor) && mCursor - SLOT_ICON == mSelected) {
        lit = true;
    }
    bool off = !lit;
    mIcons[mSelected].setVisible(l_iconPushPane, !off);
    mIcons[mSelected].setVisible(l_iconPane, off);
    bool rowLit = mBlinkTimer < blinkOn;
    int row = findEntry(mSelected) - mShownScroll;
    if (row < 0 || row >= ROW_NUM) {
        return;
    }
    int anm = ROW_ANM_NORMAL;
    if (rowLit) {
        anm = ROW_ANM_SELECTED;
    }
    if (isRow(mCursor) && mCursor == row) {
        anm = ROW_ANM_CURSOR;
    }
    setRowAnm(row, anm);
}

// .text:0x1E98
void dMenuMap_c::showSlotTutorial(int slot) {
    u16 msg;
    int button = -1;
    if (isButton(slot)) {
        button = slot - SLOT_HOUSE_BUTTON;
        msg = button + TUTORIAL_MSG_HOUSES;
    }
    if (button != -1 && dObjTutorial_c::request(0, msg, TRUE)) {
        char name[48];
        sprintf(name, "N_mapBtn_%02d", button);
        mVec3_c screenPos;
        m2d::getScreenPos(&screenPos, mWinLyt.findPane(name));
        mVec2_c pos;
        pos.x = screenPos.x;
        pos.y = screenPos.y;
        dObjTutorial_c::setMsg(&pos, msg, 1);
    }
}

// .text:0x1F70
void dMenuMap_c::hideTutorial() {
    dObjTutorial_c::hide();
}

// .text:0x1F74
void dMenuMap_c::playSeModeChange() {
    playSe(SE_HOUSE_ModeChange);
}

// .text:0x1F7C
void dMenuMap_c::playSeListChoice() {
    playSe(SE_HOUSE_ListChoice);
}
