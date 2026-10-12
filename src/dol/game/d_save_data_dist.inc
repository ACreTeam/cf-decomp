// Included at the end of src/dol/game/d_save_data.cpp. With -sym on, an included file's functions land after the
// weak copies the main file emits (the member constructors and destructors at 8010EB50..8010EE74), which is
// where the downloaded-data helpers sit (8010EEB4..8010F124).

// 8010EEB4
void dSaveDistBlock_c::clear() {
    memset(mData, 0, sizeof(mData));
    mSize = 0;
    set(l_prcMng.mData, l_prcMng.mSize);
}

// 8010EF10
void dSaveDistBlock_c::set(const void *data, u32 size) {
    if (size < sizeof(mData)) {
        memcpy(mData, data, size);
        mSize = size;
    }
}

// 8010EF58
void dSaveDistContentList_c::clear() {
    _00000 = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 6; j++) {
            mContents[i][j].clear();
        }
    }
}

// 8010EFD8
void dSaveDistContent_c::clear() {
    _0000 = 0;
    memset(mData, 0, sizeof(mData));
}

// 8010EFF0
void dSaveDistContent_c::set(const void *data, u32 size) {
    if (size < sizeof(mData)) {
        _0000 = size;
        memcpy(mData, data, size);
    }
}

// 8010F008
void dSaveDistPattern_c::clear() {
    mVersion = 0;
    mDesign.clear();
}

// 8010F018
void dSaveDistPattern_c::set(const void *data, u32 size, u32 version) {
    if (size == 0x2000) {
        mDesign.setFromDlItem((void *)data);
        mVersion = version;
    }
}

// 8010F060
void dSaveDistMail_c::clear() {
    mVersion = 0;
    mMail.clear();
}

// 8010F070
void dSaveDistMail_c::set(const void *bmg, dItem::Item present, u32 version) {
    dMail_c letter;
    if (letter.setFromBMG(bmg)) {
        mMail.copy(&letter);
        mMail.clearFlags2();
        if (present.mId != dItem::ITEM_ID_NONE) {
            mMail.setPresent(present.mId, 1);
        }
        mVersion = version;
    }
}

// 8010F114
void dSaveDistMail_c::setReceived(int player) {
    mMail.setFlag(player);
}

// 8010F11C
void dSaveDistMail_c::clearReceived(int player) {
    mMail.clearFlag(player);
}
