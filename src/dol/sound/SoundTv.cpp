// The TV programme sounds (SoundTv_base and the programmes). .text 801D0550..801D42AC, .data
// 80500700..80500D88, .bss 80625D38..80625D50, .sdata 8074C618..8074C680, .sdata2 80751B58..80751B88.
// See include/game/snd/snd_tv.hpp.
#include <game/snd/snd_tv.hpp>
#include <game/snd/snd_obj_inline.hpp>
#include <game/snd/snd_id.hpp>
#include <game/snd/snd_manager.hpp>
#include <game/snd/snd_system.hpp>

// 801D0550
void SoundTv_base::reset() {
    mOn = 0;
    mLoading = 0;
    startProgram();
    setVolume(0.0f);
}

// 801D05AC
void SoundTv_base::stop() {
    stopAll(0);
}

// 801D05C0
void SoundTv_base::execute(int frame, const nw4r::math::VEC3 *pos) {
    if (pos != NULL) {
        update(pos);
    } else {
        mHandles[0].SetVolume(0.0f, 0);
        mHandles[1].SetVolume(0.0f, 0);
    }
}

// 801D0638
void SoundTv_base::setOn(u8 on) {
    mOn = on;
    setVolume(0.45f);
}

// 801D0650
void SoundTv_base::mute() {
    setVolume(0.0f);
}

// 801D0664
bool SoundTv_base::isReady() {
    bool ready = false;
    if (SoundManager::getInstance()->isGroupReady(mGroupId)) {
        ready = true;
        mLoading = 0;
    } else if (!mLoading) {
        stopAll(0);
        if (SoundManager::getInstance()->requestGroup(mGroupId)) {
            mLoading = 1;
        }
    }
    return ready;
}

// 801D0954
void SoundTv_base::startTvSound(nw4r::snd::SoundHandle *handle, u32 id) {
    SoundManager *mgr = SoundManager::getInstance();
    if (SoundSystem::getInstance()->mState != SoundSystem::STATE_READY) {
        return;
    }
    if (!sIsPlayable && !((mgr->mArchive.GetSoundUserParam(id) >> 1) & 1)) {
        return;
    }

    calcParam(&mPos, PARAM_MODE_NORMAL);
    mgr->startSound(handle, id);
    if (mgr->isFixedVolume(id)) {
        handle->SetVolume(mPosVolume * mVolume, 0);
    } else {
        handle->SetVolume(sMasterVolume * (mPosVolume * mVolume), 0);
    }
    handle->SetPan(mPan);
}

// 801D0BC0
void SoundTv_00_storm::startProgram() {
    startTvSound(&mHandles[0], SE_TV_00);
}

// 801D0BD8
void SoundTv_01_color::startProgram() {
    startTvSound(&mHandles[0], SE_TV_01);
}

// 801D0BF0
void SoundTv_02_news::reset() {
    SoundTv_base::reset();
    mTopic = -1;
}

// 801D0C24
void SoundTv_02_news::startProgram() {
    startTvSound(&mHandles[1], SE_TV_02_v00);
}

// 801D0C3C
void SoundTv_02_news::execute(int frame, const nw4r::math::VEC3 *pos) {
    SoundTv_base::execute(frame, pos);
    switch (frame) {
    case 1:
        if (!mHandles[1].IsAttachedSound()) {
            startTvSound(&mHandles[1], SE_TV_02_v00);
        }
        mTopic++;
        if (mTopic == 3) {
            mTopic = 0;
        }
        switch (mTopic) {
        case 0:
            startTvSound(&mHandles[0], SE_TV_02_b0);
            break;
        case 1:
            startTvSound(&mHandles[0], TV_SEQ_KK_DRUMN);
            break;
        case 2:
            mHandles[0].SetVolume(40.0f / 127.0f, 45);
            break;
        }
        break;
    case 120:
        nw4r::snd::SeqSoundHandle(&mHandles[0]).WriteVariable(1, 1);
        break;
    case 150:
        nw4r::snd::SeqSoundHandle(&mHandles[0]).WriteVariable(0, 2);
        break;
    case 450:
        startTvSound(&mHandles[1], SE_TV_02_v10);
        nw4r::snd::SeqSoundHandle(&mHandles[0]).WriteVariable(0, 0);
        break;
    case 600:
        startTvSound(&mHandles[1], SE_TV_02_v20);
        nw4r::snd::SeqSoundHandle(&mHandles[0]).WriteVariable(1, 2);
        break;
    case 900:
        nw4r::snd::SeqSoundHandle(&mHandles[0]).WriteVariable(0, 3);
        break;
    case 1125:
        startTvSound(&mHandles[1], SE_TV_02_v30);
        break;
    case 1155:
        if (mTopic == 2) {
            mHandles[0].Stop(45);
        }
        break;
    }

    switch (mTopic) {
    case 0:
        nw4r::snd::SeqSoundHandle(&mHandles[0]).SetTrackVolume(0xFFF, 1.0f);
        nw4r::snd::SeqSoundHandle(&mHandles[1]).SetTrackVolume(0xFFF, 1.0f);
        break;
    case 1:
        nw4r::snd::SeqSoundHandle(&mHandles[0]).SetTrackVolume(0xFFF, 100.0f / 127.0f);
        nw4r::snd::SeqSoundHandle(&mHandles[1]).SetTrackVolume(0xFFF, 0.0f);
        break;
    case 2:
        nw4r::snd::SeqSoundHandle(&mHandles[1]).SetTrackVolume(0xFFF, 100.0f / 127.0f);
        break;
    }
}

// 801D1020
void SoundTv_03_drama::startProgram() {
    startTvSound(&mHandles[0], TV_SEQ_HATO);
    mScene = 0;
}

// 801D1068
void SoundTv_03_drama::execute(int frame, const nw4r::math::VEC3 *pos) {
    SoundTv_base::execute(frame, pos);
    switch (frame) {
    case 1:
        if (mScene == 0) {
            startTvSound(&mHandles[1], SE_TV_03_v0);
        }
        break;
    case 360:
        if (mScene == 1) {
            startTvSound(&mHandles[1], SE_TV_03_v1);
        }
        break;
    case 570:
        if (mScene == 0) {
            startTvSound(&mHandles[1], SE_TV_03_v2);
        }
        break;
    case 630:
        if (mScene == 1) {
            startTvSound(&mHandles[1], SE_TV_03_v4);
        }
        break;
    case 900:
        if (mScene == 0) {
            startTvSound(&mHandles[1], SE_TV_03_v3);
        }
        break;
    case 1047:
        mScene++;
        if (mScene == 2) {
            mScene = 0;
        }
        break;
    }
}

// 801D11E8
void SoundTv_04_song::startProgram() {
    startTvSound(&mHandles[1], TV_SEQ_KK_JAZZ);
    mVariable = -1;
    mBank = 0;
}

// 801D1238
void SoundTv_04_song::execute(int frame, const nw4r::math::VEC3 *pos) {
    SoundTv_base::execute(frame, pos);
    int bank = mBank * 4;
    switch (readVariable()) {
    case 1:
        startTvSound(&mHandles[0], SE_TV_04_v0 + bank);
        break;
    case 2:
        startTvSound(&mHandles[0], SE_TV_04_v1 + bank);
        break;
    case 3:
        startTvSound(&mHandles[0], SE_TV_04_v2 + bank);
        break;
    case 4:
        startTvSound(&mHandles[0], SE_TV_04_v3 + bank);
        mBank = (mBank + 1) % 2;
        break;
    }
}

// 801D133C
int SoundTv_04_song::readVariable() {
    s16 value;
    {
        nw4r::snd::SeqSoundHandle handle(&mHandles[1]);
        handle.ReadVariable(0, &value);
    }

    int changed = -1;
    if (value != mVariable) {
        changed = value;
    }
    mVariable = value;
    return changed;
}

// 801D13AC
void SoundTv_05_quiz::startProgram() {
    mRound = 0;
}

// 801D13B8
void SoundTv_05_quiz::execute(int frame, const nw4r::math::VEC3 *pos) {
    SoundTv_base::execute(frame, pos);
    if (!mHandles[1].IsAttachedSound()) {
        startTvSound(&mHandles[1], SE_TV_05_BG);
        if (frame >= 210 && frame < 660) {
            nw4r::snd::SeqSoundHandle(&mHandles[1]).WriteVariable(1, 1);
        } else if (frame >= 660 && frame < 840) {
            nw4r::snd::SeqSoundHandle(&mHandles[1]).WriteVariable(1, 0);
        } else if (frame >= 840 && frame < 1110) {
            nw4r::snd::SeqSoundHandle(&mHandles[1]).WriteVariable(1, 1);
        } else if (frame >= 1110) {
            nw4r::snd::SeqSoundHandle(&mHandles[1]).WriteVariable(1, 0);
        }
    }

    switch (frame) {
    case 1:
        if (mRound == 0) {
            startTvSound(&mHandles[0], SE_TV_05_MC0);
        } else {
            startTvSound(&mHandles[0], SE_TV_05_MC1);
        }
        break;
    case 210:
        nw4r::snd::SeqSoundHandle(&mHandles[1]).WriteVariable(1, 1);
        break;
    case 630:
        nw4r::snd::SeqSoundHandle(&mHandles[1]).WriteVariable(2, 1);
        break;
    case 660:
        nw4r::snd::SeqSoundHandle(&mHandles[1]).WriteVariable(1, 0);
        break;
    case 690:
        if (mRound == 0) {
            startTvSound(&mHandles[0], SE_TV_05_RES0);
        } else {
            startTvSound(&mHandles[0], SE_TV_05_RES1);
        }
        break;
    case 759:
        if (mRound == 1) {
            nw4r::snd::SeqSoundHandle(&mHandles[1]).WriteVariable(2, 2);
        }
        break;
    case 774:
        if (mRound == 0) {
            nw4r::snd::SeqSoundHandle(&mHandles[1]).WriteVariable(2, 2);
        }
        break;
    case 840:
        nw4r::snd::SeqSoundHandle(&mHandles[1]).WriteVariable(1, 1);
        break;
    case 1110:
        nw4r::snd::SeqSoundHandle(&mHandles[1]).WriteVariable(1, 0);
        nw4r::snd::SeqSoundHandle(&mHandles[1]).WriteVariable(2, 1);
        break;
    case 1149:
        if (mRound == 1) {
            nw4r::snd::SeqSoundHandle(&mHandles[0]).WriteVariable(0, 2);
        }
        break;
    case 1170:
        if (mRound == 0) {
            nw4r::snd::SeqSoundHandle(&mHandles[0]).WriteVariable(0, 2);
        }
        break;
    case 1215:
        if (mRound == 1) {
            nw4r::snd::SeqSoundHandle(&mHandles[1]).WriteVariable(2, 3);
        }
        break;
    case 1275:
        if (mRound == 0) {
            nw4r::snd::SeqSoundHandle(&mHandles[1]).WriteVariable(2, 3);
        }
        break;
    case 1439:
        mRound++;
        if (mRound == 2) {
            mRound = 0;
        }
        break;
    }
}

// 801D18F4
void SoundTv_06_travel::execute(int frame, const nw4r::math::VEC3 *pos) {
    SoundTv_base::execute(frame, pos);
    if (frame == 0) {
        nw4r::snd::SeqSoundHandle(&mHandles[1]).SetTrackVolume(1 << 0, 70.0f / 127.0f);
        nw4r::snd::SeqSoundHandle(&mHandles[1]).SetTrackVolume(1 << 1, 80.0f / 127.0f);
        nw4r::snd::SeqSoundHandle(&mHandles[1]).SetTrackVolume((1 << 2) | (1 << 3), 120.0f / 127.0f);
        nw4r::snd::SeqSoundHandle(&mHandles[1]).SetTrackVolume(1 << 4, 110.0f / 127.0f);
        if (mNarrating) {
            mHandles[0].Stop(0);
            startTvSound(&mHandles[0], SE_TV_06_VOICE0);
        }
    } else if (frame == 270) {
        nw4r::snd::SeqSoundHandle(&mHandles[1]).SetTrackVolume(1 << 0, 75.0f / 127.0f);
        nw4r::snd::SeqSoundHandle(&mHandles[1]).SetTrackVolume((1 << 1) | (1 << 2) | (1 << 3), 110.0f / 127.0f);
        nw4r::snd::SeqSoundHandle(&mHandles[1]).SetTrackVolume(1 << 4, 110.0f / 127.0f);
        if (mNarrating) {
            mHandles[0].Stop(0);
            startTvSound(&mHandles[0], SE_TV_06_VOICE1);
        }
    } else if (frame == 480) {
        nw4r::snd::SeqSoundHandle(&mHandles[1]).SetTrackVolume(1 << 0, 80.0f / 127.0f);
        nw4r::snd::SeqSoundHandle(&mHandles[1]).SetTrackVolume((1 << 1) | (1 << 2) | (1 << 3), 110.0f / 127.0f);
        nw4r::snd::SeqSoundHandle(&mHandles[1]).SetTrackVolume(1 << 4, 110.0f / 127.0f);
        if (mNarrating) {
            mHandles[0].Stop(0);
            startTvSound(&mHandles[0], SE_TV_06_VOICE2);
        }
    } else if (frame == 780) {
        nw4r::snd::SeqSoundHandle(&mHandles[1]).SetTrackVolume(1 << 0, 80.0f / 127.0f);
        nw4r::snd::SeqSoundHandle(&mHandles[1]).SetTrackVolume((1 << 1) | (1 << 2) | (1 << 3), 110.0f / 127.0f);
        nw4r::snd::SeqSoundHandle(&mHandles[1]).SetTrackVolume(1 << 4, 100.0f / 127.0f);
        if (mNarrating) {
            mHandles[0].Stop(0);
            startTvSound(&mHandles[0], SE_TV_06_VOICE3);
        }
    } else if (frame == 1113) {
        nw4r::snd::SeqSoundHandle(&mHandles[1]).SetTrackVolume(1 << 0, 75.0f / 127.0f);
        nw4r::snd::SeqSoundHandle(&mHandles[1]).SetTrackVolume(1 << 1, 0.0f);
        nw4r::snd::SeqSoundHandle(&mHandles[1]).SetTrackVolume((1 << 2) | (1 << 3), 9.0f / 127.0f);
        nw4r::snd::SeqSoundHandle(&mHandles[1]).SetTrackVolume(1 << 4, 75.0f / 127.0f);
        if (mNarrating) {
            mHandles[0].Stop(0);
            startTvSound(&mHandles[0], SE_TV_06_VOICE4);
        }
    } else if (frame == 1353) {
        nw4r::snd::SeqSoundHandle(&mHandles[1]).SetTrackVolume(1 << 0, 85.0f / 127.0f);
        nw4r::snd::SeqSoundHandle(&mHandles[1]).SetTrackVolume((1 << 1) | (1 << 2) | (1 << 3), 0.0f);
        nw4r::snd::SeqSoundHandle(&mHandles[1]).SetTrackVolume(1 << 4, 70.0f / 127.0f);
        mNarrating ^= 1;
    }
}

// 801D1E24
void SoundTv_06_travel::startProgram() {
    startTvSound(&mHandles[1], SE_TV_06_BGM0);
    startTvSound(&mHandles[0], SE_TV_06_VOICE0);
    mNarrating = true;
}

// 801D1E88
void SoundTv_07_hero::execute(int frame, const nw4r::math::VEC3 *pos) {
    SoundTv_base::execute(frame, pos);
    if (mOn == 1) {
        mOn = 0;
        mHandles[1].Stop(0);
        startTvSound(&mHandles[1], SE_TV_07_BGM0);
        mHandles[0].Stop(0);
        startTvSound(&mHandles[0], SE_TV_07_SE_LAUGH);
        mEpisode = 0;
        mLastEpisode = 0;
        mPhase = 0;
        mCount = 0;
    } else if (frame == 0) {
        mHandles[1].Stop(0);
        mHandles[0].Stop(0);
        if (SoundSystem::getInstance()->getRandom(100) < 40) {
            startTvSound(&mHandles[1], SE_TV_07_BGM0s);
            startTvSound(&mHandles[0], SE_TV_07_SE_LAUGH);
            mEpisode = 1;
        } else {
            startTvSound(&mHandles[1], SE_TV_07_BGM2);
            startTvSound(&mHandles[0], SE_TV_07_SE_POWERUP);
            mEpisode = 2;
        }
    } else if (frame == 270 && mEpisode != 0) {
        int rnd = SoundSystem::getInstance()->getRandom(3);
        mHandles[1].Stop(0);
        mHandles[0].Stop(0);
        if (rnd == 0) {
            startTvSound(&mHandles[1], SE_TV_07_BGM1);
            startTvSound(&mHandles[0], SE_TV_07_SE_RUNBOMB);
            mEpisode = 3;
        } else if (rnd == 1) {
            startTvSound(&mHandles[1], SE_TV_07_BGM1);
            startTvSound(&mHandles[0], SE_TV_07_SE_RUNWIND);
            mEpisode = 4;
        } else {
            startTvSound(&mHandles[1], SE_TV_07_BGM3);
            startTvSound(&mHandles[0], SE_TV_07_SE_RUNBOMB);
            mEpisode = 5;
        }
    } else if (frame == 360 && mEpisode == 0) {
        int rnd = SoundSystem::getInstance()->getRandom(2);
        mHandles[1].Stop(0);
        mHandles[0].Stop(0);
        if (rnd == 0) {
            startTvSound(&mHandles[1], SE_TV_07_BGM1);
            startTvSound(&mHandles[0], SE_TV_07_SE_RUNBOMB);
            mEpisode = 3;
        } else {
            startTvSound(&mHandles[1], SE_TV_07_BGM1);
            startTvSound(&mHandles[0], SE_TV_07_SE_RUNWIND);
            mEpisode = 4;
        }
    }

    if (mLastEpisode != mEpisode) {
        if (mCount == 0) {
            mCount++;
        } else if (mCount == 1) {
            if (SoundSystem::getInstance()->getRandom(2) == 0) {
                mCount++;
            } else {
                mCount = 0;
                mPhase ^= 1;
            }
        } else if (mCount >= 2) {
            mCount = 0;
            mPhase ^= 1;
        }
        if (mPhase % 2 == 1) {
            nw4r::snd::SeqSoundHandle(&mHandles[1]).SetTrackMute(0x3F, true);
        }
        mLastEpisode = mEpisode;
    }
}

// 801D23E8
void SoundTv_07_hero::reset() {
    SoundTv_base::reset();
    mPhase = 0;
    mCount = 0;
}

// 801D2420
void SoundTv_07_hero::startProgram() {
    startTvSound(&mHandles[1], SE_TV_07_BGM1);
    mEpisode = 3;
    mLastEpisode = 3;
}

// 801D246C
void SoundTv_08_variety::execute(int frame, const nw4r::math::VEC3 *pos) {
    SoundTv_base::execute(frame, pos);
    if (mOn == 1) {
        mOn = 0;
        mHandles[0].Stop(0);
        startTvSound(&mHandles[0], SE_TV_08_BGM0);
        mTimer = 0;
    } else if (frame == 0) {
        if (SoundSystem::getInstance()->getRandom(100) < 70) {
            nw4r::snd::SeqSoundHandle(&mHandles[0]).WriteVariable(0, 1);
        }
        mHandles[1].Stop(0);
        startTvSound(&mHandles[1], SE_TV_08_VOICE0);
    } else if (frame == 270) {
        if (SoundSystem::getInstance()->getRandom(100) < 60) {
            nw4r::snd::SeqSoundHandle(&mHandles[0]).WriteVariable(1, 1);
            mReaction = 1;
        }
        int rnd = SoundSystem::getInstance()->getRandom(100);
        mHandles[1].Stop(0);
        if (rnd < 33) {
            startTvSound(&mHandles[1], SE_TV_08_VOICE10);
        } else if (rnd < 66) {
            startTvSound(&mHandles[1], SE_TV_08_VOICE11);
        } else {
            startTvSound(&mHandles[1], SE_TV_08_VOICE12);
        }
    } else if (frame == 435) {
        int rnd = SoundSystem::getInstance()->getRandom(100);
        mHandles[1].Stop(0);
        if (rnd < 33) {
            startTvSound(&mHandles[1], SE_TV_08_VOICE10);
        } else if (rnd < 66) {
            startTvSound(&mHandles[1], SE_TV_08_VOICE11);
        } else {
            startTvSound(&mHandles[1], SE_TV_08_VOICE12);
        }
    } else if (frame == 540) {
        if (SoundSystem::getInstance()->getRandom(100) < 70 && mReaction != 1) {
            nw4r::snd::SeqSoundHandle(&mHandles[0]).WriteVariable(1, 1);
            mReaction = 2;
        }
        if (SoundSystem::getInstance()->getRandom(100) < 60 && mReaction != 1) {
            nw4r::snd::SeqSoundHandle(&mHandles[0]).WriteVariable(0, 1);
            mReaction = 2;
        }
    } else if (frame == 750) {
        int rnd = SoundSystem::getInstance()->getRandom(100);
        if (mReaction != 2) {
            if (rnd < 70) {
                nw4r::snd::SeqSoundHandle(&mHandles[0]).WriteVariable(2, 1);
                mReaction = 3;
            } else {
                nw4r::snd::SeqSoundHandle(&mHandles[0]).WriteVariable(0, 1);
                mReaction = 4;
            }
        }
        mHandles[1].Stop(0);
        startTvSound(&mHandles[1], SE_TV_08_VOICE2);
    } else if (mTimer == 309) {
        mHandles[0].Stop(0);
        startTvSound(&mHandles[0], SE_TV_08_BGM1);
    } else if (mTimer == 2100) {
        mHandles[0].Stop(0);
        startTvSound(&mHandles[0], SE_TV_08_BGM2);
    } else if (mTimer == 2700) {
        mHandles[0].Stop(0);
        startTvSound(&mHandles[0], SE_TV_08_BGM0);
        mTimer = 0;
    }
    mTimer++;
}

// 801D2AE0
void SoundTv_08_variety::reset() {
    SoundTv_base::reset();
    mTimer = 0;
}

// 801D2B14
void SoundTv_08_variety::startProgram() {
    startTvSound(&mHandles[0], SE_TV_08_BGM1);
    mReaction = 0;
}

// 801D2B5C
void SoundTv_09_movie::startProgram() {
    startTvSound(&mHandles[0], SE_TV_09_L0);
    startTvSound(&mHandles[1], SE_TV_09_S0);
}

// 801D2BB8
void SoundTv_09_movie::execute(int frame, const nw4r::math::VEC3 *pos) {
    SoundTv_base::execute(frame, pos);
    switch (frame) {
    case 0:
    case 3:
    case 6:
    case 9:
    case 12:
        if (!mHandles[0].IsAttachedSound()) {
            startTvSound(&mHandles[0], SE_TV_09_L0);
        }
        if (!mHandles[1].IsAttachedSound()) {
            startTvSound(&mHandles[1], SE_TV_09_S0);
        }
        nw4r::snd::SeqSoundHandle(&mHandles[0]).WriteVariable(0, 1);
        nw4r::snd::SeqSoundHandle(&mHandles[1]).WriteVariable(0, 1);
        break;
    case 270:
        nw4r::snd::SeqSoundHandle(&mHandles[0]).WriteVariable(0, 2);
        nw4r::snd::SeqSoundHandle(&mHandles[1]).WriteVariable(0, 2);
        break;
    case 420:
        nw4r::snd::SeqSoundHandle(&mHandles[0]).WriteVariable(0, 3);
        break;
    case 675:
        nw4r::snd::SeqSoundHandle(&mHandles[0]).WriteVariable(0, 4);
        break;
    case 705:
        nw4r::snd::SeqSoundHandle(&mHandles[0]).WriteVariable(0, 5);
        break;
    case 735:
        nw4r::snd::SeqSoundHandle(&mHandles[1]).WriteVariable(0, 3);
        break;
    }
}

// 801D2E2C: mState is zone * 5 + play (zones 0..2, plays NORMAL/CALL/SONG/FINEPLAY/ROUGHPLAY), 15 goal,
// 16 miss. When a play has run long enough, the next zone and play are drawn from the thresholds.
void SoundTv_10_soccer::execute(int frame, const nw4r::math::VEC3 *pos) {
    SoundTv_base::execute(frame, pos);
    int zone0Max, zone1Max;
    int normalMax, callMax, songMax, fineMax;

    mTimer++;
    if (mState == 15 && mTimer >= 420) {
        mChange = 1;
        zone0Max = 50;
        zone1Max = 75;
        normalMax = 25;
        mTimer = 0;
        callMax = 50;
        songMax = 75;
        fineMax = 88;
    } else if (mState == 16 && mTimer >= 300) {
        mChange = 1;
        zone0Max = 50;
        zone1Max = 75;
        normalMax = 25;
        mTimer = 0;
        callMax = 50;
        songMax = 75;
        fineMax = 88;
    } else {
        int play = mState % 5;
        if (play <= 2 && mTimer >= 360) {
            mChange = 1;
            mTimer = 0;
            int zone = mState / 5;
            if (zone == 0) {
                zone0Max = 50;
                zone1Max = 75;
                normalMax = 25;
                callMax = 50;
                songMax = 75;
                fineMax = 88;
            } else if (zone == 1) {
                zone0Max = 40;
                zone1Max = 80;
                normalMax = 25;
                callMax = 50;
                songMax = 75;
                fineMax = 88;
            } else {
                zone0Max = 40;
                zone1Max = 60;
                normalMax = 25;
                callMax = 50;
                songMax = 75;
                fineMax = 88;
            }
        } else if (play >= 3 && mTimer >= 210) {
            mChange = 1;
            mTimer = 0;
            int zone = mState / 5;
            if (zone == 0) {
                zone0Max = 50;
                zone1Max = 75;
                normalMax = 35;
                callMax = 70;
                songMax = 100;
                fineMax = 100;
            } else if (zone == 1) {
                zone0Max = 40;
                zone1Max = 80;
                normalMax = 35;
                callMax = 70;
                songMax = 100;
                fineMax = 100;
            } else {
                zone0Max = 40;
                zone1Max = 60;
                normalMax = 35;
                callMax = 70;
                songMax = 100;
                fineMax = 100;
            }
        }
    }

    if (mChange == 1) {
        mHandles[1].Stop(0);
        mPrevState = mState;

        int rnd = SoundSystem::getInstance()->getRandom(100);
        int zone;
        if (rnd <= zone0Max) {
            zone = 0;
        } else if (rnd <= zone1Max) {
            zone = 5;
        } else {
            zone = 10;
        }

        rnd = SoundSystem::getInstance()->getRandom(100);
        if (rnd <= normalMax) {
            mState = zone;
            if (mPrevState % 5 != mState % 5) {
                mHandles[0].Stop(0);
                startTvSound(&mHandles[0], SE_TV_10_NORMAL);
            }
        } else if (rnd <= callMax) {
            mState = zone + 1;
            if (mPrevState % 5 != mState % 5) {
                mHandles[0].Stop(0);
                startTvSound(&mHandles[0], SE_TV_10_CALL);
            }
        } else if (rnd <= songMax) {
            mState = zone + 2;
            if (mPrevState % 5 != mState % 5) {
                mHandles[0].Stop(0);
                startTvSound(&mHandles[0], SE_TV_10_SONG);
            }
        } else if (rnd <= fineMax) {
            mState = zone + 3;
            if (mPrevState % 5 != mState % 5) {
                mHandles[0].Stop(0);
                startTvSound(&mHandles[0], SE_TV_10_FINEPLAY);
            }
        } else {
            mState = zone + 4;
            if (mPrevState % 5 != mState % 5) {
                mHandles[0].Stop(0);
                startTvSound(&mHandles[0], SE_TV_10_ROUGHPLAY);
            }
        }

        int band = zone / 5;
        if (band == 0) {
            nw4r::snd::SeqSoundHandle(&mHandles[0]).SetTrackVolume((1 << 0) | (1 << 1), 100.0f / 127.0f);
            if (SoundSystem::getInstance()->getRandom(100) < 40 && frame > 90) {
                startTvSound(&mHandles[1], SE_TV_10_RUN);
                nw4r::snd::SeqSoundHandle(&mHandles[1]).SetTrackVolume((1 << 0) | (1 << 1), 100.0f / 127.0f);
            }
        } else if (band == 1) {
            nw4r::snd::SeqSoundHandle(&mHandles[0]).SetTrackVolume((1 << 0) | (1 << 1), 75.0f / 127.0f);
            startTvSound(&mHandles[1], SE_TV_10_TALK);
        } else {
            nw4r::snd::SeqSoundHandle(&mHandles[0]).SetTrackVolume((1 << 0) | (1 << 1), 1.0f);
            if (SoundSystem::getInstance()->getRandom(100) < 40 && frame > 90 && mState <= 13) {
                startTvSound(&mHandles[1], SE_TV_10_RUN);
                nw4r::snd::SeqSoundHandle(&mHandles[1]).SetTrackVolume((1 << 0) | (1 << 1), 80.0f / 127.0f);
            }
        }

        if (mPrevState == 11 || mPrevState == 13) {
            rnd = SoundSystem::getInstance()->getRandom(100);
            if (rnd <= 20) {
                mHandles[0].Stop(0);
                startTvSound(&mHandles[0], SE_TV_10_GOAL);
                mState = 15;
            } else if (rnd <= 40) {
                mHandles[0].Stop(0);
                startTvSound(&mHandles[0], SE_TV_10_MISS);
                mState = 16;
            }
        }
        mChange = 0;
    }
}

// 801D36CC
void SoundTv_10_soccer::reset() {
    SoundTv_base::reset();
    mState = 0;
    mPrevState = 0;
    mTimer = 0;
}

// 801D3708
void SoundTv_10_soccer::startProgram() {
    startTvSound(&mHandles[0], SE_TV_10_NORMAL);
    mPrevState = 0;
    mState = 0;
}

// 801D3754
void SoundTv_255_weather::startProgram() {
    mLoops = 0;
    mSong = SoundSystem::getInstance()->getRandom(3);
    mForecast = 0;
    startTvSound(&mHandles[0], TV_SEQ_KK_SWING + mSong);
}

// 801D3800
void SoundTv_255_weather::startNextSong() {
    int song;
    do {
        song = SoundSystem::getInstance()->getRandom(3);
    } while (song == mSong);
    mSong = song;
    startTvSound(&mHandles[0], TV_SEQ_KK_SWING + (u8)song);
}

// 801D38A8
void SoundTv_255_weather::execute(int frame, const nw4r::math::VEC3 *pos) {
    SoundTv_base::execute(frame, pos);
    switch (frame) {
    case 30:
        switch (mForecast) {
        case 0:
            startTvSound(&mHandles[1], SE_TV_255_v00);
            break;
        case 1:
            startTvSound(&mHandles[1], SE_TV_255_v10);
            break;
        }
        break;
    case 150:
        if (mForecast == 0) {
            startTvSound(&mHandles[1], SE_TV_255_v01);
        }
        break;
    case 170:
        if (mForecast == 1) {
            startTvSound(&mHandles[1], SE_TV_255_v11);
        }
        break;
    case 1199:
        mLoops++;
        if (mLoops == 7) {
            mLoops = 0;
            startNextSong();
        }
        mForecast++;
        if (mForecast == 3) {
            mForecast = 0;
        }
        break;
    }
}
