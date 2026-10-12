// The disc error window (dCaution::Caution_c). .text 80086558..80086C48, .ctors 80465660, .data 804DE860..804DE8C8,
// .bss 80584D80..80585138, .sdata 80749E38..80749E40. See include/game/game/d_caution.hpp.
#include <game/game/d_caution.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_main.hpp>
#include <game/game/d_nand.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_rumble.hpp>
#include <game/game/d_region.hpp>
#include <game/game/d_reset_save.hpp>
#include <game/game/d_snd_util.hpp>
#include <game/game/d_wifi_err.hpp>
#include <lib/revolution/DVD/dvd.h>
#include <lib/revolution/OS.h>
#include <string.h>

#define CAUTION_FONT_SIZE 0x81F4

namespace dCaution {

// 80086558
bool Caution_c::loadFont(nw4r::ut::ResFont *font) {
    bool result;
    const void *data = g_cautionFont;
    void *buf = dHeap::dvdErrHeap_p->alloc(CAUTION_FONT_SIZE, 0x20);
    if (buf == NULL) {
        return false;
    }
    memcpy(buf, data, CAUTION_FONT_SIZE);
    DCFlushRange(buf, CAUTION_FONT_SIZE);
    result = font->SetResource(buf);
    if (!result) {
        dHeap::dvdErrHeap_p->free(buf);
    }
    return result;
}

// 80086620
void Caution_c::removeFont(nw4r::ut::ResFont *font) {
    void *buf = font->RemoveResource();
    if (buf != NULL) {
        dHeap::dvdErrHeap_p->free(buf);
    }
}

// 80086664
wchar_t *Caution_c::getMessage(int msgId) {
    const EGG::MsgRes::MessageInfoDataBlock::Entry *entry =
        (const EGG::MsgRes::MessageInfoDataBlock::Entry *)((const u8 *)mpInfo->mContent + msgId * mpInfo->mEntrySize);
    copyText((const wchar_t *)(mpData + entry->mDataOffset));
    return mText;
}

// 800866B0
void Caution_c::copyText(const wchar_t *text) {
    wchar_t *dst = mText;
    wchar_t c;
    while ((c = *text) != 0) {
        if (c == EGG::MsgRes::cTagMark) {
            // skip the tag: its byte size is at +2
            text = (const wchar_t *)((const u8 *)text + ((const u8 *)text)[2]);
        } else {
            *dst++ = c;
            text++;
        }
    }
    *dst = c;
}

// 800866EC
void Caution_c::create() {
    mResAcc.attach(g_cautionArc, "arc");
    mLyt.mpResAcc = &mResAcc;
    mLyt.build("caution_00.brlyt", NULL);
    mLyt.setFont(&mFont);
    mAnm.create("caution_00.brlan", &mResAcc);
    mAnm.mFrameCtrl.mFlags = m2d::FrameCtrl_c::NO_LOOP;
    mLyt.bind(&mAnm);
    nw4r::lyt::TextBox *text = mLyt.findTextBox("T_dialg_00");
    u16 len = text->GetStringBufferLength();
    text->AllocStringBuffer(len + 0x40);
    setMessage();
    mAnm.play();
    mLyt.calc();
}

// 800867E4
void Caution_c::setMessage() {
    nw4r::lyt::TextBox *text = mLyt.findTextBox("T_dialg_00");
    int noDiskMsg = 1;
    int wrongDiskMsg = 2;
    int diskErrorMsg = 3;
    if (mNetError && !dNet::isNetMgrStarted()) {
        noDiskMsg = 1;
        wrongDiskMsg = 2;
        diskErrorMsg = 3;
    }
    switch (mDriveStatus) {
    case DVD_STATE_IDLE:
        break;
    case DVD_STATE_NO_DISK:
        getMessage(noDiskMsg);
        break;
    case DVD_STATE_WRONG_DISK_ID:
        getMessage(wrongDiskMsg);
        break;
    case DVD_STATE_DISK_ERROR:
        getMessage(diskErrorMsg);
        break;
    }
    text->SetString(mText, 0);
}

// 800868C4
void Caution_c::init() {
    mActive = FALSE;
    mDriveStatus = DVD_STATE_IDLE;
    const EGG::MsgRes::DataBlockHeader *bmg;
    switch (getLanguage()) {
    case LANGUAGE_US:
        bmg = (const EGG::MsgRes::DataBlockHeader *)g_cautionMsgUS;
        break;
    case LANGUAGE_QC:
        bmg = (const EGG::MsgRes::DataBlockHeader *)g_cautionMsgQC;
        break;
    case LANGUAGE_MX:
        bmg = (const EGG::MsgRes::DataBlockHeader *)g_cautionMsgMX;
        break;
    }
    const EGG::MsgRes::DataBlock *block =
        (const EGG::MsgRes::DataBlock *)((const u8 *)bmg + sizeof(EGG::MsgRes::MessageHeader));
    for (u32 i = 0; i < bmg->mNumBlocks; i++) {
        if (block->mKind == 'INF1') {
            mpInfo = (const EGG::MsgRes::MessageInfoDataBlock *)block;
        } else if (block->mKind == 'DAT1') {
            mpData = (const u8 *)((const EGG::MsgRes::MessageDataBlock *)block)->mContent;
        }
        block = (const EGG::MsgRes::DataBlock *)((const u8 *)block + block->mDataSize);
    }
}

// 80086990
u8 Caution_c::check() {
    mDriveStatus = DVDGetDriveStatus();
    if (mDriveStatus != DVD_STATE_IDLE && mDriveStatus != DVD_STATE_BUSY && !mActive) {
        dMain::addStop(dMain::STOP_REASON_CAUTION);
        loadFont(&mFont);
        create();
        mActive = TRUE;
        if (dNet::isNetMgrStarted()) {
            if (!mNetError) {
                mNetError = TRUE;
            }
            g_wifiErr.mForce = TRUE;
            dResetSave::request();
        }
        dRumble::stop();
        fn_8000EB20();
    }
    if (mActive && mDriveStatus == DVD_STATE_IDLE && dNand::isIdle()) {
        if (!mNetError) {
            mActive = FALSE;
            fn_8000EC78();
        }
        mResAcc.detach();
        mLyt.unbind(&mAnm);
        removeFont(&mFont);
        dMain::removeStop(dMain::STOP_REASON_CAUTION);
    }
    return mActive;
}

// 80086AA8
void Caution_c::draw() {
    if (mActive != 1) {
        return;
    }
    if (mResAcc.getResAccessor()->IsAttached()) {
        mLyt.draw();
    } else if (mNetError) {
        g_wifiErr.mForce = FALSE;
        mNetError = FALSE;
        mActive = FALSE;
    }
}

// 80086B00
void Caution_c::execute() {
    if (mActive == 1 && mResAcc.getResAccessor()->IsAttached()) {
        setMessage();
        mAnm.play();
        mLyt.calc();
    }
}

Caution_c g_caution; // 80584D90

} // namespace dCaution
