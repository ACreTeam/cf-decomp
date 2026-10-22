#pragma once

// RegEad NG-word checker API (src: lib/RegEad/RegEadPrc.cpp, .text
// 801F7FC4..801F8970), used by dPrcMng_c. prcSetup splits the work area into an
// expanding heap (0xB400 bytes, temporaries and sHeap) and a frame heap (the
// compiled patterns), unpacks the pattern file and compiles every line into a
// serialized RegDfa; prcCheck matches strings against all of them.
//
// Pattern file: u16 magic 'PL', +0x4 u32 (sPrcVersion), +0x20 LZ-compressed
// UTF-16 text (BOM 0xFEFF), one pattern per line (CR, LF or CRLF).

#include <lib/RegEad/RegDfa.hpp>
#include <revolution/cx.h>

namespace RegEad {

enum PrcResult {
    PRC_OK = 0,
    PRC_ERR_DFA_ALLOC = -1, // no room for the serialized DFA
    // pattern failed to compile (RegNfa::mError != ERR_NO_MEMORY)
    PRC_ERR_SYNTAX = -2,
    PRC_ERR_NO_MEMORY = -3, // out of memory while compiling / building the DFA
    PRC_ERR_TOO_MANY = -4,  // PRC_PATTERN_MAX patterns already compiled
    // Fatal errors (prcIsFatalError):
    PRC_ERR_ALREADY_SETUP = -5,
    PRC_ERR_NOT_SETUP = -6,
    PRC_ERR_INVALID_ARG = -7,
    PRC_ERR_BAD_DATA = -8, // bad magic / BOM / empty line
    PRC_ERR_ALLOC = -9,
};

enum {
    PRC_WORK_SIZE_MIN = 0x80000,
    PRC_EXP_HEAP_SIZE = 0xB400,
    PRC_PATTERN_MAX = 1000,
    PRC_CHECK_MAX = 50,
};

extern MEMiHeapHead *sPrcExpHeap; // 8074EC80
extern MEMiHeapHead *sPrcFrmHeap; // 8074EC84
extern void **sPrcPatterns;       // 8074EC88: serialized DFAs [PRC_PATTERN_MAX]
extern int sPrcPatternNum;        // 8074EC8C
extern u32 sPrcVersion;           // 8074C928 (-1 until set up)

// Builds the matcher from a pattern file. Returns a PrcResult.
int prcSetup(void *work, int workSize, const void *data,
             int dataSize); // 801F7FC4
// Compiles one pattern line (len chars, not terminated) and stores its DFA.
int prcAddPattern(const u16 *line, int len); // 801F81B4
// Copies len chars of src without spaces (U+0020, U+3000) and terminates dst.
void prcStripSpaces(u16 *dst, const u16 *src, int len); // 801F8384
// Checks num strings: results[i] = 1 for each one any pattern matches, *hits =
// how many did.
int prcCheck(const u16 **words, int num, u8 *results, int *hits); // 801F83D0
// Normalizes a string before matching: drops spaces, full-width ASCII -> ASCII,
// hiragana -> katakana, half-width katakana (with dakuten / handakuten) ->
// full-width, small kana -> large kana.
void prcNormalize(u16 *dst, const u16 *src); // 801F86D8
// Small kana -> large kana, in place.
void prcToLargeKana(u16 *ch); // 801F88BC
// Looks ch up in a {from, to} table; on a hit stores to in *dst and returns
// true.
bool prcConvertChar(u16 *dst, u16 ch, const u16 (*table)[2],
                    int num);     // 801F890C
BOOL prcIsFatalError(int result); // 801F8958: result <= PRC_ERR_ALREADY_SETUP

// {from, to} tables: small kana (12), half-width katakana (0x3A), + dakuten
// (0x15), + handakuten (5).
extern u16 sPrcKanaTable[0x60][2]; // 80506A30

} // namespace RegEad
