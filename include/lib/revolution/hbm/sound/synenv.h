#ifndef REVOLUTION_HBM_SOUND_SYNENV_H
#define REVOLUTION_HBM_SOUND_SYNENV_H

#include <revolution/hbm/sound/syn.h>

enum VolumeEnvelopeStages
{
    VolEnvState_Attack,
    VolEnvState_Decay,
    VolEnvState_Sustain,
    VolEnvState_Release,
    VolEnvState_End
};

#ifdef __cplusplus
extern "C"
{
#endif

    s32 __HBMSYNGetEnvelopeTime(s32 scale, s32 mod, u8 key);
    void __HBMSYNRunVolumeEnvelope(HBMSYNVOICE* voice);
    void __HBMSYNSetupVolumeEnvelope(HBMSYNVOICE* voice);

#ifdef __cplusplus
}
#endif

#endif
