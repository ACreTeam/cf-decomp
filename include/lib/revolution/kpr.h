#ifndef REVOLUTION_KPR_H
#define REVOLUTION_KPR_H

#include <revolution/types.h>
#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _KPRMode {
    KPR_MODE_NONE = 0,
    KPR_MODE_ALT_KEYPAD = (1 << 0),
    KPR_MODE_DEADKEY = (1 << 1),
    KPR_MODE_JP_ROMAJI_HIRAGANA = (1 << 2),
    KPR_MODE_JP_ROMAJI_KATAKANA = (1 << 3)
} KPRMode;

typedef struct _KPRQueue {
    wchar_t text[6];  // 0x00
    KPRMode mode;     // 0x0C
    u8 oCount;        // 0x10
    u8 iCount;        // 0x11
    u32 altVal;       // 0x14
} KPRQueue;

void KPRInitRegionUS(void);
void KPRInitRegionJP(void);
void KPRInitRegionEU(void);

void KPRInitQueue(KPRQueue* queue);
void KPRClearQueue(KPRQueue* queue);

void KPRSetMode(KPRQueue* queue, KPRMode mode);
KPRMode KPRGetMode(KPRQueue* queue);

u32 KPRPutChar(KPRQueue* queue, wchar_t ch);
wchar_t KPRGetChar(KPRQueue* queue);
wchar_t KPRRemoveChar(KPRQueue* queue);
u8 KPRLookAhead(KPRQueue* queue, wchar_t* buffer, u32 count);

#ifdef __cplusplus
}
#endif

#endif  // REVOLUTION_KPR_H
