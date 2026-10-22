#ifndef REVOLUTION_HBM_SOUND_SYNMIX_H
#define REVOLUTION_HBM_SOUND_SYNMIX_H

#include <revolution/hbm/sound/syn.h>

#ifdef __cplusplus
extern "C"
{
#endif

    extern s32 __HBMSYNVolumeAttenuation[];
    extern s32 __HBMSYNAttackAttnTable[];
    void __HBMSYNSetupVolume(HBMSYNVOICE* voice);
    void __HBMSYNSetupPan(HBMSYNVOICE* voice);
    s32 __HBMSYNGetVoiceInput(HBMSYNVOICE* voice);
    s32 __HBMSYNGetVoiceFader(HBMSYNVOICE* voice);
    void __HBMSYNUpdateMix(HBMSYNVOICE* voice);

#ifdef __cplusplus
}
#endif

#endif
