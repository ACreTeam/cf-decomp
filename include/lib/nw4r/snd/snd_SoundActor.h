#ifndef NW4R_SND_SOUND_ACTOR_H
#define NW4R_SND_SOUND_ACTOR_H

#include <types.h>

#include <nw4r/snd/snd_BasicSound.h>
#include <nw4r/snd/snd_ExternalSoundPlayer.h>
#include <nw4r/snd/snd_SoundStartable.h>

namespace nw4r {
namespace snd {
class SoundArchivePlayer;
}
} // namespace nw4r

namespace nw4r {
namespace snd {
class SoundActor : public SoundStartable {
  public:
    SoundActor(SoundArchivePlayer &player);
    virtual ~SoundActor();

    virtual u32 detail_ConvertLabelStringToSoundId(const char *label) override;
    virtual StartResult detail_SetupSound(SoundHandle *pHandle, u32 soundId,
                                          bool holdFlag,
                                          const StartInfo *pStartInfo) override;

    virtual StartResult SetupSound(SoundHandle *pHandle, u32 soundId,
                                   const StartInfo *pStartInfo, void *);
    virtual StartResult
    detail_SetupSoundWithAmbientInfo(SoundHandle *, u32, const StartInfo *,
                                     detail::BasicSound::AmbientInfo *, void *);

    detail::ExternalSoundPlayer *detail_GetActorPlayer(int actorPlayerId) {
        if (actorPlayerId < 0 || ACTOR_PLAYER_COUNT <= actorPlayerId) {
            return nullptr;
        }

        return &mActorPlayer[actorPlayerId];
    }

    template <typename TForEachFunc>
    TForEachFunc ForEachSound(TForEachFunc pFunc, bool reverse) {
        for (int i = 0; i < ACTOR_PLAYER_COUNT; i++) {
            mActorPlayer[i].ForEachSound(pFunc, reverse);
        }

        return pFunc;
    }

    detail::SoundActorParam const &detail_GetActorParam() const {
        return mActorParam;
    }

    void StopAllSound(int fadeFrames);
    void PauseAllSound(bool flag, int fadeFrames);
    int GetPlayingSoundCount(int playerId) const;

    void SetVolume(f32 volume) {
        mActorParam.volume = volume;
    }

    f32 GetVolume() const {
        return mActorParam.volume;
    }

  public:
    static int const ACTOR_PLAYER_COUNT = 4;

  private:
    /* base SoundStartable */                // size 0x04, offset 0x00
    SoundArchivePlayer &mSoundArchivePlayer; // size 0x04, offset 0x04
    detail::ExternalSoundPlayer
        mActorPlayer[ACTOR_PLAYER_COUNT]; // size 0x40, offset 0x08
    detail::SoundActorParam mActorParam;  // size 0x0c, offset 0x48
}; // size 0x54
} // namespace snd
} // namespace nw4r

#endif
