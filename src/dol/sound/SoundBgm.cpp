// The town tune player (SoundMelody, SoundMelodyPart) and the instruments that play it (SoundObjInst).
// .text 801D6618..801D8618, .rodata 8047EBE8..8047ED18, .data 80501070..805010F0, .bss 80625D88..80625DA0,
// .sdata 8074C6A0..8074C6A8, .sbss 8074EC28..8074EC30, .sdata2 80751BF0..80751C40.
// See include/game/snd/snd_fader.hpp and snd_obj.hpp.
#include <game/snd/snd_manager.hpp>
#include <game/snd/snd_system.hpp>
#include <game/snd/snd_obj.hpp>
#include <game/snd/snd_id.hpp>
#include <game/game/d_date.hpp>
#include <revolution/mtx.h>

// The steps that sound in a melody of type 3 (SoundMelodyPart::getNote).
static const u8 sNoteSteps[13] = {0, 2, 3, 4, 5, 6, 7, 9, 11, 15, 19, 25, 33};

static const u8 sDefaultNotes[16] = {5, 7, 10, 12, 11, 9, 7, 6, 5, 15, 13, 15, 10, 14, 14, 15};

// The island's tunes, by month.
static const u8 sMonthNotes[12][16] = {
    {10, 15, 13, 15, 3, 5, 4, 2, 6, 7, 11, 12, 10, 14, 15, 15}, // 1
    {8, 14, 9, 14, 7, 6, 8, 14, 9, 7, 6, 7, 5, 14, 15, 15}, // 2
    {5, 7, 11, 10, 12, 15, 7, 15, 6, 14, 5, 13, 7, 14, 15, 15}, // 3
    {3, 7, 12, 15, 11, 9, 7, 15, 6, 7, 13, 9, 10, 14, 15, 15}, // 4
    {4, 14, 5, 7, 15, 2, 4, 7, 10, 9, 10, 11, 14, 14, 15, 15}, // 5
    {11, 10, 15, 10, 8, 15, 13, 12, 11, 9, 7, 5, 3, 14, 14, 15}, // 6
    {9, 7, 4, 11, 9, 7, 11, 14, 9, 15, 13, 15, 7, 14, 15, 15}, // 7
    {9, 10, 9, 7, 5, 14, 3, 5, 6, 15, 6, 7, 5, 14, 15, 15}, // 8
    {5, 7, 10, 15, 4, 4, 7, 15, 6, 13, 11, 10, 14, 15, 15, 15}, // 9
    {7, 14, 3, 14, 10, 9, 8, 9, 7, 15, 13, 5, 7, 14, 15, 15}, // 10
    {3, 15, 10, 15, 12, 7, 3, 15, 10, 15, 13, 7, 3, 14, 15, 15}, // 11
    {11, 14, 10, 9, 8, 9, 10, 13, 7, 15, 9, 15, 11, 14, 15, 15}, // 12
};

// The instruments' notes (SoundObjInst::setup).
static const u32 sInstSounds[20] = {
    SE_MELODY_INST_A_GTR_AS1,
    SE_MELODY_INST_A_GTR_AS2,
    SE_MELODY_INST_BIWA,
    SE_MELODY_INST_CELLO,
    SE_MELODY_INST_CONGA,
    SE_MELODY_INST_CNT_BASS,
    SE_MELODY_INST_E_GTR_ES1,
    SE_MELODY_INST_E_GTR_ES2,
    SE_MELODY_INST_G_PIANO,
    SE_MELODY_INST_HARP,
    SE_MELODY_INST_JAMBE,
    SE_MELODY_INST_TIMPANI,
    SE_MELODY_INST_UKULELE,
    SE_MELODY_INST_VIBRAPHONE,
    SE_MELODY_INST_W_PIANO,
    SE_MELODY_INST_WADAIKO,
    SE_MELODY_INST_VIOLA,
    SE_MELODY_INST_DRUMS,
    SE_MELODY_INST_G_PIANO,
    SE_MELODY_INST_G_PIANO,
};

const u8 *SoundObjInst::sNotes;
int SoundObjInst::sPlayingNum;

// 801D6618
void SoundMelodyPart::init(SoundMelody *owner, f32 volume, int filterType) {
    if (mOwner == NULL) {
        reset();
        mOwner = owner;
    }
    mVolume = volume;
    mFilter = 0.0f;
    mFilterType = filterType;
}

// 801D6690
void SoundMelodyPart::start(u32 soundId, const u8 *notes) {
    reset();
    if (mHandle.IsAttachedSound()) {
        mHandle.Stop(0);
    }
    mNotes = notes;
    mPlaying = true;
    mSoundId = soundId;
    SoundManager::getInstance()->startSound(&mHandle, mSoundId);
}

// 801D6834
void SoundMelodyPart::playJingle(u32 idx) {
    reset();
    if (idx <= 13) {
        if (mHandle.IsAttachedSound()) {
            mHandle.Stop(0);
        }
        SoundManager::getInstance()->startSound(&mHandle, SE_MELODY_EDIT_SINGLE_G3 + idx);
    }
}

// 801D69D0
void SoundMelodyPart::stop() {
    mHandle.Stop(0);
    reset();
}

// 801D6A18
void SoundMelodyPart::reset() {
    mPlaying = false;
    mNote = -1;
    mTick = 0;
    mHold = -1;
    mType = -1;
    mEndTicks = -1;
    mStepTicks = -1;
    mSwing2 = -1;
    mSwing = -1;
}

// 801D6A48
void SoundMelodyPart::readParams() {
    s16 step;
    s16 end;
    s16 type;
    s16 hold;
    s16 swing;
    s16 swing2;
    step = end = type = hold = swing = swing2 = -1;
    nw4r::snd::SeqSoundHandle seq(&mHandle);
    seq.ReadVariable(0, &step);
    seq.ReadVariable(1, &end);
    seq.ReadVariable(2, &type);
    seq.ReadVariable(3, &hold);
    seq.ReadVariable(5, &swing);
    seq.ReadVariable(6, &swing2);
    int value = swing;
    if (value > 2) {
        value = 2;
    } else if (value < 0) {
        value = 0;
    }
    swing = (s8)value;
    value = swing2;
    if (value > 2) {
        value = 2;
    } else if (value < 0) {
        value = 0;
    }
    swing2 = (s8)value;
    mStepTicks = swing + step * 3;
    mEndTicks = swing + end * 3;
    mType = type;
    mHold = hold;
    mSwing = swing;
    mSwing2 = swing2;
}

// 801D6BCC
u8 SoundMelodyPart::getNote(u8 idx) {
    u8 note;
    switch (mType) {
    case 0:
        note = mNotes[idx];
        break;
    case 1: {
        u8 step = idx % 3;
        if (step == 1) {
            note = 14;
        } else {
            u8 group = idx / 3;
            if (step == 0) {
                note = mNotes[group * 2];
            } else {
                note = mNotes[group * 2 + 1];
            }
        }
        break;
    }
    case 2: {
        u8 step = idx % 3;
        if (step == 2) {
            note = 14;
        } else {
            u8 group = idx / 3;
            if (step == 0) {
                note = mNotes[group * 2];
            } else {
                note = mNotes[group * 2 + 1];
            }
        }
        break;
    }
    case 3: {
        BOOL found = FALSE;
        for (int i = 0; i < 13; i++) {
            if (idx == sNoteSteps[i]) {
                found = TRUE;
                note = mNotes[i];
            }
        }
        if (!found) {
            note = 14;
        }
        break;
    }
    }
    return note;
}

// 801D6D18
void SoundMelodyPart::playNote() {
    u8 note = getNote(mNote);
    switch (note) {
    case 13: {
        int randomNote = note + (u16)SoundSystem::getInstance()->getRandom(6);
        u32 id = randomNote + mSoundId + 1;
        SoundManager::getInstance()->startSound(&mHandle, id);
        mHandle.SetVolume(mVolume, 0);
        mHandle.SetBiquadFilter(mFilterType, mFilter);
        break;
    }
    case 14:
    case 15:
        break;
    default: {
        u32 id = note + mSoundId + 1;
        SoundManager::getInstance()->startSound(&mHandle, id);
        mHandle.SetVolume(mVolume, 0);
        mHandle.SetBiquadFilter(mFilterType, mFilter);
        break;
    }
    }

    if (mType == 3) {
        f32 volume;
        if (mNote == 0) {
            volume = 64.0f / 127.0f;
        } else if (mNote == 2) {
            volume = 80.0f / 127.0f;
        } else if (mNote == 3) {
            volume = 96.0f / 127.0f;
        } else if (mNote == 4) {
            volume = 112.0f / 127.0f;
        } else if (mNote == 5) {
            volume = 1.0f;
        } else if (mNote == 6) {
            volume = 1.0f;
        } else if (mNote == 7) {
            volume = 112.0f / 127.0f;
        } else if (mNote == 9) {
            volume = 96.0f / 127.0f;
        } else if (mNote == 11) {
            volume = 80.0f / 127.0f;
        } else if (mNote == 15) {
            volume = 64.0f / 127.0f;
        } else if (mNote == 19) {
            volume = 48.0f / 127.0f;
        } else if (mNote == 25) {
            volume = 32.0f / 127.0f;
        } else if (mNote == 33) {
            volume = 16.0f / 127.0f;
        } else {
            volume = -1.0f;
        }
        if (volume >= 0.0f) {
            mHandle.SetVolume(0.5f * (mVolume * volume) * mOwner->getVolume(), 0);
            mHandle.SetPan(mOwner->mPan);
        }
    }
}

// 801D71F8
BOOL SoundMelodyPart::checkEnd() {
    if (mType == 0) {
        if (mNote == 15) {
            mHandle.Stop(0);
            mOwner->mState = SoundMelody::STATE_IDLE;
            return TRUE;
        }
    } else if (mType == 1) {
        if (mNote == 23) {
            mHandle.Stop(0);
            mOwner->mState = SoundMelody::STATE_IDLE;
            return TRUE;
        }
    } else if (mType == 2) {
        if (mNote == 23) {
            mHandle.Stop(0);
            mOwner->mState = SoundMelody::STATE_IDLE;
            return TRUE;
        }
    } else if (mType == 3) {
        if (mNote == 39) {
            mHandle.Stop(0);
            mOwner->mState = SoundMelody::STATE_IDLE;
            return TRUE;
        }
    }

    u8 next = getNote(mNote + 1);
    if (next != 14 && !(next == 15 && mHold == 1)) {
        mHandle.Stop(0);
    }
    return FALSE;
}

// 801D7360
int SoundMelodyPart::calc() {
    int note = -1;
    if (mPlaying) {
        if (mStepTicks < 0) {
            readParams();
        } else {
            BOOL end = FALSE;
            if (mTick == mEndTicks) {
                end = checkEnd();
            }
            if (end) {
                reset();
            } else {
                if (mTick == 0 || mTick == mStepTicks) {
                    mTick = 0;
                    mNote++;
                    playNote();
                }
                mTick++;
            }
            note = mNote;
        }
    }
    return note;
}

// 801D7438
const u8 *SoundMelody::getMonthNotes(int month) {
    const u8 *notes = sMonthNotes[0];
    if (month >= 0 && month <= 11) {
        notes = sMonthNotes[month];
    }
    return notes;
}

// 801D7460
void SoundMelody::init() {
    mPart1.init(this, 0.8f, 0);
    mPart0.init(this, 1.0f, 4);
    mChimeBusy = false;
    mDist = 0.0f;
    mVolume = -1.0f;
    mPan = 0.0f;
    _70 = 0;
    setNotes(sDefaultNotes);
}

// 801D74E0
const u8 *SoundMelody::getDefaultNotes() {
    return sDefaultNotes;
}

// 801D74EC
void SoundMelody::setNotes(const u8 *notes) {
    for (int i = 0; i < 16; i++) {
        mNotes[i] = notes[i];
    }
}

// 801D7570
u8 *SoundMelody::getNotes() {
    return mNotes;
}

// 801D7578
void SoundMelody::play(u32 soundId, const u8 *notes) {
    mState = STATE_MELODY;
    mVolume = -1.0f;
    if (notes == NULL) {
        notes = mNotes;
        int stage = SoundManager::getInstance()->_778;
        if (stage >= 0x2D && stage <= 0x37) {
            notes = sMonthNotes[dTime_c::getCurrent()->mon];
        }
    }
    if (!((soundId >= 1 && soundId <= SE_MELODY_SP_SKK_rnd5) || soundId == SE_MELODY_CHIME_TOWN)) {
        soundId = 1;
    }
    mPart1.start(soundId, notes);
}

// 801D7748
void SoundMelody::stop() {
    mState = STATE_IDLE;
    mPart1.stop();
    mPart1.mPlaying = false;
}

// 801D778C
void SoundMelodyPart::end() {
    stop();
    mPlaying = false;
}

// 801D77C0
void SoundMelody::playChime() {
    u32 soundId = SE_MELODY_CHIME;
    const u8 *notes = mNotes;
    int stage = SoundManager::getInstance()->_778;
    if (stage >= 0x2D && stage <= 0x37) {
        notes = sMonthNotes[dTime_c::getCurrent()->mon];
        soundId = SE_MELODY_CHIME_TOWN;
    }
    mPart0.start(soundId, notes);
}

// 801D7960
void SoundMelody::playAt(const nw4r::math::VEC3 *pos, u32 kind) {
    if (mState == STATE_MELODY || mState == STATE_INST) {
        return;
    }
    if (mState == STATE_POSITION) {
        f32 dist = nw4r::math::VEC3Len(pos);
        if (dist > mDist && mDist > 0.0f) {
            return;
        }
        mPart1.stop();
    }
    mDist = nw4r::math::VEC3Len(pos);
    mVolume = calcVolume(pos);
    mPan = calcPan(pos);

    u32 soundId = SE_MELODY_DOOR;
    mState = STATE_POSITION;
    switch (kind) {
    case 1:
        soundId = SE_MELODY_DOOR_TANUKICHI;
        break;
    case 2:
        soundId = SE_MELODY_DOOR_PUB;
        break;
    case 3:
        soundId = SE_MELODY_DOOR_ABLE;
        break;
    case 4:
        soundId = SE_MELODY_DOOR_TSUNEKICHI;
        break;
    case 5:
        soundId = SE_MELODY_DOOR_SPEEDY;
        break;
    case 6:
        soundId = SE_MELODY_DOOR_FORTUNE;
        break;
    case 0:
        break;
    default:
        return;
    }

    const u8 *notes = mNotes;
    int stage = SoundManager::getInstance()->_778;
    if (stage >= 0x2D && stage <= 0x37) {
        notes = sMonthNotes[dTime_c::getCurrent()->mon];
    }
    mPart1.start(soundId, notes);
}

// 801D7BD0
void SoundMelody::playJingle(u32 idx) {
    if (mState == STATE_IDLE) {
        mPart1.playJingle(idx);
    }
}

// 801D7BE8
f32 SoundMelody::calcVolume(const nw4r::math::VEC3 *pos) {
    f32 volume = (1200.0f - nw4r::math::VEC3Len(pos)) / 600.0f;
    if (volume > 1.0f) {
        volume = 1.0f;
    } else if (volume < 0.1f) {
        volume = 0.1f;
    }
    return volume;
}

// 801D7C40
f32 SoundMelody::calcPan(const nw4r::math::VEC3 *pos) {
    f32 pan = pos->x / 300.0f;
    if (pan > 1.0f) {
        pan = 1.0f;
    } else if (pan < -1.0f) {
        pan = -1.0f;
    }
    return pan;
}

// 801D7C74
void SoundMelody::calc() {
    mChimeBusy = mPart0.calc() >= 0;
    mPart1.calc();
}

// 801D7CB4
void SoundObjInst::incPlaying() {
    sPlayingNum++;
}

// 801D7CC4
void SoundObjInst::decPlaying() {
    if (--sPlayingNum == 0) {
        SoundSystem::getInstance()->mMelody.mState = SoundMelody::STATE_IDLE;
    }
}

// 801D7D40
SoundObjInst::SoundObjInst() {}

// 801D7DA8
SoundObjInst::~SoundObjInst() {}

// 801D7E38
void SoundObjInst::setup(int inst) {
    sNotes = SoundSystem::getInstance()->mMelody.getNotes();
    int stage = SoundManager::getInstance()->_778;
    if (stage >= 0x2D && stage <= 0x37) {
        sNotes = SoundMelody::getMonthNotes(dTime_c::getCurrent()->mon);
    }
    mSoundId = sInstSounds[inst];
    mPlaying = false;
    mNote = -1;
    mTick = 0;
    mHold = -1;
    mTempo = -1;
}

// 801D8044
void SoundObjInst::stop() {
    if (mPlaying) {
        decPlaying();
    }
    mPlaying = false;
    if (mHandles[0].IsAttachedSound()) {
        mHandles[0].Stop(0);
    }
    mHandles[0].DetachSound();
}

// 801D80A4
void SoundObjInst::play() {
    if (SoundSystem::getInstance()->mMelody.mState == SoundMelody::STATE_MELODY) {
        return;
    }
    mHandles[0].Stop(0);
    if (mPlaying) {
        decPlaying();
    }
    SoundSystem::getInstance()->mMelody.mState = SoundMelody::STATE_INST;
    mPlaying = true;
    mNote = -1;
    incPlaying();
}

// 801D81A0
void SoundObjInst::calc(const nw4r::math::VEC3 *pos) {
    update(pos);
    if (!mPlaying) {
        return;
    }
    if (mTempo == -1) {
        if (!mHandles[0].IsAttachedSound()) {
            startSound(mSoundId, NULL);
        }
        readParams();
        return;
    }

    mTick++;
    bool barChanged = SoundSystem::getInstance()->mConductor.mBarChanged;
    if (mTick >= mNoteTicks) {
        BOOL end = checkEnd();
        mTick = 0;
        if (end) {
            return;
        }
    }
    if (barChanged) {
        mTick = 0;
        mNote++;
        if (mNote < 16) {
            playNote();
        } else if (mNote >= 16) {
            checkEnd();
            return;
        }
        f32 beatTime = SoundSystem::getInstance()->mConductor.mBeatTime;
        if (mTempo == 100) {
            mNoteTicks = beatTime;
        } else {
            mNoteTicks = beatTime * mTempo / 100.0f;
        }
    }
}

// 801D8390
void SoundObjInst::readParams() {
    s16 tempo;
    s16 hold;
    tempo = hold = -1;
    nw4r::snd::SeqSoundHandle seq(&mHandles[0]);
    if (seq.IsAttachedSound()) {
        seq.ReadVariable(4, &tempo);
        seq.ReadVariable(3, &hold);
    }
    mTempo = tempo;
    mHold = hold;
}

// 801D841C
void SoundObjInst::playNote() {
    u8 note = sNotes[mNote];
    switch (note) {
    case 13: {
        if (mHandles[0].IsAttachedSound()) {
            mHandles[0].Stop(0);
        }
        int rnd = SoundSystem::getInstance()->getRandom(6);
        startSound(note + rnd + mSoundId + 1, NULL);
        break;
    }
    case 14:
    case 15:
        break;
    default:
        if (mHandles[0].IsAttachedSound()) {
            mHandles[0].Stop(0);
        }
        startSound(mSoundId + (u16)(note + 1), NULL);
        break;
    }
}

// 801D854C
BOOL SoundObjInst::checkEnd() {
    if (mNote >= 15) {
        mHandles[0].Stop(0);
        mPlaying = false;
        mNote = -1;
        mTick = 0;
        decPlaying();
        return TRUE;
    }
    switch (sNotes[mNote + 1]) {
    case 15:
        if (mHold != 1) {
            mHandles[0].Stop(0);
        }
        break;
    case 14:
        break;
    default:
        mHandles[0].Stop(0);
        break;
    }
    return FALSE;
}
