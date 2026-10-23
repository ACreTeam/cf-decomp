#ifndef REVOLUTION_KBD_H
#define REVOLUTION_KBD_H

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef u8 KBDChannel;
typedef u8 KBDHidCode;
typedef u16 KBDUnicode;

typedef enum _KBDEc {
    KBD_SUCCESS = 0,
    KBD_ERR_OTHER = 1,
    KBD_ERR_NOT_INITIALIZED = 2,
    KBD_ERR_ALREADY_INITIALIZED = 3,
    KBD_ERR_BAD_VALUE = 4,
    KBD_ERR_BAD_STATUS = 5,
    KBD_ERR_BUSY = 7
} KBDEc;

typedef enum _KBDKeyMode {
    KBD_KM_NULL = 0,
    KBD_KM_DOWN = 1,
    KBD_KM_REPEAT = 2
} KBDKeyMode;

typedef enum _KBDModState {
    KBD_MS_NULL = 0,
    KBD_MS_CTRL = 1,
    KBD_MS_SHIFT = 2,
    KBD_MS_ALT = 4,
    KBD_MS_GUI = 8,
    KBD_MS_EXTRA = 16,
    KBD_MS_ALTGR = 32,
    KBD_MS_LANG1 = 64,
    KBD_MS_LANG2 = 128,
    KBD_MS_NUM_LOCK = 256,
    KBD_MS_CAPS_LOCK = 512,
    KBD_MS_SCROLL_LOCK = 1024,
    KBD_MS_SHIFTED_KEY = 2048,
    KBD_MS_SET_ALL = 4096
} KBDModState;

typedef enum _KBDChanStatus {
    KBD_CS_OK = 0,
    KBD_CS_DISCONNECTED = 1,
    KBD_CS_ROLL_OVER = 2,
    KBD_CS_ERROR = 4
} KBDChanStatus;

typedef enum _KBDCountryCode {
    KBD_CC_INTERNATIONAL = 0,
    KBD_CC_CANADIAN_BI,
    KBD_CC_CANADIAN_FR,
    KBD_CC_DANISH,
    KBD_CC_FINNISH,
    KBD_CC_FRENCH,
    KBD_CC_GERMAN,
    KBD_CC_GREEK,
    KBD_CC_ITALIAN,
    KBD_CC_JAPANESE,
    KBD_CC_LATIN_AMERICAN,
    KBD_CC_NETHERLANDS_DUTCH,
    KBD_CC_NORWEGIAN,
    KBD_CC_PORTUGUESE,
    KBD_CC_SPANISH,
    KBD_CC_SWEDISH,
    KBD_CC_UNITED_KINGDOM,
    KBD_CC_UNITED_STATES,
    KBD_CC_RESERVED
} KBDCountryCode;

typedef enum _KBDLedState {
    KBD_LED_NUM_LOCK = 0x01,
    KBD_LED_CAPS_LOCK = 0x02,
    KBD_LED_SCROLL_LOCK = 0x04,
    KBD_LED_NULL = 0x00,
    KBD_LED_RESERVED = 0x10
} KBDLedState;

typedef struct _KBDKeyEvent {
    /* 0x0 */ KBDChannel channel;
    /* 0x1 */ KBDHidCode hid;
    /* 0x4 */ KBDKeyMode mode;
    /* 0x8 */ KBDModState modState;
    /* 0xC */ KBDUnicode unicode;
} KBDKeyEvent; // size 0x10

typedef struct _KBDDevEvent {
    /* 0x0 */ KBDChannel channel;
} KBDDevEvent; // size 0x1

typedef struct _KBDLedEvent {
    /* 0x0 */ KBDChannel channel;
    /* 0x4 */ KBDLedState leds;
    /* 0x8 */ KBDEc rc;
} KBDLedEvent; // size 0xC

typedef void (*KBDDevCallbackF)(KBDDevEvent *ev);
typedef void (*KBDKeyCallbackF)(KBDKeyEvent *ev);
typedef void (*KBDLedCallbackF)(KBDLedEvent *ev, void *cbArg);

KBDEc KBDInit(void);
void KBDInitRegionUS(void);

KBDDevCallbackF KBDSetAttachCallback(KBDDevCallbackF callback);
KBDDevCallbackF KBDSetDetachCallback(KBDDevCallbackF callback);
KBDKeyCallbackF KBDSetKeyCallback(KBDKeyCallbackF callback);

KBDEc KBDSetLedsAsync(KBDChannel channel, KBDLedState leds,
                      KBDLedCallbackF callback, void *cbArg);
KBDEc KBDSetLeds(KBDChannel channel, KBDLedState leds);
KBDEc KBDSetCountry(KBDChannel channel, KBDCountryCode country);
KBDEc KBDSetLockProcessing(KBDChannel channel, BOOL enable);
KBDEc KBDSetModState(KBDChannel channel, KBDModState modState);
KBDUnicode KBDTranslateHidCode(KBDHidCode hid, KBDModState modState,
                               KBDCountryCode country);

#ifdef __cplusplus
}
#endif

#endif // REVOLUTION_KBD_H
