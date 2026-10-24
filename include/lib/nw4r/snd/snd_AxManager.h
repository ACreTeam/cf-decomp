#ifndef NW4R_SND_AX_MANAGER_H
#define NW4R_SND_AX_MANAGER_H

#include <types.h>

#include <nw4r/snd/snd_AxVoice.h>
#include <nw4r/snd/snd_global.h>

#include <nw4r/snd/snd_FxBase.h>
#include <nw4r/snd/snd_MoveValue.h>

#include <nw4r/ut/ut_LinkList.h>

#include <revolution/ai.h>
#include <revolution/ax.h>
#include <revolution/ax.h>
#include <revolution/ax.h>
#include <revolution/ax.h>

#include <nw4r/NW4RAssert.hpp>

namespace nw4r {
namespace snd {
namespace detail {
class BiquadFilterLpf;
}
} // namespace snd
} // namespace nw4r
namespace nw4r {
namespace snd {
namespace detail {
class BiquadFilterHpf;
}
} // namespace snd
} // namespace nw4r
namespace nw4r {
namespace snd {
namespace detail {
class BiquadFilterBpf512;
}
} // namespace snd
} // namespace nw4r
namespace nw4r {
namespace snd {
namespace detail {
class BiquadFilterBpf1024;
}
} // namespace snd
} // namespace nw4r
namespace nw4r {
namespace snd {
namespace detail {
class BiquadFilterBpf2048;
}
} // namespace snd
} // namespace nw4r
namespace nw4r {
namespace snd {
class BiquadFilterCallback;
}
} // namespace nw4r

namespace nw4r {
namespace snd {
namespace detail {
class AxManager {
  public:
    struct CallbackListNode {
      public:
        typedef ut::LinkList<CallbackListNode, 0x00> LinkList;

      public:
        ut::LinkListNode node;  // size 0x08, offset 0x00
        AXOutCallback callback; // size 0x04, offset 0x08
    }; // size 0x0c

  public:
    static AxManager &GetInstance();

    void Init();
    void Update();
    void Shutdown();

    OutputMode GetOutputMode();
    void const *GetZeroBufferAddress();
    bool CheckInit() {
        return mInitialized;
    }
    f32 GetOutputVolume() const;
    FxBase::LinkList &GetEffectList(AuxBus bus) {
        NW4RAssertHeaderClampedLValue_Line(173, bus, AUX_A, AUX_BUS_NUM);

        return mFxList[bus];
    }
    BiquadFilterCallback const *GetBiquadFilterCallback(int index) {
        return sBiquadFilterCallbackTable[static_cast<u8>(index)];
    }

    void SetOutputMode(OutputMode mode);
    void SetMainOutVolume(f32 volume, int frames);
    void SetBiquadFilterCallback(int type,
                                 BiquadFilterCallback const *callback);

    void RegisterCallback(CallbackListNode *node, AXOutCallback callback);
    void UnregisterCallback(CallbackListNode *node);

    bool AppendEffect(AuxBus bus, FxBase *pFx);
    void ClearEffect(AuxBus bus, int frame);
    void ShutdownEffect(AuxBus bus);

    f32 GetMasterVolume() const {
        return mMasterVolume.GetValue();
    }
    void SetMasterVolume(f32 volume, int frame);
    void PrepareReset();

    bool IsResetReady() const {
        return mResetReadyCounter == 0;
    }

    bool IsDiskError() const {
        return mDiskError;
    }

    AxVoice::SrcType GetSrcType() const {
        return AxVoice::SRC_4TAP_AUTO;
    }

  private:
    AxManager();

    static void AxCallbackFunc();
    static void AuxCallbackFunc(void *pChans, void *pContext);
    static void AiDmaCallbackFunc();

  private:
    static u8 const AUX_CALLBACK_WAIT_FRAME = 6;
    static u16 const AUX_RETURN_VOLUME_MAX = AX_MAX_VOLUME;
    static int const FX_SAMPLE_RATE = 32000;
    static SampleFormat const FX_SAMPLE_FORMAT = SAMPLE_FORMAT_PCM_S32;
    static int const FX_BUFFER_SIZE = 0x180;
    static int const ZERO_BUFFER_SIZE = 256;
    static int const SAMPLES_PAR_AUDIO_FRAME;
    static int const AUDIO_FRAME_INTERVAL;

    static byte_t sZeroBuffer[ZERO_BUFFER_SIZE];
    static BiquadFilterCallback const *sBiquadFilterCallbackTable[128];
    static BiquadFilterLpf sBiquadFilterLpf;
    static BiquadFilterHpf sBiquadFilterHpf;
    static BiquadFilterBpf512 sBiquadFilterBpf512;
    static BiquadFilterBpf1024 sBiquadFilterBpf1024;
    static BiquadFilterBpf2048 sBiquadFilterBpf2048;

  private:
    OutputMode mOutputMode;                   // size 0x04, offset 0x00
    void *mZeroBufferAddress;                 // size 0x04, offset 0x04
    CallbackListNode::LinkList mCallbackList; // size 0x0c, offset 0x08
    AXOutCallback mNextAxRegisterCallback;    // size 0x04, offset 0x14
    bool mInitialized;                        // size 0x01, offset 0x18
    bool mUpdateVoicePrioFlag;                // size 0x01, offset 0x19
    bool mDiskError;                          // size 0x01, offset 0x1a
    /* 2 bytes padding */
    MoveValue<f32, int> mMasterVolume;               // size 0x10, offset 0x1c
    MoveValue<f32, int> mMainOutVolume;              // size 0x10, offset 0x2c
    MoveValue<f32, int> mVolumeForReset;             // size 0x10, offset 0x3c
    AIDCallback mOldAidCallback;                     // size 0x04, offset 0x4c
    volatile s32 mResetReadyCounter;                 // size 0x04, offset 0x50
    MoveValue<f32, int> mAuxFadeVolume[AUX_BUS_NUM]; // size 0x30, offset 0x54
    MoveValue<f32, int> mAuxUserVolume[AUX_BUS_NUM]; // size 0x30, offset 0x84
    FxBase::LinkList mFxList[AUX_BUS_NUM];           // size 0x24, offset 0xb4
    AXAuxCallback mAuxCallback[AUX_BUS_NUM];         // size 0x0c, offset 0xd8
    void *mAuxCallbackContext[AUX_BUS_NUM];          // size 0x0c, offset 0xe4
    u8 mAuxCallbackWaitCounter[AUX_BUS_NUM];         // size 0x03, offset 0xf0
    /* 1 byte padding */
    u32 mEffectProcessTick[AUX_BUS_NUM]; // size 0x0c, offset 0xf4
}; // size 0x100
} // namespace detail
} // namespace snd
} // namespace nw4r

#endif
