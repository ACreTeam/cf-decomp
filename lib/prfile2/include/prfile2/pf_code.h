#ifndef PRFILE2_PF_CODE_H
#define PRFILE2_PF_CODE_H

#include <prfile2/pf_types.h>

#ifdef __cplusplus
extern "C" {
#endif

// pf_valid_fn_char bits
enum {
    PF_FN_CHAR_LONG = 0x1,   // allowed in a long file name
    PF_FN_CHAR_SHORT = 0x2,  // allowed in a short file name
};

// pf_valid_fn_char covers the characters from PF_FN_CHAR_FIRST up to PF_FN_CHAR_END
enum {
    PF_FN_CHAR_FIRST = ' ',
    PF_FN_CHAR_END = 0x80,
};

extern const u8 pf_valid_fn_char[PF_FN_CHAR_END - PF_FN_CHAR_FIRST];

s32 PFCODE_Combine_Width(s16 oem_width, s16 uni_width);
void PFCODE_Divide_Width(s32 width, s16* oem_width, s16* uni_width);

#ifdef __cplusplus
}
#endif
#endif
