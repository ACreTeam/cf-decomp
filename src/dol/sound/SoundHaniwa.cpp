// The gyroids: a sequence played in time with the music player's song. .text 801D5390..801D64F4, .ctors
// 8046585C..80465860, .data 80501018..80501070, .bss 80625D60..80625D88, .sdata 8074C698..8074C6A0, .sdata2
// 80751BB8..80751BE8. See include/game/snd/snd_obj.hpp.
#include <game/snd/snd_manager.hpp>
#include <game/snd/snd_obj.hpp>
#include <game/snd/snd_conductor.hpp>
#include <game/snd/snd_system.hpp>
#include <game/snd/snd_id.hpp>

// Where the gyroids in the house sound from.
static nw4r::math::VEC3 sHousePos(0.0f, 0.0f, -200.0f);

// 801D5390
s16 SoundHaniwa::getStartBar() {
    SoundSystem *system = SoundSystem::getInstance();
    u32 beat = SoundSystem::getInstance()->mConductor.calcBeat();
    s16 bar = beat >> 16;
    s16 prevBeat = beat & 0xFFFF;
    bar++;
    if (bar > 15) {
        bar = 0;
    }
    if (75.0f * (int)(system->mConductor.mBeatTicks - prevBeat) / system->mConductor.mTempo < 6.0f) {
        bar++;
        if (bar > 15) {
            bar = 0;
        }
    }
    return bar;
}

// 801D54C4
s16 SoundHaniwa::getNextBar() {
    u32 beat = SoundSystem::getInstance()->mConductor.calcBeat();
    s16 bar = beat >> 16;
    if (bar++ > 15) {
        bar = 0;
    }
    return bar;
}

// 801D5548
SoundHaniwa::SoundHaniwa()
    : mConductor(NULL), mSoundId(0), mInHouse(0), mStartBar(-1), mWaiting(false), mBarMask(0), mLoopKind(-1),
      mLoopMode(-1), mPart(-1), mNextPart(-1), mLoopLen(-1), mLoopDone(false), mNext(NULL), mPrev(NULL) {
    mConductor = &SoundSystem::getInstance()->mConductor;
    mConductor->appendHaniwa(this);
}

// 801D5648
SoundHaniwa::~SoundHaniwa() {
    stop();
    mConductor->removeHaniwa(this);
}

// 801D56F4
void SoundHaniwa::setup(u32 kind, s8 startBar) {
    if (kind > 0x7E) {
        kind = 0;
    }
    if (startBar < -1 || startBar > 15) {
        startBar = -1;
    }
    mSoundId = SEQ_HNW_BCN_DKA + kind;
    if (mHandles[0].IsAttachedSound()) {
        mHandles[0].Stop(0);
    }
    mBarMask = 0;
    mLoopKind = -1;
    mNextPart = -1;
    mLoopDone = false;
    mStartBar = startBar;
    mWaiting = false;

    s16 nextBar = getNextBar();
    int stage = SoundManager::getInstance()->_778;
    if ((stage >= 0x27 && stage <= 0x2B) || stage == 0x25) {
        mInHouse = 1;
        mStartBar = getStartBar();
        mPos = sHousePos;
    }
    if (mStartBar >= 0) {
        mWaiting = true;
        if (nextBar == mStartBar) {
            startSound(mSoundId, NULL);
        }
    }
}

// 801D5940
void SoundHaniwa::restart() {
    if (mHandles[0].IsAttachedSound()) {
        mHandles[0].Stop(0);
    }
    mWaiting = true;
}

// 801D598C
void SoundHaniwa::reset() {
    if (mStartBar >= 0) {
        mBarMask = 0;
        mLoopKind = -1;
        mNextPart = -1;
        mHandles[0].Stop(0);
        mWaiting = true;
    }
}

// 801D59F4
void SoundHaniwa::play() {
    if (mHandles[0].IsAttachedSound()) {
        mHandles[0].Stop(0);
    }
    mStartBar = getStartBar();
    mWaiting = true;
    startSound(mSoundId, NULL);
}

// 801D5A64
void SoundHaniwa::stop() {
    mStartBar = -1;
    mWaiting = false;
    mHandles[0].Stop(0);
}

// 801D5A90
int SoundHaniwa::getWaitFrames() {
    u32 beat = mConductor->calcBeat();
    int bar = beat >> 16;
    int ticks;
    int beatTicks = mConductor->getBeatTicks();
    u32 prevBeat = beat & 0xFFFF;
    int startBar = mStartBar;
    s16 tempo = mConductor->mTempo;
    if (startBar < bar) {
        startBar += 16;
    }
    if (startBar <= bar) {
        ticks = beatTicks - prevBeat;
        startSound(mSoundId, NULL);
    } else {
        ticks = beatTicks * (startBar - bar) - prevBeat;
    }
    int frames = 75.0f * ticks / tempo;
    if (frames < 1) {
        frames = 1;
    }
    return frames;
}

// 801D5B84
f32 SoundHaniwa::calc(const nw4r::math::VEC3 *pos) {
    if (mInHouse > 0) {
        pos = &sHousePos;
    }
    u32 beat = mConductor->calcBeat();
    s16 bar = beat >> 16;
    s16 nextBar = bar + 1;
    if (nextBar > 15) {
        nextBar = 0;
    }
    update(pos);

    f32 progress = -1.0f;
    if (mStartBar >= 0) {
        if (!mWaiting) {
            readVariables();
            calcPart();
            updateTempo();
            progress = calcProgress();
        } else {
            progress = 0.0f;
            if (mHandles[0].IsAttachedSound()) {
                if (bar == mStartBar) {
                    mWaiting = false;
                    readVariables();
                    calcPart();
                    updateTempo();
                    progress = calcProgress();
                }
            } else if (nextBar == mStartBar) {
                startSound(mSoundId, NULL);
            }
        }
    }
    return progress;
}

// 801D5CDC
void SoundHaniwa::calcPart() {
    if (mNextPart != -1 && mNextPart == mPart) {
        BOOL found = FALSE;
        int i;
        for (i = 15 - (mPart + 1); i >= 0; i--) {
            if ((mBarMask >> i) & 1) {
                found = TRUE;
                break;
            }
        }
        if (!found) {
            for (i = 15; i >= 0; i--) {
                if ((mBarMask >> i) & 1) {
                    break;
                }
            }
        }
        mNextPart = 15 - i;
        nw4r::snd::SeqSoundHandle seq(&mHandles[0]);
        seq.WriteVariable(12, mNextPart);
    }
}

// 801D5E34
void SoundHaniwa::readVariables() {
    if (mLoopKind < 0) {
        readLoop();
    }
    nw4r::snd::SeqSoundHandle seq(&mHandles[0]);
    s16 part;
    s16 nextPart;
    s16 len;
    seq.ReadVariable(8, &part);
    seq.ReadVariable(12, &nextPart);
    seq.ReadVariable(6, &len);
    mPart = part;
    mNextPart = nextPart;
    mLoopLen = len;
}

// 801D5EE0
void SoundHaniwa::readLoop() {
    if (mHandles[0].IsAttachedSound()) {
        s16 kind;
        s16 bars[4];
        s16 mode;
        nw4r::snd::SeqSoundHandle seq(&mHandles[0]);
        seq.ReadVariable(0, &kind);
        seq.ReadVariable(1, &bars[0]);
        seq.ReadVariable(2, &bars[1]);
        seq.ReadVariable(3, &bars[2]);
        seq.ReadVariable(4, &bars[3]);
        seq.ReadVariable(5, &mode);
        mLoopKind = kind;
        mLoopMode = mode;
        if (bars[0] >= 0) {
            mBarMask |= bars[3];
            mBarMask |= bars[2] << 4;
            mBarMask |= bars[1] << 8;
            mBarMask |= bars[0] << 12;
        }
    }
}

// 801D6020
void SoundHaniwa::updateTempo() {
    SoundSystem *system = SoundSystem::getInstance();
    f32 ratio = system->mConductor.mTempo / 120.0f;
    u32 id = SoundSystem::getInstance()->getMusicId();
    if (id != SEQ_NULL) {
        if (id == BGM_MU65_NAPOLITAN) {
            ratio *= 0.5f;
        }
        nw4r::snd::SeqSoundHandle seq(&mHandles[0]);
        seq.SetTempoRatio(ratio);
    }
}

// 801D6148
f32 SoundHaniwa::calcProgress() {
    mConductor->calcBeat();
    s16 beat = getGlobalBeat();
    f32 progress;
    s16 startBar;
    if (mLoopKind == 1 || mLoopKind == 3) {
        startBar = mStartBar;
        int bars = (s16)getGlobalBar() - startBar;
        if (bars < 0) {
            bars += 16;
        }
        progress = bars / 16.0f;
        progress += beat / 16.0f / SoundSystem::getInstance()->mConductor.mBeatTicks;
        if (progress >= 1.0f) {
            progress = 0.0f;
        }
        if (mLoopMode == 2) {
            progress *= 2.0f;
            if (progress > 1.0f) {
                progress -= 1.0f;
            }
        }
    } else {
        s16 nextPart;
        {
            nw4r::snd::SeqSoundHandle seq(&mHandles[0]);
            seq.ReadVariable(7, &startBar);
        }
        {
            nw4r::snd::SeqSoundHandle seq(&mHandles[0]);
            seq.ReadVariable(12, &nextPart);
        }
        int bars = (s16)getGlobalBar() - startBar;
        if (bars < 0) {
            bars += 16;
        }
        int len;
        switch (mLoopMode) {
        case 1:
        case 2:
        case 4:
            len = mLoopMode;
            break;
        case 99:
            len = mLoopLen;
            break;
        }
        f32 beatTicks = SoundSystem::getInstance()->mConductor.mBeatTicks;
        if (!mLoopDone && bars >= len) {
            mLoopDone = true;
        }
        if (mLoopDone && mPart == 0) {
            mLoopDone = false;
        }
        if (mLoopDone) {
            progress = 0.0f;
        } else {
            progress = (bars + beat / beatTicks) / len;
            if (progress >= 1.0f) {
                progress = 0.0f;
            }
        }
    }
    return progress;
}

// 801D646C
s16 SoundHaniwa::getGlobalBar() {
    s16 value = 0;
    nw4r::snd::SeqSoundHandle::ReadGlobalVariable(2, &value);
    return value;
}

// 801D64A0
s16 SoundHaniwa::getGlobalBeat() {
    s16 value = 0;
    nw4r::snd::SeqSoundHandle::ReadGlobalVariable(1, &value);
    return value;
}
