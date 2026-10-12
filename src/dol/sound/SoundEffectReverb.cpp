// The reverb effect of the sound system. .text 801D00E8..801D0550, .data 80500648..80500700, .sdata
// 8074C600..8074C618, .sdata2 80751AF8..80751B58. See include/game/snd/snd_effect_reverb.hpp.
#include <game/snd/snd_effect_reverb.hpp>

// 801D00E8
bool SoundEffectReverb::StartUp() {
    bool result = false;
    if (!mStarted) {
        result = FxReverbHiDpl2::StartUp();
        if (result) {
            mStarted = true;
        }
    }
    return result;
}

// 801D013C
void SoundEffectReverb::Shutdown() {
    if (mStarted) {
        FxReverbHiDpl2::Shutdown();
        mStarted = false;
    }
}

// 801D017C
void SoundEffectReverb::setPreset(u32 preset) {
    ReverbHiDpl2Param param;
    switch (preset) {
    case 0:
        param.earlyMode = 5;
        param.preDelayTimeMax = 0.05f;
        param.preDelayTime = 0.03f;
        param.fusedMode = 0;
        param.fusedTime = 1.5f;
        param.coloration = 0.5f;
        param.damping = 0.5f;
        param.crossTalk = 0.1f;
        param.earlyGain = 0.0f;
        param.fusedGain = 0.5f;
        param.outGain = 1.0f;
        break;
    case 1:
        param.earlyMode = 7;
        param.preDelayTimeMax = 0.05f;
        param.preDelayTime = 0.05f;
        param.fusedMode = 0;
        param.fusedTime = 3.0f;
        param.coloration = 0.5f;
        param.damping = 0.7f;
        param.crossTalk = 0.1f;
        param.earlyGain = 0.4f;
        param.fusedGain = 0.5f;
        param.outGain = 1.0f;
        break;
    case 2:
        param.earlyMode = 0;
        param.preDelayTimeMax = 0.05f;
        param.preDelayTime = 0.0f;
        param.fusedMode = 1;
        param.fusedTime = 0.5f;
        param.coloration = 0.5f;
        param.damping = 0.9f;
        param.crossTalk = 0.1f;
        param.earlyGain = 0.2f;
        param.fusedGain = 0.3f;
        param.outGain = 1.0f;
        break;
    case 3:
        param.earlyMode = 6;
        param.preDelayTimeMax = 0.05f;
        param.preDelayTime = 0.03f;
        param.fusedMode = 1;
        param.fusedTime = 3.0f;
        param.coloration = 0.5f;
        param.damping = 0.8f;
        param.crossTalk = 0.1f;
        param.earlyGain = 0.25f;
        param.fusedGain = 0.3f;
        param.outGain = 1.0f;
        break;
    case 4:
        param.earlyMode = 4;
        param.preDelayTimeMax = 0.05f;
        param.preDelayTime = 0.01f;
        param.fusedMode = 2;
        param.fusedTime = 0.5f;
        param.coloration = 0.6f;
        param.damping = 0.4f;
        param.crossTalk = 0.1f;
        param.earlyGain = 0.3f;
        param.fusedGain = 0.3f;
        param.outGain = 1.0f;
        break;
    case 5:
        param.earlyMode = 5;
        param.preDelayTimeMax = 0.05f;
        param.preDelayTime = 0.02f;
        param.fusedMode = 2;
        param.fusedTime = 1.0f;
        param.coloration = 0.6f;
        param.damping = 0.4f;
        param.crossTalk = 0.1f;
        param.earlyGain = 0.3f;
        param.fusedGain = 0.3f;
        param.outGain = 1.0f;
        break;
    case 6:
        param.earlyMode = 7;
        param.preDelayTimeMax = 0.05f;
        param.preDelayTime = 0.03f;
        param.fusedMode = 3;
        param.fusedTime = 1.5f;
        param.coloration = 0.6f;
        param.damping = 0.4f;
        param.crossTalk = 0.1f;
        param.earlyGain = 0.25f;
        param.fusedGain = 0.35f;
        param.outGain = 1.0f;
        break;
    case 7:
        param.earlyMode = 3;
        param.preDelayTimeMax = 0.05f;
        param.preDelayTime = 0.02f;
        param.fusedMode = 3;
        param.fusedTime = 2.0f;
        param.coloration = 0.6f;
        param.damping = 0.4f;
        param.crossTalk = 0.1f;
        param.earlyGain = 0.3f;
        param.fusedGain = 0.45f;
        param.outGain = 1.0f;
        break;
    case 8:
        param.earlyMode = 4;
        param.preDelayTimeMax = 0.05f;
        param.preDelayTime = 0.01f;
        param.fusedMode = 4;
        param.fusedTime = 2.0f;
        param.coloration = 0.6f;
        param.damping = 0.4f;
        param.crossTalk = 0.1f;
        param.earlyGain = 0.35f;
        param.fusedGain = 0.45f;
        param.outGain = 1.0f;
        break;
    case 9:
        param.earlyMode = 6;
        param.preDelayTimeMax = 0.05f;
        param.preDelayTime = 0.02f;
        param.fusedMode = 4;
        param.fusedTime = 3.0f;
        param.coloration = 0.6f;
        param.damping = 0.3f;
        param.crossTalk = 0.1f;
        param.earlyGain = 0.3f;
        param.fusedGain = 0.45f;
        param.outGain = 1.0f;
        break;
    case 10:
        param.earlyMode = 7;
        param.preDelayTimeMax = 0.05f;
        param.preDelayTime = 0.05f;
        param.fusedMode = 5;
        param.fusedTime = 4.0f;
        param.coloration = 0.6f;
        param.damping = 0.7f;
        param.crossTalk = 0.1f;
        param.earlyGain = 0.5f;
        param.fusedGain = 0.5f;
        param.outGain = 1.0f;
        break;
    case 11:
        param.earlyMode = 7;
        param.preDelayTimeMax = 0.05f;
        param.preDelayTime = 0.05f;
        param.fusedMode = 5;
        param.fusedTime = 5.0f;
        param.coloration = 0.6f;
        param.damping = 0.3f;
        param.crossTalk = 0.1f;
        param.earlyGain = 0.35f;
        param.fusedGain = 0.5f;
        param.outGain = 1.0f;
        break;
    }
    SetParam(param);
}
