// The room object actor base daRmobj_c and its model holder daRmobj_c::model_c. .text 800636C4..80064FB4,
// .data 804A4E60..804A4FB8, .bss 80565500..805655A0, .sdata 80749B68..80749B70, .sbss 8074E1D0..8074E1D8,
// .sdata2 80750318..80750330. model_c::create is in d_a_rmobj_model.cpp. See include/game/game/d_a_rmobj.hpp.
#include <game/game/d_a_rmobj.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/mLib/m_heap.hpp>
#include <revolution/MTX.h>
#include <game/game/d_a_player.hpp>

// The model heap's name ("... : general OBJ heap" in SJIS). Used by create (d_a_rmobj_model.cpp).
char daRmobj_c::model_c::m_frmHeapName[] = "daRmobj_c::m_frmHeap_p : \x89\xAE\x93\xE0OBJ\x83\x71\x81\x5B\x83\x76";
// The animations' rate right after create: stopped.
const f32 daRmobj_c::model_c::cCreateRate = 0.0f;


int daRmobj_c::preCreate() {
    return dDemoActor_c::preCreate() != FALSE;
}

int daRmobj_c::preExecute() {
    return dDemoActor_c::preExecute() != FALSE;
}

void daRmobj_c::postExecute(MAIN_STATE_e state) {
    dActor_c::postExecute(state);
}

int daRmobj_c::preDraw() {
    return dActor_c::preDraw() != FALSE;
}

int daRmobj_c::preDelete() {
    return dDemoActor_c::preDelete() != FALSE;
}

bool daRmobj_c::model_c::remove() {
    if (unload(FALSE)) {
        if (isCreated()) {
            mMdl.remove();
        }
        if (mAnmVis.IsBound()) {
            mAnmVis.remove();
        }
        if (mAnmChr.IsBound()) {
            mAnmChr.remove();
        }
        if (mAnmClr.IsBound()) {
            mAnmClr.remove();
        }
        if (mAnmPat.IsBound()) {
            mAnmPat.remove();
        }
        if (mAnmSrt.IsBound()) {
            mAnmSrt.remove();
        }
        if (m_frmHeap_p != NULL) {
            mHeap::destroyFrmHeap(m_frmHeap_p);
            m_frmHeap_p = NULL;
        }
        return true;
    }
    return false;
}

bool daRmobj_c::model_c::entry() {
    if (isCreated()) {
        return mMdl.entry();
    }
    return false;
}

nw4r::math::MTX34 *daRmobj_c::model_c::getNodeMtx(const char *name) {
    if (isCreated()) {
        int node = m3d::getNodeID(mMdl.getResMdl(), name);
        if (node != -1) {
            static nw4r::math::MTX34 l_mtx;
            mMdl.getNodeWorldMtx(node, &l_mtx);
            return &l_mtx;
        }
    }
    return NULL;
}

mVec3_c *daRmobj_c::model_c::getNodePos(const char *name) {
    static mVec3_c l_pos;
    nw4r::math::MTX34 *mtx = getNodeMtx(name);
    if (mtx != NULL) {
        PSMTXMultVec(mtx->m, mVec3_c::Zero, l_pos);
        return &l_pos;
    }
    return &l_pos;
}

nw4r::math::MTX34 *daRmobj_c::model_c::getNodeMtx(int node) {
    if (isCreated() && node != -1) {
        static nw4r::math::MTX34 l_mtx;
        mMdl.getNodeWorldMtx(node, &l_mtx);
        return &l_mtx;
    }
    return NULL;
}

mVec3_c *daRmobj_c::model_c::getNodePos(int node) {
    static mVec3_c l_pos;
    nw4r::math::MTX34 *mtx = getNodeMtx(node);
    if (mtx != NULL) {
        PSMTXMultVec(mtx->m, mVec3_c::Zero, l_pos);
        return &l_pos;
    }
    return &l_pos;
}

void daRmobj_c::model_c::setRate(f32 rate) {
    setRateChr(rate);
    setRateVis(rate);
    setRateClr(rate);
    setRatePat(rate);
    setRateSrt(rate);
}

BOOL daRmobj_c::model_c::setAnm(u32 idx, m3d::playMode_e mode, f32 blend) {
    BOOL set = FALSE;
    nw4r::g3d::ResFile file(mpData);
    if (idx < file.GetResAnmChrNumEntries() && setAnmChr(idx, mode, blend)) {
        set = TRUE;
    }
    if (idx < file.GetResAnmVisNumEntries() && setAnmVis(idx, mode)) {
        set = TRUE;
    }
    if (idx < file.GetResAnmClrNumEntries() && setAnmClr(idx, mode)) {
        set = TRUE;
    }
    if (idx < file.GetResAnmTexPatNumEntries() && setAnmPat(idx, mode)) {
        set = TRUE;
    }
    if (idx < file.GetResAnmTexSrtNumEntries() && setAnmSrt(idx, mode)) {
        set = TRUE;
    }
    return set;
}

BOOL daRmobj_c::model_c::setAnm(const char *name, m3d::playMode_e mode, f32 blend) {
    BOOL set = FALSE;
    nw4r::g3d::ResFile file(mpData);
    if (file.GetResAnmChr(name).IsValid()) {
        if (setAnmChr(name, mode, blend)) {
            set = TRUE;
        }
    }
    if (file.GetResAnmVis(name).IsValid()) {
        if (setAnmVis(name, mode)) {
            set = TRUE;
        }
    }
    if (file.GetResAnmClr(name).IsValid()) {
        if (setAnmClr(name, mode)) {
            set = TRUE;
        }
    }
    if (file.GetResAnmTexPat(name).IsValid()) {
        if (setAnmPat(name, mode)) {
            set = TRUE;
        }
    }
    if (file.GetResAnmTexSrt(name).IsValid()) {
        if (setAnmSrt(name, mode)) {
            set = TRUE;
        }
    }
    return set;
}

void daRmobj_c::model_c::play() {
    playMdl();
    playVis();
    playClr();
    playPat();
    playSrt();
}

BOOL daRmobj_c::model_c::isStop() {
    bool stop = true;
    stop &= isStopChr();
    stop &= isStopVis();
    stop &= isStopClr();
    stop &= isStopPat();
    stop &= isStopSrt();
    return stop;
}

void daRmobj_c::model_c::setRateChr(f32 rate) {
    if (mAnmChr.IsBound()) {
        mAnmChr.setRate(rate);
    }
}

void daRmobj_c::model_c::setFrameChr(f32 frame) {
    if (mAnmChr.IsBound()) {
        mAnmChr.setFrame(frame);
    }
}

BOOL daRmobj_c::model_c::setAnmChr(u32 idx, m3d::playMode_e mode, f32 blend) {
    if (mAnmChr.IsBound()) {
        nw4r::g3d::ResFile file(mpData);
        if (idx < file.GetResAnmChrNumEntries()) {
        nw4r::g3d::ResAnmChr anm = file.GetResAnmChr(idx);
        mAnmChr.setAnm(mMdl, anm, mode);
        mAnmChr.setRate(1.0f);
        if (mode == m3d::REVERSE_LOOP || mode == m3d::REVERSE_ONCE) { // from the last frame
            mAnmChr.setFrame(anm.GetNumFrame() - 1);
        }
        mMdl.setAnm(mAnmChr, blend);
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

BOOL daRmobj_c::model_c::setAnmChr(const char *name, m3d::playMode_e mode, f32 blend) {
    if (mAnmChr.IsBound()) {
        nw4r::g3d::ResFile file(mpData);
        nw4r::g3d::ResAnmChr anm = file.GetResAnmChr(name);
        if (anm.IsValid()) {
        mAnmChr.setAnm(mMdl, anm, mode);
        mAnmChr.setRate(1.0f);
        if (mode == m3d::REVERSE_LOOP || mode == m3d::REVERSE_ONCE) { // from the last frame
            mAnmChr.setFrame(anm.GetNumFrame() - 1);
        }
        mMdl.setAnm(mAnmChr, blend);
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

void daRmobj_c::model_c::playMdl() {
    mMdl.play();
}

bool daRmobj_c::model_c::isStopChr() {
    if (mAnmChr.IsBound() && (mAnmChr.getPlayMode() == m3d::FORWARD_ONCE || mAnmChr.getPlayMode() == m3d::REVERSE_ONCE)) {
        return mAnmChr.isStop();
    }
    return true;
}

void daRmobj_c::model_c::setRateVis(f32 rate) {
    if (mAnmVis.IsBound()) {
        mAnmVis.setRate(rate);
    }
}

BOOL daRmobj_c::model_c::setAnmVis(u32 idx, m3d::playMode_e mode) {
    if (mAnmVis.IsBound()) {
        nw4r::g3d::ResFile file(mpData);
        if (idx < file.GetResAnmVisNumEntries()) {
        nw4r::g3d::ResAnmVis anm = file.GetResAnmVis(idx);
        mAnmVis.setAnm(mMdl, anm, mode);
        mAnmVis.setRate(1.0f);
        if (mode == m3d::REVERSE_LOOP || mode == m3d::REVERSE_ONCE) { // from the last frame
            mAnmVis.setFrame(anm.GetNumFrame() - 1);
        }
        mMdl.setAnm(mAnmVis);
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

BOOL daRmobj_c::model_c::setAnmVis(const char *name, m3d::playMode_e mode) {
    if (mAnmVis.IsBound()) {
        nw4r::g3d::ResFile file(mpData);
        nw4r::g3d::ResAnmVis anm = file.GetResAnmVis(name);
        if (anm.IsValid()) {
        mAnmVis.setAnm(mMdl, anm, mode);
        mAnmVis.setRate(1.0f);
        if (mode == m3d::REVERSE_LOOP || mode == m3d::REVERSE_ONCE) { // from the last frame
            mAnmVis.setFrame(anm.GetNumFrame() - 1);
        }
        mMdl.setAnm(mAnmVis);
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

void daRmobj_c::model_c::playVis() {
    if (mAnmVis.IsBound()) {
        mAnmVis.play();
    }
}

bool daRmobj_c::model_c::isStopVis() {
    if (mAnmVis.IsBound() && mAnmVis.getPlayMode() == m3d::FORWARD_ONCE) {
        return mAnmVis.isStop();
    }
    return true;
}

void daRmobj_c::model_c::setRateClr(f32 rate) {
    if (mAnmClr.IsBound()) {
        mAnmClr.setRate(rate, 0);
    }
}

BOOL daRmobj_c::model_c::setAnmClr(u32 idx, m3d::playMode_e mode) {
    if (mAnmClr.IsBound()) {
        nw4r::g3d::ResFile file(mpData);
        if (idx < file.GetResAnmClrNumEntries()) {
        nw4r::g3d::ResAnmClr anm = file.GetResAnmClr(idx);
        mAnmClr.setAnm(mMdl, anm, 0, mode);
        mAnmClr.setRate(1.0f, 0);
        if (mode == m3d::REVERSE_LOOP || mode == m3d::REVERSE_ONCE) { // from the last frame
            mAnmClr.setFrame(anm.GetNumFrame() - 1, 0);
        }
        mMdl.setAnm(mAnmClr);
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

BOOL daRmobj_c::model_c::setAnmClr(const char *name, m3d::playMode_e mode) {
    if (mAnmClr.IsBound()) {
        nw4r::g3d::ResFile file(mpData);
        nw4r::g3d::ResAnmClr anm = file.GetResAnmClr(name);
        if (anm.IsValid()) {
        mAnmClr.setAnm(mMdl, anm, 0, mode);
        mAnmClr.setRate(1.0f, 0);
        if (mode == m3d::REVERSE_LOOP || mode == m3d::REVERSE_ONCE) { // from the last frame
            mAnmClr.setFrame(anm.GetNumFrame() - 1, 0);
        }
        mMdl.setAnm(mAnmClr);
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

void daRmobj_c::model_c::playClr() {
    if (mAnmClr.IsBound()) {
        mAnmClr.play();
    }
}

bool daRmobj_c::model_c::isStopClr() {
    if (mAnmClr.IsBound() && mAnmClr.getPlayMode(0) == m3d::FORWARD_ONCE) {
        return mAnmClr.isStop(0);
    }
    return true;
}

void daRmobj_c::model_c::setRatePat(f32 rate) {
    if (mAnmPat.IsBound()) {
        mAnmPat.setRate(rate, 0);
    }
}

BOOL daRmobj_c::model_c::setAnmPat(u32 idx, m3d::playMode_e mode) {
    if (mAnmPat.IsBound()) {
        nw4r::g3d::ResFile file(mpData);
        if (idx < file.GetResAnmTexPatNumEntries()) {
        nw4r::g3d::ResAnmTexPat anm = file.GetResAnmTexPat(idx);
        mAnmPat.setAnm(mMdl, anm, 0, mode);
        mAnmPat.setRate(1.0f, 0);
        if (mode == m3d::REVERSE_LOOP || mode == m3d::REVERSE_ONCE) { // from the last frame
            mAnmPat.setFrame(anm.GetNumFrame() - 1, 0);
        }
        mMdl.setAnm(mAnmPat);
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

BOOL daRmobj_c::model_c::setAnmPat(const char *name, m3d::playMode_e mode) {
    if (mAnmPat.IsBound()) {
        nw4r::g3d::ResFile file(mpData);
        nw4r::g3d::ResAnmTexPat anm = file.GetResAnmTexPat(name);
        if (anm.IsValid()) {
        mAnmPat.setAnm(mMdl, anm, 0, mode);
        mAnmPat.setRate(1.0f, 0);
        if (mode == m3d::REVERSE_LOOP || mode == m3d::REVERSE_ONCE) { // from the last frame
            mAnmPat.setFrame(anm.GetNumFrame() - 1, 0);
        }
        mMdl.setAnm(mAnmPat);
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

void daRmobj_c::model_c::playPat() {
    if (mAnmPat.IsBound()) {
        mAnmPat.play();
    }
}

bool daRmobj_c::model_c::isStopPat() {
    if (mAnmPat.IsBound() && mAnmPat.getPlayMode(0) == m3d::FORWARD_ONCE) {
        return mAnmPat.isStop(0);
    }
    return true;
}

void daRmobj_c::model_c::setRateSrt(f32 rate) {
    if (mAnmSrt.IsBound()) {
        mAnmSrt.setRate(rate, 0);
    }
}

BOOL daRmobj_c::model_c::setAnmSrt(u32 idx, m3d::playMode_e mode) {
    if (mAnmSrt.IsBound()) {
        nw4r::g3d::ResFile file(mpData);
        if (idx < file.GetResAnmTexSrtNumEntries()) {
        nw4r::g3d::ResAnmTexSrt anm = file.GetResAnmTexSrt(idx);
        mAnmSrt.setAnm(mMdl, anm, 0, mode);
        mAnmSrt.setRate(1.0f, 0);
        if (mode == m3d::REVERSE_LOOP || mode == m3d::REVERSE_ONCE) { // from the last frame
            mAnmSrt.setFrame(anm.GetNumFrame() - 1, 0);
        }
        mMdl.setAnm(mAnmSrt);
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

BOOL daRmobj_c::model_c::setAnmSrt(const char *name, m3d::playMode_e mode) {
    if (mAnmSrt.IsBound()) {
        nw4r::g3d::ResFile file(mpData);
        nw4r::g3d::ResAnmTexSrt anm = file.GetResAnmTexSrt(name);
        if (anm.IsValid()) {
        mAnmSrt.setAnm(mMdl, anm, 0, mode);
        mAnmSrt.setRate(1.0f, 0);
        if (mode == m3d::REVERSE_LOOP || mode == m3d::REVERSE_ONCE) { // from the last frame
            mAnmSrt.setFrame(anm.GetNumFrame() - 1, 0);
        }
        mMdl.setAnm(mAnmSrt);
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

void daRmobj_c::model_c::playSrt() {
    if (mAnmSrt.IsBound()) {
        mAnmSrt.play();
    }
}

bool daRmobj_c::model_c::isStopSrt() {
    if (mAnmSrt.IsBound() && mAnmSrt.getPlayMode(0) == m3d::FORWARD_ONCE) {
        return mAnmSrt.isStop(0);
    }
    return true;
}

void daRmobj_c::setLookPos(const mVec3_c *min, const mVec3_c *max) {
    dAcPlayer_c *player = dPlayerMgr::getPlayerActor(4);
    if (player == NULL) {
        return;
    }
    mVec3_c pos = mPos;
    const mVec3_c &target = player->mPos;
    if (target.x < min->x) {
        pos.x = min->x;
    } else {
        pos.x = target.x > max->x ? max->x : target.x;
    }
    if (target.z < min->z) {
        pos.z = min->z;
    } else if (target.z > max->z) {
        pos.z = max->z;
    } else {
        pos.z = target.z;
    }
    mLookPos = pos;
}

mVec3_c *daRmobj_c::getDemoPosition() {
    if (mLookPos.isZero()) {
        return &mPos;
    }
    return &mLookPos;
}
