// Original designs (patterns). .text 8010F124..8010FB6C.
// First pass: every function is written for equivalence.
#include <game/game/d_region.hpp>
#include <lib/egg/core/eggExpHeap.h>
#include <game/game/d_dsn.hpp>
#include <game/game/d_item.hpp>
#include <game/game/d_save_data.hpp>
#include <game/game/d_script.hpp>
#include <revolution/OS/OSCache.h>
#include <lib/egg/core/eggHeap.h>
#include <nw4r/g3d/res/g3d_resfile.h>
#include <cstring>
#include <game/game/d_heap.hpp>

// Dependencies whose owners are not recovered yet.
extern "C" {
void fn_8016AE68(dScript::Word_c *word, u16 index, const char *group); // load a BMG string

// Downloaded item blocks (Ghidra: DLC_Item).
}


// 8010F124
dDesign_c::dDesign_c() {
    clear();
}

// 8010F154
void dDesign_c::clear() {
    mCreator.clear();
    mStyle = 0;
    mPaletteIdx = 0;
    mIsPro = 0;
    memset(mTexture, 0, sizeof(mTexture));
    memset(mName, 0, sizeof(mName));
    memset(mPalette, 0, sizeof(mPalette));
}

// 8010F1C4
void dDesign_c::initBlank() {
    clear();
    memset(mTexture, -1, sizeof(mTexture));
    setPalette(0, TRUE);
}

// 8010F210
void dDesign_c::initDefault() {
    clear();
    setFromItem(dItem::ITEM_IDX_PLAIN_CLOTH_4);
    loadTexture(0x9D3);
}

// 8010F254
BOOL dDesign_c::setFromDlItem(void *dlItem) {
    BOOL ok = TRUE;
    const dItem::BITM *bitm = ((dSaveDLItem_c *)dlItem)->getBITM();
    setFromBITM(((dSaveDLItem_c *)dlItem)->getBITM());
    if (!bitm->m_hasRes) {
        return FALSE;
    }

    EGG::Heap *heap = lbl_8074E440;
    void *data = ((dSaveDLItem_c *)dlItem)->loadArchive(heap); // the .brres
    if (data != NULL) {
        nw4r::g3d::ResFile file(data);
        file.Init();
        file.Bind(file);
        if (file.GetResTexNumEntries() != 0) {
            setTexture(file.GetResTex(0).GetTexData(), FALSE);
        } else {
            ok = FALSE;
        }
        heap->free(data);
    }
    return ok;
}

// 8010F374
BOOL dDesign_c::setFromItem(int index) {
    const dItem::BITM *bitm = dItem::infoBank_c::get()->getBITM(index);
    if (bitm != NULL && setFromBITM(bitm)) {
        return TRUE;
    }
    return FALSE;
}

// 8010F3DC
BOOL dDesign_c::setLandFromTown() {
    dPersonalID_c creator = mCreator;
    if (dSaveData_c::getRaw()->mLandID.isValid()) {
        dSaveData_c* saveData = dSaveData_c::getRaw();
        dLandID_c* landID = &saveData->mLandID;
        creator.land.copy(landID);
        mCreator = creator;
        return TRUE;
    }
    return FALSE;
}

// 8010F5A0
BOOL dDesign_c::loadTexture(int item) {
    dItem::resLoader_c loader;
    if (loader.loadIndex(item, EGG::Heap::getCurrentHeap())) {
        nw4r::g3d::ResFile file(loader.getData());
        setTexture(file.GetResTex(0).GetTexData(), FALSE);
        loader.release();
        return TRUE;
    }
    return FALSE;
}

// 8010F65C
BOOL dDesign_c::loadTextureA(u32 idx) {
    return loadTexture((idx & 7) + 0x9CC);
}

// 8010F668
BOOL dDesign_c::loadTextureB(u32 idx) {
    return loadTexture((idx & 7) + 0x9D4);
}

// 8010F674
BOOL dDesign_c::loadTextureC() {
    return loadTexture(0x9DC);
}

// 8010F67C
BOOL dDesign_c::loadTextureD(u32 idx) {
    return loadTexture((idx & 3) + 0x9DD);
}

// 8010F688
BOOL dDesign_c::isSame(const dDesign_c *other) const {
    return (*this == *other && *getCreator() == *other->getCreator()) != FALSE;
}

// 8010F7B0
void *dDesign_c::getTexture() {
    return mTexture;
}

// 8010F7B4
void dDesign_c::setTexture(const void *src, BOOL flush) {
    memcpy(mTexture, src, sizeof(mTexture));
    if (flush) {
        DCStoreRangeNoSync(mTexture, sizeof(mTexture));
    }
}

// 8010F804
const u16 *dDesign_c::getPaletteData(int idx) {
    const u16 *palette = (const u16 *)dItem::infoBank_c::get()->mpPalettes[idx & 0xF];
    return palette ? palette : sDefaultPalette;
}

// 8010F850
u16 *dDesign_c::getPalette() {
    return mPalette;
}

// 8010F858
void dDesign_c::setPalette(int idx, BOOL flush) {
    mPaletteIdx = idx & 0xF;
    memcpy(mPalette, getPaletteData(idx), sizeof(mPalette));
    if (flush) {
        DCStoreRangeNoSync(mPalette, sizeof(mPalette));
    }
}

// 8010F8C0
void dDesign_c::setName(const wchar_t *name) {
    memcpy(mName, name, sizeof(mName));
    mName[ORG_DESIGN_NAME_LEN] = 0;
}

// 8010F8FC
int dDesign_c::getStyle() const {
    return mStyle < STYLE_NUM ? (STYLE_LOOK)mStyle : STYLE_UNSET;
}

// 8010F914
void dDesign_c::setStyle(u32 style) {
    if (style < STYLE_NUM) {
        mStyle = style;
    } else {
        mStyle = 0;
    }
}

// 8010F930
BOOL dDesign_c::setFromBITM(const dItem::BITM *bitm) {
    if (bitm != NULL) {
        dPersonalID_c creator;
        dHmnName::Word_c creatorName;
        dLandNameWord_c landName;
        dLandID_c land;
        land.clear();
        creator.clear();

        if ((s8)bitm->m_designCreatorName != 0) {
            fn_8016AE68(&landName, (s8)bitm->m_designCreatorName, "sys_STRING/STR_Unit");
        }
        if ((s8)bitm->m_designCreatorFrom != 0) {
            fn_8016AE68(&creatorName, (s8)bitm->m_designCreatorFrom, "sys_STRING/STR_Unit");
        }

        const wchar_t *landStr = static_cast<dScript::Word_c &>(landName).getBuffer();
        land.setName(landStr, getLanguage());
        creator.setPlayer(static_cast<dScript::Word_c &>(creatorName).getBuffer(), dPlayerID_c::ID_UNSET, 0);
        creator.land.copy(&land);
        mCreator = creator;

        setName(bitm->getName());
        setStyle(bitm->m_style < STYLE_NUM ? (STYLE_LOOK)bitm->m_style : STYLE_UNSET);
        setPalette(bitm->m_designPltt, TRUE);
        mIsPro = 0;
        if (bitm->m_proDesign) {
            mIsPro = 1;
        }
        return TRUE;
    }

    return FALSE;
}

// 804EE360: used when the item bank has no palette for an index.
u16 dDesign_c::sDefaultPalette[ORG_DESIGN_PALETTE_COUNT] ALIGN(32) = {
    0x0000, 0x34B9, 0x34E6, 0xD006, 0xF4E6, 0xF7A0, 0xFFFF, 0xFFFF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
};
