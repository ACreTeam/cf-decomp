#ifndef RVL_SDK_RFL_TYPES_H
#define RVL_SDK_RFL_TYPES_H
#include <types.h>
#include <revolution/GX/GXTypes.h>
#ifdef __cplusplus
extern "C" {
#endif

// RVLFaceLib (Mii) public types. Only what the game code uses so far.

typedef enum {
    RFLErrcode_Success = 0,
    RFLErrcode_NotAvailable,
    RFLErrcode_NANDCommandfail,
    RFLErrcode_Loadfail,
    RFLErrcode_Brokendata,
    RFLErrcode_Fatal
} RFLErrcode;

typedef enum {
    RFLDataSource_Official = 0
} RFLDataSource;

typedef enum {
    RFLResolution_64 = 64,
    RFLResolution_128 = 128,
    RFLResolution_256 = 256
} RFLResolution;

typedef enum {
    RFLExp_Normal,
    RFLExp_Smile,
    RFLExp_Anger,
    RFLExp_Sorrow,
    RFLExp_Surprise,
    RFLExp_Blink,
    RFLExp_Max
} RFLExpression;

typedef struct RFLMiddleDB RFLMiddleDB;

// Opaque, 4-byte aligned. The size follows from mFace::model_c (RFLCharModel base at 0x10, next member at 0x98).
#define RFL_CHAR_MODEL_SIZE 0x88
typedef struct RFLCharModel {
    u32 dummy[RFL_CHAR_MODEL_SIZE / sizeof(u32)];
} RFLCharModel;

typedef struct RFLDrawCoreSetting {
    u8 txcGenNum;              // at 0x0
    GXTexCoordID txcID;        // at 0x4
    GXTexMapID texMapID;       // at 0x8
    u8 tevStageNum;            // at 0xC
    GXTevSwapSel tevSwapTable; // at 0x10
    GXTevKColorID tevKColorID; // at 0x14
    GXTevRegID tevOutRegID;    // at 0x18
    GXPosNrmMtx posNrmMtxID;   // at 0x1C
    u8 reverseCulling;         // at 0x20 (read with lbz by RFLDrawOpaCore)
} RFLDrawCoreSetting;

#ifdef __cplusplus
}
#endif
#endif
