#ifndef PRIVATE_KBD_H
#define PRIVATE_KBD_H

#include <revolution/kbd.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KBD_KEY_TYPE_MASK 0xC000
#define KBD_KEY_TYPE_CAPS 0x0000
#define KBD_KEY_TYPE_LANG 0x4000
#define KBD_KEY_TYPE_NUMPAD 0x8000
#define KBD_KEY_TYPE_FIXED 0xC000

#define KBD_LOCK_BASE 0x01
#define KBD_LOCK_LANG1 0x04
#define KBD_LOCK_LANG2 0x10
#define KBD_LOCK_ALTGR 0x40

#define KBD_MAP_NONE 0x00
#define KBD_MAP_LANG1 0x01
#define KBD_MAP_LANG1_SHIFT 0x02
#define KBD_MAP_LANG2 0x04
#define KBD_MAP_LANG2_SHIFT 0x08
#define KBD_MAP_ALTGR 0x10
#define KBD_MAP_ALTGR_SHIFT 0x20

#define KBD_MAP_NUM_KEYS 100

typedef enum {
    KBD_COL_NONE,
    KBD_COL_BASE,
    KBD_COL_BASE_SHIFT,
    KBD_COL_ALTGR,
    KBD_COL_ALTGR_SHIFT
} KBDMapColumn;

typedef enum {
    KBD_HID_REMAP_ANSI,
    KBD_HID_REMAP_JIS,
    KBD_HID_REMAP_ISO
} KBDHidRemap;

void kbdInitMap(KBDCountryCode country, u8 num_cols, u8 flags, KBDUnicode* key_map, KBDUnicode* mod_map, u8 lang1_col,
                u8 lang2_col, u8 altgr_col, u8 num_keys, KBDHidRemap remap);
void kbdInitMapIntl(void);

#ifdef __cplusplus
}
#endif

#endif // PRIVATE_KBD_H
