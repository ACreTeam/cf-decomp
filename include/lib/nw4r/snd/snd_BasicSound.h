#ifndef NW4R_SND_BASIC_SOUND_H
#define NW4R_SND_BASIC_SOUND_H

#include <types.h>

#include <nw4r/snd/snd_global.h>
#include <nw4r/snd/snd_MoveValue.h>

#include <nw4r/ut/ut_algorithm.h>
#include <nw4r/ut/ut_LinkList.h>

namespace nw4r {
namespace snd {
namespace detail {
class BasicPlayer;
}
} // namespace snd
} // namespace nw4r
namespace nw4r {
namespace snd {
namespace detail {
class ExternalSoundPlayer;
}
} // namespace snd
} // namespace nw4r
namespace nw4r {
namespace snd {
namespace detail {
class PlayerHeap;
}
} // namespace snd
} // namespace nw4r
namespace nw4r {
namespace snd {
class SoundActor;
}
} // namespace nw4r
namespace nw4r {
namespace snd {
class SoundHandle;
}
} // namespace nw4r
namespace nw4r {
namespace snd {
class SoundPlayer;
}
} // namespace nw4r

namespace nw4r {
namespace ut {
namespace detail {
struct RuntimeTypeInfo;
}
} // namespace ut
} // namespace nw4r

namespace nw4r {
namespace snd {
struct SoundParam {
  public:
    SoundParam()
        : volume(1.0f), pitch(1.0f), pan(0.0f), surroundPan(0.0f), fxSend(0.0f),
          lpf(0.0f), biquadFilterValue(0.0f), biquadFilterType(0), priority(0) {
    }

  public:
    f32 volume;            // size 0x04, offset 0x00
    f32 pitch;             // size 0x04, offset 0x04
    f32 pan;               // size 0x04, offset 0x08
    f32 surroundPan;       // size 0x04, offset 0x0c
    f32 fxSend;            // size 0x04, offset 0x10
    f32 lpf;               // size 0x04, offset 0x14
    f32 biquadFilterValue; // size 0x04, offset 0x18
    int biquadFilterType;  // size 0x04, offset 0x1c
    int priority;          // size 0x04, offset 0x20
}; // size 0x24

struct SoundAmbientParam {
  public:
    SoundAmbientParam()
        : volume(1.0f), pitch(1.0f), pan(0.0f), surroundPan(0.0f), fxSend(0.0f),
          lpf(0.0f), biquadFilterValue(0.0f), biquadFilterType(0), priority(0) {
    }

  public:
    f32 volume;                     // size 0x04, offset 0x00
    f32 pitch;                      // size 0x04, offset 0x04
    f32 pan;                        // size 0x04, offset 0x08
    f32 surroundPan;                // size 0x04, offset 0x0c
    f32 fxSend;                     // size 0x04, offset 0x10
    f32 lpf;                        // size 0x04, offset 0x14
    f32 biquadFilterValue;          // size 0x04, offset 0x18
    int biquadFilterType;           // size 0x04, offset 0x1c
    int priority;                   // size 0x04, offset 0x20
    VoiceOutParam voiceOutParam[4]; // size 0x60, offset 0x24
}; // size 0x84

namespace detail {
struct SoundActorParam {
  public:
    SoundActorParam() : volume(1.0f), pitch(1.0f), pan(0.0f) {
    }

  public:
    f32 volume; // size 0x04, offset 0x00
    f32 pitch;  // size 0x04, offset 0x04
    f32 pan;    // size 0x04, offset 0x08
}; // size 0x0c
} // namespace detail
} // namespace snd
} // namespace nw4r

namespace nw4r {
namespace snd {
namespace detail {
class BasicSound {
  public:
    enum PauseState {
        PAUSE_STATE_NORMAL,
        PAUSE_STATE_PAUSING,
        PAUSE_STATE_PAUSED,
        PAUSE_STATE_UNPAUSING,
    };

  public:
    typedef ut::LinkList<BasicSound, 0xec> PriorityLinkList;
    typedef ut::LinkList<BasicSound, 0xf4> SoundPlayerPlayLinkList;
    typedef ut::LinkList<BasicSound, 0xfc> SoundPlayerPriorityLinkList;
    typedef ut::LinkList<BasicSound, 0x104> ExtSoundPlayerPlayLinkList;

  public:
    struct AmbientInfo {
      public:
        class AmbientParamUpdateCallback {
          public:
            enum ParamUpdateFlags {
                PARAM_UPDATE_VOLUME = (1 << 0),
                PARAM_UPDATE_PAN = (1 << 1),
                PARAM_UPDATE_SURROUND_PAN = (1 << 2),
                PARAM_UPDATE_PRIORITY = (1 << 3),
            };

          public:
            virtual ~AmbientParamUpdateCallback() {
            }
            virtual void detail_UpdateAmbientParam(const void *, u32, int,
                                                   SoundAmbientParam *);
            virtual int detail_GetAmbientPriority(const void *, u32);
            virtual int detail_GetRequiredVoiceOutCount(const void *, u32);

          private:
            /* vtable */ // size 0x04, offset 0x00
        }; // size 0x04

        class AmbientArgUpdateCallback {
          public:
            virtual ~AmbientArgUpdateCallback() {
            }
            virtual void at_0x0c(void *, BasicSound *);

          private:
            /* vtable */ // size 0x04, offset 0x00
        }; // size 0x04

        class AmbientArgAllocaterCallback {
          public:
            virtual ~AmbientArgAllocaterCallback() {
            }
            virtual void *detail_AllocAmbientArg(u32 size); // at 0x8

            virtual void
            detail_FreeAmbientArg(void *pArg,
                                  const detail::BasicSound *pSound); // at 0xC

          private:
            /* vtable */ // size 0x04, offset 0x00
        }; // size 0x04

      public:
        AmbientParamUpdateCallback
            *paramUpdateCallback;                    // size 0x04, offset 0x00
        AmbientArgUpdateCallback *argUpdateCallback; // size 0x04, offset 0x04
        AmbientArgAllocaterCallback
            *argAllocaterCallback; // size 0x04, offset 0x08
        void *arg;                 // size 0x04, offset 0x0c
        u32 argSize;               // size 0x04, offset 0x10
    }; // size 0x14

  public:
    BasicSound(int priority, int ambientPriority);

    virtual ut::detail::RuntimeTypeInfo const *GetRuntimeTypeInfo() const {
        return &typeInfo;
    }
    virtual ~BasicSound() {
    }
    virtual void Shutdown();
    virtual bool IsPrepared() const = 0;
    virtual bool IsAttachedTempSpecialHandle() = 0;
    virtual void DetachTempSpecialHandle() = 0;
    virtual void InitParam();
    virtual BasicPlayer &GetBasicPlayer() = 0;
    virtual BasicPlayer const &GetBasicPlayer() const = 0;
    virtual void OnUpdatePlayerPriority();
    virtual void UpdateMoveValue();
    virtual void UpdateParam();

    void StartPrepared();
    void Update();
    void Pause(bool flag, int fadeFrames);
    bool IsPause() const;
    void Stop(int fadeFrames);
    void SetAutoStopCounter(int count);
    void FadeIn(int fadeFrames);

    bool GetStartedFlag() const {
        return mStartedFlag;
    }
    u32 GetId() const {
        return mId;
    }
    PlayerHeap *GetPlayerHeap() {
        return mPlayerHeap;
    }
    SoundPlayer *GetSoundPlayer() {
        return mSoundPlayer;
    }
    int GetVoiceOutCount() const;
    int GetPriority() const {
        return mPriority;
    }

    void SetPlayerPriority(int priority);
    void SetInitialVolume(f32 volume);
    void SetVolume(f32 volume, int frames);
    void SetPitch(f32 pitch);
    void SetPan(f32 pan);
    void SetSurroundPan(f32 pan);
    void SetLpfFreq(f32 freq);
    void SetBiquadFilter(int type, f32 value);
    void SetFxSend(AuxBus bus, f32 send);
    void SetRemoteFilter(int filter);
    void SetPanMode(PanMode mode);
    void SetPanCurve(PanCurve curve);
    void SetAmbientInfo(AmbientInfo const &ambientArgInfo);
    void SetId(u32 id);
    void SetOutputLineFlag(int flag);

    void AttachPlayerHeap(PlayerHeap *heap);
    void AttachSoundPlayer(SoundPlayer *player);
    void AttachSoundActor(SoundActor *actor);
    void AttachExternalSoundPlayer(ExternalSoundPlayer *extPlayer);

    bool IsAttachedGeneralHandle();
    bool IsAttachedTempGeneralHandle();

    void DetachPlayerHeap(PlayerHeap *heap);
    void DetachSoundPlayer(SoundPlayer *player);
    void DetachSoundActor(SoundActor *actor);
    void DetachExternalSoundPlayer(ExternalSoundPlayer *extPlayer);
    void DetachGeneralHandle();
    void DetachTempGeneralHandle();

    int GetRemainingFadeFrames() const;

    int CalcCurrentPlayerPriority() const {
        return ut::Clamp(mPriority + mAmbientParam.priority, PRIORITY_MIN,
                         PRIORITY_MAX);
    }

    void ClearAmbientArgUpdateCallback() {
        mAmbientInfo.argUpdateCallback = NULL;
    }

    static int GetAmbientPriority(AmbientInfo const &ambientInfo, u32 soundId);

  public:
    static u32 const INVALID_ID = -1;
    static int const PRIORITY_MAX = 127;
    static int const PRIORITY_MIN = 0;

    static ut::detail::RuntimeTypeInfo const typeInfo;

  private:
    /* vtable */                          // size 0x04, offset 0x00
    PlayerHeap *mPlayerHeap;              // size 0x04, offset 0x04
    SoundHandle *mGeneralHandle;          // size 0x04, offset 0x08
    SoundHandle *mTempGeneralHandle;      // size 0x04, offset 0x0c
    SoundPlayer *mSoundPlayer;            // size 0x04, offset 0x10
    SoundActor *mSoundActor;              // size 0x04, offset 0x14
    ExternalSoundPlayer *mExtSoundPlayer; // size 0x04, offset 0x18
    AmbientInfo mAmbientInfo;             // size 0x14, offset 0x1c
    SoundParam mAmbientParam;             // size 0x24, offset 0x30
    SoundActorParam mActorParam;          // size 0x0c, offset 0x54
    MoveValue<f32, int> mFadeVolume;      // size 0x10, offset 0x60
    MoveValue<f32, int> mPauseFadeVolume; // size 0x10, offset 0x70
    bool mStartFlag;                      // size 0x01, offset 0x80
    bool mStartedFlag;                    // size 0x01, offset 0x81
    bool mAutoStopFlag;                   // size 0x01, offset 0x82
    bool mFadeOutFlag;                    // size 0x01, offset 0x83
    PauseState mPauseState;               // size 0x04, offset 0x84
    bool mUnPauseFlag;                    // size 0x01, offset 0x88
    /* 3 bytes padding */
    s32 mAutoStopCounter; // size 0x04, offset 0x8c
    u32 mUpdateCounter;   // size 0x04, offset 0x90
    u8 mPriority;         // size 0x01, offset 0x94
    u8 mVoiceOutCount;    // size 0x01, offset 0x95
    u8 mBiquadFilterType; // size 0x01, offset 0x96
    /* 1 byte padding */
    u32 mId;                            // size 0x04, offset 0x98
    MoveValue<f32, int> mExtMoveVolume; // size 0x10, offset 0x9c
    f32 mInitVolume;                    // size 0x04, offset 0xac
    f32 mExtPan;                        // size 0x04, offset 0xb0
    f32 mExtSurroundPan;                // size 0x04, offset 0xb4
    f32 mExtPitch;                      // size 0x04, offset 0xb8
    f32 mLpfFreq;                       // size 0x04, offset 0xbc
    f32 mBiquadFilterValue;             // size 0x04, offset 0xc0
    int mOutputLineFlag;                // size 0x04, offset 0xc4
    f32 mMainOutVolume;                 // size 0x04, offset 0xc8
    f32 mMainSend;                      // size 0x04, offset 0xcc
    f32 mFxSend[AUX_BUS_NUM];           // size 0x0c, offset 0xd0
    f32 mRemoteOutVolume[4];            // size 0x10, offset 0xdc
  public:
    ut::LinkListNode mPriorityLink;            // size 0x08, offset 0xec
    ut::LinkListNode mSoundPlayerPlayLink;     // size 0x08, offset 0xf4
    ut::LinkListNode mSoundPlayerPriorityLink; // size 0x08, offset 0xfc
    ut::LinkListNode mExtSoundPlayerPlayLink;  // size 0x08, offset 0x104

  private:
    friend class snd::SoundHandle;
}; // size 0x10C
} // namespace detail
} // namespace snd
} // namespace nw4r

#endif
