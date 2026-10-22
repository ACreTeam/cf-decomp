// Quantization setup and constant tables for TMC JPEG.
#include <MWM/tmc_jpeg_internal.h>

const int s_naturalOrder[64] = {
    0,  1,  8,  16, 9,  2,  3,  10, 17, 24, 32, 25, 18, 11, 4,  5,
    12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6,  7,  14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63,
};

// Retained for the original layout; setup uses the 13-bit scales.
const int s_aanScales14[64] = {
    16384, 11812, 12540,  13933,  16384, 20853,  30274, 59384, 11812, 8516,
    9041,  10045, 11812,  15034,  21826, 42813,  12540, 9041,  9598,  10664,
    12540, 15960, 23170,  45451,  13933, 10045,  10664, 11849, 13933, 17734,
    25746, 50502, 16384,  11812,  12540, 13933,  16384, 20853, 30274, 59384,
    20853, 15034, 15960,  17734,  20853, 26541,  38531, 75581, 30274, 21826,
    23170, 25746, 30274,  38531,  55938, 109727, 59384, 42813, 45451, 50502,
    59384, 75581, 109727, 215238,
};

const int s_aanScales13[64] = {
    8192,  5906,  6270,  6967,   8192,  10426, 15137, 29692, 5906,  4258,
    4520,  5023,  5906,  7517,   10913, 21407, 6270,  4520,  4799,  5332,
    6270,  7980,  11585, 22725,  6967,  5023,  5332,  5925,  6967,  8867,
    12873, 25251, 8192,  5906,   6270,  6967,  8192,  10426, 15137, 29692,
    10426, 7517,  7980,  8867,   10426, 13270, 19266, 37791, 15137, 10913,
    11585, 12873, 15137, 19266,  27969, 54864, 29692, 21407, 22725, 25251,
    29692, 37791, 54864, 107619,
};

const int s_lumWeight[64] = {
    512, 536, 627, 435, 341, 260, 296, 486, 492, 354, 322, 264,  227,
    129, 181, 389, 447, 347, 299, 222, 156, 140, 167, 405, 497,  295,
    242, 204, 136, 101, 160, 407, 455, 268, 169, 124, 120, 95,   146,
    385, 434, 214, 145, 138, 128, 127, 170, 410, 308, 170, 148,  147,
    146, 159, 233, 542, 412, 232, 239, 257, 265, 377, 532, 1087,
};

const int s_stdLumQuant[64] = {
    16, 11, 10, 16, 24,  40,  51,  61,  12, 12, 14, 19, 26,  58,  60,  55,
    14, 13, 16, 24, 40,  57,  69,  56,  14, 17, 22, 29, 51,  87,  80,  62,
    18, 22, 37, 56, 68,  109, 103, 77,  24, 35, 55, 64, 81,  104, 113, 92,
    49, 64, 78, 87, 103, 121, 120, 101, 72, 92, 95, 98, 112, 100, 103, 99,
};

const int s_chromWeight[64] = {
    481, 328, 261, 148, 82,  105, 152, 299, 328, 202, 173, 76,   59,
    75,  110, 216, 261, 173, 85,  53,  63,  80,  117, 229, 148,  76,
    53,  59,  70,  89,  130, 255, 82,  59,  63,  70,  82,  105,  152,
    299, 105, 75,  80,  89,  105, 134, 194, 381, 152, 110, 117,  130,
    152, 194, 282, 554, 299, 216, 229, 255, 299, 381, 554, 1087,
};

const int s_stdChromQuant[64] = {
    17, 18, 24, 47, 99, 99, 99, 99, 18, 21, 26, 66, 99, 99, 99, 99,
    24, 26, 56, 99, 99, 99, 99, 99, 47, 66, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
};

void set_quant_tables(TMCJPEGWork *work) {
    int *quantLum = work->quantLum;
    int *quantChrom = work->quantChrom;
    int *recipLum = work->recipLum;
    int *recipChrom = work->recipChrom;
    int i;
    int lum;
    int chrom;

    work->order = s_naturalOrder;
    if (work->mode < 2) {
        const int *aanScales13 = s_aanScales13;
        const int *stdLum = s_stdLumQuant;
        const int *stdChrom = s_stdChromQuant;
        u32 quality = work->quality;
        for (i = 0; i < 64; i++) {
            lum = (stdLum[i] * 50) / quality;
            chrom = (stdChrom[i] * 50) / quality;
            lum = (lum < 256 && lum > 0) ? lum : (lum > 0 ? 255 : 1);
            quantLum[i] = lum;
            chrom = (chrom < 256 && chrom > 0) ? chrom : (chrom > 0 ? 255 : 1);
            quantChrom[i] = chrom;
            recipLum[i] = aanScales13[i] / quantLum[i];
            recipChrom[i] = aanScales13[i] / quantChrom[i];
        }
    } else if (work->mode == 2) {
        const int *aanScales13 = s_aanScales13;
        for (i = 0; i < 64; i++) {
            recipLum[i] = aanScales13[i] / quantLum[i];
            recipChrom[i] = aanScales13[i] / quantChrom[i];
        }
    } else {
        const int *lumWeight = s_lumWeight;
        const int *stdLum = s_stdLumQuant;
        const int *chromWeight = s_chromWeight;
        const int *stdChrom = s_stdChromQuant;
        for (i = 0; i < 64; i++) {
            quantLum[i] = stdLum[i];
            quantChrom[i] = stdChrom[i];
            recipLum[i] = lumWeight[i];
            recipChrom[i] = chromWeight[i];
        }
    }
}
