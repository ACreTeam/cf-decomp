#ifndef NW4R_SND_STRM_SOUND_H
#define NW4R_SND_STRM_SOUND_H
#include <nw4r/types_nw4r.h>

#include <nw4r/snd/snd_BasicSound.h>
#include <nw4r/snd/snd_StrmPlayer.h>

#include <nw4r/ut.h>

namespace nw4r {
namespace snd {

// Forward declarations
class StrmSoundHandle;

namespace detail {
template <typename T> class SoundInstanceManager;
class StrmBufferPool;
} // namespace detail

namespace detail {

class StrmSound : public BasicSound {
    friend class nw4r::snd::StrmSoundHandle;

public:
    NW4R_UT_RTTI_DECL(StrmSound);

public:
    StrmSound(SoundInstanceManager<StrmSound>* pManager, int priority, int ambientPriority);

    virtual void Shutdown(); // at 0x10
    virtual bool IsPrepared() const {
        return mStrmPlayer.IsPrepared();
    } // at 0x14

    virtual void OnUpdatePlayerPriority(); // at 0x2C
    virtual bool IsAttachedTempSpecialHandle();   // at 0x18
    virtual void DetachTempSpecialHandle();       // at 0x1C

    virtual BasicPlayer& GetBasicPlayer() {
        return mStrmPlayer;
    } // at 0x24
    virtual const BasicPlayer& GetBasicPlayer() const {
        return mStrmPlayer;
    } // at 0x28

    bool Prepare(StrmBufferPool* pPool, StrmPlayer::StartOffsetType offsetType,
                 s32 offset, int voices, ut::FileStream* pStream);

    void* GetFileStreamBuffer() {
        return mFileStreamBuffer;
    }
    s32 GetFileStreamBufferSize() {
        return sizeof(mFileStreamBuffer);
    }

private:
    static const int FILE_STREAM_BUFFER_SIZE = 512;

private:
    StrmPlayer mStrmPlayer;                    // at 0x10C
    StrmSoundHandle* mTempSpecialHandle;       // at 0x954
    SoundInstanceManager<StrmSound>* mManager; // at 0x958
    char UNK_0x928[0x93C - 0x928];
    char mFileStreamBuffer[FILE_STREAM_BUFFER_SIZE]; // at 0x970
};

} // namespace detail
} // namespace snd
} // namespace nw4r

#endif
