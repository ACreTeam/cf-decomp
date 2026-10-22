#include "revolution/ai.h"
#include "revolution/os.h"
#include "revolution/esp.h"
#include "revolution/nand.h"
#include "private/nand.h"
#include "private/os.h"
#include "private/vi.h"
#include <string.h>

OSAlarm __OSExpireAlarm;
OSTime __OSExpireTime;
OSThread* __OSExpireThread;
BOOL __OSExpireSetExpiredFlag;

void* __OSPlayTimeRebootThread(void*);

typedef struct {
	s16 new_ai_buffer[2][48*2*3];
	s32 idx_ai_buffer;
	s16* src_buffer;
	u32 frames;
	s16 aLeft;
	s16 aRight;
	AIDCallback gameAIDCallback;
} __OSExpireAIFadeStruct;

__OSExpireAIFadeStruct* __OSExpireAIFade = NULL;

BOOL OSPlayTimeIsLimited() {
    return __OSExpireTime != 0;
}

void __OSPlayTimeRebootCallback(OSAlarm* alarm, OSContext* context) {
    void* arenaHi;
    u32 memSize;
    OSThread* thread;
    void* stack;

    for (thread = __OSActiveThreadQueue.head; thread; thread = thread->linkActive.next) {
        OSSuspendThread(thread);
    }

    memSize = OSRoundUp32B(sizeof(OSThread)) + 0x1000;
    arenaHi = *(void**)OSPhysicalToCached(0x3128);
    arenaHi = (void*)((u32)arenaHi - memSize);

    thread = (OSThread*)arenaHi;
    arenaHi = (void*)((u32)arenaHi + OSRoundUp32B(sizeof(OSThread)));
    stack = arenaHi;

    if (!OSCreateThread(thread, __OSPlayTimeRebootThread, NULL, (void*)((u32)stack + 0x1000), 0x1000, 0, 0)) {
        __OSHotResetForError();
    }

    OSResumeThread(thread);
}

void __OSPlayTimeFadeLastAIDCallback(void) {
    if (__OSExpireAIFade->gameAIDCallback) {
        __OSExpireAIFade->gameAIDCallback();
    }

	if (__OSExpireAIFade->frames == 0) {
	    __OSExpireAIFade->src_buffer = (s16*)OSPhysicalToCached(AIGetDMAStartAddr());
	}

	if (__OSExpireAIFade->frames == 1) {
        DCInvalidateRange(__OSExpireAIFade->src_buffer, 4);
    	__OSExpireAIFade->aLeft = (s16)(*__OSExpireAIFade->src_buffer++);
        __OSExpireAIFade->aRight = (s16)(*__OSExpireAIFade->src_buffer);
	}

	if (__OSExpireAIFade->frames >= 1) {
		s16* buffer = __OSExpireAIFade->new_ai_buffer[__OSExpireAIFade->idx_ai_buffer];
        s16* dest = buffer;
        u32 bytes = AIGetDMALength();
        u32 i = bytes;
        f32 delta = 0.995f;

        while (i) {
            *dest++ = __OSExpireAIFade->aLeft;
            *dest++ = __OSExpireAIFade->aRight;

            __OSExpireAIFade->aLeft *= delta;
            __OSExpireAIFade->aRight *= delta;

            i -= 4;
        }

        DCFlushRange(buffer, bytes);
        AIInitDMA((u32)buffer, bytes);
        __OSExpireAIFade->idx_ai_buffer++;
        __OSExpireAIFade->idx_ai_buffer &= 1;
	}

	__OSExpireAIFade->frames++;
}

BOOL __OSWriteExpiredFlag(void) {
    s32 rv = 0;
    NANDFileInfo nInfo;
    BOOL openNInfo = FALSE;
    u8 titleId[32] __attribute__ ((aligned(32)));

    rv = NANDPrivateCreate("/shared2/expired", 63, 0);

    if (rv != NAND_RESULT_OK && rv != NAND_RESULT_EXISTS) {
        goto out;
    }

    rv = NANDPrivateOpen("/shared2/expired", &nInfo, 2);

    if (rv != NAND_RESULT_OK) {
        goto out;
    }

    openNInfo = TRUE;

    rv = ESP_InitLib();
    if (rv != 0) {
        goto out;
    }

    memset(titleId, 0, 32);
    rv = ESP_GetTitleId((ESTitleId*)titleId);

    if (rv != 0) {
        goto out;
    }

    rv = NANDWrite(&nInfo, titleId, 32);
    if (rv < 0) {
        goto out;
    }
    else if (rv != 32) {
        rv = NAND_RESULT_INVALID;
        goto out;
    }
    else {
        rv = 0;
    }

out:
    if (openNInfo) {
        NANDClose(&nInfo);
    }

    return rv == 0 ? TRUE : FALSE;
}

static BOOL __OSWriteExpiredFlagIfSet(void) {
    if (__OSExpireSetExpiredFlag) {
        return __OSWriteExpiredFlag();
    }

    return FALSE;
}

void* __OSPlayTimeRebootThread(void* args) {
    BOOL enabled;
    u32 frames, fadeShift = 1;
    __OSExpireAIFadeStruct aiFade __attribute__ ((aligned(32)));

    __OSExpireAIFade = &aiFade;
    memset(__OSExpireAIFade, 0, sizeof(__OSExpireAIFadeStruct));
    __OSExpireAIFade->gameAIDCallback = AIRegisterDMACallback(__OSPlayTimeFadeLastAIDCallback);

	for (frames = 0; frames < 20; frames++) {
		fadeShift = (frames / 5) + 1;
		if (fadeShift > 7) {
			fadeShift = 7;
        }

		VIWaitForRetrace(); 
		__OSSetVIForceDimming(TRUE, fadeShift, fadeShift);
	}

	AIRegisterDMACallback(NULL);

	VISetBlack(TRUE);
	VIFlush();

	enabled = OSDisableInterrupts();

    __OSWriteExpiredFlagIfSet();
    OSRestoreInterrupts(enabled);
    OSReturnToMenu();
    return NULL;
}

void __OSPlayTimeAlarmExpired(OSAlarm* alarm, OSContext* context) {
	void *arenaHi;
	u32 memSize;
	OSThread *thread;
	void *stack;

    if (__OSExpireThread != NULL) {
        for (thread = __OSActiveThreadQueue.head; thread; thread = thread->linkActive.next) {
            if (thread != __OSExpireThread) {
                OSSuspendThread(thread);
            }
        }

        OSCreateAlarm(&__OSExpireAlarm);
        OSSetAlarm(&__OSExpireAlarm,  OSSecondsToTicks((OSTime)0xF0), __OSPlayTimeRebootCallback);
    
        while (__OSExpireThread->suspend > 0) {
            OSResumeThread(__OSExpireThread);
        }
    }
    else {
        for (thread = __OSActiveThreadQueue.head; thread; thread = thread->linkActive.next) {
            OSSuspendThread(thread);
        }

        memSize = OSRoundUp32B(sizeof(OSThread)) + 0x1000;
        arenaHi = *(void**)OSPhysicalToCached(0x3128);
        arenaHi = (void*)((u32)arenaHi - memSize);

        thread  = (OSThread*)arenaHi;
        arenaHi = (void*)((u32)arenaHi + OSRoundUp32B(sizeof(OSThread)));
        stack   = arenaHi;

        if (!OSCreateThread(thread, __OSPlayTimeRebootThread, NULL, (void*)((u32)stack + 0x1000), 0x1000, 0, 0)) {
            __OSHotResetForError();
        }

        OSResumeThread(thread);
    }
}

static s32 __OSPlayTimeGetConsumption(ESTicketView *tk, ESLpEntry *lpEntry, u32 *nEntries) {
	s32 rv = ESP_GetConsumption(tk->ticketId, NULL, nEntries);
    
    /* a few of these blocks have useless code, that's because of debug stripped asserts and reports */
    if (rv > 0) {
        goto out;
    }
	else if (rv != 0) {
        rv = rv;
		goto out;
	}

	if (*nEntries != 0) {
       	rv = ESP_GetConsumption(tk->ticketId, lpEntry, nEntries);
    	
        if (rv != 0) {
            rv = rv;
    		goto out;
    	}
	}

out:
	return rv;
}

s32 __OSGetPlayTime(ESTicketView *ticket, OSPlayTimeType *type, u32 *playTime) {
	s32 rv;
	u32 i;
	ESLpEntry lpEntry[8] __attribute__ ((aligned(32)));
	u32 numCc = 0, seenOther = 0;
	ESTicketView ticketAligned __attribute__ ((aligned(32)));

	if ((u32)(ticket) & 31) {
		memcpy(&ticketAligned, ticket, sizeof(ESTicketView));
		ticket = &ticketAligned;
	}

	rv = __OSPlayTimeGetConsumption(ticket, lpEntry, &numCc);
	if (rv != 0) {
		goto out;
    }

	for (i = 0; i < 8; i++) {
		if (ticket->limits.data[i].code == 1) {
			*type = OS_PLAYTIME_TIME_LIMIT;
			
            if (numCc == 0) {
				*playTime = ticket->limits.data[i].limit;
			}
			else {
				if (lpEntry[i].limit >= ticket->limits.data[i].limit) {
					*playTime = 0;
                }
				else {
					*playTime = ticket->limits.data[i].limit - lpEntry[i].limit;
                }
			}
			goto out;
		}
		else if (ticket->limits.data[i].code != 0) {
			seenOther = i + 1;
		}
	}

	if (!seenOther) {
		*type = OS_PLAYTIME_PERMANENT;
		*playTime = 0xFFFFFFFF;
	}
	else {
		seenOther--;

		if (ticket->limits.data[seenOther].code == 4) {
			*type = OS_PLAYTIME_LAUNCH_LIMIT;
			*playTime = ticket->limits.data[seenOther].limit;
    
			if (numCc > 0) {
				*playTime -= lpEntry[seenOther].limit;
            }
		}
		else {
			*type = OS_PLAYTIME_UNKNOWN;
		}
	}

out:
	return rv;
}

static s32 __OSGetPlayTimeCurrent(OSPlayTimeType* type, u32* playTime) {
    s32 rv;
    ESTicketView ticket __attribute__ ((aligned(32)));

    rv = ESP_DiGetTicketView(NULL, &ticket);

    if (rv == -1017) {
        goto out;
    }

    if (rv != 0) {
        rv = rv;
        goto out;
    }

    rv = __OSGetPlayTime(&ticket, type, playTime);

out:
    return rv;
}

void __OSInitPlayTime() {
    s32 rv;
    u32 limit;
    OSPlayTimeType type;

    __OSExpireTime = 0;
    __OSExpireThread = NULL;
    __OSExpireSetExpiredFlag = TRUE;

    rv = ESP_InitLib();

    if (rv != 0) {
        rv = rv;
        goto out;
    }

    rv = __OSGetPlayTimeCurrent(&type, &limit);

    if (rv != 0) {
        if (rv != -1017) {
            rv = rv;
        }

        goto out;
    }

    if (type == OS_PLAYTIME_PERMANENT) {
        goto out;
    }
    else if (type == OS_PLAYTIME_TIME_LIMIT) {
        if (limit == 0) {
            OSPanic(__FILE__, 0x307, "Expired");
        }

        OSCreateAlarm(&__OSExpireAlarm);
        OSSetAlarm(&__OSExpireAlarm, OSSecondsToTicks((OSTime)limit + 1), __OSPlayTimeAlarmExpired);
        __OSExpireTime = __OSExpireAlarm.fire;
        OSReport("PlayTime: %d seconds left\n", limit);
    }

out:
    ESP_CloseLib();
}
