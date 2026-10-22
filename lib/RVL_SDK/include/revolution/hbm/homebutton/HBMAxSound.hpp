#ifndef HOMEBUTTON_AX_SOUND_HPP
#define HOMEBUTTON_AX_SOUND_HPP

#include "revolution/hbm/sound/mix.h"

namespace homebutton
{
void PlaySeq(int id);
void InitAxSound(const void* soundData, void* work, u32 workSize);
void ShutdownAxSound();
void AxSoundMain();
void StopAllSeq();
void SetVolumeAllSeq(f32 volume);
void SetSoundMode(HBMMIXSoundMode mode);
} // namespace homebutton

#endif
