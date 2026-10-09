#ifndef NW4R_SND_FX_REVERB_HI_DPL2_H
#define NW4R_SND_FX_REVERB_HI_DPL2_H
#include "nw4r/snd/snd_AxfxImpl.h"
#include "nw4r/snd/snd_FxBase.h"
#include "nw4r/types_nw4r.h"
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
        // City Folk's version has five more words (ctor 80262D10 sets 5, f, 0, f, f).
        int field_0x18;   // at 0x18
        f32 field_0x1C;   // at 0x1C
        int field_0x20;   // at 0x20
        f32 field_0x24;   // at 0x24
        f32 field_0x28;   // at 0x28
    }; // size 0x2C

public:
    FxReverbHiDpl2();

    virtual ~FxReverbHiDpl2() {
        Shutdown();
        ReleaseWorkBuffer();
    } // at 0x8

    virtual bool StartUp();  // at 0xC
    virtual void Shutdown(); // at 0x10

    virtual void UpdateBuffer(
        int channels, void **ppBuffer, ulong size, SampleFormat format, f32 sampleRate,
        OutputMode mode
    ); // at 0x14

    virtual bool AssignWorkBuffer(void *pBuffer, ulong size); // at 0x18
    virtual void ReleaseWorkBuffer();                       // at 0x1C

    ulong GetRequiredMemSize();
    bool SetParam(const ReverbHiDpl2Param &rParam);

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
