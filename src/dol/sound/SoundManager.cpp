// Sound archive, stage groups, playback and output fades.
#include <game/snd/snd_manager.hpp>
#include <game/snd/snd_system.hpp>
#include <game/game/d_scene.hpp>
#include <revolution/ai.h>
#include <revolution/ax.h>
#include <revolution/mix.h>

static const u32 sVolumePlayers[5] = {21, 22, 23, 25, 24};
static const u16 sStageGroups[dScene::SCENE_NUM] = {
    132,134,134,134,134,134,134,134,134,134,134,134,134,134,134,134,
    134,134,134,134,134,134,134,134,134,134,134,140,141,141,135,135,
    135,135,135,135,135,136,135,142,143,143,143,143,144,133,145,146,
    147,148,138,134,130,130,137,149,132,129,129,161,139,139,139,141,
    141,141,132,134
};

static inline void initReverbParam(nw4r::snd::FxReverbHiDpl2::ReverbHiDpl2Param &param) {
    param.earlyMode = 4;
    param.preDelayTimeMax = 0.05f;
    param.preDelayTime = 0.01f;
    param.fusedMode = 2;
    param.fusedTime = 0.5f;
    param.coloration = 0.6f;
    param.damping = 0.4f;
    param.crossTalk = 0.1f;
    param.earlyGain = 0.3f;
    param.fusedGain = 0.3f;
    param.outGain = 1.0f;
}

// 801CE58C
bool SoundManager::init(int soundPriority, int dvdPriority, void *heapBuf, u32 heapSize) {
    AIInit(NULL);
    AXInit();
    MIXInit();
    nw4r::snd::SoundSystem::InitSoundSystem(soundPriority, dvdPriority);
    if (nw4r::snd::SoundSystem::GetOutputMode() == nw4r::snd::OUTPUT_MODE_DPL2) {
        nw4r::snd::SoundSystem::SetOutputMode(nw4r::snd::OUTPUT_MODE_STEREO);
    }
    mHeapBuffer = heapBuf;
    mStageHeapBuffer = (u8 *)heapBuf + heapSize - 0x85F000;
    mHeapSize = heapSize;
    u8 *commonBuf = mStageHeapBuffer - 0x25800;
    mHeap.Create(heapBuf, heapSize - 0x884800);
    mHeap324.Create(commonBuf, 0x25800);
    mHeap2CC.Create(mStageHeapBuffer, 0x85F000);
    nw4r::snd::FxReverbHiDpl2::ReverbHiDpl2Param param(0.0f, 0.0f);
    initReverbParam(param);
    mReverb.SetParam(param);
    u32 effectSize = mReverb.GetRequiredMemSize();
    void *effectBuf = mHeap.Alloc(effectSize, NULL, NULL);
    mReverb.AssignWorkBuffer(effectBuf, effectSize);
    nw4r::snd::SoundSystem::AppendEffect(nw4r::snd::AUX_A, &mReverb);
    mHeap.SaveState();
    if (!mArchive.Open("/Sound/rv_forest.brsar")) {
        OSPanic("SoundManager.cpp", 0xA7, "Cannot open \"/Sound/rv_forest.brsar\"");
    }
    u32 headerSize = mArchive.GetHeaderSize();
    void *header = mHeap.Alloc(headerSize, NULL, NULL);
    mArchive.LoadHeader(header, headerSize);
    u32 playerSize = mPlayer.GetRequiredMemSize(&mArchive);
    void *streamBuf;
    u32 streamSize = mPlayer.GetRequiredStrmBufferSize(&mArchive);
    streamBuf = mHeap.Alloc(streamSize, NULL, NULL);
    void *playerBuf = mHeap.Alloc(playerSize, NULL, NULL);
    mPlayer.Setup(&mArchive, playerBuf, playerSize, streamBuf, streamSize);
    mHeap.GetFreeSize();
    loadGroup(0x81);
    loadGroup(0);
    u32 voiceGroup = 0;
    switch (SoundSystem::getInstance()->mVoice.mLanguage) {
    case SoundVoice::LANG_JP: voiceGroup = 0x98; break;
    case SoundVoice::LANG_EN: voiceGroup = 0x99; break;
    case SoundVoice::LANG_ES: voiceGroup = 0x9D; break;
    case SoundVoice::LANG_FR: voiceGroup = 0x9A; break;
    case SoundVoice::LANG_IT: voiceGroup = 0x9C; break;
    case SoundVoice::LANG_DE: voiceGroup = 0x9B; break;
    case SoundVoice::LANG_KR: voiceGroup = 0x98; break;
    }
    loadGroup(voiceGroup);
    mFaders[0].set(0.0f);
    for (int i = 0; i < 30; i++) mPlayerVolumes[i] = 1.0f;
    mReverbFader.set(1.0f);
    mFaders[1].set(1.0f);
    mFaders[2].set(1.0f);
    mFaders[3].set(1.0f);
    setMasterVolume(1.0f, 0);
    return true;
}

// 801CE978
void SoundManager::calc() {
    SoundSystem::getInstance()->calc();
    mPlayer.Update();
    mLoader.restart();
    if (mFaders[0].mFrames > 0) mFaders[0].calc();
    if (mUpdateLpf) {
        for (u32 i = 0; i < 30; i++) mPlayer.GetSoundPlayer(i).SetLpfFreq(mLpfFreq);
        mUpdateLpf = false;
    }
    if (mReverbState == 0) {
        setPlayerVolume(mFaders[1].calc());
    } else if (mReverbState == 1) {
        if (mReverbFader.mFrames > 0) mReverbFader.calc();
        else {
            SoundSystem::getInstance()->pauseMusic(true);
            mReverbState = 2;
        }
    } else if (mReverbState == 2) {
    } else if (mReverbState == 3) {
        if (mReverbFader.mFrames > 0) mReverbFader.calc();
        else mReverbState = 0;
    }
    SoundObjBase::setMasterVolume(mFaders[2].calc());
    f32 sceneVolume = mFaders[3].calc();
    mPlayer.GetSoundPlayer(14).SetVolume(sceneVolume);
    mPlayer.GetSoundPlayer(11).SetVolume(sceneVolume);
    mPlayer.GetSoundPlayer(20).SetVolume(sceneVolume);
}

// 801CEB70
void SoundManager::stopAll(int frames) {
    if (!mStopped) {
        SoundSystem::getInstance()->mMelody.stop();
        SoundSystem::getInstance()->mMelody.mPart0.end();
        SoundObjBase::stopAllObj(frames);
        nw4r::snd::SoundSystem::SetMasterVolume(0.0f, frames * 16.666668f);
        mStopped = true;
    }
}

// 801CEC90
void SoundManager::resume() {
    nw4r::snd::SoundSystem::SetMasterVolume(1.0f, 0);
    if (mStopped) mStopped = false;
    if (mReverbState != 0) startReverb();
    SoundSystem::getInstance()->mMelody.mState = SoundMelody::STATE_IDLE;
}

// 801CED38
void SoundManager::shutdown() {
    nw4r::snd::SoundSystem::ClearEffect(nw4r::snd::AUX_A, 0);
    nw4r::snd::SoundSystem::PrepareReset();
    nw4r::snd::SoundSystem::WaitForResetReady();
}

// 801CED70
static void stopHandle(nw4r::snd::SoundHandle &handle) {
    handle.Stop(10);
}

// 801CED88
void SoundManager::beginSceneChange() {
    SoundObjBase::stopAllObj(30);
    SoundAmbient::sIsStageSet = false;
    mFaders[3].set(1.0f);
    mFaders[3].fadeTo(0.0f, 45);
    SoundSystem::getInstance()->mMelody.stop();
}

// 801CEE24
bool SoundManager::setupStage(int stage) {
    if (stage == dScene::SCENE_DM_SAVE || stage == dScene::SCENE_DM_LOAD) mFaders[3].set(0.0f);
    else {
        SoundObjBase::sIsPlayable = true;
        mFaders[3].set(1.0f);
    }
    if (stage == dScene::SCENE_DM_TITLE) {
        SoundSystem::getInstance()->mState = SoundSystem::STATE_WAIT;
        if (mStopped) mStopped = false;
    }
    if (stage == dScene::SCENE_FIELD && _778 != dScene::SCENE_FIELD) clearMusicHeap();
    if (_778 == stage) {
        SoundAmbient::sIsStageSet = true;
        return true;
    }
    _780 = stage;
    if (stage >= dScene::SCENE_FIELD && stage <= dScene::SCENE_RM_TAILOR) {
        if (!isGroupReady(2)) {
            u32 loading = mLoader.mState == SoundLoader::STATE_LOADING ? mLoader.mGroupId : 0;
            if (loading != 2) mHeap324.Clear();
            mLoader.requestGroup(2, &mPlayer, &mHeap324);
            return false;
        }
    } else if (stage >= dScene::SCENE_TOWN && stage <= dScene::SCENE_RM_RESET && !isGroupReady(0xA0)) {
        u32 loading = mLoader.mState == SoundLoader::STATE_LOADING ? mLoader.mGroupId : 0;
        if (loading != 0xA0) mHeap324.Clear();
        mLoader.requestGroup(0xA0, &mPlayer, &mHeap324);
        return false;
    }
    u32 group = sStageGroups[stage];
    if (!isGroupReady(group)) {
        if (_77C != group) {
            if (group == 0x86 && _77C != 0x86) {
                destroyStageHeaps();
                u8 *buf = mStageHeapBuffer;
                u8 *musicBuf = buf + 0x83C000;
                mHeap2CC.Create(buf, 0x823C00);
                mHeap2A0.Create(musicBuf, 0x23000);
                mHeap2F8.Create(musicBuf - 0x18400, 0x18400);
            } else if (group == 0x84 && _77C != 0x84) {
                destroyStageHeaps();
                u8 *buf = mStageHeapBuffer;
                mHeap2CC.Create(buf, 0x7CCC00);
                mHeap2A0.Create(buf + 0x7CCC00, 0x92400);
            } else if (_77C != group) {
                destroyStageHeaps();
                u8 *buf = mStageHeapBuffer;
                mHeap2CC.Create(buf, 0x846C00);
                mHeap2F8.Create(buf + 0x846C00, 0x18400);
            }
            _77C = group;
        }
        requestStageGroup(group);
        return false;
    }
    SoundSystem::getInstance()->setMusicFilter(0);
    if (stage > dScene::SCENE_DM_TITLE && stage <= dScene::SCENE_DM_BUS_TO_LAND) {
        SoundSystem::getInstance()->mMelody.stop();
        SoundSystem::getInstance()->mMelody.mPart0.end();
    }
    if (stage == dScene::SCENE_DM_SAVE || stage == dScene::SCENE_DM_LOAD) {
        mPlayer.GetSoundPlayer(11).ForEachSound(stopHandle);
        mPlayer.GetSoundPlayer(14).ForEachSound(stopHandle);
        mPlayer.GetSoundPlayer(20).ForEachSound(stopHandle);
    }
    SoundSystem::getInstance()->mAllowConductSequence = 0;
    SoundSystem::getInstance()->getMusicId();
    SoundSystem *system = SoundSystem::getInstance();
    if (stage == dScene::SCENE_FIELD || stage == dScene::SCENE_TOWN) {
        system->setMelodyVolume(1.0f);
        system->setMelodyFilter(0.0f);
    } else if (stage == dScene::SCENE_RM_THEATER_ENT || stage == dScene::SCENE_RM_THEATER_SHOW || (stage >= dScene::SCENE_DM_TITLE && stage <= dScene::SCENE_DM_CHKP_MISS)) {
        system->setMelodyVolume(0.0f);
        system->setMelodyFilter(0.0f);
    } else {
        system->setMelodyVolume(0.4f);
        system->setMelodyFilter(0.9f);
    }
    if (stage == dScene::SCENE_RM_MM_CAFE) {
        nw4r::snd::FxReverbHiDpl2::ReverbHiDpl2Param param(0.0f, 0.0f);
        initReverbParam(param);
        mReverb.setPreset(4);
        mReverb.SetParam(param);
        SoundObjBase::setFxSend(0.0f);
    } else if (stage >= dScene::SCENE_RM_MM_ENT && stage <= dScene::SCENE_RM_MM_ASTRO) {
        if (_778 < dScene::SCENE_RM_MM_ENT || _778 > dScene::SCENE_RM_MM_ASTRO || _778 == dScene::SCENE_RM_MM_CAFE) {
            nw4r::snd::FxReverbHiDpl2::ReverbHiDpl2Param param(0.0f, 0.0f);
            initReverbParam(param);
            mReverb.setPreset(4);
            mReverb.SetParam(param);
            SoundObjBase::setFxSend(0.8f);
        }
    } else if (stage == dScene::SCENE_FIELD || stage == dScene::SCENE_TOWN) {
        nw4r::snd::FxReverbHiDpl2::ReverbHiDpl2Param param(0.0f, 0.0f);
        initReverbParam(param);
        mReverb.setPreset(5);
        mReverb.SetParam(param);
        SoundObjBase::setFxSend(0.0f);
    } else SoundObjBase::setFxSend(0.0f);
    if (stage >= dScene::SCENE_DM_BUS_PL_CRT && stage <= dScene::SCENE_DM_BUS_TO_LAND) SoundSystem::getInstance()->enableSceneFade();
    else SoundSystem::getInstance()->disableSceneFade();
    SoundAmbient::sIsStageSet = true;
    _778 = stage;
    _780 = -1;
    return true;
}

// 801CF6A4
void SoundManager::setMasterVolume(f32 volume, int frames) {
    if (volume > 1.0f) {
        volume = 1.0f;
    } else if (volume < 0.0f) {
        volume = 0.0f;
    }
    nw4r::snd::SoundSystem::SetMasterVolume(volume, frames);
}

// 801CF714
void SoundManager::setVolume(f32 volume, int frames) {
    if (volume > 1.0f) {
        volume = 1.0f;
    } else if (volume < 0.0f) {
        volume = 0.0f;
    }
    mFaders[1].fadeTo(volume, frames);
    if (frames == 0) {
        setPlayerVolume(volume);
    }
}

// 801CF7A4
void SoundManager::setObjectVolume(f32 volume, int frames) {
    mFaders[2].fadeTo(volume, frames);
}

// 801CF7AC
void SoundManager::setPlayerVolume(f32 volume) {
    for (int i = 0; i < 5; i++) {
        mPlayer.GetSoundPlayer(sVolumePlayers[i]).SetVolume(volume);
    }
}

// 801CF820
bool SoundManager::startSound(nw4r::snd::SoundHandle *handle, u32 id) {
    bool result = false;
    if (isPlayable()) {
        result = mPlayer.StartSound(handle, id);
        if (result && mLpfFreq < 0.0f) {
            handle->SetLpfFreq(mLpfFreq);
        }
    }
    return result;
}

// 801CF8D8
bool SoundManager::holdSound(nw4r::snd::SoundHandle *handle, u32 id) {
    bool result = false;
    if (isPlayable()) {
        result = mPlayer.HoldSound(handle, id);
        if (result && mLpfFreq < 0.0f) {
            handle->SetLpfFreq(mLpfFreq);
        }
    }
    return result;
}

// 801CF990
bool SoundManager::startSound(nw4r::snd::SoundHandle *handle, u32 id,
                              const nw4r::snd::SoundStartable::StartInfo *info) {
    bool result = false;
    if (isPlayable()) {
        result = mPlayer.StartSound(handle, id, info);
        if (result && mLpfFreq < 0.0f) {
            handle->SetLpfFreq(mLpfFreq);
        }
    }
    return result;
}

// 801CFA4C
void SoundManager::clearMusicHeap() {
    if (mHeap2A0.IsValid()) {
        mHeap2A0.LoadState(0);
    }
}

// 801CFA68
void SoundManager::setFrameCallback(SoundAudioFrameCallback::Callback callback) {
    mFrameCallback.set(callback);
}

// 801CFA70
void SoundManager::clearFrameCallback() {
    mFrameCallback.clear();
}

// 801CFA78
void SoundManager::stopReverb() {
    mReverbFader.fadeTo(0.0f, 18);
    mReverbState = 1;
    setMasterVolume(0.0f, 300);
    nw4r::snd::SoundSystem::ClearEffect(nw4r::snd::AUX_A, 250);
}

// 801CFAD8
void SoundManager::startReverb() {
    SoundSystem::getInstance()->pauseMusic(false);
    mReverbFader.fadeTo(1.0f, 18);
    mReverbState = 3;
    setMasterVolume(1.0f, 300);
    nw4r::snd::SoundSystem::AppendEffect(nw4r::snd::AUX_A, &mReverb);
}

// 801CFB88
bool SoundManager::isGroupReady(u32 groupId) {
    return mPlayer.LoadGroup(groupId, NULL, 0);
}

// 801CFB98
u32 SoundManager::getSoundPlayerId(u32 id) {
    u32 result = 0;
    nw4r::snd::SoundArchive::SoundInfo info;
    if (mArchive.ReadSoundInfo(id, &info)) {
        result = info.playerId;
    }
    return result;
}

// 801CFBDC
bool SoundManager::isSoundReady(u32 id) {
    bool result = false;
    nw4r::snd::SoundArchive::SoundInfo info;
    if (mArchive.ReadSoundInfo(id, &info) && mPlayer.detail_GetFileAddress(info.fileId)) {
        result = true;
    }
    return result;
}

// 801CFC40
bool SoundManager::isBankReady(u32 id) {
    bool result = false;
    nw4r::snd::SoundArchive::BankInfo info;
    if (mArchive.detail_ReadBankInfo(id, &info) && mPlayer.detail_GetFileAddress(info.fileId)) {
        result = true;
    }
    return result;
}

// 801CFCA4
bool SoundManager::isSeqReady(u32 id) {
    bool result = false;
    nw4r::snd::SoundArchive::SeqSoundInfo info;
    if (mArchive.detail_ReadSeqSoundInfo(id, &info) && isSoundReady(id) && isBankReady(info.bankId)) {
        result = true;
    }
    return result;
}

// 801CFD28
void SoundManager::loadGroup(u32 groupId) {
    mLoader.loadGroup(groupId, &mPlayer, &mHeap);
}

// 801CFD38
bool SoundManager::requestMusicGroup(u32 groupId) {
    bool result = false;
    if (isGroupReady(groupId)) {
        result = true;
    } else if (mHeap2A0.IsValid()) {
        u32 loading = mLoader.mState == SoundLoader::STATE_LOADING ? mLoader.mGroupId : 0;
        if (groupId != loading && mLoader.mState == SoundLoader::STATE_IDLE) {
            clearMusicHeap();
        }
        result = mLoader.requestGroup(groupId, &mPlayer, &mHeap2A0);
    }
    return result;
}

// 801CFDE8
bool SoundManager::requestGroup(u32 groupId) {
    bool result = false;
    if (isGroupReady(groupId)) {
        result = true;
    } else if (mHeap2F8.IsValid()) {
        u32 loading = mLoader.mState == SoundLoader::STATE_LOADING ? mLoader.mGroupId : 0;
        if (groupId != loading && mLoader.mState == SoundLoader::STATE_IDLE) {
            mHeap2F8.Clear();
        }
        result = mLoader.requestGroup(groupId, &mPlayer, &mHeap2F8);
    }
    return result;
}

// 801CFE98
int SoundManager::getSoundPriority(u32 id) {
    int result = 0;
    nw4r::snd::SoundArchive::SoundInfo info;
    if (mArchive.ReadSoundInfo(id, &info)) {
        result = info.playerPriority;
    }
    return result;
}

// 801CFEDC
void SoundManager::destroyStageHeaps() {
    if (mHeap2A0.IsValid()) mHeap2A0.Destroy();
    if (mHeap2CC.IsValid()) mHeap2CC.Destroy();
    if (mHeap2F8.IsValid()) mHeap2F8.Destroy();
}

// 801CFF40
void SoundManager::mute() {
    setMasterVolume(0.0f, 0);
}

// 801CFF4C
void SoundManager::unmute() {
    if (!mStopped && mReverbState == 0) {
        setMasterVolume(1.0f, 300);
    }
}

// 801CFF74
bool SoundManager::requestStageGroup(u32 groupId) {
    if (groupId == 0x86) {
        if (!isGroupReady(0x9E)) {
            mLoader.requestGroup(0x9E, &mPlayer, &mHeap2CC);
            return false;
        }
        if (!isGroupReady(1)) {
            mLoader.requestGroup(1, &mPlayer, &mHeap2CC);
            return false;
        }
        if (!isGroupReady(0x9F)) {
            mLoader.requestGroup(0x9F, &mPlayer, &mHeap2CC);
            return false;
        }
    }
    if (!isGroupReady(groupId)) {
        mLoader.requestGroup(groupId, &mPlayer, &mHeap2CC);
        return false;
    }
    return true;
}

// 801D0070
bool SoundManager::isFixedVolume(u32 id) {
    bool result = false;
    u32 param = mArchive.GetSoundUserParam(id);
    if (param & 1) {
        result = true;
    } else if ((param & 4) && ((_778 >= dScene::SCENE_RM_SHOP0 && _778 <= dScene::SCENE_RM_SHOP3_F2) || _778 == dScene::SCENE_RM_MM_CAFE)) {
        result = true;
    }
    return result;
}
