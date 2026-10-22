#ifndef NW4HBM_SND_ANIM_SOUND_H
#define NW4HBM_SND_ANIM_SOUND_H

#include "revolution/hbm/nw4hbm/snd/AnimSoundFile.h"
#include "revolution/hbm/nw4hbm/snd/SoundHandle.h"

namespace nw4hbm
{
namespace snd
{
class SoundStartable;

namespace detail
{

class AnimEventPlayer
{
public:
    AnimEventPlayer();
    ~AnimEventPlayer();

    void ForceStop();

    void StopEvent(const AnimEvent* event)
    {
        if (this->event == event)
        {
            ForceStop();
        }
    }

    /* 0x00 */ SoundHandle handle;
    /* 0x04 */ const AnimEvent* event;
    /* 0x08 */ bool shutdownStopFlag;
};

class AnimSoundImpl
{
public:
    enum PlayDirection
    {
        PLAY_FORWARD,
        PLAY_BACKWARD
    };

    enum EventType
    {
        EVENT_TRIGGER_START,
        EVENT_TRIGGER_STOP,
        EVENT_RANGE_START,
        EVENT_RANGE_STOP
    };

    typedef void (*EventCallback)(EventType type, s32 frame, const char* soundName, u32 userParam, void* arg);

    AnimSoundImpl(SoundStartable& starter, AnimEventPlayer* eventPlayer, int numEventPlayers);
    ~AnimSoundImpl();

    void UpdateTrigger(const AnimEventRef* eventRef, s32 current, PlayDirection direction);
    void StartEvent(const AnimEvent* event, bool shutdownStopFlag);
    void StopEvent(const AnimEvent* event);

private:
    /* 0x00 */ SoundStartable& starter;
    /* 0x04 */ AnimSoundFileReader fileReader;
    /* 0x0C */ f32 curFrame;
    /* 0x10 */ AnimEventPlayer* eventPlayer;
    /* 0x14 */ int numEventPlayers;
    /* 0x18 */ u8 activeFlag;
    /* 0x19 */ u8 initFrameFlag;
    /* 0x1A */ u8 resetFlag;
    /* 0x1C */ s32 loopCounter;
    /* 0x20 */ EventCallback eventCallback;
    /* 0x24 */ void* eventCallbackArg;
};

} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif
