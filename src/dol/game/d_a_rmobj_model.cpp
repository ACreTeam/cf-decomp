// daRmobj_c::model_c::create: loads /Rmobj/<name>.brres and builds the model and its animations. A TU of its own:
// .text 80064FB4..80065230, .data 804A4FB8..804A4FC8 (the path format); the heap name and the start rate are
// d_a_rmobj.cpp's. See include/game/game/d_a_rmobj.hpp.
#include <game/game/d_a_rmobj.hpp>
#include <game/game/d_heap.hpp>
#include <game/mLib/m_heap.hpp>
#include <cstdio>

bool daRmobj_c::model_c::create(const char *name, u32 bufferOption) {
    char path[0x20];
    sprintf(path, "/Rmobj/%s.brres", name);
    if (load(path, dHeap::localHeap_p, 0)) {
        if (isCreated()) {
            return true;
        }
        m_frmHeap_p = mHeap::createFrmHeap(0x7800, dHeap::localHeap_p, m_frmHeapName, 0x20, mHeap::OPT_NONE);
        mAllocator.attach(m_frmHeap_p, 0x20);
        nw4r::g3d::ResFile file(mpData);
        nw4r::g3d::ResMdl mdl = file.GetResMdl(0);
        mMdl.create(mdl, &mAllocator, bufferOption, 1, NULL);
        if (file.HasResAnmChr()) {
            mAnmChr.create(mdl, file.GetResAnmChr(0), &mAllocator, NULL);
            mAnmChr.setRate(cCreateRate);
        }
        if (file.HasResAnmVis()) {
            mAnmVis.create(mdl, file.GetResAnmVis(0), &mAllocator, NULL);
            mAnmVis.setRate(cCreateRate);
        }
        if (file.HasResAnmClr()) {
            mAnmClr.create(mdl, file.GetResAnmClr(0), &mAllocator, NULL, 1);
            mAnmClr.setRate(cCreateRate, 0);
        }
        if (file.HasResAnmTexSrt()) {
            mAnmSrt.create(mdl, file.GetResAnmTexSrt(0), &mAllocator, NULL, 1);
            mAnmSrt.setRate(cCreateRate, 0);
        }
        if (file.HasResAnmTexPat()) {
            mAnmPat.create(mdl, file.GetResAnmTexPat(0), &mAllocator, NULL, 1);
            mAnmPat.setRate(cCreateRate, 0);
        }
        return true;
    }
    return false;
}
