#pragma once
#include <types.h>
#include <lib/nw4r/snd.h>
#include <lib/nw4r/snd/snd_FxReverbHiDpl2.h>
#include <lib/nw4r/snd/snd_SoundThread.h>
#include <game/snd/snd_effect_reverb.hpp>
#include <game/snd/snd_fader.hpp>
#include <game/snd/snd_obj.hpp> // SoundAudioFrameCallback

// The game's sound manager: the 0x784-byte object built by the inline SoundManager::getInstance
// (static local 805641E0, guard 8074E148; the first copy is in d_snd_util, the dtor 800104BC is
// the weak copy kept there). The class has no RTTI; its name comes from the split holding its
// methods (dol/sound/SoundManager.cpp, 801CE58C..801D00E8). The member classes with RTTI are
// SoundLoader, SoundEffectReverb and SoundAudioFrameCallback. Only what is used so far; member
// names are inferred, unknown members keep their offsets.

// RTTI "SoundLoader" (dol/sound/SoundLoader.cpp): loads a sound archive group, at once or as a task of
// the sound task thread.
class SoundLoader : public nw4r::snd::detail::Task {
public:
    enum State_e {
        STATE_IDLE = 0,
        STATE_QUEUED = 1,
        STATE_LOADING = 2,
    };

    SoundLoader();          // 801E2C3C
    virtual ~SoundLoader(); // 801E2C70

    virtual void Execute();  // 801E2DE4: loads the requested group
    virtual void Cancel();   // 801E2E3C: a queued request is put back by restart()
    virtual void OnCancel(); // 801E2E54

    void loadGroup(u32 groupId, nw4r::snd::SoundArchivePlayer *player,
                   nw4r::snd::SoundMemoryAllocatable *allocatable); // 801E2CC8: waits for a running load
    bool requestGroup(u32 groupId, nw4r::snd::SoundArchivePlayer *player,
                      nw4r::snd::SoundMemoryAllocatable *allocatable); // 801E2D28: false when busy
    void restart(); // 801E2D8C: queues a cancelled request again

    /* 0x10 */ nw4r::snd::SoundArchivePlayer *mPlayer;
    /* 0x14 */ nw4r::snd::SoundMemoryAllocatable *mAllocatable;
    /* 0x18 */ u32 mGroupId;
    /* 0x1C */ volatile bool mCancelled;
    /* 0x20 */ volatile int mState; // State_e
}; // size 0x24

class SoundManager {
public:
    SoundManager() {
        mStopped = 0;
        mDisabled = 0;
        mLpfFreq = 0.0f;
        mUpdateLpf = 0;
        for (int i = 0; i < 4; i++) {
            mFaders[i].mValue = 0.0f;
            mFaders[i].mTarget = 0.0f;
            mFaders[i].mStep = 0.0f;
            mFaders[i].mFrames = -1;
        }
        mReverbState = 0;
        mReverbFader.mValue = 0.0f;
        mReverbFader.mTarget = 0.0f;
        mReverbFader.mStep = 0.0f;
        mReverbFader.mFrames = -1;
        _778 = -1;
        _77C = 0;
        _780 = -1;
    }

    static SoundManager *getInstance() {
        static SoundManager instance;
        return &instance;
    }

    bool init(int soundPriority, int dvdPriority, void *heapBuf, u32 heapSize); // 801CE58C
    void calc();                       // 801CE978
    void stopAll(int frames);          // 801CEB70
    void resume();                     // 801CEC90
    void shutdown();                   // 801CED38
    void beginSceneChange();           // 801CED88
    bool setupStage(int stage);        // 801CEE24: polls asynchronous group loading
    void setMasterVolume(f32 volume, int frames); // 801CF6A4: milliseconds
    void mute();                       // 801CFF40
    void unmute();                     // 801CFF4C
    void setObjectVolume(f32 volume, int frames); // 801CF7A4
    void setPlayerVolume(f32 volume);  // 801CF7AC: melody and music players
    void clearMusicHeap();             // 801CFA4C: restores saved heap level 0
    u32 getSoundPlayerId(u32 id);       // 801CFB98
    bool isSoundReady(u32 id);          // 801CFBDC
    bool isBankReady(u32 id);           // 801CFC40
    bool isSeqReady(u32 id);            // 801CFCA4
    void loadGroup(u32 groupId);        // 801CFD28: synchronous, permanent heap
    bool requestMusicGroup(u32 groupId); // 801CFD38
    void destroyStageHeaps();          // 801CFEDC
    bool requestStageGroup(u32 groupId); // 801CFF74: includes common town groups

    bool isPlayable() const { return !mStopped && !mDisabled && mPlayer.IsAvailable(); }

    // The function the sound thread calls each frame (one at a time).
    void setFrameCallback(SoundAudioFrameCallback::Callback callback); // 801CFA68
    void clearFrameCallback();                                         // 801CFA70

    bool startSound(nw4r::snd::SoundHandle *handle, u32 id); // 801CF820: when the archive is available
    bool holdSound(nw4r::snd::SoundHandle *handle, u32 id);  // 801CF8D8
    bool startSound(nw4r::snd::SoundHandle *handle, u32 id,
                    const nw4r::snd::SoundStartable::StartInfo *startInfo); // 801CF990
    bool isGroupReady(u32 groupId); // 801CFB88: SoundArchivePlayer::LoadGroup without an allocator
    bool requestGroup(u32 groupId); // 801CFDE8: true when ready, else queues the load on mLoader
    int getSoundPriority(u32 id); // 801CFE98: the archive's player priority of the sound (0: unknown)
    bool isFixedVolume(u32 id);   // 801D0070: user param bit 0, or bit 2 in scenes 0x25/0x27..0x2B

    void setVolume(f32 volume, int frames); // 801CF714: clamps volume to [0, 1], fades the fader at 0x6B8
    void stopReverb();  // 801CFA78: fades out _764, shuts down the AUX A effect (mReverb)
    void startReverb(); // 801CFAD8: fades in _764, appends mReverb to AUX A

    /* 0x000 */ nw4r::snd::SoundHeap mHeap;
    /* 0x02C */ nw4r::snd::DvdSoundArchive mArchive;
    /* 0x1B8 */ nw4r::snd::SoundArchivePlayer mPlayer;
    /* 0x2A0 */ nw4r::snd::SoundHeap mHeap2A0;
    /* 0x2CC */ nw4r::snd::SoundHeap mHeap2CC;
    /* 0x2F8 */ nw4r::snd::SoundHeap mHeap2F8;
    /* 0x324 */ nw4r::snd::SoundHeap mHeap324;
    /* 0x350 */ void *mHeapBuffer;
    /* 0x354 */ u32 mHeapSize;
    /* 0x358 */ u8 *mStageHeapBuffer;
    /* 0x35C */ SoundLoader mLoader;
    /* 0x380 */ SoundEffectReverb mReverb;
    /* 0x68C */ SoundAudioFrameCallback mFrameCallback;
    /* 0x69C */ u8 mStopped;
    /* 0x69D */ u8 mDisabled;
    /* 0x6A0 */ f32 mLpfFreq;
    /* 0x6A4 */ u8 mUpdateLpf;
    /* 0x6A8 */ SoundFader mFaders[4]; // general, player, object, scene volumes
    /* 0x6E8 */ f32 mPlayerVolumes[30];
    /* 0x760 */ int mReverbState;
    /* 0x764 */ SoundFader mReverbFader;
    /* 0x774 */ u8 _774[4];
    /* 0x778 */ int _778;
    /* 0x77C */ u32 _77C;
    /* 0x780 */ int _780;
}; // size 0x784
