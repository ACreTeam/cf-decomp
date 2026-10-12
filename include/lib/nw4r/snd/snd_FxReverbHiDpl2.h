#ifndef NW4R_SND_FX_REVERB_HI_DPL2_H
#define NW4R_SND_FX_REVERB_HI_DPL2_H
#include <nw4r/types_nw4r.h>

#include <nw4r/snd/snd_AxfxImpl.h>
#include <nw4r/snd/snd_FxBase.h>

#include <revolution/AXFX.h> // IWYU pragma: export

namespace nw4r {
namespace snd {

class FxReverbHiDpl2 : public FxBase {
public:
    struct ReverbHiDpl2Param {
        f32 preDelayTime; // at 0x0
        f32 fusedTime;    // at 0x4
        f32 coloration;   // at 0x8
        f32 damping;      // at 0xC
        f32 crossTalk;    // at 0x10
        f32 outGain;      // at 0x14
        // Newer nw4r (City Folk) has the early reflection and fused settings.
        int earlyMode;        // at 0x18
        f32 preDelayTimeMax;  // at 0x1C
        int fusedMode;        // at 0x20
        f32 earlyGain;        // at 0x24
        f32 fusedGain;        // at 0x28

        ReverbHiDpl2Param()
            : preDelayTime(0.02f),
              fusedTime(3.0f),
              coloration(0.6f),
              damping(0.4f),
              crossTalk(0.1f),
              outGain(1.0f),
              earlyMode(5),
              preDelayTimeMax(0.02f),
              fusedMode(0),
              earlyGain(0.0f),
              fusedGain(1.0f) {}

        ReverbHiDpl2Param(f32 preDelay, f32 fused) {
            coloration = 0.6f;
            damping = 0.4f;
            crossTalk = 0.1f;
            outGain = 1.0f;
            preDelayTime = preDelay;
            fusedTime = fused;
            earlyMode = 5;
            preDelayTimeMax = preDelay;
            fusedMode = 0;
            earlyGain = 0.0f;
            fusedGain = 1.0f;
        }

    }; // size 0x2C

public:
    FxReverbHiDpl2();

    virtual ~FxReverbHiDpl2() {
        Shutdown();
        ReleaseWorkBuffer();
    } // at 0x8

    virtual bool StartUp();  // at 0xC
    virtual void Shutdown(); // at 0x10

    virtual void UpdateBuffer(int channels, void** ppBuffer, u32 size,
                              SampleFormat format, f32 sampleRate,
                              OutputMode mode); // at 0x14

    virtual void OnChangeOutputMode(); // at 0x18

    virtual bool AssignWorkBuffer(void* pBuffer, u32 size); // at 0x1C
    virtual void ReleaseWorkBuffer();                       // at 0x20

    u32 GetRequiredMemSize();
    bool SetParam(const ReverbHiDpl2Param& rParam);

private:
    // City Folk's layout (ctor 80262D10, UpdateBuffer 8026333C; size 0x308).
    u8 mIsActive;                         // at 0xC
    int mOutputMode;                      // at 0x10 (1: DPL2)
    detail::AxfxImpl mImpl;               // at 0x14
    ReverbHiDpl2Param mParam;             // at 0x1C
    AXFX_REVERBHI_EXP mAxfxParam;         // at 0x48
    AXFX_REVERBHI_EXP_DPL2 mAxfxParamDpl; // at 0x190
};

} // namespace snd
} // namespace nw4r

#endif
