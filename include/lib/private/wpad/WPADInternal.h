#ifndef RVL_SDK_WPAD_INTERNAL_H
#define RVL_SDK_WPAD_INTERNAL_H

#include <revolution/os.h>

#include <revolution/wpad.h>

#include <private/wpad/WPADHIDParser.h>
#include <private/wpad/WPADMem.h>

#ifdef __cplusplus
extern "C" {
#endif

void DEBUGPrint(const char* format, ...);

#define WPAD_WRITE_LEN_MASK (u16)(0x001F)

#define RX_BUFFER_SIZE ((int)OSRoundUp32B(sizeof(WPADStatusEx)))

#define WPAD_CONFIG_BLOCK_CHECKSUM_BIAS 0x55
#define WPAD_MAX_CONFIG_BLOCKS 2

#define WPAD_DPD_CONFIG_BLOCK_CHECKSUM 10
#define WPAD_DPD_CONFIG_BLOCK_SIZE 11

#define WPAD_DEV_CONFIG_BLOCK_CHECKSUM 9
#define WPAD_DEV_CONFIG_BLOCK_SIZE 10

#define DPD_CFG_OBJ_BL 0
#define DPD_CFG_OBJ_BR 1
#define DPD_CFG_OBJ_TR 2
#define DPD_CFG_OBJ_TL 3

/*
 * Wiimote memory addresses
 */
#define WM_ADDR_MEM_DEV_CONFIG_0 WM_MEM_ADDR(0x0000)
#define WM_ADDR_MEM_GAME_INFO_0 WM_MEM_ADDR(0x002A)
#define WM_ADDR_MEM_GAME_INFO_1 WM_MEM_ADDR(0x0062)
#define WM_ADDR_MEM_176C WM_MEM_ADDR(0x176C)
#define WM_ADDR_MEM_1770 WM_MEM_ADDR(0x1770)

/*
 * Wiimote extension register addresses
 */
// Speaker register addresses
#define WM_REG_SPEAKER_01 WM_EXT_REG_ADDR(SPEAKER, 0x01)
#define WM_REG_SPEAKER_08 WM_EXT_REG_ADDR(SPEAKER, 0x08)
#define WM_REG_SPEAKER_09 WM_EXT_REG_ADDR(SPEAKER, 0x09)

// Extension register addresses
#define WM_REG_EXTENSION_CONFIG WM_EXT_REG_ADDR(EXTENSION, 0x20)
#define WM_REG_EXTENSION_40 WM_EXT_REG_ADDR(EXTENSION, 0x40)
#define WM_REG_EXTENSION_CERT_PARAM WM_EXT_REG_ADDR(EXTENSION, 0x50)
#define WM_REG_EXTENSION_F0 WM_EXT_REG_ADDR(EXTENSION, 0xF0)
#define WM_REG_EXTENSION_CERT_CHALLENGE WM_EXT_REG_ADDR(EXTENSION, 0xF1)
#define WM_REG_EXTENSION_F2 WM_EXT_REG_ADDR(EXTENSION, 0xF2)
#define WM_REG_EXTENSION_F3 WM_EXT_REG_ADDR(EXTENSION, 0xF3)
#define WM_REG_EXTENSION_EXT_TYPE_2 WM_EXT_REG_ADDR(EXTENSION, 0xF6)
#define WM_REG_EXTENSION_CERT_PROBE WM_EXT_REG_ADDR(EXTENSION, 0xF7)
#define WM_REG_EXTENSION_FA WM_EXT_REG_ADDR(EXTENSION, 0xFA)
#define WM_REG_EXTENSION_FB WM_EXT_REG_ADDR(EXTENSION, 0xFB)
#define WM_REG_EXTENSION_DEV_MODE WM_EXT_REG_ADDR(EXTENSION, 0xFE)
#define WM_REG_EXTENSION_ID_BYTE WM_EXT_REG_ADDR(EXTENSION, 0xFF)

// IR camera register addresses
#define WM_REG_DPD_CONFIG_BLOCK_1 WM_EXT_REG_ADDR(DPD, 0x00)
#define WM_REG_DPD_CONFIG_BLOCK_2 WM_EXT_REG_ADDR(DPD, 0x1A)
#define WM_REG_DPD_30 WM_EXT_REG_ADDR(DPD, 0x30)
#define WM_REG_DPD_DATA_FORMAT WM_EXT_REG_ADDR(DPD, 0x33)

#define WM_EXTENSION_CONFIG_SIZE 0x20  // 0xA40020 - 0xA4003F

// Speaker register values
#define SPK_CLK_ON 1
#define SPK_CLK_OFF 0
#define SPK_DERESET 0x01
#define SPK_RESET 0x80
#define SPK_CTRL_PLAY 1

typedef enum {
    WPAD_STATE_EXT_UNINITIALIZED,
    WPAD_STATE_EXT_INITIALIZED,
    WPAD_STATE_EXT_ENCRYPTED,
    WPAD_STATE_EXT_ENCRYPTED_3RD,
} WPADExtensionState;

typedef enum {
    WPAD_RADIO_QUALITY_GOOD,  // 80+
    WPAD_RADIO_QUALITY_BAD    // 80-
    // if radioQuality calculation == 80, state stays the same
} WPADRadioQuality;

typedef enum {
    WPAD_ILBUF_NONE = 0,

    WPAD_ILBUF_BUF3E = (1 << 0),
    WPAD_ILBUF_BUF3F = (1 << 1),

    WPAD_ILBUF_ALL = WPAD_ILBUF_BUF3E | WPAD_ILBUF_BUF3F,
} WPADInterleaveBufferFlags;

#define WPAD_COMMAND_CMD_MAX_LEN 24
#define WPAD_COMMAND_EXT_MAX_LEN 12

typedef struct WPADCommand {
    u32 command;  // 0x00
    u8 data[22];  // 0x04
    u16 len;      // 0x1A

    void* readBuf;  // 0x1C

    u16 readLen;   // 0x20
    u32 readAddr;  // 0x24

    WPADInfo* info;       // 0x28
    WPADCallback callback;  // 0x2C
} WPADCommand;

typedef struct WPADCommandQueue {
    s8 head;            // 0x00
    s8 tail;            // 0x01
    WPADCommand* cmd;   // 0x04
    u32 cmdlen;         // 0x08
} WPADCommandQueue;

typedef struct WPADDevConfig {
    DPDObject obj[WPAD_MAX_DPD_OBJECTS];  // 0x00

    WPADAccGravityUnit acc_0g;  // 0x20
    WPADAccGravityUnit acc_1g;  // 0x26

    u8 motor;   // 0x2C
    u8 volume;  // 0x2D
} WPADDevConfig;

typedef struct WPADStick {
    s16 x;      // 0x00
    s16 x_min;  // 0x02
    s16 x_max;  // 0x04
    s16 y;      // 0x06
    s16 y_min;  // 0x08
    s16 y_max;  // 0x0A
} WPADStick;

typedef struct WPADFsConfig {
    WPADStick stick;            // 0x00
    WPADAccGravityUnit acc_0g;  // 0x0C
    WPADAccGravityUnit acc_1g;  // 0x12
} WPADFsConfig;

typedef struct WPADClConfig {
    WPADStick lstk;  // 0x00
    WPADStick rstk;  // 0x0C
    u8 triggerL;     // 0x18
    u8 triggerR;     // 0x19
} WPADClConfig;

typedef struct WPADExtConfig {
    union {
        WPADFsConfig fs;
        WPADClConfig cl;
    };
} WPADExtConfig;

typedef struct WPADCB {
    WPADGameInfo gameInfo;  // 0x00
    s32 gameInfoErr[2];     // 0x38
    u8 prBuf[RX_BUFFER_SIZE] ATTRIBUTE_ALIGN(32);       // 0x40
    u8 rxBuf[RX_BUFFER_SIZE * 2] ATTRIBUTE_ALIGN(32);   // 0xA0
    WPADCommandQueue cmdq;                              // 0x160
    WPADCommand cmds[WPAD_COMMAND_CMD_MAX_LEN];         // 0x16C
    WPADCommandQueue extq;                              // 0x5EC
    WPADCommand exts[WPAD_COMMAND_EXT_MAX_LEN];         // 0x5F8
    WPADInfo infos;                                     // 0x838
    WPADInfo* infoBuf;                                  // 0x850
    WPADDevConfig conf;                                 // 0x854
    WPADExtConfig extConf;                              // 0x882
    WPADCallback resultCallback;                        // 0x89C
    WPADExtensionCallback extensionCallback;            // 0x8A0
    WPADConnectCallback connectCallback;                // 0x8A4
    WPADSamplingCallback sampleCallback;                // 0x8A8
    void* samplingBufs_ptr;                             // 0x8AC
    u32 bufLatestIndex;                                 // 0x8B0
    u32 bufLength;                                      // 0x8B4
    u32 dataFormat;                                     // 0x8B8
    s32 status;                                         // 0x8BC
    u8 getStatFlag;                                     // 0x8C0
    u8 devType;                                         // 0x8C1
    u8 devMode;                                         // 0x8C2
    s8 dev_handle;                                      // 0x8C3
    WPADCallback reqVolCb;                              // 0x8C4
    u8 rxBufIndex;                                      // 0x8C8
    u8 reqVolume;                                       // 0x8C9
    u8 dpdDummyObjSize;                                 // 0x8CA
    u8 dpdFormat;                                       // 0x8CB
    u8 dpdCommand;                                      // 0x8CC
    u8 audioStop;                                       // 0x8CD
    u8 audioStopCnt;                                    // 0x8CE
    u8 audioFrames;                                     // 0x8CF
    BOOL rumble;                                        // 0x8D0
    BOOL motor;                                         // 0x8D4
    BOOL used;                                          // 0x8D8
    BOOL setup;                                         // 0x8DC
    BOOL oldFw;                                         // 0x8E0
    OSThreadQueue threadq;                              // 0x8E4
    OSTime lastUpdateTime;                              // 0x8F0
    u16 diffCountDpd;                                   // 0x8F8
    u16 hystCountDpd;                                   // 0x8FA
    u16 diffCountAcc;                                   // 0x8FC
    u16 hystCountAcc;                                   // 0x8FE
    u16 diffCountfsAcc;                                 // 0x900
    u16 hystCountfsAcc;                                 // 0x902
    OSTime cmdTimer;                                    // 0x908
    u8 cmdTimeoutAction;                                // 0x910
    u8 isSetStickOrigin;                                // 0x911
    u16 recalibrateCount;                               // 0x912
    u8 key[16];                                         // 0x914
    u8 ft[8];                                           // 0x924
    u8 sb[8];                                           // 0x92C
    u8 readBuf[64];                                     // 0x934
    void* readBufPtr;                                   // 0x974
    u32 readBaseAddr;                                   // 0x978
    s32 readError;                                      // 0x97C
    u16 readLength;                                     // 0x980
    u8 keyIdx;                                          // 0x982
    u8 radioSense;                                      // 0x983
    u16 packetCnt;                                      // 0x984
    u8 disconnect;                                      // 0x986
    u8 cmdId;                                           // 0x987
    WPADCallback infoCallback;                          // 0x988
    u8 infoLocked;                                      // 0x98C
    u8 extEnc;                                          // 0x98D
    u8 pwrSave;                                         // 0x98E
    u8 padding2;                                        // 0x98F
} WPADCB;

extern WPADCB _wpd[WPAD_MAX_CONTROLLERS];
extern WPADCB* _wpdcb[WPAD_MAX_CONTROLLERS];

void WPADiInitSub();
s32 WPADiRetrieveChannel(u8 dev_handle);

void WPADiExcludeButton(s32 chan);
void WPADiCopyOut(s32 chan);

BOOL WPADiSendSetPort(WPADCommandQueue* pQueue, u8 port, WPADCallback pCallback);
BOOL WPADiSendSetReportType(WPADCommandQueue* pQueue, s32 format, BOOL unk, WPADCallback pCallback);
BOOL WPADiSendEnableDPD(WPADCommandQueue* pQueue, BOOL enable, WPADCallback pCallback);
BOOL WPADiSendEnableSpeaker(WPADCommandQueue* pQueue, BOOL enable, WPADCallback pCallback);
BOOL WPADiSendGetContStat(WPADCommandQueue* pQueue, WPADInfo* pInfo, WPADCallback pCallback);
BOOL WPADiSendWriteDataCmd(WPADCommandQueue* pQueue, u8 cmd, u32 addr, WPADCallback pCallback);
BOOL WPADiSendWriteData(WPADCommandQueue* pQueue, const void* pSrc, u16 len, u32 addr, WPADCallback pCallback);
BOOL WPADiSendReadData(WPADCommandQueue* pQueue, void* pDst, u16 len, u32 addr, WPADCallback pCallback);
BOOL WPADiSendMuteSpeaker(WPADCommandQueue* pQueue, BOOL mute, WPADCallback pCallback);
BOOL WPADiSendDPDCSB(WPADCommandQueue* pQueue, BOOL enable, WPADCallback pCallback);

void WPADiClearQueue(WPADCommandQueue* pQueue);

u16 _WPADGetStackBufferStatus(s32 chan);
u16 _WPADGetModuleBufferStatus(s32 chan);

inline void __WPADShutdown();
void __WPADReconnect(BOOL recFlag);

#ifdef __cplusplus
}
#endif
#endif
