#include <game/game/d_menu_mng.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_base.hpp>
#include <game/game/d_bgm.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_demo.hpp>
#include <game/game/d_dsn.hpp>
#include <game/game/d_fg_item.hpp>
#include <game/game/d_hmn_tool_mng.hpp>
#include <game/game/d_line.hpp>
#include <game/game/d_mail.hpp>
#include <game/game/d_menu_base.hpp>
#include <game/game/d_menu_res.hpp>
#include <game/game/d_obj_keyboard.hpp>
#include <game/game/d_photo.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_post_office.hpp>
#include <game/game/d_private_data.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_wifi.hpp>
#include <string.h>

#define MENU_NONE 0x79

CUSTOM_BASE_PROFILE(MENU_MNG, dMenuMng_c, 0xB5, 0xE6);

static u8 s_menuNo = MENU_NONE;
static u16 s_itemMsgParam = 0x22;
static u16 s_C104 = dItem::ITEM_ID_NONE;

static dMenuMng_c *s_instance;
static u8 s_state;
static dLineMg_c s_menuList;
static u32 s_flags;
static bool s_bgmVol;
u8 lbl_8074EA7D;
static u16 s_pocketMask;
static u16 s_selMask;
static u8 s_selSlot;
static u8 s_EA83;
static u32 s_EA84;
static u32 s_EA88;
static u32 s_EA8C;
static int s_friendIdx;
static int s_postType;
static BOOL s_EA98;
static int s_toolPocket;
static int s_toolNext;

static u16 l_selItems[dMenuMng_c::POCKET_NUM];
mVec3_c g_menuPos;
static wchar_t l_inputBuf[0x21];
static dMail_c l_mail;
static dDesign_c l_design;
static dTime_c l_time;
static dPostRequest_c l_postReq;
static wchar_t l_postMsg[0x10D];
static wchar_t l_postTitle[0x21];

int dMenuMng_c::create() {
    bool loaded = true;
    loaded &= mWinRes.load("Layout/sb_win_anim.arc");
    loaded &= mBtnRes.load("Layout/cmn_btn.arc");
    loaded &= mWiiMenuRes.load("Layout/wii_menu.arc");
    loaded &= mWiiMenu2Res.load("Layout/wii_menu2.arc");
    if (!loaded) {
        return NOT_READY;
    }
    dMenuRes::create();
    s_instance = this;
    s_menuNo = MENU_NONE;
    s_state = STATE_IDLE;
    s_flags = 0;
    s_bgmVol = false;
    clearInputBuf();
    lbl_8074EA7D = 0;
    resetToolPocket();
    return SUCCEEDED;
}

int dMenuMng_c::doDelete() {
    if (!mWinRes.remove()) {
        return NOT_READY;
    }
    if (!mBtnRes.remove()) {
        return NOT_READY;
    }
    if (!mWiiMenuRes.remove()) {
        return NOT_READY;
    }
    if (!mWiiMenu2Res.remove()) {
        return NOT_READY;
    }
    if (!dMenuRes::destroy()) {
        return NOT_READY;
    }
    s_menuList.init();
    s_instance = nullptr;
    return SUCCEEDED;
}

int dMenuMng_c::execute() {
    if (dMenuRes::update()) {
        createMenu();
    }
    return SUCCEEDED;
}

int dMenuMng_c::draw() {
    return SUCCEEDED;
}

static ProfileName l_menuProfName[MENU_RES_MENU_NUM] = {
    0xB6, 0xB6, 0xBD, 0xC6, 0xC6, 0xC7, 0xCD, 0xCA, 0xCA, 0xCA, 0xCA, 0xE6, 0xCB, 0xC8, 0xC8, 0xC9,
    0xC9, 0xC9, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
    0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xB9, 0xB9, 0xB9, 0xB9, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xB8,
    0xB8, 0xB8, 0xB8, 0xB8, 0xB8, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD6, 0xD6,
    0xD6, 0xD7, 0xD7, 0xD7, 0xD7, 0xD9, 0xD9, 0xD9, 0xD9, 0xD9, 0xD8, 0xD8, 0xD8, 0xDA, 0xDA, 0xDA,
    0xDB, 0xBE, 0xBE, 0xBF, 0xBF, 0xC0, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC5, 0xDF, 0xBA, 0xBB,
    0xBB, 0xE0, 0xE0, 0xE1, 0xE1, 0xDC, 0xDE, 0xDC, 0xDC, 0xDC, 0xDC, 0xDE, 0xDD, 0xE2, 0xE3, 0xE3,
    0xE4, 0xE5, 0xE7, 0xE7, 0xE8, 0xE9, 0xEA, 0xB8, 0xB8, 0x00,
};

void dMenuMng_c::createMenu() {
    if (s_state == STATE_OPEN && !dObjKeyboard_c::isOpen()) {
        fBase_c *parent = this;
        dMenuNd_c *last = (dMenuNd_c *)s_menuList.getLast();
        if (last != nullptr) {
            parent = last->mpOwner;
        }
        s_state = STATE_CREATING;
        dBase_c::createBase(l_menuProfName[s_menuNo], parent, s_menuNo, 0);
    }
}

void dMenuMng_c::addMenu(dMenuNd_c *node) {
    s_menuList.addLastLineNode(node);
    if (s_state == STATE_CREATING) {
        offFlag(FLAG_NO_BGM);
        s_state = STATE_IDLE;
    }
}

void dMenuMng_c::removeMenu(dMenuNd_c *node) {
    if (s_menuList.removeLineNode(node)) {
        dMenuRes::releaseMenuRef(node->mpOwner->getMenuNo());
        if (s_menuList.getFirst() == nullptr && !dDemo::isType(dDemo::TYPE_MENU) && !isFlag(FLAG_NO_BGM)) {
            endMenuBgm();
        }
    }
}

BOOL dMenuMng_c::isFree() {
    if (s_state != STATE_IDLE) {
        return FALSE;
    }
    return s_menuList.getFirst() == nullptr;
}

BOOL dMenuMng_c::open(u8 menu, BOOL noBgm) {
    if (!isFree()) {
        return FALSE;
    }
    dMenuRes::addMenuRef(menu);
    s_state = STATE_OPEN;
    s_menuNo = menu;
    lbl_8074EA7D = 0;
    if (!noBgm && !isFlag(FLAG_NO_BGM)) {
        startMenuBgm(menu);
    }
    return TRUE;
}

BOOL dMenuMng_c::openSub(u8 menu) {
    if (s_state != STATE_IDLE) {
        return FALSE;
    }
    if (s_menuList.getFirst() == nullptr) {
        return FALSE;
    }
    dMenuRes::addMenuRef(menu);
    s_state = STATE_OPEN;
    s_menuNo = menu;
    lbl_8074EA7D = 0;
    return TRUE;
}

m2d::ResAccIf_c *dMenuMng_c::getWinResAcc() {
    if (s_instance == nullptr) {
        return nullptr;
    }
    return &s_instance->mWinRes;
}

m2d::ResAccIf_c *dMenuMng_c::getBtnResAcc() {
    if (s_instance == nullptr) {
        return nullptr;
    }
    return &s_instance->mBtnRes;
}

m2d::ResAccLoader_c *dMenuMng_c::getWiiMenuResAcc() {
    if (s_instance == nullptr) {
        return nullptr;
    }
    return &s_instance->mWiiMenuRes;
}

m2d::ResAccLoader_c *dMenuMng_c::getWiiMenu2ResAcc() {
    if (s_instance == nullptr) {
        return nullptr;
    }
    return &s_instance->mWiiMenu2Res;
}

BOOL dMenuMng_c::isMenuEnd() {
    if (isFlag(FLAG_NO_BGM)) {
        return FALSE;
    }
    return isFree();
}

void fn_8019A7A4() {
    dMenuMng_c::onFlag(dMenuMng_c::FLAG_NO_BGM);
}

void fn_8019A7AC() {
    dMenuMng_c::onFlag(0x10);
}

void fn_8019A7B4() {
    dMenuMng_c::offFlag(0x10);
}

bool fn_8019A7BC() {
    return dMenuMng_c::isFlag(0x10);
}

void dMenuMng_c::onFlag(u32 flag) {
    s_flags |= flag;
}

void dMenuMng_c::offFlag(u32 flag) {
    s_flags &= ~flag;
}

bool dMenuMng_c::isFlag(u32 flag) {
    return s_flags & flag;
}

void dMenuMng_c::setResult(u8 result) {
    lbl_8074EA7D = result;
}

BOOL dMenuMng_c::openItemSel(u16 mask, u16 msgParam, u8 menu) {
    BOOL ok = open(menu, FALSE);
    if (ok) {
        s_itemMsgParam = msgParam;
        s_pocketMask = mask;
        s_selSlot = 0xFF;
    }
    return ok;
}

BOOL dMenuMng_c::openItemSel2(u16 mask, u16 msgParam) {
    BOOL ok = open(0x2A, FALSE);
    if (ok) {
        s_itemMsgParam = msgParam;
        s_pocketMask = mask;
        s_selMask = 0;
    }
    return ok;
}

u16 dMenuMng_c::makePocketMask(BOOL (*filter)(void *pocket, int flag)) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    int i;
    u16 mask = 0;
    for (i = 0; i < POCKET_NUM; i++) {
        if (filter(&player->mPockets[i], player->getPocketFlag(i))) {
            mask |= 1 << i;
        }
    }
    return mask;
}

u8 dMenuMng_c::getSelSlot() {
    return s_selSlot;
}

void dMenuMng_c::setSelSlot(u8 slot) {
    s_selSlot = slot;
}

u16 dMenuMng_c::getSelMask() {
    return s_selMask;
}

void dMenuMng_c::setSelMask(u16 mask) {
    s_selMask = mask;
}

void dMenuMng_c::setPocketMask(u16 mask) {
    s_pocketMask = mask;
}

u16 dMenuMng_c::getPocketMask() {
    return s_pocketMask;
}

u16 dMenuMng_c::getItemMsgParam() {
    return s_itemMsgParam;
}

void dMenuMng_c::clearSelItems() {
    for (int i = 0; i < POCKET_NUM; i++) {
        if ((1 << i) & s_selMask) {
            l_selItems[i] = dItem::ITEM_ID_NONE;
        }
    }
}

void dMenuMng_c::setSelItems() {
    clearSelItems();
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    int num = 0;
    for (int i = 0; i < POCKET_NUM; i++) {
        if ((1 << i) & s_selMask) {
            l_selItems[num] = player->mPockets[i].getId();
            num++;
        }
    }
}

u16 *dMenuMng_c::getSelItems() {
    return l_selItems;
}

int dMenuMng_c::getSelItemNum() {
    int num = 0;
    for (int i = 0; i < POCKET_NUM; i++) {
        if ((1 << i) & s_selMask) {
            num++;
        }
    }
    return num;
}

void dMenuMng_c::removeSelItems() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    for (int i = 0; i < POCKET_NUM; i++) {
        if ((1 << i) & s_selMask) {
            player->clearPocket(i);
        }
    }
    s_selMask = 0;
}

BOOL fn_8019AC04(u16 arg) {
    BOOL ok = dMenuMng_c::open(0x25, FALSE);
    if (ok) {
        s_C104 = arg;
    }
    return ok;
}

BOOL fn_8019AC44(u16 arg) {
    BOOL ok = dMenuMng_c::open(0x26, FALSE);
    if (ok) {
        s_C104 = arg;
    }
    return ok;
}

BOOL fn_8019AC84(u16 arg) {
    BOOL ok = dMenuMng_c::open(0x27, FALSE);
    if (ok) {
        s_C104 = arg;
    }
    return ok;
}

BOOL fn_8019ACC4(u16 arg, const mVec3_c *pos) {
    BOOL ok = dMenuMng_c::open(0x28, FALSE);
    if (ok) {
        s_C104 = arg;
        g_menuPos = *pos;
    }
    return ok;
}

u16 fn_8019AD30() {
    return s_C104;
}

u16 fn_8019AD38(u16 arg) {
    u16 old = s_C104;
    s_C104 = arg;
    return old;
}

u8 fn_8019AD48() {
    return s_EA83;
}

void fn_8019AD50(u8 arg) {
    s_EA83 = arg;
}

wchar_t *dMenuMng_c::getInputBuf() {
    return l_inputBuf;
}

void dMenuMng_c::setInputBuf(const u16 *str, int len) {
    if (len == 0) {
        len = m2d::Word_c::getLength((const wchar_t *)str);
    }
    clearInputBuf();
    memcpy(l_inputBuf, str, len * sizeof(wchar_t));
}

void dMenuMng_c::clearInputBuf() {
    memset(l_inputBuf, 0, sizeof(l_inputBuf));
}

BOOL fn_8019ADD4() {
    dMenuMng_c::clearInputBuf();
    return dMenuMng_c::open(0x12, FALSE);
}

BOOL fn_8019AE00() {
    dMenuMng_c::clearInputBuf();
    return dMenuMng_c::open(0x13, FALSE);
}

BOOL fn_8019AE2C() {
    dMenuMng_c::clearInputBuf();
    return dMenuMng_c::open(0x1B, FALSE);
}

BOOL fn_8019AE58() {
    dMenuMng_c::clearInputBuf();
    return dMenuMng_c::open(0x1E, FALSE);
}

BOOL dMenuMng_c::openInput(u8 menu, const u16 *str, int len) {
    if (!open(menu, FALSE)) {
        return FALSE;
    }
    clearInputBuf();
    setInputBuf(str, len);
    return TRUE;
}

BOOL fn_8019AEE4() {
    return dMenuMng_c::open(0x35, FALSE);
}

BOOL fn_8019AEF0(u32 arg) {
    if (dMenuMng_c::open(0x49, FALSE)) {
        fn_8019AFDC(arg);
        return TRUE;
    }
    return FALSE;
}

BOOL dMenuMng_c::openMenu(u8 menu) {
    return open(menu, FALSE);
}

dMail_c *dMenuMng_c::getMail() {
    return &l_mail;
}

void dMenuMng_c::setMail(const dMail_c *mail) {
    l_mail.copy(mail);
}

BOOL dMenuMng_c::openLetter(const dMail_c *mail) {
    BOOL ok = openMenu(7);
    if (ok) {
        l_mail.copy(mail);
    }
    return ok;
}

void fn_8019AFBC(u32 arg) {
    s_EA88 = arg;
}

u32 fn_8019AFC4() {
    return s_EA88;
}

u32 fn_8019AFCC() {
    return s_EA84;
}

u32 fn_8019AFD4() {
    return s_EA84;
}

void fn_8019AFDC(u32 arg) {
    s_EA84 = arg;
}

u32 fn_8019AFE4() {
    return s_EA88;
}

void fn_8019AFEC(u32 arg) {
    s_EA88 = arg;
}

bool fn_8019AFF4() {
    return dMenuMng_c::isFlag(0x20);
}

bool fn_8019AFFC() {
    return dMenuMng_c::isFlag(0x40);
}

void fn_8019B004() {
    dMenuMng_c::onFlag(0x20);
}

void fn_8019B00C() {
    dMenuMng_c::onFlag(0x40);
}

void fn_8019B014() {
    dMenuMng_c::offFlag(0x20);
}

void fn_8019B01C() {
    dMenuMng_c::offFlag(0x40);
}

void dMenuMng_c::setDesign(const dDesign_c *design) {
    l_design = *design;
}

dDesign_c *dMenuMng_c::getDesign() {
    return &l_design;
}

u16 fn_8019B240() {
    return s_C104;
}

void fn_8019B248(u16 arg) {
    s_C104 = arg;
}

u32 fn_8019B250() {
    return s_EA88;
}

void fn_8019B258(u32 arg) {
    s_EA88 = arg;
}

u16 fn_8019B260() {
    return s_C104;
}

void fn_8019B268(u16 arg) {
    s_C104 = arg;
}

u32 fn_8019B270() {
    return s_EA88;
}

u32 fn_8019B278() {
    return s_EA8C;
}

void fn_8019B280(u32 a, u32 b) {
    s_EA88 = a;
    s_EA8C = b;
}

u32 fn_8019B28C() {
    return s_EA88;
}

void fn_8019B294(u32 arg) {
    s_EA88 = arg;
}

BOOL fn_8019B29C() {
    dMenuMng_c::clearSelMask();
    return dMenuMng_c::openMenu(0x4D);
}

BOOL fn_8019B2C4() {
    dMenuMng_c::clearSelMask();
    return dMenuMng_c::openMenu(0x4E);
}

BOOL fn_8019B2EC() {
    dMenuMng_c::clearSelMask();
    return dMenuMng_c::openMenu(0x4F);
}

void dMenuMng_c::clearSelMask() {
    s_selMask = 0;
}

bool dMenuMng_c::isSelMask(u16 bit) {
    return s_selMask & bit;
}

void dMenuMng_c::onSelMask(u16 bit) {
    s_selMask |= bit;
}

BOOL fn_8019B348() {
    return dMenuMng_c::isSelMask(0x1);
}

BOOL fn_8019B350() {
    return dMenuMng_c::isSelMask(0x80);
}

BOOL fn_8019B358() {
    return dMenuMng_c::isSelMask(0x20);
}

int dMenuMng_c::countSelSlots() {
    int i;
    int num = 0;
    for (i = 0; i < 4; i++) {
        if (isSelSlot(i)) {
            num++;
        }
    }
    return num;
}

BOOL dMenuMng_c::isSelSlot(int slot) {
    u16 bit = 2 << slot;
    return (s_selMask & bit) != 0;
}

BOOL fn_8019B3DC() {
    return dMenuMng_c::isSelMask(0x40);
}

void fn_8019B3E4() {
    dMenuMng_c::onSelMask(0x1);
}

void fn_8019B3EC() {
    dMenuMng_c::onSelMask(0x80);
}

void fn_8019B3F4() {
    dMenuMng_c::onSelMask(0x20);
}

void dMenuMng_c::onSelSlot(int slot) {
    u16 bit = 2 << slot;
    s_selMask |= bit;
}

void fn_8019B418() {
    dMenuMng_c::onSelMask(0x40);
}

void dMenuMng_c::setTime(const dTime_c *time) {
    l_time = *time;
}

dTime_c dMenuMng_c::getTime() {
    return l_time;
}

BOOL fn_8019B4EC() {
    return dMenuMng_c::open(0x51, FALSE);
}

BOOL fn_8019B4F8() {
    return dMenuMng_c::open(0x44, FALSE);
}

BOOL fn_8019B504() {
    return dMenuMng_c::open(0x39, FALSE);
}

void dMenuMng_c::setFriendIdx(int idx) {
    s_friendIdx = idx;
}

bool fn_8019B518() {
    return s_EA98;
}

void fn_8019B52C() {
    s_EA98 = TRUE;
}

void fn_8019B538() {
    s_EA98 = FALSE;
}

u8 *dMenuMng_c::getFriendInfo() {
    if (dWifi::isWc24On() == FALSE) {
        return nullptr;
    }
    if (dWifi::hasRoster() == FALSE) {
        return nullptr;
    }
    return dWifi::getRosterInfo(s_friendIdx);
}

void dMenuMng_c::setPostType(int type) {
    s_postType = type;
}

wchar_t *dMenuMng_c::getPostMsg() {
    return l_postMsg;
}

wchar_t *dMenuMng_c::getPostTitle() {
    return l_postTitle;
}

void dMenuMng_c::setupPost(BOOL withPhoto) {
    u32 titleSize = m2d::Word_c::getLength(l_postTitle) * sizeof(wchar_t);
    u32 msgSize = m2d::Word_c::getLength(l_postMsg) * sizeof(wchar_t);
    const void *photo = nullptr;
    u32 photoSize = 0;
    const void *arc = nullptr;
    u32 arcSize = 0;
    if (withPhoto) {
        photo = dPhoto::getJpeg();
        photoSize = dPhoto::getJpegSize();
    }
    m2d::ResAccLoader_c *res;
    if (s_postType == 0) {
        res = getWiiMenuResAcc();
    } else {
        res = getWiiMenu2ResAcc();
    }
    if (res != nullptr && res->mLoader.getStatus() > 0) {
        arc = res->mpArc;
        arcSize = res->mLoader.getStatus();
    }
    switch (s_postType) {
    case 0:
        l_postReq.mData[0] = l_postTitle;
        l_postReq.mSize[0] = titleSize;
        l_postReq.mData[1] = l_postMsg;
        l_postReq.mSize[1] = msgSize;
        l_postReq.mData[2] = arc;
        l_postReq.mSize[2] = arcSize;
        l_postReq.mData[3] = photo;
        l_postReq.mSize[3] = photoSize;
        l_postReq.mFriend = -1;
        break;
    case 1:
        l_postReq.mData[0] = l_postTitle;
        l_postReq.mSize[0] = titleSize;
        l_postReq.mData[1] = l_postMsg;
        l_postReq.mSize[1] = msgSize;
        l_postReq.mData[2] = arc;
        l_postReq.mSize[2] = arcSize;
        l_postReq.mData[3] = photo;
        l_postReq.mSize[3] = photoSize;
        l_postReq.mFriend = s_friendIdx;
        break;
    }
}

void dMenuMng_c::clearPost() {
    memset(l_postMsg, 0, sizeof(l_postMsg));
    memset(l_postTitle, 0, sizeof(l_postTitle));
}

BOOL dMenuMng_c::sendPost() {
    switch (s_postType) {
    case 0:
        return dWifi::startSendSelf(&l_postReq);
    case 1:
        return dWifi::startSendBoard(&l_postReq);
    }
    return TRUE;
}

bool dMenuMng_c::isPhotoCaptured() {
    return dPhoto::isCaptured();
}

bool dMenuMng_c::isFriendWithKey() {
    if (dWifi::hasRoster() == FALSE) {
        return false;
    }
    return dWifi::getRosterType(s_friendIdx) == 1;
}

void dMenuMng_c::startMenuBgm(u8 menu) {
    int kind = 0;
    switch (menu) {
    case 0x72:
    case 0x73:
    case 0x74:
        kind = 1;
        break;
    case 0x71:
        kind = 2;
        break;
    case 0x5D:
        kind = 3;
        break;
    }
    s_bgmVol = true;
    dBgm::l_mgr.mVolCtrl.mMenu.start(kind);
}

void dMenuMng_c::endMenuBgm() {
    if (s_bgmVol) {
        dBgm::l_mgr.mVolCtrl.mMenu.end();
    }
    s_bgmVol = false;
}

void dMenuMng_c::resetToolPocket() {
    s_toolPocket = -1;
    s_toolNext = -1;
}

BOOL dMenuMng_c::canPutAwayHeld() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dItem::Item held = player->mEquipment.mHeld;
    if (held.getKind() == 0x14) {
        return TRUE;
    }
    if (player->findEmptyPocket(0) != -1) {
        return TRUE;
    }
    return FALSE;
}

void dMenuMng_c::putAwayHeld() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dItem::Item held = player->mEquipment.mHeld;
    if (held.isValid() && held.getKind() != 0x14) {
        player->setPocket(&held, player->findEmptyPocket(0), FALSE);
    }
}

BOOL dMenuMng_c::isSwitchTool(int pocket) {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    if (player->getPocketFlag(pocket)) {
        return FALSE;
    }
    dItem::Item item = player->mPockets[pocket];
    if (!item.isValid()) {
        return FALSE;
    }
    int bone = item.getHideBone();
    if (bone == 10) {
        return FALSE;
    }
    if (bone != 9) {
        return FALSE;
    }
    if (dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_ROOM)) {
        return FALSE;
    }
    if (dScene::isCityScene(dScene::getCurrentScene()) && !dHmnToolBank_c::isHandTool(&item)) {
        return FALSE;
    }
    return TRUE;
}

u16 dMenuMng_c::getNextTool() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    int start = s_toolNext - 1;
    if (start < 0) {
        start = 0;
    } else if (start < POCKET_NUM - 1) {
        start++;
    } else {
        start = 0;
    }
    for (int i = 0; i < POCKET_NUM; i++) {
        int pocket = (start + i) % POCKET_NUM;
        if (isSwitchTool(pocket)) {
            s_toolPocket = pocket;
            s_toolNext = pocket + 1;
            return player->mPockets[pocket].getId();
        }
    }
    return dItem::ITEM_ID_NONE;
}

u16 dMenuMng_c::getPrevTool() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    int start = s_toolNext > 0 ? s_toolNext - 1 : POCKET_NUM - 1;
    for (int i = 0; i < POCKET_NUM; i++) {
        int pocket = (start + POCKET_NUM - i) % POCKET_NUM;
        if (isSwitchTool(pocket)) {
            s_toolPocket = pocket;
            s_toolNext = pocket;
            return player->mPockets[pocket].getId();
        }
    }
    return dItem::ITEM_ID_NONE;
}

void dMenuMng_c::swapHeld() {
    dPrivateData_c *player = dPlayerMgr::Mgr_c::getCurrentPlayer();
    dItem::Item held = player->mEquipment.mHeld;
    if (held.isValid() && held.getKind() == 0x14) {
        held = dItem::ITEM_ID_NONE;
    }
    int pocket = s_toolPocket;
    int next = s_toolNext;
    player->setPocket(&held, pocket, FALSE);
    s_toolPocket = pocket;
    s_toolNext = next;
}
