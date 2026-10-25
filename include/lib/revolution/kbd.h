#ifndef REVOLUTION_KBD_H
#define REVOLUTION_KBD_H

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KBD_MAX_CHANNELS 4

typedef u8 KBDChannel;
typedef u8 KBDHidCode;
typedef u16 KBDUnicode;

#define KBD_HID_NONE 0
#define KBD_HID_OVERFLOW 255

typedef enum _KBDHIDEnum {
    KBD_HID_No_Event = 0x00,
    KBD_HID_Roll_Over = 0x01,
    KBD_HID_Post_Fail = 0x02,
    KBD_HID_Undefined_Error = 0x03,
    KBD_HID_A = 0x04,
    KBD_HID_Backslash_Bar = 0x31,
    KBD_HID_NonUS_Pound_Tilde = 0x32,
    KBD_HID_Grave_Tilde = 0x35,
    KBD_HID_L5_Zenkaku_Hankaku = 0x94,
    KBD_HID_Left_Control = 0xE0,
    KBD_HID_Left_Shift = 0xE1,
    KBD_HID_Left_Alt = 0xE2,
    KBD_HID_Left_GUI = 0xE3,
    KBD_HID_Right_Control = 0xE4,
    KBD_HID_Right_Shift = 0xE5,
    KBD_HID_Right_AltGr = 0xE6,
    KBD_HID_Right_GUI = 0xE7
} KBDHIDEnum;

#define KBK_Void 0x0000
#define KBK_Error 0xFFFF

#define KBK_Mod_Control 0xF000
#define KBK_Mod_Shift 0xF001
#define KBK_Mod_Alt 0xF002
#define KBK_Mod_GUI 0xF003
#define KBK_Mod_Extra 0xF004
#define KBK_Mod_AltGr 0xF005
#define KBK_Lang_Toggle 0xF006
#define KBK_Num_Lock 0xF007
#define KBK_Caps_Lock 0xF008
#define KBK_Print_Screen 0xF020
#define KBK_Scroll_Lock 0xF021
#define KBK_Pause 0xF022
#define KBK_Application 0xF02F

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

#define KBD_MS_TRANSIENTS (KBD_MS_CTRL | KBD_MS_SHIFT | KBD_MS_ALT | KBD_MS_GUI | KBD_MS_EXTRA | KBD_MS_ALTGR)
#define KBD_MS_TOGGLES (KBD_MS_LANG1 | KBD_MS_LANG2 | KBD_MS_NUM_LOCK | KBD_MS_CAPS_LOCK | KBD_MS_SCROLL_LOCK)

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

#define KBD_NUM_COUNTRY_CODES KBD_CC_RESERVED

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
void KBDInitRegionEU(void);
void KBDInitRegionJP(void);

KBDDevCallbackF KBDSetAttachCallback(KBDDevCallbackF callback);
KBDDevCallbackF KBDSetDetachCallback(KBDDevCallbackF callback);
KBDKeyCallbackF KBDSetKeyCallback(KBDKeyCallbackF callback);

KBDEc KBDResetChannel(KBDChannel channel);

KBDEc KBDSetLedsAsync(KBDChannel channel, KBDLedState leds,
                      KBDLedCallbackF callback, void *cbArg);
KBDEc KBDSetLeds(KBDChannel channel, KBDLedState leds);
KBDEc KBDSetCountry(KBDChannel channel, KBDCountryCode country);
KBDEc KBDSetLockProcessing(KBDChannel channel, BOOL enable);
KBDEc KBDSetModState(KBDChannel channel, KBDModState modState);
KBDEc KBDGetModState(KBDChannel channel, KBDModState *modState);
KBDUnicode KBDTranslateHidCode(KBDHidCode hid, KBDModState modState,
                               KBDCountryCode country);

#ifdef __cplusplus
}
#endif

#endif // REVOLUTION_KBD_H
