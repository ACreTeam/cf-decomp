#pragma once

// dol/sound/SoundVoice.cpp (801D8618..801DC544): the animal voices ("doubutsugo"). Each character of a
// message plays a syllable sound (DOUBUTSUGO_Boy/Gal/Man_*), chosen by the console language and the
// following character, with a pitch and volume from the voice tone. No RTTI; the class name is the TU's.
//
// Character codes (the game's message font):
//   0x01..0x56 kana, in the order of the syllable sounds (code - 1 = DOUBUTSUGO_*_0a_l + n)
//   0x57..0x60 digits 0..9, 0x61..0x7A 'a'..'z', 0x7B..0x9B accented letters
//   0x9C..0xA4 punctuation (0x9C long vowel mark, 0x9E '!', 0x9F '?', 0xA2 '.', 0xA3 '~', 0xA4 ',')
//   0xA5..0xD2 symbols, 0xD3 space
//   0x1100.., 0x3131.., 0xAC00..0xD7A3 Korean jamo and syllables (Unicode)

#include <types.h>
#include <lib/nw4r/snd.h>

class SoundVoice {
public:
    // mLanguage: the syllable table, from getLanguage().
    enum Language_e {
        LANG_JP,
        LANG_EN,
        LANG_ES,
        LANG_FR,
        LANG_IT,
        LANG_DE,
        LANG_KR,
    };

    // mMode
    enum Mode_e {
        MODE_SYLLABLE, // a syllable per character
        MODE_BEEP,     // SE_pupupu per character
        MODE_SILENT,
    };

    // Per voice type: scales the syllable pitch and volume.
    struct VoiceParam {
        f32 pitch;
        f32 volume;
    };

    SoundVoice();  // 801D8618
    ~SoundVoice(); // 801D86EC

    void calc();                                       // 801D8744: counts down mWait
    u8 getVowel(u16 c);                                // 801D875C: the kana of c's vowel (0x53 'n')
    void speak(u16 c, u16 next, f32 volume);           // 801D8778
    void setMode(int mode);                            // 801D87D8: Mode_e
    void setTone(int tone);                            // 801D8800: 0..4
    void setVoiceType(int type);                       // 801D8828: 0..12
    BOOL isVoiced(u16 c);                              // 801D8850
    void playSyllable(u32 id, f32 volume, f32 pitch);  // 801D889C
    void speakJp(u16 c, u16 next);                     // 801D8B04: Japanese and Korean
    int toPlainLetter(u16 c);                          // 801D8E18: drops the accent
    int getVoiceBase();                                // 801D8E88: DOUBUTSUGO_Boy/Gal/Man_0a_l
    int getSyllable(u16 c, u16 next);                  // 801D8EC0
    int getKanaSyllable(u16 c);                        // 801D9174
    void speakWestern(u16 c, u16 next);                // 801D91C0
    int getSyllableJp(u16 c);                          // 801D9890: digits and letters
    int getSyllableKr(u16 c);                          // 801D98EC: digits and letters
    int getSyllableEn(u16 c, u16 next);                // 801D9978
    int getHangulKana(u16 c);                          // 801DA1CC: the kana of a Hangul syllable
    int getSyllableEs(u16 c, u16 next);                // 801DA658
    int getSyllableFr(u16 c, u16 next);                // 801DAE14
    int getSyllableDe(u16 c, u16 next);                // 801DB5A0
    int getSyllableIt(u16 c, u16 next);                // 801DBDAC

    /* 0x00 */ nw4r::snd::SoundHandle mHandle;
    /* 0x04 */ u8 mWait;      // frames until the next syllable may start
    /* 0x05 */ u8 mCharIdx;   // characters into the current word
    /* 0x08 */ int mMode;     // Mode_e
    /* 0x0C */ int mTone;     // 0..4
    /* 0x10 */ int mVoiceType; // 0..12
    /* 0x14 */ int mLanguage; // Language_e
    /* 0x18 */ f32 mVolume;
    /* 0x1C */ u8 mPeriodCount;
    /* 0x1E */ u16 mPrevChar;
    /* 0x20 */ u16 mPrevNext;
}; // size 0x24
