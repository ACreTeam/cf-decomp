// Follows the beat of the music player's song for the gyroids and K.K. Slider. .text 801D42AC..801D5390,
// .data 80500D88..80501018, .bss 80625D50..80625D60, .sdata 8074C680..8074C698, .sbss 8074EC20..8074EC28,
// .sdata2 80751B88..80751BB8. See include/game/snd/snd_conductor.hpp.
#include <game/snd/snd_conductor.hpp>
#include <game/snd/snd_system.hpp>
#include <game/snd/snd_obj.hpp>
#include <game/snd/snd_id.hpp>

// 801D42AC
SoundConductor::SoundConductor() {
    mHaniwaList = NULL;
    mBeat = 0;
    mBarChanged = false;
    mTempo = 120;
    mBeatTime = 1800.0f / mTempo;
    mBar = -1;
    nw4r::snd::SeqSoundHandle::WriteGlobalVariable(2, -1);
    nw4r::snd::SeqSoundHandle::WriteGlobalVariable(1, -1);
}

// 801D4344
SoundConductor::~SoundConductor() {}

// 801D4384
u32 SoundConductor::calcBeat() {
    s16 bar;
    s16 prevBeat;
    nw4r::snd::SoundHandle *music = &SoundSystem::getInstance()->mMusicHandle;
    if (!music->IsAttachedSound()) {
        bar = -1;
        prevBeat = -1;
    } else {
        nw4r::snd::SeqSoundHandle::ReadGlobalVariable(1, &prevBeat);
        nw4r::snd::SeqSoundHandle::ReadGlobalVariable(2, &bar);
        nw4r::snd::SeqSoundHandle seq(music);
        mBeat = seq.GetTick() % mBeatTicks;
        if (mBeat < prevBeat) {
            bar++;
            if (bar > 15) {
                bar = 0;
            }
        }
        nw4r::snd::SeqSoundHandle::WriteGlobalVariable(1, mBeat);
        nw4r::snd::SeqSoundHandle::WriteGlobalVariable(2, bar);
        mBar = bar;
    }
    return (bar << 16) | prevBeat;
}

// 801D44D0
void SoundConductor::start(u32 musicId) {
    mMusicId = musicId;
    mBar = 0;
    mBeat = 0;
    mBeatTicks = 24;
    nw4r::snd::SeqSoundHandle::WriteGlobalVariable(2, 0);
    nw4r::snd::SeqSoundHandle::WriteGlobalVariable(1, mBeat);
    for (SoundHaniwa *haniwa = mHaniwaList; haniwa != NULL; haniwa = haniwa->mNext) {
        if (haniwa->mStartBar >= 0) {
            haniwa->restart();
        }
    }

    switch (musicId) {
    case BGM_MU02_MARCH:
    case BGM_MU03_WALTZ:
    case BGM_MU04_SWING:
    case BGM_MU18_HAISAI:
    case BGM_MU20_IRISH:
    case BGM_MU25_AFRO:
    case BGM_MU26_MINYO:
    case BGM_MU33_GOSPEL:
    case BGM_MU40_ONLYME:
    case BGM_MU45_ENKA:
    case BGM_MU50_PARIS:
    case BGM_MU53_KEKESONG:
    case BGM_MU58_NAMINAMI:
    case BGM_MU60_DIXIE:
    case BGM_MU69_ONDO:
        mStyle = 1;
        break;
    case BGM_MU17_TANGO:
    case BGM_MU19_ALOHA:
    case BGM_MU28_ROCKNROLL:
    case BGM_MU31_FUNK:
    case BGM_MU41_COUNTRY:
    case BGM_MU44_SHOWAKAYO:
    case BGM_MU52_SENOR:
        mStyle = 2;
        break;
    case BGM_MU15_REGGAE:
    case BGM_MU32_BLUES:
        mStyle = 3;
        break;
    case BGM_MU65_NAPOLITAN:
        mStyle = 4;
        mBeatTicks = 48;
        break;
    default:
        mStyle = 0;
        break;
    }

    if (mMusicId == BGM_MU03_WALTZ || mMusicId == BGM_MU33_GOSPEL || mMusicId == BGM_MU50_PARIS ||
        mMusicId == BGM_MU52_SENOR || mMusicId == BGM_MU63_ALPINE || mMusicId == BGM_CAFE) {
        mBeatTicks = 16;
    }
}

// 801D4608
void SoundConductor::resetHaniwa() {
    for (SoundHaniwa *haniwa = mHaniwaList; haniwa != NULL; haniwa = haniwa->mNext) {
        haniwa->reset();
    }
}

// 801D4648
void SoundConductor::calc() {
    static s16 sPrevBar = mBar;
    calcBeat();
    if (SoundSystem::getInstance()->isMusicPlaying()) {
        mBarChanged = sPrevBar != mBar;
        updateTempo();
        sPrevBar = mBar;
    }
}

// 801D4720
void SoundConductor::updateTempo() {
    if (SoundSystem::getInstance()->isMusicPlaying()) {
        nw4r::snd::SeqSoundHandle seq(&SoundSystem::getInstance()->mMusicHandle);
        if (!seq.IsAttachedSound()) {
            return;
        }
        const nw4r::snd::detail::SeqSound *sound = seq.detail_GetAttachedSound();
        s16 tempo = sound->GetSeqPlayer().GetParserPlayerParam().tempo;
        mTempo = tempo;
        mBeatTime = 1800.0f / tempo;
    }
}

// 801D4838
void SoundConductor::appendHaniwa(SoundHaniwa *haniwa) {
    SoundHaniwa *it = mHaniwaList;
    SoundHaniwa *last = NULL;
    while (it != NULL) {
        last = it;
        it = it->mNext;
    }
    if (last != NULL) {
        last->mNext = haniwa;
    } else {
        mHaniwaList = haniwa;
    }
    haniwa->mPrev = last;
}

// 801D4870
void SoundConductor::removeHaniwa(SoundHaniwa *haniwa) {
    SoundHaniwa *prev = haniwa->mPrev;
    SoundHaniwa *next = haniwa->mNext;
    if (prev != NULL && next != NULL) {
        prev->mNext = next;
        next->mPrev = prev;
        return;
    }
    if (prev != NULL && next == NULL) {
        prev->mNext = NULL;
        return;
    }
    if (prev == NULL && next != NULL) {
        next->mPrev = NULL;
        mHaniwaList = next;
        return;
    }
    mHaniwaList = NULL;
}

// 801D48DC
SoundConductorKK::Anim::Anim() {
    reset();
}

// 801D490C
void SoundConductorKK::Anim::reset() {
    mPhase = -1;
    mPart = -1;
    mVar0 = -1;
    mVar2 = -1;
    mVar3 = -1;
    mFrame96 = 0.0f;
    mFrame60 = 0.0f;
    mBeatPos = 0.0f;
}

// 801D4938
SoundConductorKK::SoundConductorKK() {
    mAnim.mPhase = -1;
    mTempoType = 0;
    mSongType = 1;
    mBeatPos = -1;
    mBeatFlip = true;
}

// 801D499C
SoundConductorKK::~SoundConductorKK() {
    SoundConductor::~SoundConductor();
}

// 801D4A0C
void SoundConductorKK::reset() {
    mAnim.reset();
    mSongType = 0;
    mTempoType = 0;
}

// 801D4A48
void SoundConductorKK::calc() {
    if (!SoundSystem::getInstance()->isKKMusic()) {
        mAnim.mPhase = -1;
        return;
    }
    calcPart();
    calcBeatPos();
    if (mAnim.mPhase == -1) {
        nw4r::snd::SoundHandle *music = &SoundSystem::getInstance()->mMusicHandle;
        nw4r::snd::SeqSoundHandle seq(music);
        if (!seq.IsAttachedSound()) {
            return;
        }
        if (!music->IsAttachedSound()) {
            return;
        }

        s16 phase;
        seq.ReadVariable(4, &phase);
        mAnim.mPhase = phase;
        if (SoundSystem::getInstance()->getMusicId() == BGM_KK76_UTATANENOYUME) {
            mAnim.mPhase = 4;
        }
        mAnim.mBeatPos = 0.0f;
        mAnim.mFrame60 = 0.0f;
        mAnim.mFrame96 = 0.0f;

        u32 id = SoundSystem::getInstance()->getMusicId();
        switch (id) {
        case BGM_KK11_BOSSA:
        case BGM_KK65_NAPOLITAN:
        case BGM_KK76_UTATANENOYUME:
            mSongType = 1;
            break;
        case BGM_KK12_CARIPSO:
            mSongType = 2;
            break;
        }

        switch (id) {
        case BGM_KK11_BOSSA:
        case BGM_KK61_GAMELAN:
        case BGM_KK64_HUNAUTA2001:
        case BGM_KK66_NIDANZAKA:
            mTempoType = 1;
            break;
        case BGM_KK05_JAZZ:
        case BGM_KK06_FUSION:
        case BGM_KK10_SAMBA:
        case BGM_KK12_CARIPSO:
        case BGM_KK13_SALSA:
        case BGM_KK21_PERU:
        case BGM_KK35_URBAN:
        case BGM_KK60_DIXIE:
        case BGM_KK68_KEIJI:
            mTempoType = 2;
            break;
        case BGM_KK50_PARIS:
            mTempoType = 3;
            break;
        default:
            mTempoType = 0;
            break;
        }
    } else {
        calcAnim();
    }
}

// 801D4CD4
void SoundConductorKK::calcPart() {
    static s8 sPrevPart = -1;

    nw4r::snd::SeqSoundHandle seq(&SoundSystem::getInstance()->mMusicHandle);
    s8 part = -1;
    if (!seq.IsAttachedSound()) {
        return;
    }

    for (int i = 2; i <= 13; i++) {
        nw4r::snd::detail::SeqTrack *track = seq.detail_GetAttachedSound()->GetSeqPlayer().GetPlayerTrack(i);
        if (track != NULL) {
            int channels = track->GetChannelCount();
            if (i != 9 && channels > 0) {
                switch (i) {
                case 11:
                    part = 0;
                    break;
                case 6:
                    part = 1;
                    break;
                case 3:
                    part = 2;
                    break;
                case 5:
                case 8:
                case 10:
                    part = 3;
                    break;
                case 2:
                case 7:
                case 12:
                    part = 4;
                    break;
                case 4:
                case 13:
                    part = 5;
                    break;
                default:
                    part = 0;
                    break;
                }
                break;
            }
        }
    }

    if (part == -1) {
        mAnim.mPart = sPrevPart;
    } else {
        mAnim.mPart = part;
    }
    sPrevPart = part;
}

// 801D4E2C
void SoundConductorKK::calcBeatPos() {
    nw4r::snd::SoundHandle *music = &SoundSystem::getInstance()->mMusicHandle;
    if (!music->IsAttachedSound()) {
        return;
    }
    nw4r::snd::SeqSoundHandle seq(music);
    if (!seq.IsAttachedSound()) {
        return;
    }

    s16 var0;
    s16 var1;
    s16 var2;
    s16 var3;
    seq.ReadVariable(0, &var0);
    seq.ReadVariable(1, &var1);
    seq.ReadVariable(2, &var2);
    seq.ReadVariable(3, &var3);

    u32 len = mAnim.mPhase * 48;
    u32 pos = seq.GetTick() % len;
    if (SoundSystem::getInstance()->getMusicId() == BGM_KK76_UTATANENOYUME ||
        SoundSystem::getInstance()->getMusicId() == BGM_KK65_NAPOLITAN) {
        pos = (seq.GetTick() * 2) % len;
    }

    mAnim.mVar0 = var0;
    mAnim.mVar2 = var2;
    mAnim.mVar3 = var3;
    mAnim.mVar1 = var1;
    if (mBeatPos > pos) {
        mBeatFlip = !mBeatFlip;
    }
    mBeatPos = pos;
}

// 801D5068
void SoundConductorKK::calcAnim() {
    int pos = mBeatPos;
    mAnim.mBeatPos = pos;

    f32 frame;
    switch (mAnim.mPhase) {
    case 3:
        switch (mTempoType) {
        case 3:
            if (!mBeatFlip) {
                pos += 144;
            }
            frame = pos / 144.0f;
            break;
        default:
            if (!mBeatFlip) {
                pos += 48;
            }
            pos %= 96;
            frame = pos / 96.0f;
            break;
        }
        break;
    case 4:
    default:
        switch (mTempoType) {
        case 1:
            pos %= 48;
            frame = pos / 48.0f;
            break;
        case 2:
            pos %= 192;
            frame = pos / 192.0f;
            break;
        case 0:
        default:
            pos %= 96;
            frame = pos / 96.0f;
            break;
        }
        break;
    }
    frame = 60.0f * frame;
    frame += 12.0f;
    if (frame >= 60.0f) {
        frame -= 60.0f;
    }
    mAnim.mFrame60 = frame;

    pos = mBeatPos;
    switch (mAnim.mPhase) {
    case 3:
        frame = pos / 144.0f;
        break;
    case 4:
    default:
        switch (mTempoType) {
        case 0:
        default:
            pos %= 96;
            frame = pos / 96.0f;
            break;
        case 1:
            pos %= 48;
            frame = pos / 48.0f;
            break;
        case 2:
            pos %= 192;
            frame = pos / 192.0f;
            break;
        }
        break;
    }
    frame = 96.0f * frame;
    frame += 36.0f;
    if (frame >= 96.0f) {
        frame -= 96.0f;
    }
    mAnim.mFrame96 = frame;
}
