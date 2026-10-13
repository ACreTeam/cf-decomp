// Effect manager, draw groups and shared footprint emitters.
#include <game/game/d_effect_manager.hpp>
#include <game/game/d_heap.hpp>
#include <game/game/d_camera.hpp>
#include <game/game/d_sky.hpp>
#include <game/game/d_light.hpp>
#include <game/mLib/m_3d/global.hpp>
#include <game/mLib/m_color.hpp>
#include <nw4r/ef/ef_resource.h>
#include <nw4r/ef/ef_drawinfo.h>
#include <nw4r/g3d/g3d_state.h>
#include <revolution/SC.h>

namespace dEffect {
dOneEmitterEffect_c l_playerLeft;
dOneEmitterEffect_c l_playerRight;
dOneEmitterEffect_c l_npcLeft;
dOneEmitterEffect_c l_npcRight;
}

// 80087D84
u32 dEffectGroupManager::getGroupID(u32 flags) {
    if (flags & 2) return 4;
    if (flags & 4) return 5;
    if (flags & 8) return 6;
    if (flags & 16) return 7;
    return EGG::EffectCreator::getGroupID(flags);
}

namespace dEffect {
dEffectManager_c::DrawProc_c *l_drawWorld;
dEffectManager_c::DrawProc_c *l_drawOverlay;
dEffectManager_c::DrawProc_c *l_drawSky;
dEffectManager_c::DrawProc_c *l_drawTinted;
dEffectManager_c::DrawProc_c *l_drawBackground;
static u8 l_active;
static u8 l_calculated;
static mColor l_particleColor;
}

// 80087DCC
BOOL dEffect::initialize() {
    l_active = 0;
    l_calculated = 0;
    BOOL result = mEf::initialize<dEffectGroupManager>(dHeap::effectMngHeap_p, 128, 128, 128, 1000, 1, 8);
    if (result) {
        l_drawWorld = dEffectManager_c::DrawProc_c::create(0, dHeap::effectMngHeap_p);
        l_drawWorld->setPriorityDraw(2, 130);
        l_drawOverlay = dEffectManager_c::DrawProc_c::create(7, dHeap::effectMngHeap_p);
        l_drawOverlay->setPriorityDraw(2, 131);
        l_drawSky = dEffectManager_c::DrawProc_c::create(4, dHeap::effectMngHeap_p);
        l_drawSky->setPriorityDraw(2, 7);
        l_drawTinted = dEffectManager_c::DrawProc_c::create(5, dHeap::effectMngHeap_p);
        l_drawTinted->setPriorityDraw(2, 130);
        l_drawBackground = dEffectManager_c::DrawProc_c::create(6, dHeap::effectMngHeap_p);
        l_drawBackground->setPriorityDraw(2, 8);
        mEf::setCallbackHeap(dHeap::effectCallBackHeap_p);
    }
    return result;
}

// 80087EC0
void dEffect::start() {
    l_active = 1;
    l_calculated = 0;
    l_playerLeft.create("afm_footmark_L");
    l_playerRight.create("afm_footmark_R");
    l_npcLeft.create("afm_npc_footmark_L");
    l_npcRight.create("afm_npc_footmark_R");
}

// 80087F3C
void dEffect::shutdown() {
    if (l_active) {
        l_active = 0;
        l_calculated = 0;
        mEf::shutdown();
    }
}

namespace dEffect {
// The callback receives the particle's drawing parameters (CF adds a color override at 0x6C).
struct particleColor_c {
    /* 0x00 */ u8 _00[0x6C];
    /* 0x6C */ u8 mOverride;
    /* 0x6D */ GXColor mColor;
};
// 80087F5C
static void setParticleColor(particleColor_c *particle, u32) {
    particle->mOverride = 1;
    particle->mColor.r = l_particleColor.r;
    particle->mColor.g = l_particleColor.g;
    particle->mColor.b = l_particleColor.b;
    particle->mColor.a = l_particleColor.a;
}
}

extern "C" {
void fn_8024C6BC(const nw4r::g3d::Camera *, nw4r::math::VEC3 *);
u32 fn_80279660(nw4r::ef::EffectSystem *, int);
void fn_80279824(nw4r::ef::EffectSystem *, void (*)(dEffect::particleColor_c *, u32), u32, int, int);
void fn_80197414(mColor *, int);
}

// Accessor return temporaries preserve the original MWCC float-register allocation.
static inline f32 getTranslationY(const mMtx_c &view) { return view._13; }
static inline f32 getTranslationZ(const mMtx_c &view) { return view._23; }
// 80087F8C
void dEffect::calc() {
    if (!l_active) return;
    l_calculated = 1;
    mEf::beginCalc();
    nw4r::g3d::Camera camera = m3d::getCurrentCamera();
    mVec3_c pos;
    mMtx_c view;
    f32 x, y, width, height, nearZ, farZ;
    fn_8024C6BC(&camera, &pos);
    camera.GetCameraMtx(&view);
    camera.GetViewport(&x, &y, &width, &height, &nearZ, &farZ);
    mEf::setCamera(&pos, &view, nearZ, farZ, 0);
    mEf::setCamera(&pos, &view, nearZ, farZ, 7);
    mEf::setCamera(&pos, &view, nearZ, farZ, 5);
    mEf::setCamera(&pos, &view, nearZ, farZ, 6);
    pos.x = 0.0f; pos.y = 0.0f; pos.z = 0.0f;
    PSMTXIdentity(view);
    mEf::setCamera(&pos, &view, nearZ, farZ, 2);
    if (lbl_8074E830) {
        view = lbl_8074E830->mMtx538;
        pos.x = view._03;
        pos.y = getTranslationY(view);
        pos.z = getTranslationZ(view);
    }
    mEf::setCamera(&pos, &view, nearZ, farZ, 4);
    mEf::endCalc();
    if (fn_80279660(nw4r::ef::EffectSystem::GetInstance(), 5)) {
        fn_80197414(&l_particleColor, 9);
        fn_80279824(nw4r::ef::EffectSystem::GetInstance(), setParticleColor, 0, 5, 1);
    }
}

namespace dEffect {
// EGG fog interface as used here; the rest is not yet recovered.
struct fog_c {
    u8 _00[4];
    GXColor mColor;
    f32 mStart, mEnd, mNear, mFar;
    GXFogType mType;
    u8 mFlags;
    f32 getStart() const { return mStart; }
    f32 getEnd() const { return mEnd; }
    f32 getNear() const { return mNear; }
    f32 getFar() const { return mFar; }
};
static inline nw4r::math::MTX34 makeProjection(f32 fovy, f32 aspect) {
    nw4r::math::MTX34 proj;
    C_MTXLightPerspective(proj, fovy, aspect, 0.5f, -0.5f, 0.5f, 0.5f);
    return proj;
}
static inline nw4r::math::MTX34 makeOrtho(f32 height, f32 width) {
    nw4r::math::MTX34 proj;
    C_MTXLightOrtho(proj, height, -height, -width, width, 0.5f, -0.5f, 0.5f, 0.5f);
    return proj;
}
static inline fog_c *getFog() {
    return **reinterpret_cast<fog_c ***>(reinterpret_cast<u8 *>(m3d::getFogMgr()) + 0xC);
}
}

// 8008816C
void dEffect::draw(int creator, int group) {
    if (!l_active) return;
    if (!l_calculated) return;
    if (!fn_80279660(nw4r::ef::EffectSystem::GetInstance(), group)) return;
    nw4r::ef::DrawInfo info;
    switch (group) {
    case 0: case 5: case 6: case 7: {
        info.mLightEnable = true;
        info.mLightMask = fn_800CD250();
        nw4r::ut::Color color = fn_800CD168(0);
        color.a = 255;
        info.SetAmbientColor(color);
        info.mIsSpotLight = true;
        fog_c *fog = getFog();
        if (fog && (fog->mFlags & 1)) info.SetFog(fog->mType, fog->getStart(), fog->getEnd(), fog->getNear(), fog->getFar(), fog->mColor);
        else info.mFogType = GX_FOG_NONE;
        nw4r::g3d::Camera camera = m3d::getCurrentCamera();
        mMtx_c view;
        camera.GetCameraMtx(&view);
        info.SetViewMtx(view);
        info.SetProjMtx(makeProjection(dCamera_c::getFovy(), lbl_8074E9B0->getAspect()));
        break;
    }
    case 4: {
        info.mLightEnable = true;
        info.mLightMask = fn_800CD250();
        nw4r::ut::Color color = fn_800CD168(0);
        color.a = 255;
        info.SetAmbientColor(color);
        info.mIsSpotLight = true;
        fog_c *fog = getFog();
        if (fog && (fog->mFlags & 1)) info.SetFog(fog->mType, fog->getStart(), fog->getEnd(), fog->getNear(), fog->getFar(), fog->mColor);
        nw4r::g3d::Camera camera = m3d::getCurrentCamera();
        mMtx_c view;
        if (lbl_8074E830) view = lbl_8074E830->mMtx538;
        else camera.GetCameraMtx(&view);
        info.SetViewMtx(view);
        info.SetProjMtx(makeProjection(dCamera_c::getFovy(), lbl_8074E9B0->getAspect()));
        break;
    }
    case 2: {
        info.mLightEnable = false;
        info.mFogType = GX_FOG_NONE;
        mMtx_c view;
        PSMTXIdentity(view);
        info.SetViewMtx(view);
        f32 halfWidth, halfHeight;
        if (!SCGetAspectRatio()) { halfWidth = 320.0f; halfHeight = 240.0f; }
        else { halfWidth = 416.0f; halfHeight = 240.0f; }
        info.SetProjMtx(makeOrtho(halfHeight, halfWidth));
        break;
    }
    case 1: break;
    case 3: return;
    default: return;
    }
    mEf::draw(&info, creator, group);
}

// 800887DC
u32 dEffect::getResourceHeapSize() { return 0xFA000; }
// 800887E8
u32 dEffect::getManagerHeapSize() { return 0x73000; }
// 800887F4
u32 dEffect::getCallbackHeapSize() { return 0x1000; }

namespace dEffect {
// Effect resource-bank list entries, shared by the resource loader (not yet decompiled).
struct resourceBank_c {
    u8 _00[0x10];
    nw4r::ef::EffectProject *mEffectProject;
    nw4r::ef::TextureProject *mTextureProject;
    bool isValid() const { return mEffectProject && mTextureProject; }
};
}
extern "C" nw4r::ut::List lbl_80746E70;

// 800887FC
void dEffect::setTexturePalette(const char *name, u16 *palette) {
    nw4r::ef::Resource *resource = nw4r::ef::Resource::GetInstance();
    resourceBank_c *bank = NULL;
    while ((bank = static_cast<resourceBank_c *>(nw4r::ut::List_GetNext(&lbl_80746E70, bank))) != NULL) {
        if (bank->isValid()) {
            nw4r::ef::ResTexture texture = resource->FindTexture(name, bank->mTextureProject);
            if (texture.IsValid()) texture.ptr()->tlut = reinterpret_cast<u8 *>(palette);
        }
    }
}

// 800888A8
dEffectManager_c::DrawProc_c *dEffectManager_c::DrawProc_c::create(int group, EGG::Heap *heap) {
    DrawProc_c *proc = new (heap, 4) DrawProc_c(group);
    proc->mAllocator.attach(heap, 4);
    proc->m3d::proc_c::create(NULL, NULL);
    return proc;
}

// 8008893C
void dEffectManager_c::DrawProc_c::drawXlu() {
    m3d::resetMaterial();
    nw4r::g3d::G3DState::Invalidate(0x7FF);
    dEffect::draw(0, mGroup);
}

// 8008897C
BOOL dEffect::isActive() { return l_active != 0; }

// 80088990
dEffectManager_c::DrawProc_c::~DrawProc_c() {}

// 80088A00: compiler-generated static initializers.
// 80088AD4 / 80088B74: header definitions, emitted after __sinit with -sym on.
