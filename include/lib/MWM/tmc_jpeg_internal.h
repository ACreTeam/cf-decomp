#ifndef MWM_TMC_JPEG_INTERNAL_H
#define MWM_TMC_JPEG_INTERNAL_H
#include <MWM/tmc_jpeg.h>

// Output buffer / bit writer
typedef struct TMCJPEGBitWriter {
    /* 0x00 */ u8 *start;
    /* 0x04 */ u8 *cur;
    /* 0x08 */ int freeBits;
    /* 0x0C */ u8 overflow;
    /* 0x10 */ u32 size;
    /* 0x14 */ u32 count;
} TMCJPEGBitWriter;

typedef void (*TMCJPEGPutBitsFn)(TMCJPEGBitWriter *bw, int bits, int nbits);

// Derived Huffman table (code and size indexed by symbol)
typedef struct TMCJPEGHuffTable {
    /* 0x00 */ int count;
    /* 0x04 */ int *code;
    /* 0x08 */ int *size;
} TMCJPEGHuffTable;

// Per component entropy coding state (a pair of Huffman tables: DC then AC)
typedef struct TMCJPEGEntropy {
    /* 0x00 */ int _00;
    /* 0x04 */ int stride;
    /* 0x08 */ int lastDc;
    /* 0x0C */ int minRecip;
    /* 0x10 */ TMCJPEGHuffTable *huff;
    /* 0x14 */ int *recip;
    /* 0x18 */ const int *order;
    /* 0x1C */ TMCJPEGBitWriter *bw;
    /* 0x20 */ TMCJPEGPutBitsFn put;
} TMCJPEGEntropy;

typedef struct TMCJPEGComp {
    /* 0x00 */ u16 width;
    /* 0x02 */ u16 height;
    /* 0x04 */ u8 blkW;
    /* 0x05 */ u8 blkH;
    /* 0x06 */ u8 blocks;
    /* 0x07 */ u8 lastW;
    /* 0x08 */ u8 lastH;
    /* 0x09 */ u8 sampling;
    /* 0x0A */ u8 _0A[2];
    /* 0x0C */ TMCJPEGEntropy ent;
} TMCJPEGComp;

struct TMCJPEGWork;
struct TMCJPEGCtx;

typedef int (*TMCJPEGLoadFn)(u8 *dst, struct TMCJPEGCtx *ctx, int x, int y);

// MCU encoding position + component state + sample loaders (work + 0x1580)
typedef struct TMCJPEGSource {
    /* 0x00 */ u16 mcusX;
    /* 0x02 */ u16 mcusY;
    /* 0x04 */ u16 curX;
    /* 0x06 */ u16 curY;
    /* 0x08 */ u8 remX;
    /* 0x09 */ u8 remY;
    /* 0x0A */ u8 _0A[6];
    /* 0x10 */ TMCJPEGComp lum;
    /* 0x40 */ u32 _40;
    /* 0x44 */ TMCJPEGComp cb;
    /* 0x74 */ u32 _74;
    /* 0x78 */ TMCJPEGComp cr;
    /* 0xA8 */ struct TMCJPEGCtx *ctx;
    /* 0xAC */ TMCJPEGLoadFn load;
    /* 0xB0 */ TMCJPEGLoadFn loadPadded;
} TMCJPEGSource;

typedef int (*TMCJPEGEncodeMcuFn)(int x, int y, TMCJPEGSource *src);
typedef int (*TMCJPEGEncodeMcuPaddedFn)(int x, int y, int kind,
                                        TMCJPEGSource *src);

// Work area supplied by the caller (0x164C bytes of state, followed by the
// scratch output buffer)
typedef struct TMCJPEGWork {
    /* 0x0000 */ u8 sampling; // 0 = 4:1:1, 1 = 4:2:2, 2 = 4:2:0
    /* 0x0001 */ u8 _01;
    /* 0x0002 */ u16 width;
    /* 0x0004 */ u16 height;
    /* 0x0006 */ u16 cbWidth;
    /* 0x0008 */ u16 cbHeight;
    /* 0x000A */ u16 crWidth;
    /* 0x000C */ u16 crHeight;
    /* 0x000E */ u8 hShift; // log2 of the luma horizontal sampling factor
    /* 0x000F */ u8 vShift;
    /* 0x0010 */ int restartInterval;
    /* 0x0014 */ int restartCounter;
    /* 0x0018 */ int restartNum;
    /* 0x001C */ u8 mode;
    /* 0x0020 */ u32 quality;
    /* 0x0024 */ TMCJPEGEncodeMcuFn encodeMcu;
    /* 0x0028 */ TMCJPEGEncodeMcuPaddedFn encodeMcuPadded;
    /* 0x002C */ const u8 *stdTable[8]; // dc lum bits/vals, dc chrom bits/vals,
                                        // ac lum bits/vals, ac chrom bits/vals
    /* 0x004C */ int dcLumCode[16];
    /* 0x008C */ int dcLumSize[16];
    /* 0x00CC */ int dcChromCode[16];
    /* 0x010C */ int dcChromSize[16];
    /* 0x014C */ int acLumCode[256];
    /* 0x054C */ int acLumSize[256];
    /* 0x094C */ int acChromCode[256];
    /* 0x0D4C */ int acChromSize[256];
    /* 0x114C */ TMCJPEGHuffTable huff[4]; // dc lum, ac lum, dc chrom, ac chrom
    /* 0x117C */ int quantLum[64];
    /* 0x127C */ int recipLum[64];
    /* 0x137C */ int quantChrom[64];
    /* 0x147C */ int recipChrom[64];
    /* 0x157C */ const int *order;
    /* 0x1580 */ TMCJPEGSource src;
    /* 0x1634 */ TMCJPEGBitWriter bw;
} TMCJPEGWork;

// Per-call compression state (lives on the caller's stack, 0x6FC bytes)
typedef struct TMCJPEGCtx {
    TMCJPEGWork *work;    // 0x00
    int _04;              // 0x04
    GXTexFmt format;      // 0x08
    u16 curX;             // 0x0C
    u16 curY;             // 0x0E
    u32 outSize;          // 0x10
    int quality;          // 0x14
    int totalMcus;        // 0x18
    u16 mcusX;            // 0x1C
    u16 mcusY;            // 0x1E
    u8 hSamp;             // 0x20
    u8 vSamp;             // 0x21
    u8 edgeWidth;         // 0x22 width of the last MCU column in pixels
    u8 edgeHeight;        // 0x23 height of the last MCU row in pixels
    u32 lastMcuX;         // 0x24 pixel x of the last MCU column
    u32 lastMcuY;         // 0x28 pixel y of the last MCU row
    u8 scale;             // 0x2C
    u8 _2D;               // 0x2D
    u8 sampling;          // 0x2E
    u8 _2F;               // 0x2F
    u16 width;            // 0x30
    u16 height;           // 0x32
    u16 cbWidth;          // 0x34
    u16 cbHeight;         // 0x36
    int lumRowBytes;      // 0x38
    int lumRows;          // 0x3C
    int cbRowBytes;       // 0x40
    int cbRows;           // 0x44
    int alignedWidth;     // 0x48 width rounded up to a 4 pixel tile
    int alignedHeight;    // 0x4C
    int planeW1;          // 0x50
    int planeH1;          // 0x54
    const u8 *plane[3];   // 0x58
    const u8 *pixels;     // 0x64
    u8 _68[0x6FC - 0x68]; // 0x68
} TMCJPEGCtx;             // 0x6FC

// Caller supplied compression parameters
typedef struct TMCJPEGParams {
    /* 0x00 */ TMCJPEGWork *work;
    /* 0x04 */ GXTexFmt format;
    /* 0x08 */ u16 width;
    /* 0x0A */ u16 height;
    /* 0x0C */ u8 sampling;
    /* 0x0E */ u16 restartInterval;
    /* 0x10 */ u8 mode;
    /* 0x14 */ u32 quality;
    /* 0x18 */ u32 maxSize;
    /* 0x1C */ u8 _1C[0x80];
    /* 0x9C */ u32 bufSize;
} TMCJPEGParams;

// Constant tables shared by the quantization setup.
extern const int s_naturalOrder[64];
extern const int s_aanScales14[64];
extern const int s_aanScales13[64];
extern const int s_lumWeight[64];
extern const int s_stdLumQuant[64];
extern const int s_chromWeight[64];
extern const int s_stdChromQuant[64];

// Shared encoder and size-estimation routines.
int search_quality(TMCJPEGCtx *ctx, const void *image, TMCJPEGParams *params);
int select_load_mcu_rgba8(TMCJPEGSource *src, TMCJPEGCtx *ctx);
int select_load_mcu_rgb565(TMCJPEGSource *src, TMCJPEGCtx *ctx);
void select_count_mcu(TMCJPEGWork *work, int sampling);

int start_compress(TMCJPEGCtx *ctx, u8 *dst, const void *image,
                   TMCJPEGParams *params);
int encode_mcus(TMCJPEGCtx *ctx, int count, const void *image);
void dest_init(TMCJPEGBitWriter *bw, u8 *buf, u32 size);
void dest_get_size(TMCJPEGBitWriter *bw, u32 *size);
void emit_byte(TMCJPEGBitWriter *bw, u8 value);
void emit_2bytes(TMCJPEGBitWriter *bw, u16 value);
void emit_bytes(TMCJPEGBitWriter *bw, const u8 *data, int count);
void flush_bits_emit_marker(TMCJPEGBitWriter *bw, u16 marker);
void flush_bits_emit_eoi(TMCJPEGBitWriter *bw);
void select_put_bits(TMCJPEGEntropy *ent);
void put_bits_fast(TMCJPEGBitWriter *bw, int bits, int nbits);
void put_bits(TMCJPEGBitWriter *bw, int bits, int nbits);
int encode_one_block(int *block, TMCJPEGEntropy *ent);
void select_encode_mcu_padded(TMCJPEGWork *work, int sampling);
int encode_mcus_padded(TMCJPEGWork *work, TMCJPEGCtx *ctx, int count);
int encode_mcu_padded_411(int x, int y, int kind, TMCJPEGSource *src);
int encode_mcu_padded_420(int x, int y, int kind, TMCJPEGSource *src);
int encode_mcu_padded_422(int x, int y, int kind, TMCJPEGSource *src);
int write_frame_header(TMCJPEGWork *work);
int write_file_header(TMCJPEGWork *work);
int check_params(TMCJPEGParams *params, TMCJPEGWork *work);
void initial_setup(TMCJPEGWork *work);
void emit_restart(TMCJPEGWork *work);
void setup_source(TMCJPEGWork *work, TMCJPEGCtx *ctx);
void select_encode_mcu(TMCJPEGWork *work, int sampling);
int encode_mcu_411(int x, int y, TMCJPEGSource *src);
int encode_mcu_420(int x, int y, TMCJPEGSource *src);
int encode_mcu_422(int x, int y, TMCJPEGSource *src);
int write_scan_header(TMCJPEGWork *work);
void fdct_luma(const u8 *in, int *out, int stride);
void fdct_chroma(const s8 *in, int *out, int stride);
void std_huff_tables(TMCJPEGWork *work);
void make_derived_huff_tables(TMCJPEGWork *work);
void set_quant_tables(TMCJPEGWork *work);
void select_count_mcu(TMCJPEGWork *work, int mode);
void select_count_mcu_padded(TMCJPEGWork *work, int mode);
int count_mcu_411(int mcuX, int mcuY, TMCJPEGSource *sampler);
int count_mcu_420(int mcuX, int mcuY, TMCJPEGSource *sampler);
int count_mcu_422(int mcuX, int mcuY, TMCJPEGSource *sampler);
int count_mcu_padded_411(int mcuX, int mcuY, int edge, TMCJPEGSource *sampler);
int count_mcu_padded_420(int mcuX, int mcuY, int edge, TMCJPEGSource *sampler);
int count_mcu_padded_422(int mcuX, int mcuY, int edge, TMCJPEGSource *sampler);
int load_mcu_rgba8_411(u8 *dst, TMCJPEGCtx *ctx, int px, int py);
int load_mcu_rgba8_422(u8 *dst, TMCJPEGCtx *ctx, int px, int py);
int load_mcu_rgba8_420(u8 *dst, TMCJPEGCtx *ctx, int px, int py);
int load_mcu_rgba8_padded_411(u8 *dst, TMCJPEGCtx *ctx, int px, int py);
int load_mcu_rgba8_padded_422(u8 *dst, TMCJPEGCtx *ctx, int px, int py);
int load_mcu_rgba8_padded_420(u8 *dst, TMCJPEGCtx *ctx, int px, int py);
int load_mcu_rgb565_411(u8 *dst, TMCJPEGCtx *ctx, int px, int py);
int load_mcu_rgb565_422(u8 *dst, TMCJPEGCtx *ctx, int px, int py);
int load_mcu_rgb565_420(u8 *dst, TMCJPEGCtx *ctx, int px, int py);
int load_mcu_rgb565_padded_411(u8 *dst, TMCJPEGCtx *ctx, int px, int py);
int load_mcu_rgb565_padded_422(u8 *dst, TMCJPEGCtx *ctx, int px, int py);
int load_mcu_rgb565_padded_420(u8 *dst, TMCJPEGCtx *ctx, int px, int py);
int count_mcus(TMCJPEGWork *work, TMCJPEGCtx *ctx, int limit, int batch);
void count_restart(TMCJPEGWork *work);
int count_eoi(TMCJPEGWork *work);
void count_get_size(TMCJPEGBitWriter *writer, u32 *size);


#endif
