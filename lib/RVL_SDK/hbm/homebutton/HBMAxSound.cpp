#include "revolution/hbm/homebutton/HBMAxSound.hpp"

#include <math.h>

#include "revolution/ai.h"
#include "revolution/arc.h"
#include "revolution/ax.h"
#include "revolution/ax/AXOut.h"
#include "revolution/hbm/nw4hbm/db/assert.h"
#include "revolution/hbm/sound/seq.h"
#include "revolution/os/OSInterrupt.h"
#include "revolution/os/OSMessage.h"

#define HBM_MEM_SIZE_SOUND 0x18700

namespace
{
enum AudioThreadMsg
{
    AudioThreadMsg_RunAudioFrame = 1,
    AudioThreadMsg_Exit = 8,
};

typedef int UserCallback(int evt, int num);

struct SeqPlayer
{
    HBMSEQSEQUENCE seq;
    bool activeFlag;
    SeqPlayer* next;
    SeqPlayer* prev;
    int seqNum;
};

struct SeqPlayerList
{
    SeqPlayer* head;
    SeqPlayer* tail;
};

struct HBMAxSoundWork
{
    SeqPlayer playersDefault[4];
    SeqPlayer playersFocus[3];
    SeqPlayerList playerListDefault;
    SeqPlayerList playerListFocus;
    UserCallback* userCallback;
    AXOutCallback oldAxRegisterCallback;
    ARCHandle soundArcHandle;
    OSThread mThread;
    OSMessageQueue mMsgQueue;
    OSMessage mMessageArray[4];
    u64* mpThreadStack;
    void* wt;
    void* pcm;
};

void* GetMidiDataFromArc(int num);
SeqPlayerList* GetUsePlayerListFromSeqNum(int num);
SeqPlayer* GetFreePlayer(int num);
void AudioFrameCallback();
void* AudioSoundThreadProc(void* arg);
void StopSeq(SeqPlayer* player);
void UpdateSeqPlayerList(SeqPlayerList* list);

static const char WT_FILENAME[] = "wt\\HomeButtonSe.wt";
static const char PCM_FILENAME[] = "wt\\HomeButtonSe.pcm";
static HBMAxSoundWork* sWork;

void* GetMidiDataFromArc(int num)
{
    static const char* SOUND_FILENAME[] = {
        "midi\\00_home_button.mid",
        "midi\\01_return_app.mid",
        "midi\\02_goto_menu.mid",
        "midi\\03_reset_app.mid",
        "midi\\04_focus.mid",
        "midi\\05_select.mid",
        "midi\\06_cancel.mid",
        "midi\\07_open_controller.mid",
        "midi\\08_close_controller.mid",
        "midi\\09_volume_plus.mid",
        "midi\\10_volume_minus.mid",
        "midi\\11_volume_plus_limit.mid",
        "midi\\12_volume_minus_limit.mid",
        "midi\\13_nothing_done.mid",
        "midi\\14_vibe_on.mid",
        "midi\\15_vibe_off.mid",
        "midi\\16_start_connect_window.mid",
        "midi\\17_connected1.mid",
        "midi\\18_connected2.mid",
        "midi\\19_connected3.mid",
        "midi\\20_connected4.mid",
        "midi\\21_end_connect_window.mid",
        "midi\\22_manual_open.mid",
        "midi\\23_manual_focus.mid",
        "midi\\24_manual_select.mid",
        "midi\\25_manual_scroll.mid",
        "midi\\26_manual_cancel.mid",
        "midi\\27_manual_return_app.mid"
    };

    ARCFileInfo info;
    BOOL result = ARCOpen(&sWork->soundArcHandle, SOUND_FILENAME[num], &info);
    if (!result)
        return NULL;

    return ARCGetStartAddrInMem(&info);
}

SeqPlayerList* GetUsePlayerListFromSeqNum(int num)
{
    switch (num)
    {
    case 4:  /* "04_focus.mid" */
    case 23: /* "23_manual_focus.mid" */
    case 25: /* "25_manual_scroll.mid" */
        return &sWork->playerListFocus;

    default:
        return &sWork->playerListDefault;
    }
}

SeqPlayer* GetFreePlayer(int num)
{
    SeqPlayer* playersArray;
    int arraySize;
    switch (num)
    {
    case 4:  /* "04_focus.mid" */
    case 23: /* "23_manual_focus.mid" */
    case 25: /* "25_manual_scroll.mid" */
        playersArray = sWork->playersFocus;
        arraySize = ARRAY_COUNT(sWork->playersFocus);
        break;

    default:
        playersArray = sWork->playersDefault;
        arraySize = ARRAY_COUNT(sWork->playersDefault);
        break;
    }

    SeqPlayer* player = NULL;
    for (int i = 0; i < arraySize; i++)
    {
        if (!playersArray[i].activeFlag)
        {
            player = &playersArray[i];
            player->activeFlag = true;
            break;
        }
    }

    if (!player)
    {
        SeqPlayerList* list = GetUsePlayerListFromSeqNum(num);
        NW4HBMAssert(list != NULL);

        player = list->head;
        StopSeq(list->head);
        player->activeFlag = true;
    }

    return player;
}

void AudioFrameCallback()
{
    if (!sWork)
        return;

    OSSendMessage(&sWork->mMsgQueue, reinterpret_cast<OSMessage>(AudioThreadMsg_RunAudioFrame), OS_MESSAGE_NO_FLAGS);

    if (sWork->oldAxRegisterCallback)
        (*sWork->oldAxRegisterCallback)();
}

void* AudioSoundThreadProc(void*)
{
    OSMessage message = 0;

    while (true)
    {
        OSReceiveMessage(&sWork->mMsgQueue, &message, OS_MESSAGE_BLOCK);

        if (reinterpret_cast<u32>(message) == AudioThreadMsg_RunAudioFrame)
        {
            HBMSEQRunAudioFrame();
            HBMSYNRunAudioFrame();
            HBMMIXUpdateSettings();

            if (sWork)
            {
                UpdateSeqPlayerList(&sWork->playerListDefault);
                UpdateSeqPlayerList(&sWork->playerListFocus);
            }
        }
        else if (reinterpret_cast<u32>(message) == AudioThreadMsg_Exit)
        {
            break;
        }
    }

    return NULL;
}

void StopSeq(SeqPlayer* player)
{
    SeqPlayerList* list = GetUsePlayerListFromSeqNum(player->seqNum);

    HBMSEQSetState(&player->seq, 0);
    HBMSEQRemoveSequence(&player->seq);

    player->activeFlag = false;

    // remove from linked list

    if (!player->prev)
        list->head = player->next;
    else
        player->prev->next = player->next;

    if (!player->next)
        list->tail = player->prev;
    else
        player->next->prev = player->prev;

    player->next = NULL;
    player->prev = NULL;
}

void UpdateSeqPlayerList(SeqPlayerList* list)
{
    SeqPlayer* current = list->head;
    for (SeqPlayer* next; current; current = next)
    {
        next = current->next;

        if (current->activeFlag && HBMSEQGetState(&current->seq) == 0
            && !current->seq.synth.notes)
        {
            StopSeq(current);
        }
    }
}

s32 GainToTenthsDb(f32 gain)
{
    return static_cast<s32>(static_cast<f32>(log10(gain)) * 20.0f * 10.0f);
}

void* OffsetAddress(void* base, u32 byteOffset)
{
    return reinterpret_cast<void*>(reinterpret_cast<u32>(base) + byteOffset);
}

} // namespace

namespace homebutton
{

void PlaySeq(int num)
{
    void* midi = GetMidiDataFromArc(num);
    if (!midi)
        return;

    SeqPlayer* player = GetFreePlayer(num);
    HBMSEQAddSequence(&player->seq, static_cast<u8*>(midi), static_cast<u8*>(sWork->wt), static_cast<u8*>(sWork->pcm), 0);
    HBMSEQSetState(&player->seq, 1);
    player->seqNum = num;

    SeqPlayerList* list = GetUsePlayerListFromSeqNum(num);
    if (!list->tail)
    {
        list->head = player;
        list->tail = player;

        player->next = NULL;
        player->prev = NULL;
    }
    else
    {
        list->tail->next = player;

        player->prev = list->tail;
        player->next = NULL;

        list->tail = player;
    }
}

void InitAxSound(const void* soundData, void* mem, u32 memSize)
{
    NW4HBMAssert(memSize >= HBM_MEM_SIZE_SOUND);
    if (memSize < HBM_MEM_SIZE_SOUND)
        return;

    if (!AICheckInit())
    {
        AIInit(NULL);
        AXInit();
    }

    HBMMIXInit();
    HBMSYNInit();
    HBMSEQInit();

    HBMAxSoundWork* work = static_cast<HBMAxSoundWork*>(mem);

    for (int i = 0; i < (int)ARRAY_COUNT(work->playersDefault); i++)
    {
        work->playersDefault[i].activeFlag = false;
        work->playersDefault[i].next = NULL;
        work->playersDefault[i].prev = NULL;
    }

    for (int i = 0; i < (int)ARRAY_COUNT(work->playersFocus); i++)
    {
        work->playersFocus[i].activeFlag = false;
        work->playersFocus[i].next = NULL;
        work->playersFocus[i].prev = NULL;
    }

    work->wt = NULL;
    work->pcm = NULL;
    work->mpThreadStack = reinterpret_cast<u64*>(
        reinterpret_cast<u32>(mem) + (HBM_MEM_SIZE_SOUND - 0x4000));
    work->playerListDefault.head = NULL;
    work->playerListDefault.tail = NULL;
    work->playerListFocus.head = NULL;
    work->playerListFocus.tail = NULL;

    BOOL result = ARCInitHandle(const_cast<void*>(soundData), &work->soundArcHandle);
    if (!result)
        return;

    ARCFileInfo wtInfo;
    result = ARCOpen(&work->soundArcHandle, WT_FILENAME, &wtInfo);
    if (!result)
        return;

    work->wt = ARCGetStartAddrInMem(&wtInfo);

    ARCFileInfo pcmInfo;
    result = ARCOpen(&work->soundArcHandle, PCM_FILENAME, &pcmInfo);
    if (!result)
        return;

    work->pcm = ARCGetStartAddrInMem(&pcmInfo);

    OSInitMessageQueue(&work->mMsgQueue, work->mMessageArray, ARRAY_COUNT(work->mMessageArray));

    void* stackTop = OffsetAddress(OffsetAddress(work->mpThreadStack, memSize),
        -(HBM_MEM_SIZE_SOUND - 0x4000));
    result = OSCreateThread(&work->mThread, AudioSoundThreadProc, NULL, stackTop,
        memSize - (HBM_MEM_SIZE_SOUND - 0x4000), 4, 0);
    if (!result)
        return;

    sWork = work;
    OSResumeThread(&work->mThread);

    work->oldAxRegisterCallback = AXRegisterCallback(&AudioFrameCallback);
}

void ShutdownAxSound()
{
    if (!sWork)
        return;

    StopAllSeq();
    AXRegisterCallback(sWork->oldAxRegisterCallback);

    BOOL result = OSJamMessage(&sWork->mMsgQueue, reinterpret_cast<OSMessage>(AudioThreadMsg_Exit), OS_MESSAGE_BLOCK);
    NW4HBMAssert(result);

    OSJoinThread(&sWork->mThread, NULL);
    HBMSEQQuit();
    HBMSYNQuit();
    HBMMIXQuit();
    sWork = NULL;
}

void AxSoundMain()
{
}

void StopAllSeq()
{
    if (!sWork)
        return;

    for (int i = 0; i < (int)ARRAY_COUNT(sWork->playersDefault); i++)
    {
        SeqPlayer& player = sWork->playersDefault[i];

        if (player.activeFlag)
            StopSeq(&player);
    }

    NW4HBMAssert(sWork->playerListDefault.head == NULL);
    NW4HBMAssert(sWork->playerListDefault.tail == NULL);

    for (int i = 0; i < (int)ARRAY_COUNT(sWork->playersFocus); i++)
    {
        SeqPlayer& player = sWork->playersFocus[i];

        if (player.activeFlag)
            StopSeq(&player);
    }

    NW4HBMAssert(sWork->playerListFocus.head == NULL);
    NW4HBMAssert(sWork->playerListFocus.tail == NULL);
}

void SetVolumeAllSeq(f32 gain)
{
    if (!sWork)
        return;

    s32 db;
    if (gain <= 0.0f)
        db = -904;
    else
        db = GainToTenthsDb(gain);

    // clamp
    if (db > 60)
        db = 60;

    if (db < -904)
        db = -904;

    for (int i = 0; i < (int)ARRAY_COUNT(sWork->playersDefault); i++)
    {
        SeqPlayer& player = sWork->playersDefault[i];

        if (player.activeFlag)
            HBMSEQSetVolume(&player.seq, db);
    }

    for (int i = 0; i < (int)ARRAY_COUNT(sWork->playersFocus); i++)
    {
        SeqPlayer& player = sWork->playersFocus[i];

        if (player.activeFlag)
            HBMSEQSetVolume(&player.seq, db);
    }
}

void SetSoundMode(HBMMIXSoundMode mode)
{
    HBMMIXSetSoundMode(mode);
}

} // namespace homebutton
