// The held-up and encyclopedia fish / insect models (dFIMdlMgr_c). .text
// 8018FE10..80190BE4, .rodata 8047C050..8047CC50, .data 804F9AE8..804F9CD0,
// .bss 80624040..80624060, .sdata 8074BF40..8074BF70, .sbss 8074E9E8..8074E9F0.
// See include/game/game/d_fi_mdl_mgr.hpp.
#include <game/framework/f_profile_name.hpp>
#include <game/game/d_fi_mdl_mgr.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_environment.hpp>
#include <game/game/d_fish_info.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_insect_info.hpp>
#include <game/game/d_item.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/mLib/m_mtx.hpp>
#include <cstdio>
#include <revolution/mem.h>

CUSTOM_BASE_PROFILE(FI_MDL_MGR, dFIMdlMgr_c, fProf::FI_MDL_MGR,
                    fProf::DRAW_ORDER::FI_MDL_MGR);

static char l_path[0x20];

// 8018FE40
void dFIMdlBank_c::setHeap(EGG::FrmHeap *heap) {
    mpResHeap = heap;
}

// 8018FE48
BOOL dFIMdlBank_c::load() {
    switch (mType) {
    case TYPE_INSECT_MENU:
        snprintf(l_path, sizeof(l_path), "/Insect/m_%s.brres",
                 dInsectInfo::getModelName(mKind));
        return bank_c::load(l_path, mpResHeap, 0);
    case TYPE_INSECT:
        snprintf(l_path, sizeof(l_path), "/Insect/%s.brres",
                 dInsectInfo::getModelName(mKind));
        return bank_c::load(l_path, mpResHeap, 0);
    case TYPE_FISH:
        if (mMode == MODE_BOOK && mKind == FISH_FROG) {
            snprintf(l_path, sizeof(l_path), "/Fish/m_%s.brres",
                     dFishInfo::getModelName(mKind));
        } else {
            snprintf(l_path, sizeof(l_path), "/Fish/%s.brres",
                     dFishInfo::getModelName(mKind));
        }
        return bank_c::load(l_path, mpResHeap, 0);
    }
    return FALSE;
}

// 8018FFAC
void dFIMdlBank_c::freeData() {
    if (mpResHeap != NULL && mpData != NULL) {
        mpResHeap->free(MEM_FRM_HEAP_FREE_ALL);
    }
}

// 8018FFD4
void dFIObj_c::create(dFIMdlBank_c *bank, int mode) {
    mpBank = bank;
    nw4r::g3d::ResFile file(bank->getData());
    nw4r::g3d::ResMdl mdl = file.GetResMdl(0);

    nw4r::g3d::ResAnmChr anmChr;
    switch (mode) {
    case dFIMdlBank_c::MODE_GET:
        anmChr = file.GetResAnmChr("get");
        break;
    case dFIMdlBank_c::MODE_BOOK:
    default:
        anmChr = file.GetResAnmChr("book");
        break;
    }
    if (!anmChr.IsValid()) {
        anmChr = file.GetResAnmChr(0);
    }

    nw4r::g3d::ResAnmTexSrt anmTexSrt;
    if (file.HasResAnmTexSrt()) {
        switch (mode) {
        case dFIMdlBank_c::MODE_GET:
            anmTexSrt = file.GetResAnmTexSrt("get");
            break;
        case dFIMdlBank_c::MODE_BOOK:
        default:
            anmTexSrt = file.GetResAnmTexSrt("book");
            break;
        }
        if (!anmTexSrt.IsValid()) {
            anmTexSrt = file.GetResAnmTexSrt(0);
        }
    }

    u32 bufferOption = 0;
    if (mode == dFIMdlBank_c::MODE_BOOK) {
        bufferOption |= nw4r::g3d::ScnMdl::BUFFER_RESMATMISC;
    }
    if (file.HasResAnmTexSrt()) {
        bufferOption |= nw4r::g3d::ScnMdl::BUFFER_RESTEXSRT |
                        nw4r::g3d::ScnMdl::BUFFER_RESMATINDMTXSCALE;
    }
    mMdl.create(mdl, &mAllocator, bufferOption, 1, NULL);
    if (mode == dFIMdlBank_c::MODE_BOOK) {
        int lightIdx = dBookLight_c::sLightObjIdx;
        mMdl.setLightSetIdxAll(BOOK_LIGHT_SET);
        mMdl.setEnvMapRefAll(BOOK_LIGHT_SET, lightIdx);
    }

    mAnmChr.create(mdl, anmChr, &mAllocator, NULL);
    mMdl.setAnm(mAnmChr);
    if (file.HasResAnmTexSrt()) {
        mAnmTexSrt.create(mdl, anmTexSrt, &mAllocator, NULL, 1);
        mMdl.setAnm(mAnmTexSrt);
    }
}

// 801901C0
void dFIObj_c::remove() {
    mMdl.remove();
    mAnmChr.remove();
    mAnmTexSrt.remove();
    mpBank = NULL;
}

// 8019022C
void dFIObj_c::setAnm(const char *name) {
    nw4r::g3d::ResFile file(mpBank->getData());
    nw4r::g3d::ResMdl mdl = file.GetResMdl(0);
    mAnmChr.setAnm(mMdl, file.GetResAnmChr(name), m3d::PLAYMODE_INHERIT);
    mMdl.setAnm(mAnmChr);
    if (file.HasResAnmTexSrt()) {
        mAnmTexSrt.setAnm(mMdl, file.GetResAnmTexSrt(name), 0,
                          m3d::PLAYMODE_INHERIT);
        mMdl.setAnm(mAnmTexSrt);
    }
}

// The book poses by fish / insect kind (calcBook).
static const dFIObj_c::Disp_c l_fishDisp[FISH_NUM] = {
    {{-26.5f, 9.0f, -160.0f}, {0.0f, -87.0f, 0.0f}},         // 0
    {{-26.5f, 9.2f, -160.0f}, {0.0f, -85.0f, 0.0f}},         // 1
    {{-26.5f, 8.5f, -160.0f}, {82.529f, -87.275f, 84.111f}}, // 2
    {{-26.2f, 8.5f, -160.0f}, {33.9f, -85.5f, 37.0f}},       // 3
    {{-26.3f, 8.0f, -160.0f}, {0.0f, -81.0f, 0.0f}},         // 4
    {{-26.1f, 7.9f, -160.0f}, {1.0f, -86.0f, -2.5f}},        // 5
    {{-26.1f, 7.9f, -160.0f}, {1.0f, -86.0f, -2.5f}},        // 6
    {{-27.0f, 8.8f, -160.0f}, {0.0f, -76.201f, 0.791f}},     // 7
    {{-27.0f, 8.8f, -160.0f}, {0.0f, -80.0f, 0.0f}},         // 8
    {{-26.0f, 8.8f, -160.0f}, {0.0f, -81.0f, 0.0f}},         // 9
    {{-25.8f, 8.0f, -160.0f}, {-0.8f, -80.0f, 5.5f}},        // 10
    {{-26.0f, 6.2f, -160.0f}, {20.0f, -26.0f, 2.0f}},        // 11
    {{-29.5f, 9.6f, -180.0f}, {0.0f, -77.7f, 0.0f}},         // 12
    {{-26.2f, 9.2f, -160.0f}, {0.0f, -80.0f, 0.0f}},         // 13
    {{-26.2f, 8.0f, -160.0f}, {0.0f, -80.0f, 0.0f}},         // 14
    {{-25.0f, 9.0f, -160.0f}, {21.0f, -87.0f, 27.5f}},       // 15
    {{-25.5f, 9.2f, -160.0f}, {0.0f, -90.0f, 0.0f}},         // 16
    {{-26.2f, 8.5f, -160.0f}, {1.0f, -82.5f, 1.0f}},         // 17
    {{-26.5f, 8.0f, -160.0f}, {0.0f, -85.0f, 0.0f}},         // 18
    {{-26.7f, 8.5f, -160.0f}, {3.0f, -78.0f, 0.0f}},         // 19
    {{-24.0f, 8.0f, -140.0f}, {0.0f, -85.0f, 0.0f}},         // 20
    {{-23.5f, 9.2f, -160.0f}, {0.0f, -90.0f, 0.0f}},         // 21
    {{-26.0f, 8.0f, -160.0f}, {0.0f, -90.0f, 0.0f}},         // 22
    {{-26.2f, 8.5f, -160.0f}, {0.0f, -85.0f, 0.0f}},         // 23
    {{-26.2f, 8.5f, -160.0f}, {0.0f, -85.0f, 0.0f}},         // 24
    {{-28.0f, 8.5f, -160.0f}, {0.0f, -85.0f, 0.0f}},         // 25
    {{-26.0f, 8.0f, -167.75f}, {0.0f, -82.5f, 0.0f}},        // 26
    {{-26.0f, 8.0f, -160.0f}, {-8.5f, -81.0f, -8.5f}},       // 27
    {{-26.5f, 8.0f, -160.0f}, {0.0f, -80.0f, 0.0f}},         // 28
    {{-26.5f, 8.7f, -160.0f}, {0.0f, -75.0f, 0.0f}},         // 29
    {{-27.0f, 9.0f, -160.0f}, {0.0f, -85.0f, 0.0f}},         // 30
    {{-26.5f, 8.7f, -160.0f}, {0.0f, -88.0f, 0.0f}},         // 31
    {{-25.5f, 8.5f, -160.0f}, {0.0f, -78.0f, 0.0f}},         // 32
    {{-25.2f, 8.0f, -160.0f}, {0.0f, -90.0f, 0.0f}},         // 33
    {{-26.5f, 8.0f, -160.0f}, {-1.0f, -85.0f, -1.0f}},       // 34
    {{-29.0f, 8.5f, -163.25f}, {3.955f, -90.0f, 5.537f}},    // 35
    {{-36.0f, 9.8f, -185.5f}, {0.0f, -79.365f, 0.0f}},       // 36
    {{-25.8f, 6.2f, -160.0f}, {0.0f, -85.0f, 0.0f}},         // 37
    {{-25.8f, 8.0f, -160.0f}, {5.5f, -95.0f, -1.0f}},        // 38
    {{-26.5f, 4.0f, -160.0f}, {0.0f, -90.0f, 0.0f}},         // 39
    {{-24.5f, 8.5f, -160.0f}, {0.0f, -70.0f, 0.0f}},         // 40
    {{-25.5f, 8.5f, -160.0f}, {0.0f, -85.0f, 0.0f}},         // 41
    {{-26.0f, 8.2f, -160.0f}, {0.0f, -80.0f, 0.0f}},         // 42
    {{-25.0f, 9.0f, -172.25f}, {0.0f, -90.0f, 0.0f}},        // 43
    {{-30.0f, 8.0f, -181.0f}, {0.0f, -76.0f, 0.0f}},         // 44
    {{-27.5f, 9.0f, -160.0f}, {0.0f, -58.0f, 0.0f}},         // 45
    {{-26.0f, 8.5f, -160.0f}, {0.0f, -80.0f, 0.0f}},         // 46
    {{-26.0f, 8.56f, -160.0f}, {0.0f, -75.0f, 0.0f}},        // 47
    {{-25.2f, 8.5f, -160.0f}, {0.0f, -80.0f, 0.0f}},         // 48
    {{-26.0f, 8.5f, -160.0f}, {0.0f, -75.0f, 0.0f}},         // 49
    {{-26.0f, 7.5f, -160.0f}, {0.0f, -85.0f, 0.0f}},         // 50
    {{-29.0f, 8.0f, -172.25f}, {0.0f, -71.543f, 0.0f}},      // 51
    {{-25.7f, 9.0f, -160.0f}, {4.0f, -81.0f, -1.0f}},        // 52
    {{-25.7f, 9.5f, -160.0f}, {0.0f, -78.0f, -0.0f}},        // 53
    {{-26.5f, 5.25f, -167.75f}, {0.0f, -82.529f, 0.0f}},     // 54
    {{-27.0f, 9.0f, -172.25f}, {0.0f, -80.0f, 0.0f}},        // 55
    {{-25.5f, 4.5f, -160.0f}, {0.0f, -80.0f, 0.0f}},         // 56
    {{-26.8f, 8.5f, -167.75f}, {0.0f, -80.0f, 0.0f}},        // 57
    {{-27.0f, 8.5f, -181.0f}, {0.0f, -80.0f, 0.0f}},         // 58
    {{-37.0f, 12.0f, -221.25f}, {0.0f, -69.961f, 0.0f}},     // 59
    {{-23.0f, 13.0f, -181.0f}, {0.0f, -87.275f, 0.0f}},      // 60
    {{-25.0f, 8.0f, -176.75f}, {0.0f, -66.797f, -0.0f}},     // 61
    {{-28.0f, 9.0f, -194.5f}, {10.0f, -69.961f, 0.0f}},      // 62
    {{-29.5f, 9.8f, -176.75f}, {0.0f, -85.0f, 0.0f}},        // 63
};
static const dFIObj_c::Disp_c l_insectDisp[INSECT_NUM] = {
    {{-17.65f, 6.7f, -110.0f}, {90.0f, 180.0f, 8.0f}},  // 0
    {{-17.65f, 6.7f, -110.0f}, {90.0f, 180.0f, 8.0f}},  // 1
    {{-17.4f, 5.7f, -110.0f}, {90.0f, 180.0f, 8.0f}},   // 2
    {{-17.4f, 5.7f, -110.0f}, {90.0f, 180.0f, 8.0f}},   // 3
    {{-17.6f, 6.4f, -110.0f}, {90.0f, 180.0f, 8.0f}},   // 4
    {{-14.4f, 5.2f, -90.0f}, {90.0f, 180.0f, 8.0f}},    // 5
    {{-17.7f, 6.1f, -110.0f}, {90.0f, 180.0f, 8.0f}},   // 6
    {{-13.9f, 4.0f, -87.25f}, {90.0f, -180.0f, 3.0f}},  // 7
    {{-17.7f, 6.5f, -110.0f}, {90.0f, 180.0f, 8.0f}},   // 8
    {{-17.7f, 6.2f, -110.0f}, {90.0f, 180.0f, 8.0f}},   // 9
    {{-18.3f, 8.5f, -115.0f}, {90.0f, -179.5f, 8.0f}},  // 10
    {{-17.6f, 6.5f, -110.0f}, {90.0f, 180.0f, 5.5f}},   // 11
    {{-17.6f, 6.1f, -110.0f}, {90.0f, 180.0f, 8.0f}},   // 12
    {{-16.5f, 5.9f, -110.0f}, {90.0f, 178.0f, 8.0f}},   // 13
    {{-17.8f, 5.9f, -110.0f}, {90.0f, 178.0f, 8.0f}},   // 14
    {{-18.5f, 5.0f, -110.0f}, {90.0f, 175.0f, 9.0f}},   // 15
    {{-18.0f, 4.5f, -110.0f}, {85.0f, 180.0f, 8.0f}},   // 16
    {{-17.6f, 7.5f, -110.0f}, {90.0f, 180.0f, 7.0f}},   // 17
    {{-17.6f, 7.5f, -110.0f}, {90.0f, 180.0f, 7.0f}},   // 18
    {{-17.7f, 7.5f, -110.0f}, {90.0f, 180.0f, 8.0f}},   // 19
    {{-17.6f, 7.5f, -110.0f}, {90.0f, 180.0f, 8.0f}},   // 20
    {{-17.5f, 7.0f, -110.0f}, {90.0f, 180.0f, 8.0f}},   // 21
    {{-17.65f, 7.5f, -110.0f}, {90.0f, 180.0f, 7.5f}},  // 22
    {{-17.65f, 7.6f, -110.0f}, {90.0f, 180.0f, 7.5f}},  // 23
    {{-17.5f, 8.7f, -110.0f}, {90.0f, 180.0f, 5.0f}},   // 24
    {{-15.3f, 9.5f, -96.25f}, {90.0f, 180.0f, 6.0f}},   // 25
    {{-11.2f, 3.8f, -69.25f}, {90.0f, -178.0f, 7.0f}},  // 26
    {{-17.7f, 6.2f, -110.0f}, {90.0f, 180.0f, 8.0f}},   // 27
    {{-17.7f, 6.5f, -110.0f}, {93.0f, 180.0f, 8.0f}},   // 28
    {{-17.5f, 4.5f, -110.0f}, {70.0f, -177.5f, 8.0f}},  // 29
    {{-17.5f, 5.5f, -110.0f}, {90.0f, 180.0f, 8.0f}},   // 30
    {{-19.1f, 5.5f, -120.0f}, {95.0f, -180.0f, 8.0f}},  // 31
    {{-17.5f, 5.9f, -110.0f}, {90.0f, 178.0f, 8.0f}},   // 32
    {{-17.65f, 6.5f, -110.0f}, {90.0f, 180.0f, 10.0f}}, // 33
    {{-15.2f, 5.3f, -95.0f}, {90.0f, 180.0f, 5.0f}},    // 34
    {{-17.5f, 6.5f, -110.0f}, {90.0f, 180.0f, 8.0f}},   // 35
    {{-17.8f, 5.8f, -110.0f}, {90.0f, 180.0f, 8.0f}},   // 36
    {{-17.7f, 6.2f, -110.0f}, {90.0f, 180.0f, 10.2f}},  // 37
    {{-12.9f, 4.2f, -80.0f}, {90.0f, 178.0f, 5.5f}},    // 38
    {{-12.6f, 6.0f, -78.0f}, {90.0f, 180.0f, 13.5f}},   // 39
    {{-15.5f, 5.8f, -96.0f}, {90.0f, 180.0f, 8.5f}},    // 40
    {{-17.6f, 6.5f, -110.0f}, {90.0f, 180.0f, 8.0f}},   // 41
    {{-17.7f, 6.5f, -110.0f}, {90.0f, 180.0f, 6.0f}},   // 42
    {{-17.7f, 6.5f, -110.0f}, {90.0f, 180.0f, 6.0f}},   // 43
    {{-15.5f, 6.0f, -96.5f}, {90.0f, 178.0f, 8.5f}},    // 44
    {{-17.6f, 5.6f, -110.0f}, {90.0f, 179.0f, 7.0f}},   // 45
    {{-17.6f, 5.6f, -110.0f}, {90.0f, 179.0f, 7.0f}},   // 46
    {{-17.6f, 5.7f, -110.0f}, {90.0f, 180.0f, 7.0f}},   // 47
    {{-17.5f, 6.1f, -110.0f}, {90.0f, 180.0f, 8.5f}},   // 48
    {{-17.2f, 4.0f, -108.0f}, {90.0f, 180.0f, 8.0f}},   // 49
    {{-17.7f, 6.0f, -110.0f}, {90.0f, 180.0f, 8.0f}},   // 50
    {{-17.5f, 6.0f, -110.0f}, {86.0f, -179.0f, 8.5f}},  // 51
    {{-16.5f, 6.8f, -105.0f}, {90.0f, 180.0f, 8.6f}},   // 52
    {{-17.3f, 6.8f, -110.0f}, {90.0f, 180.0f, 7.031f}}, // 53
    {{-15.1f, 6.0f, -96.25f}, {90.0f, 180.0f, 7.0f}},   // 54
    {{-17.5f, 6.7f, -110.0f}, {90.0f, 180.0f, 8.0f}},   // 55
    {{-4.7f, 1.6f, -29.0f}, {90.0f, 180.0f, 8.0f}},     // 56
    {{-16.7f, 6.0f, -105.0f}, {90.0f, 180.0f, 5.537f}}, // 57
    {{-16.8f, 6.25f, -105.0f}, {90.0f, 180.0f, 8.0f}},  // 58
    {{-17.5f, 6.4f, -110.0f}, {90.0f, 180.0f, 7.0f}},   // 59
    {{-18.0f, 6.5f, -110.0f}, {79.0f, 180.0f, 8.0f}},   // 60
    {{-17.5f, 6.5f, -110.0f}, {90.0f, 180.0f, 8.0f}},   // 61
    {{-17.5f, 6.0f, -110.0f}, {90.0f, 180.0f, 8.0f}},   // 62
    {{-17.5f, 5.5f, -110.0f}, {103.0f, 180.0f, 8.0f}},  // 63
};

// 80190304
void dFIObj_c::calcBook() {
    Vec pos;
    mAng rotX, rotY, rotZ;
    if (mpBank->mType == dFIMdlBank_c::TYPE_FISH) {
        pos.x = l_fishDisp[mpBank->mKind].mPos.x;
        pos.y = l_fishDisp[mpBank->mKind].mPos.y;
        pos.z = l_fishDisp[mpBank->mKind].mPos.z;
        Vec rot = l_fishDisp[mpBank->mKind].mRot;
        rotX = mAng::fromDegree(rot.x);
        rotY = mAng::fromDegree(rot.y);
        rotZ = mAng::fromDegree(rot.z);
    } else {
        pos.x = l_insectDisp[mpBank->mKind].mPos.x;
        pos.y = l_insectDisp[mpBank->mKind].mPos.y;
        pos.z = l_insectDisp[mpBank->mKind].mPos.z;
        Vec rot = l_insectDisp[mpBank->mKind].mRot;
        rotX = mAng::fromDegree(rot.x);
        rotY = mAng::fromDegree(rot.y);
        rotZ = mAng::fromDegree(rot.z);
    }
    mMtx_c mtx;
    PSMTXTrans(mtx, pos.x, pos.y, pos.z);
    mtx.XrotM(rotX);
    mtx.YrotM(rotY);
    mtx.ZrotM(rotZ);
    if (mAnmTexSrt.IsBound()) {
        mAnmTexSrt.play();
    }
    mMdl.play();
    mMdl.setLocalMtx(&mtx);
    mMdl.calc(false);
}

// 801904F4
dFIMdlMgr_c::dFIMdlMgr_c() {
    mCreated = 0;
    mDeleting = 0;
}

// 8019057C
dFIMdlMgr_c::~dFIMdlMgr_c() {
}

// 8019060C
int dFIMdlMgr_c::create() {
    EGG::Heap *parent = dHeap::localHeap_p;
    if (!mCreated) {
        for (int i = 0; i < OBJ_NUM; i++) {
            mpResHeaps[i] = mHeap::createFrmHeap(RES_HEAP_SIZE, parent,
                                                 "dFIMdlMgr_c::m_ResHeap", 0x20,
                                                 mHeap::OPT_NONE);
            mBanks[i].setHeap(mpResHeaps[i]);
        }
        sInstance = this;
        mCreated = 1;
    }
    return 1;
}

// 801906A8
int dFIMdlMgr_c::execute() {
    loadObjs();
    unloadObjs();
    return 1;
}

// 801906E0
int dFIMdlMgr_c::draw() {
    return 1;
}

// 801906E8
int dFIMdlMgr_c::doDelete() {
    if (!mDeleting) {
        mDeleting = 1;
    }
    if (!unloadObjs()) {
        return 0;
    }
    for (int i = 0; i < OBJ_NUM; i++) {
        mHeap::destroyFrmHeap(mpResHeaps[i]);
        mpResHeaps[i] = NULL;
    }
    sInstance = NULL;
    return 1;
}

// 8019077C
BOOL dFIMdlMgr_c::loadObjs() {
    BOOL done = TRUE;
    for (int i = 0; i < OBJ_NUM; i++) {
        if (mBanks[i].mState == dFIMdlBank_c::STATE_LOADING) {
            if (mBanks[i].load()) {
                if (mBanks[i].mMakeMdl) {
                    mBanks[i].mState = dFIMdlBank_c::STATE_MDL;
                    dFIObj_c *obj = &mObjs[i];
                    obj->mAllocator.attach(mpResHeaps[i], 4);
                    obj->create(&mBanks[i], mBanks[i].mMode);
                } else {
                    mBanks[i].mState = dFIMdlBank_c::STATE_LOADED;
                }
            } else {
                done = FALSE;
            }
        }
    }
    return done;
}

// 80190844
BOOL dFIMdlMgr_c::unloadObjs() {
    BOOL done = TRUE;
    for (int i = 0; i < OBJ_NUM; i++) {
        if (mBanks[i].mState == dFIMdlBank_c::STATE_REMOVE) {
            mObjs[i].remove();
            mBanks[i].mState = dFIMdlBank_c::STATE_UNLOADING;
        }
        if (mBanks[i].mState == dFIMdlBank_c::STATE_UNLOADING) {
            if (mBanks[i].unload(FALSE)) {
                mBanks[i].mState = dFIMdlBank_c::STATE_NONE;
            } else {
                done = FALSE;
            }
        } else if (mBanks[i].mState == dFIMdlBank_c::STATE_LOADED ||
                   mBanks[i].mState == dFIMdlBank_c::STATE_MDL) {
            done = FALSE;
        }
    }
    return done;
}

// 80190900
BOOL dFIMdlMgr_c::request(int idx, int type, const dItem::Item &item,
                          BOOL makeMdl, int mode) {
    return request(idx, type, dItem::seeker_c::get()->findLike(item), makeMdl,
                   mode);
}

// 80190970
BOOL dFIMdlMgr_c::request(int idx, int type, int kind, BOOL makeMdl, int mode) {
    BOOL loaded = FALSE;
    dFIMdlBank_c *bank = &mBanks[idx];
    switch (bank->mState) {
    case dFIMdlBank_c::STATE_NONE:
        bank->mState = dFIMdlBank_c::STATE_LOADING;
        bank->mType = type;
        bank->mKind = kind;
        bank->mMakeMdl = makeMdl;
        bank->mMode = mode;
        break;
    case dFIMdlBank_c::STATE_LOADING:
        break;
    case dFIMdlBank_c::STATE_LOADED:
    case dFIMdlBank_c::STATE_MDL:
        loaded = TRUE;
        break;
    case dFIMdlBank_c::STATE_REMOVE:
    case dFIMdlBank_c::STATE_UNLOADING:
        break;
    }
    return loaded;
}

// 801909D4
void dFIMdlMgr_c::release(int idx) {
    dFIMdlBank_c *bank = &mBanks[idx];
    if (bank->mState == dFIMdlBank_c::STATE_MDL) {
        bank->mState = dFIMdlBank_c::STATE_REMOVE;
    } else if (bank->mState == dFIMdlBank_c::STATE_LOADING ||
               bank->mState == dFIMdlBank_c::STATE_LOADED) {
        bank->mState = dFIMdlBank_c::STATE_UNLOADING;
    }
}

dFIMdlMgr_c *dFIMdlMgr_c::sInstance;
