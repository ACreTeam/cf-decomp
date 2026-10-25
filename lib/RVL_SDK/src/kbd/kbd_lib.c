#include <private/kbd.h>
#include <private/usb_kbd.h>
#include <revolution/os.h>
#include <revolution/vi.h>

#define KBD_NUM_LED_CMDS 12
#define KBD_QUEUE_SIZE 32
#define KBD_QUEUE_EMPTY KBD_QUEUE_SIZE
#define KBD_MAX_HID_REMAPS 4
#define KBD_NUM_MODIFIER_KEYS 8
#define KBD_NUM_NO_REPEAT_KEYS 7
#define KBD_ID_NONE 0
#define KBD_DEFAULT_REPEAT_DELAY 500
#define KBD_DEFAULT_REPEAT_INTERVAL 33
#define KBD_SHUTDOWN_PRIORITY 127
#define KBD_SHUTDOWN_MAX_ATTEMPTS 16
#define KBD_SHUTDOWN_LED_WAIT_US 1

typedef struct KBDMap {
    /* 0x00 */ u8 num_cols;
    /* 0x01 */ u8 flags;
    /* 0x04 */ KBDUnicode* key_map;
    /* 0x08 */ KBDUnicode* mod_map;
    /* 0x0C */ u8 lang1_col;
    /* 0x0D */ u8 lang2_col;
    /* 0x0E */ u8 altgr_col;
    /* 0x0F */ u8 num_keys;
    /* 0x10 */ KBDHidCode hid_remap[KBD_MAX_HID_REMAPS][2];
    /* 0x18 */ u8 num_hid_remaps;
} KBDMap; // size 0x1C

typedef struct KBDLedRequest {
    /* 0x00 */ KBDLedEvent ev;
    /* 0x0C */ KBDLedCallbackF cb;
    /* 0x10 */ void* cb_arg;
} KBDLedRequest; // size 0x14

typedef struct KBDChanInfo {
    /* 0x000 */ KBDChannel channel;
    /* 0x004 */ KBDCountryCode country;
    /* 0x008 */ KBDChanStatus status;
    /* 0x00C */ KBDLedState leds;
    /* 0x010 */ KBDLedState retry_leds;
    /* 0x018 */ OSAlarm led_alarm;
    /* 0x048 */ USBKBDId id;
    /* 0x04C */ USBKBDReport last_report;
    /* 0x054 */ KBDKeyEvent queue[KBD_QUEUE_SIZE];
    /* 0x254 */ u8 queue_write;
    /* 0x255 */ u8 queue_read;
    /* 0x258 */ BOOL lock_processing;
    /* 0x25C */ KBDModState mod_state;
    /* 0x260 */ u8 ctrl_count;
    /* 0x261 */ u8 shift_count;
    /* 0x262 */ u8 alt_count;
    /* 0x263 */ u8 gui_count;
    /* 0x264 */ u8 altgr_count;
    /* 0x265 */ u8 extra_count;
    /* 0x266 */ u16 repeat_delay;
    /* 0x268 */ u16 repeat_interval;
    /* 0x26A */ KBDHidCode repeat_hid;
    /* 0x270 */ OSAlarm repeat_alarm;
    /* 0x2A0 */ BOOL access_sticky;
} KBDChanInfo; // size 0x2A8

const char* __KBDVersion = "<< RVL_SDK - KBD \trelease build: Aug  8 2007 02:10:45 (0x4199_60831) >>";

KBDUnicode kbdKeyMapUS_International[] = {
    0x00FF, 'a', 'A', 0x00E1, 0x00C1,
    0x003F, 'b', 'B', 0x0000, 0x0000,
    0x003F, 'c', 'C', 0x00A9, 0x00A2,
    0x00FF, 'd', 'D', 0x00F0, 0x00D0,
    0x00FF, 'e', 'E', 0x00E9, 0x00C9,
    0x003F, 'f', 'F', 0x0000, 0x0000,
    0x003F, 'g', 'G', 0x0000, 0x0000,
    0x003F, 'h', 'H', 0x0000, 0x0000,
    0x00FF, 'i', 'I', 0x00ED, 0x00CD,
    0x003F, 'j', 'J', 0x0000, 0x0000,
    0x003F, 'k', 'K', 0x0000, 0x0000,
    0x00FF, 'l', 'L', 0x00F8, 0x00D8,
    0x003F, 'm', 'M', 0x00B5, 0x0000,
    0x00FF, 'n', 'N', 0x00F1, 0x00D1,
    0x00FF, 'o', 'O', 0x00F3, 0x00D3,
    0x00FF, 'p', 'P', 0x00F6, 0x00D6,
    0x00FF, 'q', 'Q', 0x00E4, 0x00C4,
    0x003F, 'r', 'R', 0x00AE, 0x0000,
    0x003F, 's', 'S', 0x00DF, 0x00A7,
    0x00FF, 't', 'T', 0x00FE, 0x00DE,
    0x00FF, 'u', 'U', 0x00FA, 0x00DA,
    0x003F, 'v', 'V', 0x0000, 0x0000,
    0x00FF, 'w', 'W', 0x00E5, 0x00C5,
    0x003F, 'x', 'X', 0x0000, 0x0000,
    0x00FF, 'y', 'Y', 0x00FC, 0x00DC,
    0x00FF, 'z', 'Z', 0x00E6, 0x00C6,
    0x0000, '1', '!', 0x00A1, 0x00B9,
    0x0000, '2', '@', 0x00B2, 0x0000,
    0x0000, '3', '#', 0x00B3, 0x0000,
    0x0000, '4', '$', 0x00A4, 0x00A3,
    0x0000, '5', '%', 0x20AC, 0x0000,
    0x0000, '6', 0x0302, 0x00BC, 0x0000,
    0x0000, '7', '&', 0x00BD, 0x0000,
    0x0000, '8', '*', 0x00BE, 0x0000,
    0x0000, '9', '(', 0x2018, 0x0000,
    0x0000, '0', ')', 0x2019, 0x0000,
    0x0000, 0xF1CD, 0xF1CD, 0xF1CD, 0xF1CD,
    0x0000, 0xF1DB, 0xF1DB, 0xF1DB, 0xF1DB,
    0x0000, 0xF1C8, 0xF1C8, 0xF1C8, 0xF1C8,
    0x0000, 0xF1C9, 0xF1C9, 0xF1C9, 0xF1C9,
    0x0000, ' ', ' ', ' ', ' ',
    0x0000, '-', '_', 0x00A5, 0x0000,
    0x0000, '=', '+', 0x00D7, 0x00F7,
    0x0000, '[', '{', 0x00AB, 0x0000,
    0x0000, ']', '}', 0x00BB, 0x0000,
    0x0000, '\\', '|', 0x00AC, 0x00A6,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, ';', ':', 0x00B6, 0x00B0,
    0x0000, 0x030D, 0x030E, 0x00B4, 0x00A8,
    0x0000, 0x0300, 0x0303, 0x0000, 0x0000,
    0x00C0, ',', '<', 0x00E7, 0x00C7,
    0x0000, '.', '>', 0x0000, 0x0000,
    0x0000, '/', '?', 0x00BF, 0x0000,
    0xC000, 0xF008, 0xF008, 0xF008, 0xF008,
    0x0000, 0xF061, 0xF061, 0xF061, 0xF061,
    0x0000, 0xF062, 0xF062, 0xF062, 0xF062,
    0x0000, 0xF063, 0xF063, 0xF063, 0xF063,
    0x0000, 0xF064, 0xF064, 0xF064, 0xF064,
    0x0000, 0xF065, 0xF065, 0xF065, 0xF065,
    0x0000, 0xF066, 0xF066, 0xF066, 0xF066,
    0x0000, 0xF067, 0xF067, 0xF067, 0xF067,
    0x0000, 0xF068, 0xF068, 0xF068, 0xF068,
    0x0000, 0xF069, 0xF069, 0xF069, 0xF069,
    0x0000, 0xF06A, 0xF06A, 0xF06A, 0xF06A,
    0x0000, 0xF06B, 0xF06B, 0xF06B, 0xF06B,
    0x0000, 0xF06C, 0xF06C, 0xF06C, 0xF06C,
    0xC000, 0xF020, 0xF020, 0xF020, 0xF020,
    0xC000, 0xF021, 0xF021, 0xF021, 0xF021,
    0xC000, 0xF022, 0xF022, 0xF022, 0xF022,
    0x0000, 0xF1B0, 0xF1B0, 0xF1B0, 0xF1B0,
    0x0000, 0xF1B7, 0xF1B7, 0xF1B7, 0xF1B7,
    0x0000, 0xF1B9, 0xF1B9, 0xF1B9, 0xF1B9,
    0x0000, 0xF1AE, 0xF1AE, 0xF1AE, 0xF1AE,
    0x0000, 0xF1B1, 0xF1B1, 0xF1B1, 0xF1B1,
    0x0000, 0xF1B3, 0xF1B3, 0xF1B3, 0xF1B3,
    0x0000, 0xF1B6, 0xF1B6, 0xF1B6, 0xF1B6,
    0x0000, 0xF1B4, 0xF1B4, 0xF1B4, 0xF1B4,
    0x0000, 0xF1B2, 0xF1B2, 0xF1B2, 0xF1B2,
    0x0000, 0xF1B8, 0xF1B8, 0xF1B8, 0xF1B8,
    0xC000, 0xF007, 0xF007, 0xF007, 0xF007,
    0x0000, 0xF12F, 0xF12F, 0xF12F, 0xF12F,
    0x0000, 0xF12A, 0xF12A, 0xF12A, 0xF12A,
    0x0000, 0xF12D, 0xF12D, 0xF12D, 0xF12D,
    0x0000, 0xF12B, 0xF12B, 0xF12B, 0xF12B,
    0x0000, 0xF10D, 0xF10D, 0xF10D, 0xF10D,
    0x80FF, 0xF171, 0xF131, 0x0000, 0x0000,
    0x80FF, 0xF172, 0xF132, 0x0000, 0x0000,
    0x80FF, 0xF173, 0xF133, 0x0000, 0x0000,
    0x80FF, 0xF174, 0xF134, 0x0000, 0x0000,
    0x80FF, 0xF175, 0xF135, 0x0000, 0x0000,
    0x80FF, 0xF176, 0xF136, 0x0000, 0x0000,
    0x80FF, 0xF177, 0xF137, 0x0000, 0x0000,
    0x80FF, 0xF178, 0xF138, 0x0000, 0x0000,
    0x80FF, 0xF179, 0xF139, 0x0000, 0x0000,
    0x80FF, 0xF170, 0xF130, 0x0000, 0x0000,
    0x80FF, 0xF16E, 0xF12E, 0x0000, 0x0000,
    0x0000, '\\', '|', 0x0000, 0x0000,
    0xC000, 0xF02F, 0xF02F, 0xF02F, 0xF02F,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0xF13D, 0xF13D, 0xF13D, 0xF13D,
};

KBDUnicode kbdModifierMapUS_International[] = {
    0xC000, 0xF000, 0xF000, 0xF000, 0xF000,
    0xC000, 0xF001, 0xF001, 0xF001, 0xF001,
    0xC000, 0xF002, 0xF002, 0xF002, 0xF002,
    0xC000, 0xF003, 0xF003, 0xF003, 0xF003,
    0xC000, 0xF000, 0xF000, 0xF000, 0xF000,
    0xC000, 0xF001, 0xF001, 0xF001, 0xF001,
    0xC000, 0xF005, 0xF005, 0xF005, 0xF005,
    0xC000, 0xF003, 0xF003, 0xF003, 0xF003,
};

KBDUnicode noRepeatKeys[KBD_NUM_NO_REPEAT_KEYS] = {
    KBK_Lang_Toggle, KBK_Caps_Lock, KBK_Num_Lock, KBK_Scroll_Lock, KBK_Print_Screen, KBK_Pause, KBK_Application,
};

static BOOL kbdShutdown(BOOL final, u32 event);
OSShutdownFunctionInfo kbdShutdownInfo = {NULL, KBD_SHUTDOWN_PRIORITY};

static KBDMap kbdMaps[KBD_NUM_COUNTRY_CODES] = {0};
static USBKBDCmd kbdLedCmds[KBD_NUM_LED_CMDS] ATTRIBUTE_ALIGN(32) = {0};
static KBDChanInfo kbdChannels[KBD_MAX_CHANNELS] = {0};
static KBDLedRequest kbdLedRequests[KBD_NUM_LED_CMDS] = {0};

static BOOL kbdInitialized = FALSE;
static KBDDevCallbackF kbdAttachCallback = NULL;
static KBDDevCallbackF kbdDetachCallback = NULL;
static KBDKeyCallbackF kbdKeyCallback = NULL;

static void kbdProcKey(KBDHidCode hid, KBDKeyMode mode, KBDChannel ch);

static void kbdAttachHandler(USBKBDId id) {
    KBDDevEvent ev;
    KBDChannel ch;

    for (ch = 0; ch < KBD_MAX_CHANNELS; ch++) {
        if (kbdChannels[ch].id == KBD_ID_NONE) {
            break;
        }
    }

    if (ch == KBD_MAX_CHANNELS) {
        return;
    }

    kbdChannels[ch].id = id;
    kbdChannels[ch].status &= ~KBD_CS_DISCONNECTED;
    KBDResetChannel(ch);
    VIResetDimmingCount();

    if (kbdAttachCallback != NULL) {
        ev.channel = ch;
        kbdAttachCallback(&ev);
    }
}

static void kbdDetachHandler(USBKBDId id) {
    KBDDevEvent ev;
    KBDChannel ch;

    for (ch = 0; ch < KBD_MAX_CHANNELS; ch++) {
        if (kbdChannels[ch].id == id) {
            break;
        }
    }

    if (ch == KBD_MAX_CHANNELS) {
        return;
    }

    kbdChannels[ch].id = KBD_ID_NONE;
    kbdChannels[ch].status |= KBD_CS_DISCONNECTED;

    if (kbdDetachCallback != NULL) {
        ev.channel = ch;
        kbdDetachCallback(&ev);
    }

    KBDResetChannel(ch);
}

static void kbdEventHandler(USBKBDId id, USBKBDReport* report) {
    KBDChannel ch;
    KBDChanInfo* info;
    s8 i;
    s8 j;
    int bit;

    for (ch = 0; ch < KBD_MAX_CHANNELS; ch++) {
        if (kbdChannels[ch].id == id) {
            break;
        }
    }

    if (ch == KBD_MAX_CHANNELS) {
        return;
    }

    info = &kbdChannels[ch];

    if (report->keycode[0] == KBD_HID_Roll_Over || report->keycode[0] == KBD_HID_Post_Fail ||
        report->keycode[0] == KBD_HID_Undefined_Error) {
        OSCancelAlarm(&info->repeat_alarm);
        if (report->keycode[0] == KBD_HID_Roll_Over) {
            info->status |= KBD_CS_ROLL_OVER;
        } else {
            info->status |= KBD_CS_ERROR;
        }
        return;
    }

    info->status = KBD_CS_OK;
    VIResetDimmingCount();

    for (bit = 1, i = 0; i < KBD_NUM_MODIFIER_KEYS; i++, bit <<= 1) {
        if (info->last_report.modifier & bit) {
            if (!(report->modifier & bit)) {
                kbdProcKey(KBD_HID_Left_Control + i, KBD_KM_NULL, ch);
            }
        } else if (report->modifier & bit) {
            kbdProcKey(KBD_HID_Left_Control + i, KBD_KM_DOWN, ch);
        }
    }

    for (i = 0; i < USB_KBD_NUM_HID_KEY_CODES; i++) {
        if (info->last_report.keycode[i] != KBD_HID_NONE) {
            for (j = 0; j < USB_KBD_NUM_HID_KEY_CODES; j++) {
                if (info->last_report.keycode[i] == report->keycode[j]) {
                    break;
                }
            }

            if (j == USB_KBD_NUM_HID_KEY_CODES) {
                kbdProcKey(info->last_report.keycode[i], KBD_KM_NULL, ch);
                info->last_report.keycode[i] = KBD_HID_NONE;
            }
        }
    }

    for (i = 0; i < USB_KBD_NUM_HID_KEY_CODES; i++) {
        if (report->keycode[i] != KBD_HID_NONE) {
            for (j = 0; j < USB_KBD_NUM_HID_KEY_CODES; j++) {
                if (report->keycode[i] == info->last_report.keycode[j]) {
                    break;
                }
            }

            if (j == USB_KBD_NUM_HID_KEY_CODES) {
                kbdProcKey(report->keycode[i], KBD_KM_DOWN, ch);
            }
        }
    }

    info->last_report.modifier = report->modifier;
    for (i = 0; i < USB_KBD_NUM_HID_KEY_CODES; i++) {
        info->last_report.keycode[i] = report->keycode[i];
    }
}

static void kbdRepeatHandler(OSAlarm* alarm, OSContext* context) {
    KBDChanInfo* info = OSGetAlarmUserData(alarm);

    kbdProcKey(info->repeat_hid, KBD_KM_DOWN | KBD_KM_REPEAT, info->channel);
}

static void kbdProcMod(KBDUnicode unicode, KBDKeyMode mode, KBDChannel ch);
static void kbdSendKey(KBDKeyEvent* ev);

static void kbdProcKey(KBDHidCode hid, KBDKeyMode mode, KBDChannel ch) {
    KBDKeyEvent ev;
    KBDChanInfo* info = &kbdChannels[ch];
    KBDMap* map;
    KBDUnicode unicode;
    int i;
    u32 j;
    OSTime ticks;

    ev.channel = ch;
    ev.mode = mode;
    KBDGetModState(ch, &ev.modState);

    map = &kbdMaps[info->country];
    for (i = 0; i < map->num_hid_remaps; i++) {
        if (hid == map->hid_remap[i][0]) {
            hid = map->hid_remap[i][1];
            break;
        }
    }

    ev.hid = hid;
    unicode = KBDTranslateHidCode(hid, ev.modState, info->country);
    ev.unicode = unicode;

    if (unicode == KBK_Mod_Control || unicode == KBK_Mod_Shift || unicode == KBK_Mod_Alt || unicode == KBK_Mod_GUI ||
        unicode == KBK_Mod_Extra || unicode == KBK_Mod_AltGr || unicode == KBK_Lang_Toggle || unicode == KBK_Num_Lock ||
        unicode == KBK_Caps_Lock || unicode == KBK_Scroll_Lock) {
        kbdProcMod(unicode, mode, ch);
        if (unicode != KBK_Lang_Toggle && unicode != KBK_Num_Lock && unicode != KBK_Caps_Lock && unicode != KBK_Scroll_Lock) {
            kbdSendKey(&ev);
            return;
        }
    }

    if (!(mode & KBD_KM_DOWN)) {
        if (hid == info->repeat_hid) {
            OSCancelAlarm(&info->repeat_alarm);
        }
        kbdSendKey(&ev);
        return;
    }

    OSCancelAlarm(&info->repeat_alarm);

    for (j = 0; j < KBD_NUM_NO_REPEAT_KEYS; j++) {
        if (unicode == noRepeatKeys[j]) {
            break;
        }
    }

    if (j == KBD_NUM_NO_REPEAT_KEYS) {
        info->repeat_hid = hid;
        if (info->repeat_delay != 0) {
            if (mode & KBD_KM_REPEAT) {
                ticks = OSMillisecondsToTicks(info->repeat_interval);
            } else {
                ticks = OSMillisecondsToTicks(info->repeat_delay);
            }
            OSSetAlarm(&info->repeat_alarm, ticks, kbdRepeatHandler);
        }
    }

    ev.unicode = unicode;
    kbdSendKey(&ev);
}

static void kbdProcMod(KBDUnicode unicode, KBDKeyMode mode, KBDChannel ch) {
    KBDChanInfo* info = &kbdChannels[ch];
    BOOL down = mode & KBD_KM_DOWN;
    s8 delta = down ? 1 : -1;
    u8 flags = kbdMaps[info->country].flags;
    KBDModState ms;

    KBDGetModState(ch, &ms);

    switch (unicode) {
        case KBK_Mod_Shift:
            if (info->access_sticky) {
                if (down) {
                    ms ^= KBD_MS_SHIFT;
                }
            } else {
                info->shift_count += delta;
                ms = (ms & ~KBD_MS_SHIFT) | (KBD_MS_SHIFT * (info->shift_count != 0));
            }
            break;
        case KBK_Mod_AltGr:
            if (info->access_sticky) {
                if (down) {
                    ms ^= KBD_MS_ALTGR;
                }
            } else {
                info->altgr_count += delta;
                ms = (ms & ~KBD_MS_ALTGR) | (KBD_MS_ALTGR * (info->altgr_count != 0));
            }
            break;
        case KBK_Mod_Control:
            if (info->access_sticky) {
                if (down) {
                    ms ^= KBD_MS_CTRL;
                }
            } else {
                info->ctrl_count += delta;
                ms = (ms & ~KBD_MS_CTRL) | (KBD_MS_CTRL * (info->ctrl_count != 0));
            }
            break;
        case KBK_Caps_Lock:
            if (down && info->lock_processing) {
                ms ^= KBD_MS_CAPS_LOCK;
            }
            break;
        case KBK_Num_Lock:
            if (down && info->lock_processing) {
                ms ^= KBD_MS_NUM_LOCK;
            }
            break;
        case KBK_Lang_Toggle:
            if (down && info->lock_processing) {
                switch (ms & (KBD_MS_LANG1 | KBD_MS_LANG2)) {
                    case KBD_MS_NULL:
                        if ((flags & KBD_MAP_LANG1) == KBD_MAP_LANG1) {
                            ms |= KBD_MS_LANG1;
                        }
                        break;
                    case KBD_MS_LANG1:
                        ms &= ~KBD_MS_LANG1;
                        if ((flags & KBD_MAP_LANG2) == KBD_MAP_LANG2) {
                            ms |= KBD_MS_LANG2;
                        }
                        break;
                    case KBD_MS_LANG2:
                        ms &= ~KBD_MS_LANG2;
                        break;
                    case KBD_MS_LANG1 | KBD_MS_LANG2:
                        ms &= ~(KBD_MS_LANG1 | KBD_MS_LANG2);
                        break;
                }
            }
            break;
        case KBK_Mod_Alt:
            if (info->access_sticky) {
                if (down) {
                    ms ^= KBD_MS_ALT;
                }
            } else {
                info->alt_count += delta;
                ms = (ms & ~KBD_MS_ALT) | (KBD_MS_ALT * (info->alt_count != 0));
            }
            break;
        case KBK_Mod_GUI:
            if (info->access_sticky) {
                if (down) {
                    ms ^= KBD_MS_GUI;
                }
            } else {
                info->gui_count += delta;
                ms = (ms & ~KBD_MS_GUI) | (KBD_MS_GUI * (info->gui_count != 0));
            }
            break;
        case KBK_Mod_Extra:
            if (info->access_sticky) {
                if (down) {
                    ms ^= KBD_MS_EXTRA;
                }
            } else {
                info->extra_count += delta;
                ms = (ms & ~KBD_MS_EXTRA) | (KBD_MS_EXTRA * (info->extra_count != 0));
            }
            break;
        case KBK_Scroll_Lock:
            if (down && info->lock_processing) {
                ms ^= KBD_MS_SCROLL_LOCK;
            }
            break;
    }

    KBDSetModState(ch, ms | KBD_MS_SET_ALL);
}

static void kbdSendKey(KBDKeyEvent* ev) {
    KBDChanInfo* info;
    u8 next;

    if (kbdKeyCallback != NULL) {
        kbdKeyCallback(ev);
    }

    info = &kbdChannels[ev->channel];
    if (info->queue_write == info->queue_read) {
        return;
    }

    next = (info->queue_write + 1) % KBD_QUEUE_SIZE;
    if (next == info->queue_read) {
        info->queue[info->queue_write].channel = ev->channel;
        info->queue[info->queue_write].hid = KBD_HID_OVERFLOW;
        info->queue[info->queue_write].mode = KBD_KM_NULL;
        info->queue[info->queue_write].modState = KBD_MS_NULL;
        info->queue[info->queue_write].unicode = KBK_Void;
    } else {
        info->queue[info->queue_write] = *ev;
        if (info->queue_read == KBD_QUEUE_EMPTY) {
            info->queue_read = info->queue_write;
        }
    }

    info->queue_write = next;
}

static void kbd_led_handler(BOOL success, void* cb_arg) {
    u32 i = (u32)cb_arg;

    kbdLedCmds[i].id = KBD_ID_NONE;
    if (success == TRUE) {
        kbdChannels[kbdLedRequests[i].ev.channel].leds = kbdLedRequests[i].ev.leds;
    }

    if (kbdLedRequests[i].cb == NULL) {
        return;
    }

    kbdLedRequests[i].ev.rc = success == TRUE ? KBD_SUCCESS : KBD_ERR_BUSY;
    kbdLedRequests[i].cb(&kbdLedRequests[i].ev, kbdLedRequests[i].cb_arg);
}

KBDEc KBDSetLedsAsync(KBDChannel ch, KBDLedState leds, KBDLedCallbackF cb, void* cb_arg) {
    BOOL enabled;
    u8 state;
    u32 i;

    if (!kbdInitialized) {
        return KBD_ERR_NOT_INITIALIZED;
    }

    if (ch >= KBD_MAX_CHANNELS) {
        return KBD_ERR_BAD_VALUE;
    }

    if (kbdChannels[ch].status == KBD_CS_DISCONNECTED || kbdChannels[ch].status == KBD_CS_ERROR) {
        return KBD_ERR_BAD_STATUS;
    }

    state = leds;
    enabled = OSDisableInterrupts();
    for (i = 0; i < KBD_NUM_LED_CMDS; i++) {
        if (kbdLedCmds[i].id == KBD_ID_NONE) {
            kbdLedCmds[i].id = kbdChannels[ch].id;
            break;
        }
    }
    OSRestoreInterrupts(enabled);

    if (i == KBD_NUM_LED_CMDS) {
        return KBD_ERR_BUSY;
    }

    kbdLedRequests[i].ev.channel = ch;
    kbdLedRequests[i].ev.leds = leds;
    kbdLedRequests[i].ev.rc = KBD_ERR_OTHER;
    kbdLedRequests[i].cb = cb;
    kbdLedRequests[i].cb_arg = cb_arg;

    if (USBKBDSetLEDAsync(kbdChannels[ch].id, state, &kbdLedCmds[i], kbd_led_handler, (void*)i) != USB_KBD_SUCCESS) {
        return KBD_ERR_BUSY;
    }

    return KBD_SUCCESS;
}

KBDEc KBDSetLeds(KBDChannel ch, KBDLedState leds) {
    BOOL enabled;
    USBKBDEc rc;
    u8 state;
    u32 i;

    if (!kbdInitialized) {
        return KBD_ERR_NOT_INITIALIZED;
    }

    if (ch >= KBD_MAX_CHANNELS) {
        return KBD_ERR_BAD_VALUE;
    }

    if (kbdChannels[ch].status == KBD_CS_DISCONNECTED || kbdChannels[ch].status == KBD_CS_ERROR) {
        return KBD_ERR_BAD_STATUS;
    }

    state = leds;
    enabled = OSDisableInterrupts();
    for (i = 0; i < KBD_NUM_LED_CMDS; i++) {
        if (kbdLedCmds[i].id == KBD_ID_NONE) {
            kbdLedCmds[i].id = kbdChannels[ch].id;
            break;
        }
    }
    OSRestoreInterrupts(enabled);

    if (i == KBD_NUM_LED_CMDS) {
        return KBD_ERR_BUSY;
    }

    rc = USBKBDSetLED(kbdChannels[ch].id, state, &kbdLedCmds[i]);
    kbdLedCmds[i].id = KBD_ID_NONE;
    if (rc != USB_KBD_SUCCESS) {
        return KBD_ERR_BUSY;
    }

    kbdChannels[ch].leds = leds;
    return KBD_SUCCESS;
}

void kbdInitMap(KBDCountryCode country, u8 num_cols, u8 flags, KBDUnicode* key_map, KBDUnicode* mod_map, u8 lang1_col,
                u8 lang2_col, u8 altgr_col, u8 num_keys, KBDHidRemap remap) {
    KBDMap* map = &kbdMaps[country];

    map->num_cols = num_cols;
    map->flags = flags;
    map->key_map = key_map;
    map->mod_map = mod_map;
    map->lang1_col = lang1_col;
    map->lang2_col = lang2_col;
    map->altgr_col = altgr_col;
    map->num_keys = num_keys;

    switch (remap) {
        case KBD_HID_REMAP_ANSI:
            map->hid_remap[0][0] = KBD_HID_NonUS_Pound_Tilde;
            map->hid_remap[0][1] = KBD_HID_Backslash_Bar;
            map->num_hid_remaps = 1;
            break;
        case KBD_HID_REMAP_JIS:
            map->hid_remap[0][0] = KBD_HID_Backslash_Bar;
            map->hid_remap[0][1] = KBD_HID_NonUS_Pound_Tilde;
            map->hid_remap[1][0] = KBD_HID_L5_Zenkaku_Hankaku;
            map->hid_remap[1][1] = KBD_HID_Grave_Tilde;
            map->num_hid_remaps = 2;
            break;
        case KBD_HID_REMAP_ISO:
            map->hid_remap[0][0] = KBD_HID_Backslash_Bar;
            map->hid_remap[0][1] = KBD_HID_NonUS_Pound_Tilde;
            map->num_hid_remaps = 1;
            break;
    }
}

void kbdInitMapIntl(void) {
    kbdInitMap(KBD_CC_INTERNATIONAL, KBD_COL_ALTGR_SHIFT + 1, KBD_MAP_ALTGR | KBD_MAP_ALTGR_SHIFT, kbdKeyMapUS_International,
               kbdModifierMapUS_International, KBD_COL_NONE, KBD_COL_NONE, KBD_COL_ALTGR, KBD_MAP_NUM_KEYS, KBD_HID_REMAP_ANSI);
}

static BOOL kbdShutdown(BOOL final, u32 event) {
    static u8 attempts;
    KBDChannel ch;
    OSTime start;

    if (!kbdInitialized) {
        return TRUE;
    }

    if (attempts++ <= KBD_SHUTDOWN_MAX_ATTEMPTS && !final) {
        for (ch = 0; ch < KBD_MAX_CHANNELS; ch++) {
            if (!(kbdChannels[ch].status & KBD_CS_DISCONNECTED) && kbdChannels[ch].leds != KBD_LED_NULL) {
                KBDSetLedsAsync(ch, KBD_LED_NULL, NULL, NULL);
                start = OSGetTime();
                while (OSTicksToMicroseconds(OSGetTime() - start) < KBD_SHUTDOWN_LED_WAIT_US) {
                }
            }
        }
    }

    attempts = 0;
    return TRUE;
}

KBDEc KBDInit(void) {
    KBDChannel ch;
    KBDChanInfo* info;

    if (kbdInitialized) {
        return KBD_ERR_ALREADY_INITIALIZED;
    }

    OSRegisterVersion(__KBDVersion);
    kbdInitialized = TRUE;

    if (kbdShutdownInfo.func == NULL) {
        kbdShutdownInfo.func = kbdShutdown;
        OSRegisterShutdownFunction(&kbdShutdownInfo);
    }

    for (ch = 0; ch < KBD_MAX_CHANNELS; ch++) {
        info = &kbdChannels[ch];
        info->channel = ch;
        info->country = KBD_CC_UNITED_STATES;
        info->status = KBD_CS_DISCONNECTED;
        OSCreateAlarm(&info->led_alarm);
        OSSetAlarmUserData(&info->led_alarm, info);
        info->id = KBD_ID_NONE;
        info->lock_processing = TRUE;
        info->repeat_delay = KBD_DEFAULT_REPEAT_DELAY;
        info->repeat_interval = KBD_DEFAULT_REPEAT_INTERVAL;
        info->access_sticky = FALSE;
        OSCreateAlarm(&info->repeat_alarm);
        OSSetAlarmUserData(&info->repeat_alarm, info);
        KBDResetChannel(ch);
    }

    if (USBKBDInitialize(kbdAttachHandler, kbdDetachHandler) != USB_KBD_SUCCESS) {
        return KBD_ERR_OTHER;
    }

    return USBKBDRegisterEventCallback(kbdEventHandler) != USB_KBD_SUCCESS ? KBD_ERR_OTHER : KBD_SUCCESS;
}

KBDDevCallbackF KBDSetAttachCallback(KBDDevCallbackF callback) {
    KBDDevCallbackF old = kbdAttachCallback;

    kbdAttachCallback = callback;
    return old;
}

KBDDevCallbackF KBDSetDetachCallback(KBDDevCallbackF callback) {
    KBDDevCallbackF old = kbdDetachCallback;

    kbdDetachCallback = callback;
    return old;
}

KBDKeyCallbackF KBDSetKeyCallback(KBDKeyCallbackF callback) {
    KBDKeyCallbackF old = kbdKeyCallback;

    kbdKeyCallback = callback;
    return old;
}

KBDEc KBDResetChannel(KBDChannel ch) {
    KBDChanInfo* info;
    int i;

    if (!kbdInitialized) {
        return KBD_ERR_NOT_INITIALIZED;
    }

    if (ch >= KBD_MAX_CHANNELS) {
        return KBD_ERR_BAD_VALUE;
    }

    info = &kbdChannels[ch];
    info->last_report.modifier = 0;
    for (i = 0; i < USB_KBD_NUM_HID_KEY_CODES; i++) {
        info->last_report.keycode[i] = KBD_HID_NONE;
    }
    info->queue_read = KBD_QUEUE_EMPTY;
    KBDSetModState(ch, KBD_MS_SET_ALL);
    info->leds = KBD_LED_RESERVED;
    info->retry_leds = KBD_LED_RESERVED;
    info->ctrl_count = 0;
    info->shift_count = 0;
    info->alt_count = 0;
    info->gui_count = 0;
    info->altgr_count = 0;
    info->extra_count = 0;
    info->repeat_hid = KBD_HID_NONE;
    OSCancelAlarm(&info->repeat_alarm);
    return KBD_SUCCESS;
}

KBDEc KBDSetCountry(KBDChannel ch, KBDCountryCode country) {
    if (!kbdInitialized) {
        return KBD_ERR_NOT_INITIALIZED;
    }

    if (ch >= KBD_MAX_CHANNELS) {
        return KBD_ERR_BAD_VALUE;
    }

    if (country >= KBD_NUM_COUNTRY_CODES) {
        return KBD_ERR_BAD_VALUE;
    }

    if (kbdMaps[country].num_cols == 0) {
        return KBD_ERR_BAD_VALUE;
    }

    kbdChannels[ch].country = country;
    KBDResetChannel(ch);
    return KBD_SUCCESS;
}

KBDEc KBDSetLockProcessing(KBDChannel ch, BOOL enable) {
    if (!kbdInitialized) {
        return KBD_ERR_NOT_INITIALIZED;
    }

    if (ch >= KBD_MAX_CHANNELS) {
        return KBD_ERR_BAD_VALUE;
    }

    kbdChannels[ch].lock_processing = enable;
    return KBD_SUCCESS;
}

KBDEc KBDSetModState(KBDChannel ch, KBDModState modState) {
    BOOL enabled;
    KBDModState state;

    if (!kbdInitialized) {
        return KBD_ERR_NOT_INITIALIZED;
    }

    if (ch >= KBD_MAX_CHANNELS) {
        return KBD_ERR_BAD_VALUE;
    }

    if (modState & KBD_MS_SET_ALL) {
        kbdChannels[ch].mod_state = modState & ~KBD_MS_SET_ALL;
    } else {
        enabled = OSDisableInterrupts();
        state = kbdChannels[ch].mod_state;
        kbdChannels[ch].mod_state = (state & KBD_MS_TRANSIENTS) | (modState & (KBD_MS_TOGGLES | KBD_MS_SHIFTED_KEY));
        OSRestoreInterrupts(enabled);
    }

    return KBD_SUCCESS;
}

KBDEc KBDGetModState(KBDChannel ch, KBDModState* modState) {
    if (!kbdInitialized) {
        return KBD_ERR_NOT_INITIALIZED;
    }

    if (ch >= KBD_MAX_CHANNELS) {
        return KBD_ERR_BAD_VALUE;
    }

    if (modState != NULL) {
        *modState = kbdChannels[ch].mod_state;
    }

    return KBD_SUCCESS;
}

KBDUnicode KBDTranslateHidCode(KBDHidCode hid, KBDModState modState, KBDCountryCode country) {
    KBDMap* map;
    KBDUnicode* table;
    int type;
    u16 lock_mask;
    KBDUnicode entry;
    u8 idx;
    int shift;
    int col;
    int mask;

    if (!kbdInitialized) {
        return KBK_Error;
    }

    if (country >= KBD_NUM_COUNTRY_CODES) {
        return KBK_Error;
    }

    map = &kbdMaps[country];
    if (map->num_cols == 0) {
        return KBK_Void;
    }

    if (hid < KBD_HID_A) {
        return KBK_Error;
    }

    if (hid < map->num_keys + KBD_HID_A) {
        idx = hid - KBD_HID_A;
        table = map->key_map;
    } else if (hid >= KBD_HID_Left_Control && hid <= KBD_HID_Right_GUI) {
        idx = hid - KBD_HID_Left_Control;
        table = map->mod_map;
    } else {
        return KBK_Void;
    }

    entry = table[idx * map->num_cols];
    type = entry & KBD_KEY_TYPE_MASK;

    if (type == KBD_KEY_TYPE_FIXED) {
        col = KBD_COL_BASE;
        shift = 0;
    } else {
        if ((modState & KBD_MS_ALTGR) || (modState & (KBD_MS_CTRL | KBD_MS_ALT)) == (KBD_MS_CTRL | KBD_MS_ALT)) {
            col = map->altgr_col;
            mask = KBD_LOCK_ALTGR;
        } else if (modState & KBD_MS_LANG1) {
            col = map->lang1_col;
            mask = KBD_LOCK_LANG1;
        } else if (modState & KBD_MS_LANG2) {
            col = map->lang2_col;
            mask = KBD_LOCK_LANG2;
        } else {
            col = KBD_COL_BASE;
            mask = KBD_LOCK_BASE;
        }

        if (!(modState & KBD_MS_SHIFTED_KEY) && (modState & (KBD_MS_CTRL | KBD_MS_ALT | KBD_MS_GUI | KBD_MS_EXTRA))) {
            shift = 0;
        } else {
            shift = (modState & KBD_MS_SHIFT) ? 1 : 0;
            lock_mask = mask << shift;
            if (type == KBD_KEY_TYPE_CAPS && (entry & lock_mask)) {
                shift ^= (modState & KBD_MS_CAPS_LOCK) ? 1 : 0;
            } else if (type == KBD_KEY_TYPE_NUMPAD && (entry & lock_mask)) {
                shift = (modState & KBD_MS_NUM_LOCK) == KBD_MS_NUM_LOCK && shift == 0;
            } else if (type == KBD_KEY_TYPE_LANG && col == KBD_COL_BASE && (modState & KBD_MS_CAPS_LOCK)) {
                col = map->lang1_col;
            }
        }

        if (!shift) {
            if (col == map->altgr_col && !(map->flags & KBD_MAP_ALTGR)) {
                return KBK_Void;
            }
            if (col == map->lang1_col && !(map->flags & KBD_MAP_LANG1)) {
                return KBK_Void;
            }
            if (col == map->lang2_col && !(map->flags & KBD_MAP_LANG2)) {
                return KBK_Void;
            }
        } else {
            if (col == map->altgr_col && !(map->flags & KBD_MAP_ALTGR_SHIFT)) {
                return KBK_Void;
            }
            if (col == map->lang1_col && !(map->flags & KBD_MAP_LANG1_SHIFT)) {
                return KBK_Void;
            }
            if (col == map->lang2_col && !(map->flags & KBD_MAP_LANG2_SHIFT)) {
                return KBK_Void;
            }
        }
    }

    return table[idx * map->num_cols + col + shift];
}
