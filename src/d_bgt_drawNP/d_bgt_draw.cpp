#include <game/game/d_bgt_draw.hpp>
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_bg_util.hpp>
#include <game/game/d_bgcf.hpp>
#include <game/game/d_field.hpp>
#include <game/game/d_field_info.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_world.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/mLib/m_mtx.hpp>
#include <cstdio>

namespace dBgtDraw {

// .text:0x5C
void *create() {
    return new dBgtDraw_c;
}

fProf::fBaseProfile_c g_profile_BGT_DRAW = {&create, fProf::BGT_DRAW,
                                            fProf::DRAW_ORDER::BGT_DRAW};

} // namespace dBgtDraw

// .text:0x1DC
int dBgtDraw_c::create() {
    initHeap(dHeap::localHeap_p);
    if (mBlocks.load(dHeap::localHeap_p) && mBlocks.create(&mAllocator)) {
        mCenter = dBGCF::getNextCenter(0);
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text:0x270
int dBgtDraw_c::execute() {
    return mBlocks.execute(&mAllocator);
}

// .text:0x280
int dBgtDraw_c::draw() {
    return mBlocks.draw(&mAllocator);
}

// .text:0x290
int dBgtDraw_c::doDelete() {
    mBlocks.remove(&mAllocator);
    if (mBlocks.unload()) {
        deleteHeap();
        return SUCCEEDED;
    }
    return NOT_READY;
}

// .text:0x2E8
BOOL dBgtDraw_c::block_c::load(void *heap, int blockX, int blockZ) {
    dFdBase_c *info = dField::getInfo(0);
    if (heap != NULL && info != NULL) {
        char path[0x28];
        mBgId = info->getBlock(blockX, blockZ)->getType();
        mBlockX = blockX;
        mBlockZ = blockZ;
        snprintf(path, sizeof(path), "/BgData/BgModel/%03d.brres", mBgId);
        return mBank.load(path, heap, 0);
    }
    return FALSE;
}

// .text:0x39C
BOOL dBgtDraw_c::block_c::create(mAllocator_c *allocator) {
    nw4r::g3d::ResFile res(mBank.getData());
    nw4r::g3d::ResMdl mdl = res.GetResMdl(0);
    mMdl.create(mdl, allocator, 0, 1, NULL);
    if (res.HasResAnmChr()) {
        mAnmChr.create(mdl, res.GetResAnmChr(0), allocator, NULL);
        mAnmChr.mPlayMode = 0;
        mMdl.setAnm(mAnmChr);
    }
    if (res.HasResAnmVis()) {
        mAnmVis.create(mdl, res.GetResAnmVis(0), allocator, NULL);
        mAnmVis.mPlayMode = 0;
        mMdl.setAnm(mAnmVis);
    }
    if (res.HasResAnmClr()) {
        mAnmClr.create(mdl, res.GetResAnmClr(0), allocator, NULL, 1);
        mAnmClr.setPlayMode((m3d::playMode_e)0, 0);
        mMdl.setAnm(mAnmClr);
    }
    if (res.HasResAnmTexSrt()) {
        mAnmSrt.create(mdl, res.GetResAnmTexSrt(0), allocator, NULL, 1);
        mAnmSrt.setPlayMode((m3d::playMode_e)0, 0);
        mMdl.setAnm(mAnmSrt);
    }
    if (res.HasResAnmTexPat()) {
        mAnmPat.create(mdl, res.GetResAnmTexPat(0), allocator, NULL, 1);
        mAnmPat.setPlayMode((m3d::playMode_e)0, 0);
        mMdl.setAnm(mAnmPat);
    }
    mMtx_c mtx;
    PSMTXTrans(mtx, 0.0f, 0.0f, 0.0f);
    mtx.XrotM(mAng(mBlockZ * dWorld::getAngle()));
    mMdl.setLocalMtx(&mtx);
    mMdl.setPriorityDraw(2, 9);
    mMdl.setScale(1.0f, 1.0f, 1.0f);
    dBgUtil::createSigns(res, mBlockX, mBlockZ);
    return TRUE;
}

// .text:0x68C
BOOL dBgtDraw_c::block_c::draw(mAllocator_c *allocator) {
    mMdl.entry();
    return TRUE;
}

// .text:0x6BC
void dBgtDraw_c::block_c::remove(mAllocator_c *allocator) {
    mMdl.remove();
    if (mAnmChr.IsBound()) {
        mAnmChr.remove();
    }
    if (mAnmVis.IsBound()) {
        mAnmVis.remove();
    }
    if (mAnmClr.IsBound()) {
        mAnmClr.remove();
    }
    if (mAnmSrt.IsBound()) {
        mAnmSrt.remove();
    }
    if (mAnmPat.IsBound()) {
        mAnmPat.remove();
    }
}

// .text:0x7AC
bool dBgtDraw_c::blockMng_c::load(void *heap) {
    dFdBase_c *info = dField::getInfo(0);
    if (heap != NULL && info != NULL) {
        bool ok = true;
        for (u32 z = 0; z < BLOCK_Z_MAX; z++) {
            for (u32 x = 0; x < BLOCK_X_MAX; x++) {
                ok &= mBlocks[z][x].load(heap, x, z);
            }
        }
        return ok;
    }
    return false;
}

// .text:0x858
bool dBgtDraw_c::blockMng_c::unload() {
    bool ok = true;
    for (u32 z = 0; z < BLOCK_Z_MAX; z++) {
        for (u32 x = 0; x < BLOCK_X_MAX; x++) {
            ok &= mBlocks[z][x].mBank.unload(FALSE);
        }
    }
    return ok;
}

// .text:0x8D8
bool dBgtDraw_c::blockMng_c::create(mAllocator_c *allocator) {
    bool ok = true;
    for (u32 z = 0; z < BLOCK_Z_MAX; z++) {
        for (u32 x = 0; x < BLOCK_X_MAX; x++) {
            ok &= mBlocks[z][x].create(allocator);
        }
    }
    return ok;
}

// .text:0x95C
bool dBgtDraw_c::blockMng_c::execute(mAllocator_c *allocator) {
    return true;
}

// .text:0x964
bool dBgtDraw_c::blockMng_c::draw(mAllocator_c *allocator) {
    bool ok = true;
    for (u32 z = 0; z < BLOCK_Z_MAX; z++) {
        for (u32 x = 0; x < BLOCK_X_MAX; x++) {
            ok &= mBlocks[z][x].draw(allocator);
        }
    }
    return ok;
}

// .text:0x9E8
void dBgtDraw_c::blockMng_c::remove(mAllocator_c *allocator) {
    for (u32 z = 0; z < BLOCK_Z_MAX; z++) {
        for (u32 x = 0; x < BLOCK_X_MAX; x++) {
            mBlocks[z][x].remove(allocator);
        }
    }
}

// .text:0xA54
void dBgtDraw_c::initHeap(EGG::Heap *parent) {
    if (m_frmHeap_p == NULL) {
        m_frmHeap_p = mHeap::createFrmHeap(
            0x200000, parent, "dBgtDraw_c::m_frmHeap_p : ŠX‚a‚f ƒoƒbƒtƒ@", 0x20,
            mHeap::OPT_NONE);
        if (m_frmHeap_p != NULL) {
            mAllocator.attach(m_frmHeap_p, 0x20);
        }
    }
}

// .text:0xABC
void dBgtDraw_c::deleteHeap() {
    if (m_frmHeap_p != NULL) {
        mHeap::destroyFrmHeap(m_frmHeap_p);
        m_frmHeap_p = NULL;
    }
}

// .text:0xB00
dBgtDraw_c::~dBgtDraw_c() {
}

namespace dBgtDraw {

// Stripped: an unused function that built an m3d::anmChrPart_c. Its vtable,
// generated in the end-of-unit vtable pass before anmVis_c's, is what first
// references m3d::anmChr_c::getType, so that weak copy precedes
// anmVis_c::getType (.text 0xD94 / 0xD9C). MWCC drops the function itself;
// nothing of it reaches the object.
static void unusedPartAnm() {
    m3d::anmChrPart_c anm;
}

} // namespace dBgtDraw
