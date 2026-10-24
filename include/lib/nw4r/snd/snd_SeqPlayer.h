#ifndef NW4R_SND_SEQ_PLAYER_H
#define NW4R_SND_SEQ_PLAYER_H
#include <nw4r/types_nw4r.h>

#include <nw4r/snd/snd_BasicPlayer.h>
#include <nw4r/snd/snd_DisposeCallback.h>
#include <nw4r/snd/snd_SeqTrack.h>
#include <nw4r/snd/snd_SoundThread.h>

#include <nw4r/ut.h>

namespace nw4r {
namespace snd {

// City Folk's SoundArchivePlayer stores a seq user proc callback (ctor
// 8026ABB0).
struct SeqUserprocCallbackParam {
    volatile s16 *localVariable;  // at 0x0
    volatile s16 *globalVariable; // at 0x4
    volatile s16 *trackVariable;  // at 0x8
    bool cmpFlag;                 // at 0xC
};

namespace detail {

// Forward declarations
class Channel;
struct NoteOnInfo;
class NoteOnCallback;
class SeqTrack;
class SeqTrackAllocator;

class SeqPlayer : public BasicPlayer,
                  public DisposeCallback,
                  public SoundThread::PlayerCallback {
  public:
    struct ParserPlayerParam {
        u8 volume;                // at 0x0
        u8 priority;              // at 0x1
        u8 timebase;              // at 0x2
        u16 tempo;                // at 0x4
        NoteOnCallback *callback; // at 0x8
    };

    typedef void SeqUserprocCallback(u16 procId,
                                     SeqUserprocCallbackParam *pParam,
                                     void *pCallbackData);

    enum OffsetType { OFFSET_TYPE_TICK, OFFSET_TYPE_MILLISEC };

    enum SetupResult {
        SETUP_SUCCESS,
        SETUP_ERR_CANNOT_ALLOCATE_TRACK,
        SETUP_ERR_UNKNOWN
    };

    static const int LOCAL_VARIABLE_NUM = 16;
    static const int GLOBAL_VARIABLE_NUM = 16;
    static const int VARIABLE_NUM = LOCAL_VARIABLE_NUM + GLOBAL_VARIABLE_NUM;

    static const int TRACK_NUM = 16;

  public:
    SeqPlayer();
    virtual ~SeqPlayer(); // at 0x8

    virtual bool Start();          // at 0xC
    virtual void Stop();           // at 0x10
    virtual void Pause(bool flag); // at 0x14

    virtual bool IsActive() const {
        return mActiveFlag;
    } // at 0x18

    virtual bool IsStarted() const {
        return mStartedFlag;
    } // at 0x1C

    virtual bool IsPause() const {
        return mPauseFlag;
    } // at 0x20

    virtual void InvalidateData(const void *pStart,
                                const void *pEnd); // at 0x50

    virtual void InvalidateWaveData(const void * /* pStart */,
                                    const void * /* pEnd */) {
    } // at 0x54

    virtual void ChannelCallback(Channel * /* pChannel */) {
    } // at 0x58

    virtual void OnUpdateFrameSoundThread() {
        Update();
    } // at 0x5C

    virtual void OnShutdownSoundThread() {
        Stop();
    } // at 0x60

    void InitParam(int voices, NoteOnCallback *pCallback);

    SetupResult Setup(SeqTrackAllocator *pAllocator, u32 allocTrackFlags,
                      int voices, NoteOnCallback *pCallback);
    void SetSeqData(const void *pBase, s32 offset);

    void Skip(OffsetType type, int offset);

    void SetTempoRatio(f32 tempo);
    void SetChannelPriority(int priority);
    void SetReleasePriorityFix(bool flag);

    void SetTrackMute(u32 trackFlags, SeqMute mute); // CF 8026656C
    s16 GetLocalVariable(int idx) const;             // CF 80266524
    void SetLocalVariable(int idx, s16 value);
    static void SetGlobalVariable(int idx, s16 value);
    static s16 GetGlobalVariable(int idx); // CF 80266534

    void SetTrackVolume(u32 trackFlags, f32 volume);
    void SetTrackPan(u32 trackFlags, f32 pan);

    SeqTrack *GetPlayerTrack(int idx) const;
    SeqTrack *GetPlayerTrack(int idx);
    void SetSeqUserprocCallback(SeqUserprocCallback *callback, void *arg);
    void CallSeqUserprocCallback(u16 procId, SeqTrack *track);
    volatile s16 *GetVariablePtr(int idx);
    void Update();

    Channel *NoteOn(int bankNo, const NoteOnInfo &rInfo);

    template <typename T>
    void SetTrackParam(u32 trackFlags, void (SeqTrack::*pSetter)(T), T param) {
        ut::AutoInterruptLock lock;

        for (int i = 0; i < TRACK_NUM && trackFlags != 0;
             trackFlags >>= 1, i++) {

            if (trackFlags & 1) {
                SeqTrack *pTrack = GetPlayerTrack(i);

                if (pTrack != NULL) {
                    (pTrack->*pSetter)(param);
                }
            }
        }
    }

    bool IsReleasePriorityFix() const {
        return mReleasePriorityFixFlag;
    }

    f32 GetPanRange() const {
        return mPanRange;
    }

    f32 GetBaseTempo() const {
        return mTempoRatio * (mParserParam.timebase * mParserParam.tempo) /
               60000.0f;
    }

    u32 GetTick() const {
        return mTickCounter;
    }

    int GetVoiceOutCount() const {
        return mVoiceOutCount;
    }

    ParserPlayerParam &GetParserPlayerParam() {
        return mParserParam;
    }
    const ParserPlayerParam &GetParserPlayerParam() const {
        return mParserParam;
    }

    static const int DEFAULT_VARIABLE_VALUE = -1;

    static void InitGlobalVariable();

  private:
    static const int DEFAULT_TEMPO = 120;
    static const int DEFAULT_TIMEBASE = 48;
    static const int DEFAULT_PRIORITY = 64;

    static const int MAX_SKIP_TICK_PER_FRAME = 768;

  private:
    void CloseTrack(int idx);
    void SetPlayerTrack(int idx, SeqTrack *pTrack);

    void FinishPlayer();
    void UpdateChannelParam();
    int ParseNextTick(bool doNoteOn);

    void UpdateTick(int msec);
    void SkipTick();

  private:
    bool mActiveFlag;             // at 0xEC
    bool mStartedFlag;            // at 0xED
    bool mPauseFlag;              // at 0xEE
    bool mReleasePriorityFixFlag; // at 0xEF

    f32 mPanRange;                                   // at 0xF0
    f32 mTempoRatio;                                 // at 0xF4
    f32 mTickFraction;                               // at 0xF8
    u32 mSkipTickCounter;                            // at 0xFC
    f32 mSkipTimeCounter;                            // at 0x100
    int mVoiceOutCount;                              // at 0x104
    ParserPlayerParam mParserParam;                  // at 0x108
    SeqTrackAllocator *mSeqTrackAllocator;           // at 0x114
    SeqUserprocCallback *mSeqUserprocCallback;       // at 0x118
    void *mSeqUserprocCallbackArg;                   // at 0x11c
    SeqTrack *mTracks[TRACK_NUM];                    // at 0x120
    volatile s16 mLocalVariable[LOCAL_VARIABLE_NUM]; // at 0x160
    u32 mTickCounter;                                // at 0x180

    static volatile s16 mGlobalVariable[LOCAL_VARIABLE_NUM];
};

} // namespace detail
} // namespace snd
} // namespace nw4r

#endif
