#include <revolution/hbm/sound/syn.h>
#include <revolution/hbm/sound/mix.h>
#include <revolution/hbm/sound/synctrl.h>
#include <revolution/hbm/sound/synvoice.h>
#include <revolution/os.h>
#include <revolution/ax.h>
#include <revolution/ax/AXAlloc.h>
#include <decomp.h>
#include <assert.h>

static void __HBMSYNAddSynthToList(HBMSYNSYNTH* synth);
static void __HBMSYNRemoveSynthFromList(HBMSYNSYNTH* synth);
static HBMSYNSYNTH* __HBMSYNSynthList;
HBMSYNVOICE* __HBMSYNVoice;
static HBMSYNVOICE __s_HBMSYNVoice[16];
static int __init;

static void __HBMSYNAddSynthToList(HBMSYNSYNTH* synth)
{
    BOOL intrStatus = OSDisableInterrupts();

    if (__HBMSYNSynthList)
        synth->next = __HBMSYNSynthList;
    else
        synth->next = NULL;

    __HBMSYNSynthList = synth;

    OSRestoreInterrupts(intrStatus);
}

static void __HBMSYNRemoveSynthFromList(HBMSYNSYNTH* synth)
{
    HBMSYNSYNTH *tempHead, *tempTail;
    HBMSYNSYNTH* tempSynth;

    BOOL intrStatus = OSDisableInterrupts();

    tempHead = NULL;
    tempTail = NULL;

    for (tempSynth = __HBMSYNSynthList; tempSynth; tempSynth = tempSynth->next)
    {
        if (tempSynth != synth)
        {
            if (tempHead)
            {
                tempTail->next = tempSynth;
                tempTail = tempSynth;
            }
            else
            {
                tempTail = tempSynth;
                tempHead = tempTail;
            }
        }
    }

    if (tempTail)
        tempTail->next = NULL;

    __HBMSYNSynthList = tempHead;

    OSRestoreInterrupts(intrStatus);
}

void HBMSYNInit(void)
{
    int i;

    assert(AXIsInit());

    if (AXIsInit() && !__init)
    {
        __HBMSYNVoice = __s_HBMSYNVoice;

        for (i = 0; i < (int)ARRAY_LENGTH(__s_HBMSYNVoice); i++)
            __HBMSYNVoice[i].synth = NULL;

        __HBMSYNSynthList = NULL;
        __init = TRUE;
    }
}

void HBMSYNQuit(void)
{
    __HBMSYNVoice = NULL;
    __init = FALSE;
}

void HBMSYNRunAudioFrame(void)
{
    int i;
    HBMSYNSYNTH* synth;

    if (!__init)
        return;

    for (i = 0; i < (int)ARRAY_LENGTH(__s_HBMSYNVoice); i++)
        __HBMSYNServiceVoice(i);

    for (synth = __HBMSYNSynthList; synth; synth = synth->next)
        __HBMSYNRunInputBufferEvents(synth);
}

void HBMSYNInitSynth(HBMSYNSYNTH* synth, u8* wavetable, u8* samples,
    u8* zerobuffer)
{
    u32* p;
    u32 mramBase;
    u32 midiChannel, noteNumber;

    assert(synth);
    assert(wavetable);
    assert(samples);

    p = (u32*)wavetable;
    mramBase = (u32)OSCachedToPhysical(samples);
    synth->percussiveInst = (WTINST*)(wavetable + *p);
    p++;
    synth->melodicInst = (WTINST*)(wavetable + *p);
    p++;
    synth->region = (WTREGION*)(wavetable + *p);
    p++;
    synth->art = (WTART*)(wavetable + *p);
    p++;
    synth->sample = (WTSAMPLE*)(wavetable + *p);
    p++;
    synth->adpcm = (WTADPCM*)(wavetable + *p);
    p++;

    synth->samplesBaseWord = mramBase >> 1;
    synth->samplesBaseByte = mramBase;
    synth->samplesBaseNibble = mramBase << 1;
    synth->masterVolume = 0;

    __HBMSYNResetAllControllers(synth);

    synth->inputPosition = *synth->input;
    synth->inputCounter = 0;
    synth->notes = 0;

    for (midiChannel = 0; midiChannel < NUM_MIDI_CHANNELS; midiChannel++)
    {
        for (noteNumber = 0; noteNumber < NUM_MIDI_NOTES; noteNumber++)
            synth->voice[midiChannel][noteNumber] = NULL;
    }

    __HBMSYNAddSynthToList(synth);
}

void HBMSYNQuitSynth(HBMSYNSYNTH* synth)
{
    int i;
    BOOL intrStatus = OSDisableInterrupts();

    if (synth->notes)
    {
        for (i = 0; i < (int)ARRAY_LENGTH(__s_HBMSYNVoice); i++)
        {
            HBMSYNVOICE* voice = __HBMSYNVoice + i;

            if (voice->synth == synth)
            {
                HBMMIXReleaseChannel(voice->axvpb);
                HBMFreeIndexByKey(voice->axvpb->index);
                AXFreeVoice(voice->axvpb);
                voice->synth = NULL;
            }
        }
    }

    __HBMSYNRemoveSynthFromList(synth);

    OSRestoreInterrupts(intrStatus);
}

void HBMSYNMidiInput(HBMSYNSYNTH* synth, u8* input)
{
    u8* src;

    assert(synth);
    assert(input);

    src = input;
    *synth->inputPosition = *src;
    synth->inputPosition++;
    src++;
    *synth->inputPosition = *src;
    synth->inputPosition++;
    src++;
    *synth->inputPosition = *src;
    synth->inputPosition++;
    src++;

    synth->inputCounter++;

    assert(synth->inputCounter < HBMSYN_INPUT_BUFFER_SIZE);
}

void HBMSYNSetMasterVolume(HBMSYNSYNTH* synth, s32 dB)
{
    assert(synth);

    synth->masterVolume = dB << 16;
}
