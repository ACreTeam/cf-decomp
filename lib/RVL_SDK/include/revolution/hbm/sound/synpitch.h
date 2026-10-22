#ifndef REVOLUTION_HBM_SOUND_SYNPITCH_H
#define REVOLUTION_HBM_SOUND_SYNPITCH_H

#include <revolution/hbm/sound/syn.h>

#ifdef __cplusplus
extern "C"
{
#endif

    f32 __HBMSYNGetRelativePitch(HBMSYNVOICE* voice);
    void __HBMSYNSetupPitch(HBMSYNVOICE* voice);
    void __HBMSYNSetupSrc(HBMSYNVOICE* voice);
    void __HBMSYNUpdateSrc(HBMSYNVOICE* voice);

#ifdef __cplusplus
}
#endif

#endif
