#ifndef DWC_SUPPORT_REVOLUTION_NCD_NCDTYPES_H
#define DWC_SUPPORT_REVOLUTION_NCD_NCDTYPES_H

#include <revolution/types.h>

typedef s32 NCDErr;

enum {
    NCD_RESULT_SUCCESS = 0,
    NCD_RESULT_INPROGRESS = -8,
};

typedef struct NCDWiredProfile {
    u8 reserved[4]; // 0x00
} NCDWiredProfile;

typedef struct NCDPrivacy {
    u16 mode;       // 0x00
    u8 reserved[2]; // 0x02
    union {
        struct {
            u16 keyId;
            u16 option;
            u8 key[4][5];
        } wep40;
        struct {
            u16 keyId;
            u16 option;
            u8 key[4][13];
        } wep104;
        struct {
            u16 keyLen;
            u8 reserved[2];
            u8 key[64];
        } tkip;
        struct {
            u16 keyLen;
            u8 reserved[2];
            u8 key[64];
        } aes;
    }; // 0x04
} PACKED NCDPrivacy;

typedef struct NCDApConfig {
    u8 ssid[32];        // 0x00
    u16 ssidLength;     // 0x20
    u8 reserved[2];     // 0x22
    NCDPrivacy privacy; // 0x24
} PACKED NCDApConfig;

typedef struct NCDUsbapConfig {
    u16 nickname[11]; // 0x00
    u8 reserved[2];   // 0x16
} PACKED NCDUsbapConfig;

typedef struct NCDAossConfig {
    struct {
        u8 ssid[32];
        u16 ssidLength;
        u16 keyId;
        u8 key[4][5];
    } wep40; // 0x00
    struct {
        u8 ssid[32];
        u16 ssidLength;
        u16 keyId;
        u8 key[4][13];
    } wep104; // 0x38
    struct {
        u8 ssid[32];
        u16 ssidLength;
        u16 keyLen;
        u8 key[64];
    } tkip; // 0x90
    struct {
        u8 ssid[32];
        u16 ssidLength;
        u16 keyLen;
        u8 key[64];
    } aes; // 0xF4
} PACKED NCDAossConfig;

typedef struct NCDWirelessProfile {
    u16 rateset;     // 0x00
    u8 configMethod; // 0x02
    u8 retryLimit;   // 0x03
    union {
        NCDApConfig manual;
        NCDUsbapConfig usbap;
        NCDAossConfig aoss;
        NCDApConfig rakuraku;
    } config; // 0x04
} PACKED NCDWirelessProfile;

typedef struct NCDIfConfig {
    u8 selectedMedia; // 0x00
    u8 linkTimeout;   // 0x01
    union {
        NCDWiredProfile wired;
        NCDWirelessProfile wireless;
    } netif; // 0x02
} NCDIfConfig;

#endif
