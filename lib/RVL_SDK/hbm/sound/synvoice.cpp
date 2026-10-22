#include <revolution/hbm/sound/synvoice.h>
#include <revolution/hbm/sound/mix.h>
#include <revolution/hbm/sound/synenv.h>
#include <revolution/hbm/sound/synmix.h>
#include <revolution/hbm/sound/synpitch.h>
#include <revolution/ax/AXAlloc.h>
#include <assert.h>

void __HBMSYNClearVoiceReferences(void* p)
{
    AXVPB* axvpb;
    HBMSYNSYNTH* synth;
    HBMSYNVOICE* voice;
    int index;

    assert(p);

    axvpb = (AXVPB*)p;
    synth = (HBMSYNSYNTH*)axvpb->userContext;

    index = HBMGetIndex(axvpb->index);
    HBMFreeIndex(index);

    voice = __HBMSYNVoice + index;
    HBMMIXReleaseChannel(axvpb);

    if (synth->voice[voice->midiChannel][voice->keyNum] == voice)
        synth->voice[voice->midiChannel][voice->keyNum] = NULL;

    voice->synth = NULL;
    synth->notes--;
}

void __HBMSYNSetVoiceToRelease(HBMSYNVOICE* voice)
{
    assert(voice);

    voice->veState = VolEnvState_Release;
}

void __HBMSYNServiceVoice(int i)
{
    HBMSYNVOICE* voice = __HBMSYNVoice + i;
    HBMSYNSYNTH* synth = voice->synth;

    if (!synth)
        return;

    if (voice->type == 0 && voice->axvpb->pb.state == 0)
    {
        if (synth->voice[voice->midiChannel][voice->keyNum] == voice)
            synth->voice[voice->midiChannel][voice->keyNum] = NULL;

        voice->veState = VolEnvState_End;
    }

    __HBMSYNRunVolumeEnvelope(voice);

    if (voice->veState == VolEnvState_End)
    {
        voice->synth = NULL;

        HBMMIXReleaseChannel(voice->axvpb);
        HBMFreeIndexByKey(voice->axvpb->index);
        AXFreeVoice(voice->axvpb);

        synth->notes--;
    }
    else
    {
        __HBMSYNUpdateMix(voice);
        __HBMSYNUpdateSrc(voice);
    }
}
