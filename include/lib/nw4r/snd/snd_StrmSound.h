#ifndef NW4R_SND_STRM_SOUND_H
#define NW4R_SND_STRM_SOUND_H

#include <types.h>

#include <nw4r/snd/snd_BasicSound.h>
#include <nw4r/snd/snd_debug.h>
#include <nw4r/snd/snd_MoveValue.h>
#include <nw4r/snd/snd_StrmPlayer.h>

#include <nw4r/ut/ut_LinkList.h>

namespace nw4r {
namespace snd {
namespace detail {
template <class> class SoundInstanceManager;
}
} // namespace snd
} // namespace nw4r
namespace nw4r {
namespace snd {
namespace detail {
class StrmBufferPool;
}
} // namespace snd
} // namespace nw4r
namespace nw4r {
namespace snd {
class StrmSoundHandle;
}
} // namespace nw4r

namespace nw4r {
namespace ut {
namespace detail {
class RuntimeTypeInfo;
}
} // namespace ut
} // namespace nw4r
namespace nw4r {
namespace ut {
class FileStream;
}
} // namespace nw4r

namespace nw4r {
namespace snd {
namespace detail {
class StrmSound : public BasicSound {
  public:
    typedef ut::LinkList<StrmSound, 0xec> PriorityLinkList;

  public:
    StrmSound(SoundInstanceManager<StrmSound> *manager, int priority,
              int ambientPriority);

    virtual ut::detail::RuntimeTypeInfo const *GetRuntimeTypeInfo() const {
        return &typeInfo;
    }
    virtual ~StrmSound() {
    }
    virtual void Shutdown();
    virtual bool IsPrepared() const {
        return mStrmPlayer.IsPrepared();
    }
    virtual bool IsAttachedTempSpecialHandle();
    virtual void DetachTempSpecialHandle();
    virtual void InitParam();
    virtual BasicPlayer &GetBasicPlayer() {
        return mStrmPlayer;
    }
    virtual BasicPlayer const &GetBasicPlayer() const {
        return mStrmPlayer;
    }
    virtual void OnUpdatePlayerPriority();
    virtual void UpdateMoveValue();
    virtual void UpdateParam();

    StrmPlayer::SetupResult Setup(StrmBufferPool *bufferPool,
                                  int allocChannelCount, u16 allocTrackFlag);

    void *GetFileStreamBuffer() {
        return mFileStreamBuffer;
    }
    s32 GetFileStreamBufferSize() {
        return sizeof mFileStreamBuffer;
    }

    bool Prepare(StrmPlayer::StartOffsetType startOffsetType, s32 offset,
                 ut::FileStream *fileStream);

    void SetTrackVolume(u32 trackFlags, f32 volume, int fadeFrames);

    static DebugSoundType GetSoundType() {
        return DEBUG_SOUND_TYPE_STRMSOUND;
    }

    u32 GetPlaySamplePosition() const {
        return mStrmPlayer.GetPlaySamplePosition();
    }

  public:
    static int const FILE_STREAM_BUFFER_SIZE = 128;

    static ut::detail::RuntimeTypeInfo const typeInfo;

  private:
    /* base BasicSound */                           // size 0x100, offset 0x000
    StrmPlayer mStrmPlayer;                         // size 0xd08, offset 0x100
    StrmSoundHandle *mTempSpecialHandle;            // size 0x004, offset 0xe08
    SoundInstanceManager<StrmSound> *mManager;      // size 0x004, offset 0xe0c
    MoveValue<f32, int> mTrackVolume[8];            // size 0x080, offset 0xe10
    ut::FileStream *mFileStream;                    // size 0x004, offset 0xe90
    int mFileStreamBuffer[FILE_STREAM_BUFFER_SIZE]; // size 0x200, offset 0xe94

  private:
    friend class snd::StrmSoundHandle;
}; // size 0x1094
} // namespace detail
} // namespace snd
} // namespace nw4r

#endif
