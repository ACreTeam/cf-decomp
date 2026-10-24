#ifndef NW4R_SND_WAVE_SOUND_H
#define NW4R_SND_WAVE_SOUND_H
#include <nw4r/types_nw4r.h>

#include <nw4r/snd/snd_BasicSound.h>
#include <nw4r/snd/snd_WsdPlayer.h>

#include <nw4r/ut.h>

namespace nw4r {
namespace snd {

// Forward declarations
class WaveSoundHandle;

namespace detail {
template <typename T> class SoundInstanceManager;
} // namespace detail

namespace detail {

class WaveSound : public BasicSound {
    friend class nw4r::snd::WaveSoundHandle;

public:
    NW4R_UT_RTTI_DECL(WaveSound);

    typedef ut::LinkList<WaveSound, 0xEC> PriorityLinkList;

  public:
    WaveSound(SoundInstanceManager<WaveSound>* pManager, int priority, int ambientPriority);

    virtual void Shutdown(); // at 0x10
    virtual bool IsPrepared() const {
        return mPreparedFlag;
    } // at 0x14

    virtual void OnUpdatePlayerPriority(); // at 0x2C
    virtual bool IsAttachedTempSpecialHandle();   // at 0x18
    virtual void DetachTempSpecialHandle();       // at 0x1C

    virtual BasicPlayer& GetBasicPlayer() {
        return mWsdPlayer;
    } // at 0x24
    virtual const BasicPlayer& GetBasicPlayer() const {
        return mWsdPlayer;
    } // at 0x28

    bool Prepare(const void *pWsdData, s32 wsdOffset,
                 WsdPlayer::StartOffsetType startType, s32 startOffset,
                 const WsdPlayer::WsdCallback *pCallback, u32 callbackArg);

    void SetChannelPriority(int priority);
    void SetReleasePriorityFix(bool flag);

private:
    WsdPlayer mWsdPlayer;                      // at 0x10C
    WaveSoundHandle* mTempSpecialHandle;       // at 0x1E4
    SoundInstanceManager<WaveSound>* mManager; // at 0x1E8
    bool mPreparedFlag;                        // at 0x1EC
};

} // namespace detail
} // namespace snd
} // namespace nw4r

#endif
