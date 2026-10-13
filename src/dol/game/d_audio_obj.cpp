// Actor-facing sound objects and the TV/furniture sound adapters.
// .text 800107AC..800120A8; see include/game/game/d_audio_obj.hpp.
#include <game/game/d_audio_obj.hpp>
#include <game/game/d_snd_obj.hpp>
#include <game/game/d_snd_util.hpp>
#define D_AUDIO_OBJ_PINWHEEL_DTOR_IN_SOURCE
#include <game/snd/snd_obj_inline.hpp>
#undef D_AUDIO_OBJ_PINWHEEL_DTOR_IN_SOURCE
#include <game/snd/snd_tv.hpp>
#include <lib/egg/core/eggHeap.h>

static inline const nw4r::math::VEC3 *soundVec(const mVec3_c *pos) {
    return reinterpret_cast<const nw4r::math::VEC3 *>(pos);
}

// 800107AC
dAudioObjCharBase_c::dAudioObjCharBase_c() : mMaxSpeed(1.0f) {}

// 800107C4
dAudioObjCharBase_c::~dAudioObjCharBase_c() {}

// 80010804
void dAudioObjCharBase_c::setMaxSpeed(f32 speed) { mMaxSpeed = speed; }

// 8001080C
f32 dAudioObjCharBase_c::getSpeedRate(f32 speed) {
    if (speed > mMaxSpeed) speed = mMaxSpeed;
    return speed / mMaxSpeed;
}

// 80010828
dAudioObjPlayer_c::dAudioObjPlayer_c() {}

// 8001086C
dAudioObjPlayer_c::~dAudioObjPlayer_c() {}

// 800108D4
void dAudioObjPlayer_c::setPos(const mVec3_c *pos) {
    if (pos != NULL) {
        mVec3_c soundPos;
        getSoundPos(pos, &soundPos);
        mObj.update(soundVec(&soundPos));
    }
}

// 80010928
void dAudioObjPlayer_c::startSound(u32 id) { mObj.startSound(id, NULL); }

// 8001093C
void dAudioObjPlayer_c::startSound(u32 id, const mVec3_c *pos) {
    mVec3_c soundPos;
    getSoundPos(pos, &soundPos);
    mObj.setPosition(soundVec(&soundPos));
    mObj.startSound(id, NULL);
}

// 800109B0
void dAudioObjPlayer_c::holdSound(u32 id) { mObj.holdSound(id); }

// 800109C0
void dAudioObjPlayer_c::holdSound(u32 id, const mVec3_c *pos) {
    mVec3_c soundPos;
    getSoundPos(pos, &soundPos);
    mObj.setPosition(soundVec(&soundPos));
    mObj.holdSound(id);
}

// 80010A30
void dAudioObjPlayer_c::startFootSound(int id, u8 attr, f32 speed) {
    if (id >= 0) mObj.mFoot.start(id, attr, getSpeedRate(speed));
}

// 80010A8C
void dAudioObjPlayer_c::startFootSound(int id, u8 attr, const mVec3_c *pos, f32 speed) {
    if (id >= 0) {
        mVec3_c soundPos;
        getSoundPos(pos, &soundPos);
        mObj.setPosition(soundVec(&soundPos));
        mObj.mFoot.start(id, attr, getSpeedRate(speed));
    }
}

// 80010B20
dAudioObjNpc_c::dAudioObjNpc_c() {}

// 80010B64
dAudioObjNpc_c::~dAudioObjNpc_c() {}

// 80010BCC
void dAudioObjNpc_c::setPos(const mVec3_c *pos) {
    if (pos != NULL) {
        mVec3_c soundPos;
        getSoundPos(pos, &soundPos);
        mObj.update(soundVec(&soundPos));
    }
}

// 80010C20
void dAudioObjNpc_c::startSound(u32 id) { mObj.startSound(id, NULL); }

// 80010C34
void dAudioObjNpc_c::startSound(u32 id, const mVec3_c *pos) {
    mVec3_c soundPos;
    getSoundPos(pos, &soundPos);
    mObj.setPosition(soundVec(&soundPos));
    mObj.startSound(id, NULL);
}

// 80010CA8
void dAudioObjNpc_c::holdSound(u32 id) { mObj.holdSound(id); }

// 80010CB8
void dAudioObjNpc_c::holdSound(u32 id, const mVec3_c *pos) {
    mVec3_c soundPos;
    getSoundPos(pos, &soundPos);
    mObj.setPosition(soundVec(&soundPos));
    mObj.holdSound(id);
}

// 80010D28
void dAudioObjNpc_c::startFootSound(int id, u8 attr, f32 speed) {
    if (id >= 0) mObj.mFoot.start(id, attr, getSpeedRate(speed));
}

// 80010D84
void dAudioObjNpc_c::startFootSound(int id, u8 attr, const mVec3_c *pos, f32 speed) {
    if (id >= 0) {
        mVec3_c soundPos;
        getSoundPos(pos, &soundPos);
        mObj.setPosition(soundVec(&soundPos));
        mObj.mFoot.start(id, attr, getSpeedRate(speed));
    }
}

// 80010E18
dSndObjInsect_c::dSndObjInsect_c() {}
// 80010E48
dSndObjInsect_c::~dSndObjInsect_c() {}
// 80010EA0
void dSndObjInsect_c::setPos(const mVec3_c *pos) {
    if (pos != NULL) {
        mVec3_c soundPos;
        getSoundPos(pos, &soundPos);
        mObj.update(soundVec(&soundPos));
    }
}
// 80010EF4
void dSndObjInsect_c::startSound(u32 id) { mObj.startSound(id, NULL); }
// 80010F08
void dSndObjInsect_c::holdSound(u32 id) { mObj.holdSound(id); }
// 80010F18
bool dSndObjInsect_c::isSounding() { return mObj.isSounding(); }

// 80010F1C
dSndObjSimple_c::dSndObjSimple_c() {}
// 80010F4C
dSndObjSimple_c::~dSndObjSimple_c() {}
// 80010FA4
void dSndObjSimple_c::setPos(const mVec3_c *pos) {
    if (pos != NULL) {
        mVec3_c soundPos;
        getSoundPos(pos, &soundPos);
        mObj.update(soundVec(&soundPos));
    }
}
// 80010FF8
void dSndObjSimple_c::startSound(u32 id) { mObj.startSound(id, NULL); }
// 8001100C
void dSndObjSimple_c::holdSound(u32 id) { mObj.holdSound(id); }

// 8001101C
dSndHaniwa_c::dSndHaniwa_c() {}
// 8001104C
dSndHaniwa_c::~dSndHaniwa_c() {}

// 800110A4
void dSndHaniwa_c::setup(u32 kind, s8 startBar) { mObj.setup(kind, startBar); }
// 800110AC
f32 dSndHaniwa_c::calc(const mVec3_c *pos) {
    mVec3_c soundPos;
    getSoundPos(pos, &soundPos);
    return mObj.calc(soundVec(&soundPos));
}
// 800110EC
void dSndHaniwa_c::play() { mObj.play(); }
// 800110F0
void dSndHaniwa_c::stop() { mObj.stop(); }
// 800110F4
u8 dSndHaniwa_c::getStartBar() { return mObj.mStartBar; }
// 800110FC
int dSndHaniwa_c::getWaitFrames() { return mObj.getWaitFrames(); }

// 80011100
dSndObjInst_c::dSndObjInst_c() {}
// 80011130
dSndObjInst_c::~dSndObjInst_c() {}
// 80011188
void dSndObjInst_c::setup(int inst) { mObj.setup(inst); }
// 8001118C
void dSndObjInst_c::calc(const mVec3_c *pos) {
    mVec3_c soundPos;
    getSoundPos(pos, &soundPos);
    mObj.calc(soundVec(&soundPos));
}
// 800111CC
void dSndObjInst_c::play() { mObj.play(); }
// 800111D0
void dSndObjInst_c::stop() { mObj.stop(); }

// 800111D4
dSndObjFurniture_c::dSndObjFurniture_c() {}
// 80011204
dSndObjFurniture_c::~dSndObjFurniture_c() {}
// 8001125C
void dSndObjFurniture_c::setup(u8 item, bool on) { mObj.setup(item, on); }
// 80011260
void dSndObjFurniture_c::setupSeq(const SoundObjFurniture::SeqData *data, const void *seq, bool on) {
    mObj.setupSeq(data, seq, on);
}
// 80011264
void dSndObjFurniture_c::update(const mVec3_c *pos, f32 frame) {
    mVec3_c soundPos;
    getSoundPos(pos, &soundPos);
    mObj.update(soundVec(&soundPos), frame);
}
// 800112B4
void dSndObjFurniture_c::turnOn() { mObj.turnOn(); }
// 800112B8
void dSndObjFurniture_c::turnOff() { mObj.turnOff(); }
// 800112BC
int dSndObjFurniture_c::getType() { return mObj.getType(); }

// 800112C0
dSndObjPond_c::dSndObjPond_c() {}
// 800112F0
dSndObjPond_c::~dSndObjPond_c() {}
// 80011348
void dSndObjPond_c::setPos(const mVec3_c *pos) {
    mVec3_c soundPos;
    getSoundPos(pos, &soundPos);
    mObj.update(soundVec(&soundPos));
}
// 80011394
void dSndObjPond_c::holdSound(u32 id) { mObj.holdSound(id); }

// 800113A4
dSndObjFall_c::dSndObjFall_c() {}
// 800113D4
dSndObjFall_c::~dSndObjFall_c() {}
// 8001142C
void dSndObjFall_c::setPos(const mVec3_c *pos) {
    mVec3_c soundPos;
    getSoundPos(pos, &soundPos);
    mObj.update(soundVec(&soundPos));
}
// 80011478
void dSndObjFall_c::holdSound(u32 id) { mObj.holdSound(id); }

// 80011488
dSndObjRiver_c::dSndObjRiver_c() {}
// 800114B8
dSndObjRiver_c::~dSndObjRiver_c() {}
// 80011510
void dSndObjRiver_c::setPos(const mVec3_c *pos) {
    mVec3_c soundPos;
    getSoundPos(pos, &soundPos);
    mObj.update(soundVec(&soundPos));
}

// 8001155C
dSndTv_c::dSndTv_c() : mObj(NULL) {}

// 80011568
dSndTv_c::~dSndTv_c() { clear(); }

// 800115BC: channel 0..10 selects the programme; other values select weather.
bool dSndTv_c::create(EGG::Heap *heap, u32 channel) {
    switch (channel) {
    case 0: mObj = new (heap, 4) SoundTv_00_storm; break;
    case 1: mObj = new (heap, 4) SoundTv_01_color; break;
    case 2: mObj = new (heap, 4) SoundTv_02_news; break;
    case 3: mObj = new (heap, 4) SoundTv_03_drama; break;
    case 4: mObj = new (heap, 4) SoundTv_04_song; break;
    case 5: mObj = new (heap, 4) SoundTv_05_quiz; break;
    case 6: mObj = new (heap, 4) SoundTv_06_travel; break;
    case 7: mObj = new (heap, 4) SoundTv_07_hero; break;
    case 8: mObj = new (heap, 4) SoundTv_08_variety; break;
    case 9: mObj = new (heap, 4) SoundTv_09_movie; break;
    case 10: mObj = new (heap, 4) SoundTv_10_soccer; break;
    default: mObj = new (heap, 4) SoundTv_255_weather; break;
    }
    mOn = 0;
    mStarted = 0;
    mPendingOn = 1;
    return mObj != NULL;
}

// 80011B3C
void dSndTv_c::clear() {
    if (mObj != NULL) {
        delete mObj;
        mObj = NULL;
    }
    mStarted = 0;
    mOn = 0;
    mPendingOn = 0;
}

// 80011BA4
bool dSndTv_c::isReady() { return mObj->isReady(); }

// 80011BB8
void dSndTv_c::reset() {
    mObj->reset();
    mStarted = 0;
    mOn = 1;
    mPendingOn = 1;
}

// 80011C08
void dSndTv_c::stop() {
    mObj->stop();
    mStarted = 0;
    mOn = 0;
}

// 80011C50
void dSndTv_c::execute(int frame, const mVec3_c *pos) {
    if (pos == NULL) {
        mObj->execute(frame, NULL);
    } else {
        mVec3_c soundPos;
        getSoundPos(pos, &soundPos);
        mObj->execute(frame, soundVec(&soundPos));
    }
}

// 80011CD0
void dSndTv_c::setOn() {
    if (!mStarted) {
        mObj->setOn(mPendingOn);
        mStarted = 1;
        mPendingOn = 0;
    }
}

// 80011D2C
void dSndTv_c::mute() {
    if (mStarted) {
        mObj->mute();
        mStarted = 0;
    }
}

// 80011D7C
dSndObjPinwheel_c::dSndObjPinwheel_c() {}
// 80011DAC
inline SoundObjPinwheel::~SoundObjPinwheel() {}
// 80011E3C
dSndObjPinwheel_c::~dSndObjPinwheel_c() {}
// 80011ED0
void dSndObjPinwheel_c::update(const mVec3_c *pos, f32 speed) {
    if (pos != NULL) {
        mVec3_c soundPos;
        getSoundPos(pos, &soundPos);
        mObj.update(soundVec(&soundPos), speed);
    }
}
// 80011F28
void dSndObjPinwheel_c::holdSound(u32 id) { mObj.holdSound(id); }
