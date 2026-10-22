#ifndef REVOLUTION_HBM_SOUND_SYNCTRL_H
#define REVOLUTION_HBM_SOUND_SYNCTRL_H

#include <revolution/hbm/sound/syn.h>

#ifdef __cplusplus
extern "C"
{
#endif

    extern f32 __HBMSYNn128[];

    void __HBMSYNSetController(HBMSYNSYNTH* synth, u8 midiChannel, u8 function, u8 value);
    BOOL __HBMSYNGetWavetableData(HBMSYNVOICE* voice);
    void __HBMSYNResetAllControllers(HBMSYNSYNTH* synth);
    void __HBMSYNRunInputBufferEvents(HBMSYNSYNTH* synth);

#ifdef __cplusplus
}
#endif

#endif
