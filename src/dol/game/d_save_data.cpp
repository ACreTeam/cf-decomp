// The whole save file in memory. Skeleton: .text 8010D7E8..8010F124.
// See notes/save_file_layout.txt.
#include <game/game/d_save_data.hpp>
#include <game/game/d_sv_mgr.hpp>
#include <lib/egg/core/eggHeap.h>
#include <cstring>
#include <cstddef>
#include <game/game/d_player_mgr.hpp>

// Dependencies whose owners are not recovered yet.
extern "C" {

// Checksum helpers (auto_03_80115CDC).
BOOL fn_80115CDC(dSaveData_c *save);
BOOL fn_80115CE0(dSaveData_c *save);
void fn_80115CE4(dSaveData_c *save);
BOOL fn_80115D34(dSaveData_c *save);
void fn_80115E04(dSaveData_c *save);
void fn_80116900(dSaveExtra_c *extra);
void fn_80117078(dSaveExtra_c *extra); // update checksum
BOOL fn_801170B0(dSaveExtra_c *extra, int arg);

u32 fn_8014B3CC();
void fn_800FADE4();
void fn_80101424();
void fn_800CAC20();
}

// Game heap (owner not recovered).
extern EGG::Heap *lbl_8074E3D8;

// 8074E6D8
dSaveOption_c dSaveData_c::sOption;
// 8074E6E0
dSaveData_c *dSaveData_c::sSaveData;

static inline dSaveOption_c *getOption(dPrivateData_c *player) {
    return (dSaveOption_c *)&player->_83E8;
}

// 8010D7E8
void fn_8010D7E8(dSaveOption_c *opt) {
    opt->mBit7 = 1;
    opt->_1 = 6;
    opt->mBit65 = 0;
    opt->mBit4 = 0;
    opt->mBit3 = 0;
    opt->mBit2 = 0;
    opt->mBit1 = 0;
    opt->mBit0 = 1;
}

// 8010D808
void fn_8010D808(dSaveOption_c *opt, u8 value) {
    opt->_1 = value;
}

// 8010D810
void fn_8010D810(const dSaveOption_c *opt, void *dst) {
    memcpy(dst, opt, 1);
}

// 8010D824
void fn_8010D824(dSaveOption_c *opt, const void *src) {
    memcpy(opt, src, 1);
}

// 8010D82C
void fn_8010D82C(dSaveOption_c *opt) {
    opt->mDirty7 = 0;
    opt->mDirty65 = 0;
    opt->mDirty4 = 0;
    opt->mDirty3 = 0;
    opt->mDirty2 = 0;
    opt->mDirty1 = 0;
    opt->mDirty0 = 0;
}

// 8010D83C
void fn_8010D83C(dSaveOption_c *opt) {
    fn_8010D7E8(opt);
    fn_8010D82C(opt);
}

// 8010D870
void fn_8010D870(dSaveOption_c *opt) {
    BOOL flag = TRUE;

    if (fn_80115CE0(dSaveData_c::getTown())) {
        for (int i = 0; i < PLAYER_NUM; i++) {
            dPrivateData_c *player = dPlayerMgr_c::getPlayer(i);
            if (player->mPID.isValid() && getOption(player)->mBit7 == 1) {
                flag = FALSE;
            }
        }
    } else {
        flag = FALSE;
    }

    if (flag) {
        opt->mBit7 = 0;
    } else {
        opt->mBit7 = 1;
    }
    opt->mBit65 = 0;
    opt->mBit4 = 0;
    opt->mBit3 = 0;
    opt->mBit2 = 0;
    opt->mBit1 = 0;
    opt->mBit0 = 1;
}

// 8010D944
int fn_8010D944() {
    return dSaveData_c::sOption.mBit7;
}

// 8010D950
void fn_8010D950(u8 v) {
    dSaveData_c::sOption.mBit7 = v;
    dSaveData_c::sOption.mDirty7 = 1;
}

// 8010D970
u8 fn_8010D970() {
    return dSaveData_c::sOption.mBit65;
}

// 8010D97C
void fn_8010D97C(u8 v) {
    dSaveData_c::sOption.mBit65 = v;
    dSaveData_c::sOption.mDirty65 = 1;
}

// 8010D99C
u8 fn_8010D99C() {
    return dSaveData_c::sOption.mBit4;
}

// 8010D9A8
void fn_8010D9A8(u8 v) {
    dSaveData_c::sOption.mBit4 = v;
    dSaveData_c::sOption.mDirty4 = 1;
}

// 8010D9C8
u8 fn_8010D9C8() {
    return dSaveData_c::sOption.mBit3;
}

// 8010D9D4
void fn_8010D9D4(u8 v) {
    dSaveData_c::sOption.mBit3 = v;
    dSaveData_c::sOption.mDirty3 = 1;
}

// 8010D9F4
int fn_8010D9F4() {
    return 0;
}

// 8010D9FC
u8 fn_8010D9FC() {
    return dSaveData_c::sOption.mBit1;
}

// 8010DA08
void fn_8010DA08(u8 v) {
    dSaveData_c::sOption.mBit1 = v;
    dSaveData_c::sOption.mDirty1 = 1;
}

// 8010DA28
u8 fn_8010DA28() {
    return dSaveData_c::sOption.mBit0;
}

// 8010DA34
void fn_8010DA34(u8 v) {
    dSaveData_c::sOption.mBit0 = v;
    dSaveData_c::sOption.mDirty0 = 1;
}

// 8010DA54
u8 fn_8010DA54() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player == NULL) {
        return 0;
    }
    return getOption(player)->_1;
}

// 8010DA8C
void fn_8010DA8C(u8 v) {
    fn_8010D808(getOption(dPlayerMgr_c::getCurrentPlayer()), v);
}

// 8010DAC8
void fn_8010DAC8() {
    dPrivateData_c *player = dPlayerMgr_c::getCurrentPlayer();
    if (player != NULL) {
        fn_8010DAFC(getOption(player));
    }
}

// 8010DAFC
void fn_8010DAFC(dSaveOption_c *opt) {
    dSaveOption_c *s = &dSaveData_c::sOption;
    u8 byte;

    if (s->mDirty7) {
        opt->mBit7 = fn_8010D944();
    }
    if (s->mDirty65) {
        opt->mBit65 = fn_8010D970();
    }
    if (s->mDirty4) {
        opt->mBit4 = fn_8010D99C();
    }
    if (s->mDirty3) {
        opt->mBit3 = fn_8010D9C8();
    }
    if (s->mDirty1) {
        opt->mBit1 = fn_8010D9FC();
    }
    if (s->mDirty0) {
        opt->mBit0 = fn_8010DA28();
    }

    fn_8010D810(opt, &byte);
    fn_8010D82C(s);
    fn_8010D824(s, &byte);
}

// 8010DBFC
extern "C" int fn_8010DBFC() {
    return 0;
}

// 8010DC04
u32 dSaveData_c::getDLDataOffset() {
    return offsetof(dSaveData_c, mDLItems);
}

// 8010DC10
u16 dSaveData_c::getVersion() {
    return SAVE_VERSION;
}

// 8010DC18
dSaveData_c *dSaveData_c::get() {
    dSvMgr_c::isFullTransferComplete();
    return sSaveData;
}

// 8010DC3C
dSaveData_c *dSaveData_c::getRaw() {
    return sSaveData;
}

// 8010DC44
BOOL dSaveData_c::isGood() {
    if (!dPrivateData_c::fn_80136E10(mPlayers)) {
        return FALSE;
    }
    if (!fn_80115CE0(this)) {
        return FALSE;
    }
    return fn_80115CDC(this) != FALSE;
}

// 8010DCB0
void dSaveData_c::initialize() {
    fn_80115E04(this);
    dSaveDLItemList_c::get()->updateChecksum();
    fn_80116900(&mExtra);
}

// TODO: 8010DCF0 (new-game setup from the current player), 8010DDE0, 8010DEF8, 8010DFC0

// 8010DED0
extern "C" void fn_8010DED0() {
    fn_800FADE4();
    fn_80101424();
    fn_800CAC20();
}

// 8010E0A8
void dSaveData_c::updateChecksum() {
    fn_80115CE4(this);
    dSaveDLItemList_c *items = dSaveDLItemList_c::get();
    items->mChecksum = items->calcChecksum();
    fn_80117078(&mExtra);
}

// 8010E0F8
BOOL dSaveData_c::isExtraGood(int arg) {
    if (!fn_80115D34(this)) {
        return FALSE;
    }
    if (!fn_801170B0(&mExtra, arg)) {
        return FALSE;
    }
    u16 version = mItemVersion;
    if (version != dItem::BITM::getVersion()) {
        return FALSE;
    }
    u32 stamp = _05EB04;
    if (!(fn_8014B3CC() == stamp)) {
        return FALSE;
    }
    return dSaveDLItemList_c::getRaw()->isChecksumOK() != FALSE;
}

// 8010E1B8
dSaveDLItemList_c *dSaveData_c::getDLData() {
    dSvMgr_c::isDLDataTransferComplete();
    return &get()->mDLItems;
}

// 8010E1E4
dSaveData_c *dSaveData_c::getTown() {
    dSvMgr_c::isTownTransferComplete();
    return get();
}

// 8010E208
dSaveExtra_c *dSaveData_c::getExtra() {
    dSvMgr_c::isTownTransferComplete();
    return &get()->mExtra;
}

// 8010E234
dSaveData_c *dSaveData_c::getRaw2() {
    return sSaveData;
}

// 8010E23C
u32 dSaveData_c::getSize() {
    return sizeof(dSaveData_c);
}

// 8010E248
// The ctor is inlined here. Skeleton members (u8 arrays) don't run their ctors yet.
void dSaveData_c::create() {
    if (sSaveData == NULL) {
        sSaveData = (dSaveData_c *)lbl_8074E3D8->alloc(sizeof(dSaveData_c), 32);
        memset((void*)sSaveData, 0, sizeof(dSaveData_c));
        new (sSaveData) dSaveData_c();
        sSaveData->initialize();
    }
}

// 8010EA88
void dSaveData_c::clear() {
    memset((void *)this, 0, sizeof(dSaveData_c));
}

// TODO: 8010EA98 (sets up a player via dPrivateData_c::setup)
// TODO: 8010EB50..8010EE74 weak member ctors/dtors (records, house, design box, mailbox)
// TODO: 8010EEB4..8010F11C small wrapper classes (0xC800 / 0x1804 / design / mail records)
