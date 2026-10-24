#include <nw4r/snd/snd_BasicPlayer.h>

#include <types.h>

#include <nw4r/snd/snd_BasicSound.h>
#include <nw4r/snd/snd_global.h>

#include <nw4r/NW4RAssert.hpp>

namespace nw4r {
namespace snd {
namespace detail {
void PlayerParamSet::Init() {
    volume = 1.0f;
    pitch = 1.0f;
    pan = 0.0f;
    surroundPan = 0.0f;
    lpfFreq = 0.0f;
    biquadType = 0;
    biquadValue = 0.0f;
    remoteFilter = 0;
    outputLineFlag = OUTPUT_LINE_MAIN;
    mainOutVolume = 1.0f;
    mainSend = 0.0f;
    panMode = PAN_MODE_DUAL;
    panCurve = PAN_CURVE_SQRT;

    for (int i = 0; i < AUX_BUS_NUM; i++) {
        fxSend[i] = 0.0f;
    }

    for (int i = 0; i < 4; i++) {
        remoteOutVolume[i] = 1.0f;
        remoteSend[i] = 0.0f;
        remoteFxSend[i] = 0.0f;
    }

    for (int i = 0; i < (int)ARRAY_SIZE(voiceOutParam); i++) {
        VoiceOutParam *param = &voiceOutParam[i];

        param->volume = 1.0f;
        param->pitch = 1.0f;
        param->pan = 0.0f;
        param->surroundPan = 0.0f;
        param->fxSend = 0.0f;
        param->lpf = 0.0f;
    }
}

BasicPlayer::BasicPlayer() : mId(BasicSound::INVALID_ID) {
    InitParam();
}

void BasicPlayer::InitParam() {
    mPlayerParamSet.Init();
}

void BasicPlayer::SetFxSend(AuxBus bus, f32 send) {
    NW4RAssertHeaderClampedLValue_Line(81, bus, AUX_A, AUX_BUS_NUM);

    mPlayerParamSet.fxSend[bus] = send;
}

f32 BasicPlayer::GetFxSend(AuxBus bus) const {
    NW4RAssertHeaderClampedLValue_Line(87, bus, AUX_A, AUX_BUS_NUM);

    return mPlayerParamSet.fxSend[bus];
}

void BasicPlayer::SetBiquadFilter(int type, f32 value) {
    NW4RAssertHeaderClampedLRValue_Line(93, type, 0, 127);
    NW4RAssertHeaderClampedLRValue_Line(94, value, 0.0f, 1.0f);

    mPlayerParamSet.biquadType = type;
    mPlayerParamSet.biquadValue = value;
}

void BasicPlayer::SetRemoteFilter(int filter) {
    NW4RAssertHeaderClampedLRValue_Line(102, filter, 0, 127);

    mPlayerParamSet.remoteFilter = filter;
}

void BasicPlayer::SetRemoteOutVolume(int remote, f32 volume) {
    mPlayerParamSet.remoteOutVolume[remote] = volume;
}

f32 BasicPlayer::GetRemoteOutVolume(int remote) const {
    return mPlayerParamSet.remoteOutVolume[remote];
}
f32 BasicPlayer::GetRemoteSend(int remoteIndex) const {
    return mPlayerParamSet.remoteSend[remoteIndex];
}

f32 BasicPlayer::GetRemoteFxSend(int remoteIndex) const {
    return mPlayerParamSet.remoteFxSend[remoteIndex];
}
} // namespace detail
} // namespace snd
} // namespace nw4r
