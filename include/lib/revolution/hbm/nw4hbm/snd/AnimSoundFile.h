#ifndef NW4HBM_SND_ANIM_SOUND_FILE_H
#define NW4HBM_SND_ANIM_SOUND_FILE_H

#include "revolution/hbm/nw4hbm/snd/Util.h"
#include "revolution/hbm/nw4hbm/ut/binaryFileFormat.h"

namespace nw4hbm
{
namespace snd
{
namespace detail
{

struct AnimEvent
{
    /* 0x00 */ u32 optionFlag;
    /* 0x04 */ u32 soundId;
    /* 0x08 */ Util::DataRef<char> soundNameRef;
    /* 0x10 */ u8 volume;
    /* 0x11 */ u8 reserved[3];
    /* 0x14 */ f32 pitch;
    /* 0x18 */ u32 reserved2[1];
    /* 0x1C */ u32 userParam;

    const char* GetSoundLabel() const { return Util::GetDataRefAddress0(soundNameRef, this); }
};

struct AnimEventFrameInfo
{
    /* 0x00 */ s32 startFrame;
    /* 0x04 */ s32 endFrame;
    /* 0x08 */ u8 frameFlag;
    /* 0x09 */ s8 loopOffset;
    /* 0x0A */ u8 loopInterval;
    /* 0x0B */ u8 reserved;
};

struct AnimEventRef
{
    /* 0x00 */ AnimEventFrameInfo frameInfo;
    /* 0x0C */ Util::DataRef<AnimEvent> ref;
};

struct AnimSoundFile
{
    struct EventTable
    {
        u32 eventCount;
        AnimEventRef eventRef[1];
    };

    struct DataBlock
    {
        ut::BinaryBlockHeader blockHeader;
        u32 frameSize;
        Util::DataRef<EventTable> eventTableOffset;
    };

    struct Header
    {
        ut::BinaryFileHeader fileHeader;
        u32 dataBlockOffset;
        u32 dataBlockSize;
    };
};

class AnimSoundFileReader
{
public:
    AnimSoundFileReader();

    const AnimEvent* GetEvent(const AnimEventRef* eventRef) const;

private:
    /* 0x00 */ const AnimSoundFile::Header* header;
    /* 0x04 */ const AnimSoundFile::DataBlock* dataBlock;
};

} // namespace detail
} // namespace snd
} // namespace nw4hbm

#endif
