#ifndef NW4R_SND_WSD_PLAYER_H
#define NW4R_SND_WSD_PLAYER_H

#include <types.h>

#include <nw4r/snd/snd_BasicPlayer.h>
#include <nw4r/snd/snd_Channel.h>
#include <nw4r/snd/snd_DisposeCallbackManager.h>
#include <nw4r/snd/snd_Lfo.h>
#include <nw4r/snd/snd_SoundThread.h>
#include <nw4r/snd/snd_WaveFile.h>

namespace nw4r {
namespace snd {
struct WsdDataInfo {
    /* 0x00 */ bool loopFlag;
    /* 0x04 */ int sampleRate;
    /* 0x08 */ u32 loopStart;
    /* 0x0C */ u32 loopEnd;
};

namespace detail {
class WsdPlayer : public BasicPlayer,
                  public DisposeCallback,
                  public SoundThread::PlayerCallback {
  public:
    enum StartOffsetType {
        START_OFFSET_TYPE_SAMPLE,
        START_OFFSET_TYPE_MILLISEC,
    };

  public:
    class WsdCallback {
      public:
        virtual ~WsdCallback() {
        }

        virtual bool GetWaveSoundData(WaveSoundInfo *info,
                                      WaveSoundNoteInfo *noteInfo,
                                      WaveData *waveData,
                                      void const *waveSoundData, int index,
                                      int noteIndex, u32 userData) const = 0;

      private:
        /* vtable */ // size 0x04, offset 0x00
    }; // size 0x04

  public:
    WsdPlayer();

    virtual bool Start();
    virtual void Stop();
    virtual void Pause(bool flag);
    virtual bool IsActive() const {
        return mActiveFlag != false;
    }
    virtual bool IsStarted() const {
        return mStartedFlag != false;
    }
    virtual bool IsPause() const {
        return mPauseFlag != false;
    }

    virtual void InvalidateData(void const *start, void const *end);
    virtual void InvalidateWaveData(void const *, void const *) {
    }

    virtual void OnUpdateFrameSoundThread() {
        Update();
    }
    virtual void OnShutdownSoundThread() {
        Stop();
    }

    bool Prepare(void const *waveSoundBase, int index,
                 StartOffsetType startOffsetType, int startOffset,
                 int voiceOutCount, WsdCallback const *callback,
                 u32 callbackData);

    f32 GetPanRange() const {
        return mPanRange;
    }
    int GetVoiceOutCount() const {
        return mVoiceOutCount;
    }
    int GetChannelPriority() const {
        return mPriority;
    }
    void const *GetWsdDataAddress() const {
        return mWsdData;
    }

    void SetReleasePriorityFix(bool flag);
    void SetChannelPriority(int priority);

    bool IsChannelActive() const {
        return mChannel != nullptr && mChannel->IsActive();
    }
    bool ReadWsdDataInfo(WsdDataInfo *) const;
    u32 GetPlaySamplePosition() const;

  private:
    static void ChannelCallbackFunc(Channel *dropChannel,
                                    Channel::ChannelCallbackStatus status,
                                    u32 userData);

    void InitParam(int voiceOutCount, WsdCallback const *callback,
                   u32 callbackData);

    void Update();

    bool StartChannel(WsdCallback const *callback, u32 callbackData);
    void UpdateChannel();
    void CloseChannel();

    void FinishPlayer();

  public:
    static int const DEFAULT_PRIORITY = 64;
    static int const MUTE_RELEASE_VALUE;
    static int const PAUSE_RELEASE_VALUE;

  private:
    /* base BasicPlayer */                 // size 0x0a4, offset 0x000
    /* base DisposeCallback */             // size 0x00c, offset 0x0a4
    /* base SoundThread::PlayerCallback */ // size 0x00c, offset 0x0b0
    bool mActiveFlag;                      // size 0x001, offset 0x0bc
    bool mStartedFlag;                     // size 0x001, offset 0x0bd
    bool mPauseFlag;                       // size 0x001, offset 0x0be
    bool mWavePlayFlag;                    // size 0x001, offset 0x0bf
    bool mReleasePriorityFixFlag;          // size 0x001, offset 0x0c0
    /* 3 bytes padding */
    f32 mPanRange;      // size 0x004, offset 0x0c4
    int mVoiceOutCount; // size 0x004, offset 0x0c8
    u8 mPriority;       // size 0x001, offset 0x0cc
    /* 3 bytes padding */
    WsdCallback const *mCallback;     // size 0x004, offset 0x0d0
    u32 mCallbackData;                // size 0x004, offset 0x0d4
    void const *mWsdData;             // size 0x004, offset 0x0d8
    int mWsdIndex;                    // size 0x004, offset 0x0dc
    StartOffsetType mStartOffsetType; // size 0x004, offset 0x0e0
    int mStartOffset;                 // size 0x004, offset 0x0e4
    LfoParam mLfoParam;               // size 0x010, offset 0x0e8
    WaveSoundInfo mWaveSoundInfo;     // size 0x00c, offset 0x0f8
    Channel *mChannel;                // size 0x004, offset 0x104
}; // size 0x108
} // namespace detail
} // namespace snd
} // namespace nw4r

#endif
