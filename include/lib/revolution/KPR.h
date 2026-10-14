#ifndef RVL_SDK_PUBLIC_KPR_H
#define RVL_SDK_PUBLIC_KPR_H
#include <types.h>
#ifdef __cplusplus
extern "C" {
#endif

// KPR: the SDK's kana composition queue (lib/RVL_SDK/kpr/kpr_lib.c, not decompiled). Only what the game uses.

typedef struct KPRQueue {
    u8 _00[0x18];
} KPRQueue;

// 801FA7B8: copies up to maxSize pending characters to buf; the number pending (buf == NULL or maxSize == 0: only counts).
u8 KPRLookAhead(KPRQueue *queue, wchar_t *buf, int maxSize);

#ifdef __cplusplus
}
#endif
#endif
