// The animal voices: a syllable sound per message character. .text 801D8618..801DC544, .rodata
// 8047ED18..8047F100, .data 805010F0..80503930, .bss 80625DA0..80625DB0, .sdata 8074C6A8..8074C6B0,
// .sbss 8074EC30..8074EC60, .sdata2 80751C40..80751C80. See include/game/snd/snd_voice.hpp.
#include <game/snd/snd_voice.hpp>
#include <game/snd/snd_manager.hpp>
#include <game/snd/snd_id.hpp>
#include <game/game/d_region.hpp>
#include <revolution/os.h>

// A syllable as an offset from the voice's first one, and as the kana character that plays it.
#define S(name) (DOUBUTSUGO_Boy_##name - DOUBUTSUGO_Boy_0a_l)
#define K(name) (DOUBUTSUGO_Boy_##name - DOUBUTSUGO_Boy_0a_l + 1)

// The kana of each kana's vowel (for the long vowel mark).
static const u8 sCharVowel[0xD8] = {
    0, K(0a), K(0a), K(0i), K(0i), K(0u), K(0u), K(0e), K(0e), K(0o), K(0o), K(0a),
    K(0a), K(0i), K(0i), K(0u), K(0u), K(0e), K(0e), K(0o), K(0o), K(0a), K(0a), K(0i),
    K(0i), K(0u), K(0u), K(0e), K(0e), K(0o), K(0o), K(0a), K(0a), K(0i), K(0i), K(0n),
    K(0u), K(0u), K(0e), K(0e), K(0o), K(0o), K(0a), K(0i), K(0u), K(0e), K(0o), K(0a),
    K(0a), K(0a), K(0i), K(0i), K(0i), K(0u), K(0u), K(0u), K(0e), K(0e), K(0e), K(0o),
    K(0o), K(0o), K(0a), K(0i), K(0u), K(0e), K(0o), K(0a), K(0a), K(0u), K(0u), K(0o),
    K(0o), K(0a), K(0i), K(0u), K(0e), K(0o), K(0a), K(0a), K(0n), K(0n), K(0n), K(0i_l),
    K(0i), K(0i), K(0i), K(0n), K(0n), K(0o), K(0u), K(0a), K(0i), K(0u), K(0i), K(0i),
    K(0i), K(0i), K(0i), K(0u), K(0i), K(0i), K(0i), K(0i), K(0e), K(0u), K(0u), K(0n),
    K(0o), K(0i), K(0u), K(0a), K(0u), K(0i), K(0u), K(0i), K(0u), K(0u), K(0i), K(0i),
    K(0a), K(0a), K(0a), K(0a), K(0a), K(0a), K(0a), K(0i), K(0i), K(0i), K(0i), K(0i),
    K(0i), K(0i), K(0i), K(0i), K(0i), K(0n), K(0o), K(0o), K(0o), K(0o), K(0o), K(0n),
    K(0n), K(0u), K(0u), K(0u), K(0u), K(0n), K(0n), K(0u), K(0i), K(0n), K(0n), K(0n),
    K(0n), K(0n), K(0n), K(0n), K(0n), K(0n),
};

// speakJp: pitch and volume of each character of a word, per tone.
static const s16 sJpPitch[5][16] = {
    {0, 128, 320, 192, 64, 0, -64, 0, 320, 192, 64, -64, 0, 192, 0, 192},
    {240, 180, 150, -77, 190, 120, 90, -77, 240, 180, 150, -77, 190, 120, 90, -77},
    {77, -17, 43, -51, 9, -51, -25, -85, -59, -119, -93, -153, -127, -187, -161, -221},
    {90, 30, 0, -227, 40, -30, -60, -227, 90, 30, 0, -227, 40, -30, -60, -227},
    {128, 256, 448, 320, 192, 128, 64, 128, 448, 320, 192, 64, 128, 320, 128, 320},
};
static const u8 sJpVolume[5][16] = {
    {75, 95, 125, 105, 85, 75, 65, 75, 125, 105, 85, 65, 75, 105, 75, 105},
    {125, 110, 90, 105, 125, 110, 90, 105, 125, 110, 90, 105, 125, 110, 90, 105},
    {115, 97, 109, 91, 103, 85, 97, 79, 91, 73, 85, 67, 79, 61, 73, 55},
    {105, 90, 70, 85, 105, 90, 70, 85, 105, 90, 70, 85, 105, 90, 70, 85},
    {75, 95, 125, 105, 85, 75, 65, 75, 125, 105, 85, 65, 75, 105, 75, 105},
};

// Digits and letters in Japanese (from 0x57).
static const u16 sLatinJp[] = {
    S(ze), S(0i), S(ni), S(sa), S(si), S(go), S(ro), S(na),
    S(ha), S(ku), S(ei), S(bi), S(si), S(di), S(0i), S(ef),
    S(zi), S(ei), S(ai), S(je), S(ke), S(0e), S(em), S(en),
    S(oo), S(pi), S(qu), S(0a), S(es), S(ty), S(yu), S(vi),
    S(da), S(0e), S(wa), S(ze), S(ei), S(ei), S(ei), S(ei),
    S(ei), S(ei), S(ei), S(si), S(0i), S(0i), S(0i), S(0i),
    S(ai), S(ai), S(ai), S(ai), S(di), S(en), S(oo), S(oo),
    S(oo), S(oo), S(oo), S(wi_dummy), S(wi_dummy), S(yu), S(yu), S(yu),
    S(yu), S(wi_dummy), S(wi_dummy), S(be), S(wa),
};

// Korean jamo as kana: compatibility jamo (from 0x3131), initials (0x1100), vowels (0x1161) and finals
// (0x11A8).
static const u16 sJamoCompatKana[] = {
    K(gu), K(ku), K(gu), K(0n), K(0n), K(0n), K(do), K(tu),
    K(ru), K(ru), K(ru), K(ru), K(ru), K(ru), K(ru), K(ru),
    K(mu), K(bu), K(pu), K(bu), K(su), K(su), 0, K(zi),
    K(ti), K(ti), K(ku), K(tu), K(pu), K(fu), K(0a), K(0a),
    K(ya), K(0e), K(0e), K(0e), K(0e), K(0e), K(0o), K(wa),
    K(wa), K(0o), K(yo), K(0u), K(0o), K(0e), K(0i), K(yu),
    K(0e), K(0u), K(0i),
};
static const u16 sJamoInitialKana[] = {
    K(gu), K(ku), K(0n), K(do), K(tu), K(ru), K(mu), K(bu),
    K(pu), K(su), K(su), 0, K(zi), K(ti), K(ti), K(ku),
    K(tu), K(pu), K(fu),
};
static const u16 sJamoVowelKana[] = {
    K(0a), K(0a), K(ya), K(0e), K(0e), K(0e), K(0e), K(0e),
    K(0o), K(wa), K(wa), K(0o), K(yo), K(0u), K(0o), K(0e),
    K(0i), K(yu), K(0e), K(0u), K(0i),
};
static const u16 sJamoFinalKana[] = {
    K(gu), K(ku), K(gu), K(0n), K(0n), K(0n), K(do), K(ru),
    K(ru), K(ru), K(ru), K(ru), K(ru), K(ru), K(ru), K(mu),
    K(bu), K(bu), K(su), K(su), 0, K(zi), K(ti), K(ku),
    K(tu), K(pu), K(fu),
};

static const SoundVoice::VoiceParam sVoiceParams[13] = {
    {1.0f, 1.0f}, // 0
    {1.0f, 1.0f}, // 1
    {1.0f, 0.9f}, // 2
    {0.7f, 1.38f}, // 3
    {1.72f, 1.23f}, // 4
    {0.65f, 1.1f}, // 5
    {0.6f, 0.9f}, // 6
    {0.5f, 1.1f}, // 7
    {0.7f, 1.03f}, // 8
    {0.7f, 0.8f}, // 9
    {1.81f, 0.9f}, // 10
    {0.55f, 0.9f}, // 11
    {1.122f, 1.0f}, // 12
};

// Symbols (from 0xA5 and from 0xBE).
static u16 sSymbolSyllableA5[] = {
    S(0a), S(be), S(ga), S(de), S(0i), S(ze), S(0e), S(te),
    S(0i), S(ka), S(ra), S(mu), S(nu), S(za), S(0o), S(pa),
    S(ro), S(su), S(si), S(ta), S(yu), S(ha), S(ka), S(sa),
    S(0o),
};
static u16 sSymbolSyllableBE[] = {
    S(ha), S(he), S(he), S(hi), S(ho), S(fu), S(ho), S(hi),
    S(0i), S(0u), S(ha), S(he), S(he), S(hi), S(fu), S(sa),
    S(hi), S(fu), S(ho), S(fu), S(ho),
};

// Digits in Korean (from 0x57).
static u16 sDigitKr[] = {
    S(yo), S(0i), S(0i), S(sa), S(sa), S(0o), S(yu), S(ti),
    S(pa), S(ku),
};

// speakWestern state.
static u32 sCharCount;
static u32 sWordCount;
static u32 sLastWordLen;
static bool sStressed;
static u32 sWordNum;
static int sRiseCount;
static u32 sWaveLen;
static u32 sWavePos;
static bool sWaveDown;
static u32 sEC54;
static int sMarkVolume;
static int sMarkPitch;


// 801D8618
SoundVoice::SoundVoice()
    : mWait(0), mCharIdx(0), mMode(MODE_SYLLABLE), mTone(0), mVoiceType(0), mLanguage(-1), mPeriodCount(0),
      mPrevChar(0), mPrevNext(0) {
    switch (getLanguage()) {
    case LANGUAGE_JP:
        mLanguage = LANG_JP;
        break;
    case LANGUAGE_US:
    case LANGUAGE_EN:
        mLanguage = LANG_EN;
        break;
    case LANGUAGE_MX:
    case LANGUAGE_ES:
        mLanguage = LANG_ES;
        break;
    case LANGUAGE_QC:
    case LANGUAGE_FR:
        mLanguage = LANG_FR;
        break;
    case LANGUAGE_IT:
        mLanguage = LANG_IT;
        break;
    case LANGUAGE_DE:
        mLanguage = LANG_DE;
        break;
    case LANGUAGE_KR:
        mLanguage = LANG_KR;
        break;
    }
}

// 801D86EC
SoundVoice::~SoundVoice() {}

// 801D8744
void SoundVoice::calc() {
    if (mWait != 0) {
        mWait--;
    }
}

// 801D875C
u8 SoundVoice::getVowel(u16 c) {
    u8 vowel = K(0n);
    if (c <= 0x9B) {
        vowel = sCharVowel[c];
    }
    return vowel;
}

// 801D8778
void SoundVoice::speak(u16 c, u16 next, f32 volume) {
    mVolume = volume;
    if (mMode == MODE_SYLLABLE) {
        switch (mLanguage) {
        case LANG_JP:
        case LANG_KR:
            speakJp(c, next);
            break;
        case LANG_EN:
            speakWestern(c, next);
            break;
        default:
            speakWestern(c, next);
            break;
        }
    } else if (mMode == MODE_BEEP) {
        playSyllable(SE_pupupu, 1.0f, 1.0f);
    }
}

// 801D87D8
void SoundVoice::setMode(int mode) {
    if (mode < 0 || mode >= 3) {
        mode = MODE_SYLLABLE;
    }
    mMode = mode;
    mWait = 0;
    mCharIdx = 0;
}

// 801D8800
void SoundVoice::setTone(int tone) {
    if (tone < 0 || tone >= 5) {
        tone = 0;
    }
    mTone = tone;
    mWait = 0;
    mCharIdx = 0;
}

// 801D8828
void SoundVoice::setVoiceType(int type) {
    if (type < 0 || type >= 13) {
        type = 0;
    }
    mVoiceType = type;
    mWait = 0;
    mCharIdx = 0;
}

// 801D8850
BOOL SoundVoice::isVoiced(u16 c) {
    BOOL voiced = FALSE;
    if ((c >= 0x01 && c <= 0x9B) || (c >= 0xA5 && c <= 0xD2) || (c >= 0x3131 && c <= 0x3163) ||
        (c >= 0xAC00 && c <= 0xD7A3)) {
        voiced = TRUE;
    }
    return voiced;
}

// 801D889C
void SoundVoice::playSyllable(u32 id, f32 volume, f32 pitch) {
    if (mWait == 0 && id != 0) {
        mHandle.Stop(0);
        SoundManager::getInstance()->startSound(&mHandle, id);
        if (mHandle.IsAttachedSound() && mVoiceType >= 0 && mVoiceType < 13) {
            pitch *= sVoiceParams[mVoiceType].pitch;
            volume *= sVoiceParams[mVoiceType].volume;
            f32 vol = mVolume * (0.44444445f * volume);
            if (vol > 1.0f) {
                vol = 1.0f;
            } else if (vol < 0.0f) {
                vol = 0.0f;
            }
            mHandle.SetVolume(vol, 0);
            mHandle.SetPitch(pitch);
            mWait = 3;
        }
    }
}

// 801D8B04
void SoundVoice::speakJp(u16 c, u16 next) {
    if (c == 0xD3) {
        return;
    }
    if (c == 0x9D && next > 0x56) {
        playSyllable(SE_pupupu, 1.0f, 1.0f);
        return;
    }

    u8 vol;
    u8 volume = sJpVolume[mTone][mCharIdx];
    s16 pitch = sJpPitch[mTone][mCharIdx];
    u8 periods = 0;
    switch (mTone) {
    case 0:
        periods = 2;
        break;
    case 1:
        periods = 1;
        break;
    case 2:
        periods = 2;
        break;
    case 3:
        periods = 1;
        break;
    case 4:
        periods = 2;
        break;
    }

    switch (next) {
    case 0x9E:
        pitch = 0x180;
        volume = 110;
        break;
    case 0x9F:
        pitch = 0x100;
        volume = 127;
        break;
    }

    u16 id;
    switch (c) {
    case 0x9E:
        pitch = 0x180;
        volume = 110;
        if (mPrevNext == 0) {
            id = getSyllable(mPrevChar, 0);
        } else {
            id = getSyllable(mPrevNext, 0);
        }
        break;
    case 0x9F:
        pitch = 0x100;
        volume = 127;
        if (mPrevNext == 0) {
            id = getSyllable(mPrevChar, 0);
        } else {
            id = getSyllable(mPrevNext, 0);
        }
        break;
    case 0x9C: {
        u16 vowel;
        if (mPrevNext == 0) {
            vowel = getVowel(mPrevChar);
        } else {
            vowel = getVowel(mPrevNext);
        }
        id = getSyllable(vowel, 0);
        break;
    }
    case 0x9D:
    case 0xA0:
    case 0xA1:
    case 0xA2:
    case 0xA4:
        id = 0;
        break;
    default:
        id = getSyllable(c, 0);
        break;
    }

    if (id != 0) {
        vol = volume;
        if (vol > 127) {
            vol = 127;
        }
        playSyllable(id, vol / 127.0f, 1.0f + (pitch - 0x40) / 1076.297f);
    }

    mCharIdx++;
    if (mCharIdx > 15 || (c >= 0x9D && c <= 0xA4) || (next >= 0x9D && next <= 0xA4)) {
        mCharIdx = 0;
    }
    if (c == 0xA2 || next == 0xA2) {
        if (mPeriodCount == periods) {
            mCharIdx = 0;
            mPeriodCount = 0;
        } else {
            mPeriodCount++;
        }
    }
    if (c != 0x9C) {
        mPrevChar = c;
    }
    if (next != 0x9C) {
        mPrevNext = next;
    }
}

// 801D8E18
int SoundVoice::toPlainLetter(u16 c) {
    int letter = 0;
    switch (c) {
    case 0x7B:
    case 0x7C:
    case 0x7D:
    case 0x7E:
    case 0x7F:
    case 0x80:
    case 0x81:
        letter = 'a';
        break;
    case 0x82:
        letter = 'c';
        break;
    case 0x83:
    case 0x84:
    case 0x85:
    case 0x86:
        letter = 'e';
        break;
    case 0x87:
    case 0x88:
    case 0x89:
    case 0x8A:
        letter = 'i';
        break;
    case 0x8C:
        letter = 'n';
        break;
    case 0x8D:
    case 0x8E:
    case 0x8F:
    case 0x90:
    case 0x91:
        letter = 'o';
        break;
    case 0x94:
    case 0x95:
    case 0x96:
    case 0x97:
        letter = 'u';
        break;
    case 0x9B:
        letter = 'y';
        break;
    case 0x8B:
    case 0x92:
    case 0x93:
    case 0x98:
    case 0x99:
        break;
    default:
        letter = c;
        break;
    }
    return letter;
}

// 801D8E88
int SoundVoice::getVoiceBase() {
    int base = DOUBUTSUGO_Boy_0a_l;
    switch (mVoiceType) {
    case 1:
    case 3:
    case 5:
    case 6:
    case 7:
        base = DOUBUTSUGO_Gal_0a_l;
        break;
    case 2:
    case 10:
        base = DOUBUTSUGO_Man_0a_l;
        break;
    case 0:
    case 4:
    case 8:
    case 9:
    case 11:
    case 12:
        break;
    }
    return base;
}

// 801D8EC0
int SoundVoice::getSyllable(u16 c, u16 next) {
    if (c >= 0x01 && c <= 0x56) {
        return getKanaSyllable(c);
    }
    if (c >= 0x1100 && c <= 0x1112) {
        return getKanaSyllable(sJamoInitialKana[c - 0x1100]);
    }
    if (c >= 0x1161 && c <= 0x1175) {
        return getKanaSyllable(sJamoVowelKana[c - 0x1161]);
    }
    if (c >= 0x11A8 && c <= 0x11C2) {
        return getKanaSyllable(sJamoFinalKana[c - 0x11A8]);
    }
    if (c >= 0x3131 && c <= 0x3163) {
        return getKanaSyllable(sJamoCompatKana[c - 0x3131]);
    }
    if (c >= 0xAC00 && c <= 0xD7A3) {
        int kana = getHangulKana(c);
        return getKanaSyllable(kana);
    }
    if (c >= 0x57 && c <= 0xA4) {
        u32 ch = c;
        u32 nx = next;
        if (ch >= 0x7B && ch <= 0x99) {
            ch = toPlainLetter(ch);
        }
        if (nx >= 0x7B && nx <= 0x99) {
            nx = toPlainLetter(nx);
        }
        switch (mLanguage) {
        case LANG_JP:
            return getSyllableJp(ch);
        case LANG_EN:
            return getSyllableEn(ch, nx);
        case LANG_ES:
            return getSyllableEs(ch, nx);
        case LANG_FR:
            return getSyllableFr(ch, nx);
        case LANG_IT:
            return getSyllableIt(ch, nx);
        case LANG_DE:
            return getSyllableDe(ch, nx);
        case LANG_KR:
            return getSyllableKr(ch);
        default:
            return getSyllableJp(ch);
        }
    }
    if (c >= 0xA5 && c <= 0xBD) {
        return sSymbolSyllableA5[c - 0xA5] + getVoiceBase();
    }
    if (c >= 0xBE && c <= 0xD2) {
        return sSymbolSyllableBE[c - 0xBE] + getVoiceBase();
    }
    return 0;
}

// 801D9174
int SoundVoice::getKanaSyllable(u16 c) {
    int id = 0;
    if (c >= 0x01 && c <= 0x56) {
        id = getVoiceBase() + (c - 1);
    }
    return id;
}

// A random length of base..base + 3.
static inline u8 randomLen(int base) {
    return OSGetTick() % 4 + base;
}

// 801D91C0
void SoundVoice::speakWestern(u16 c, u16 next) {
    // The pitch and volume waves of tones 1 and 3.
    static const s16 sTone1Pitch[8] = {240, 180, 150, 190, 120, 90, -77, -77};
    static const u8 sTone1Volume[8] = {125, 110, 90, 125, 110, 90, 105, 105};
    static const s16 sTone3Pitch[8] = {90, 30, 0, 40, -30, -60, -227, -227};
    static const u8 sTone3Volume[8] = {100, 85, 65, 100, 85, 65, 80, 80};
    // Letters whose name starts with a vowel sound.
    static const u16 sVowelLetters[] = {
        'a', 'e', 'f', 'h', 'i', 'l', 'm', 'n', 'o', 'r', 'u', 'y',
    };
    static int sVowelLetterNum = ARRAY_SIZE(sVowelLetters);

    if (c == 0xD3) {
        return;
    }
    if (c == 0xA3 && !isVoiced(mPrevChar)) {
        playSyllable(SE_pupupu, 1.0f, 1.0f);
        mPrevChar = c;
        mPrevNext = next;
        sCharCount = 0;
        sWordCount = 0;
        mCharIdx = 0;
        return;
    }

    int pitch = 0;
    int volume = 0;
    switch (mTone) {
    case 0: {
        if (mCharIdx == 0) {
            if (sWordCount == 0) {
                for (int i = 0; i < sVowelLetterNum; i++) {
                    if (c == sVowelLetters[i]) {
                        sStressed = true;
                        break;
                    }
                }
                sRiseCount = 0;
            } else {
                bool flip = false;
                if (sWordNum >= 4) {
                    flip = true;
                } else if (sWordNum >= 3 && sLastWordLen > 6) {
                    flip = true;
                } else if (sWordNum >= 2 && sLastWordLen > 2) {
                    flip = true;
                }
                if (flip) {
                    sStressed = !sStressed;
                    sWordNum = 0;
                    sRiseCount = 0;
                }
            }
        }
        if (sStressed) {
            if (mCharIdx > 4) {
                pitch = -((mCharIdx - 4) * 32);
            }
            if (pitch < -0x60) {
                pitch = 0x60;
            }
            volume = 65;
        } else {
            pitch = ++sRiseCount * 40;
            if (pitch > 0xA0) {
                pitch = 0xA0;
            }
            volume = 75;
        }

        u8 idx = mCharIdx;
        bool accent = false;
        int period = 0;
        if (idx > 7) {
            period = 5;
        } else {
            switch (sLastWordLen) {
            case 0:
                if (idx % 5 == 3) {
                    accent = true;
                }
                break;
            case 1:
            case 2:
                period = 2;
                break;
            case 3:
            case 4:
                period = 3;
                break;
            case 5:
            case 6:
                period = 4;
                break;
            default:
                period = 5;
                break;
            }
        }
        if (period > 0 && idx % period == period - 1) {
            accent = true;
        }
        if (accent) {
            pitch -= 30;
            volume = 0.5f * volume;
        }
        break;
    }
    case 1: {
        if (sWavePos == 0) {
            sWavePos = randomLen(10);
        } else {
            sWavePos--;
        }
        pitch = sTone1Pitch[sWavePos % 8];
        volume = sTone1Volume[sWavePos % 8];
        break;
    }
    case 2:
        if (sWavePos == 0) {
            sWaveLen = randomLen(14);
            sWavePos = 1;
        } else if (sWavePos == sWaveLen) {
            sWavePos = 0;
            sWaveDown = !sWaveDown;
        } else {
            sWavePos++;
        }
        if (!sWaveDown) {
            pitch = -(sWavePos * 6);
        } else {
            pitch = 0x60 - sWavePos * 6;
        }
        if (sWaveDown == false) {
            volume = (u8)(70 - sWavePos * 3);
        } else {
            volume = (u8)(80 - sWavePos * 3);
        }
        if (sCharCount == 0) {
            sWavePos = 1;
        }
        if (volume > 127) {
            volume = 127;
        } else if (volume < 0) {
            volume = 0;
        }
        break;
    case 3: {
        if (sWavePos == 0) {
            sWavePos = randomLen(10);
        } else {
            sWavePos--;
        }
        pitch = sTone3Pitch[sWavePos % 8];
        volume = sTone3Volume[sWavePos % 8];
        break;
    }
    case 4:
        if (mCharIdx == 4) {
            sStressed = !sStressed;
        }
        if (sStressed) {
            pitch = ++sRiseCount * 40;
            if (pitch > 0xA0) {
                pitch = 0xA0;
            }
        }
        if (mCharIdx > 4 && sStressed) {
            pitch -= (mCharIdx - 4) * 32;
            if (pitch < 0x60) {
                pitch = 0x60;
            }
        }
        if (sEC54 >= 2) {
            pitch = 0xAC;
            volume = 20;
        }
        pitch += 0x80;
        volume += 80;
        break;
    }

    mCharIdx++;
    sCharCount++;
    if ((c >= 0x9C && c <= 0xA4 && c != 0xA2) || (next >= 0x9C && next <= 0xA4 && next != 0xA2)) {
        sCharCount = 0;
        sWordCount = 0;
        mCharIdx = 0;
        sWordNum = 0;
    }
    if (c == 0xA2 || next == 0xA2 || c == 0xA4 || next == 0xA4) {
        sWordCount++;
        sWordNum++;
        sLastWordLen = mCharIdx;
        mCharIdx = 0;
    }

    u16 sayChar = 0;
    if (c == 0x9E || c == 0x9F || c == 0xA3) {
        if (c == 0x9E) {
            sayChar = mPrevNext;
            pitch = 0x180;
            volume = 110;
        }
        if (c == 0x9F) {
            sayChar = mPrevNext;
            pitch = 0x100;
            volume = 127;
        }
        if (c == 0xA3) {
            pitch = -0x80;
            volume = 90;
        }
        sMarkVolume = volume;
        sMarkPitch = pitch;
    } else if (next == 0x9E || next == 0x9F || next == 0xA3) {
        if (next == 0x9E) {
            pitch = 0x180;
            volume = 110;
        }
        if (next == 0x9F) {
            pitch = 0x100;
            volume = 127;
        }
        if (next == 0xA3) {
            pitch = -0x80;
            volume = 90;
        }
        sMarkVolume = volume;
        sayChar = 0xA2;
        sMarkPitch = pitch;
    }

    u16 sayNext = 0xA2;
    if (sayChar == 0) {
        sayChar = c;
        sayNext = next;
    }
    if (volume > 127) {
        volume = 127;
    } else if (volume < 0) {
        volume = 0;
    }
    playSyllable(getSyllable(sayChar, sayNext), volume / 127.0f, 1.0f + pitch / 1076.297f);
    mPrevChar = c;
    mPrevNext = next;
}

// 801D9890
int SoundVoice::getSyllableJp(u16 c) {
    if (c >= 0x57 && c <= 0x9B) {
        int base = getVoiceBase();
        return base + sLatinJp[c - 0x57];
    }
    return 0;
}

// 801D98EC
int SoundVoice::getSyllableKr(u16 c) {
    if (c >= 0x57 && c <= 0x60) {
        int base = getVoiceBase();
        return base + sDigitKr[c - 0x57];
    }
    if (c >= 0x61 && c <= 0x9B) {
        int base = getVoiceBase();
        return base + sLatinJp[c - 0x57];
    }
    return 0;
}

// 801D9978
int SoundVoice::getSyllableEn(u16 c, u16 next) {
    int id = 0;
    int base = getVoiceBase();
    switch (c) {
    case 0x57:
        id = DOUBUTSUGO_Boy_ze;
        break;
    case 0x58:
        id = DOUBUTSUGO_Boy_wa;
        break;
    case 0x59:
        id = DOUBUTSUGO_Boy_tu;
        break;
    case 0x5A:
        id = DOUBUTSUGO_Boy_su;
        break;
    case 0x5B:
        id = DOUBUTSUGO_Boy_ho;
        break;
    case 0x5C:
        id = DOUBUTSUGO_Boy_ha;
        break;
    case 0x5D:
        id = DOUBUTSUGO_Boy_si;
        break;
    case 0x5E:
        id = DOUBUTSUGO_Boy_se;
        break;
    case 0x5F:
        id = DOUBUTSUGO_Boy_ei;
        break;
    case 0x60:
        id = DOUBUTSUGO_Boy_na;
        break;
    case 'a':
        id = DOUBUTSUGO_Boy_ai;
        switch (next) {
        case 'c':
            id = DOUBUTSUGO_Boy_0a;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ai;
            break;
        case 'n':
            id = DOUBUTSUGO_Boy_an;
            break;
        case 'y':
            id = DOUBUTSUGO_Boy_ai;
            break;
        }
        break;
    case 'b':
        id = DOUBUTSUGO_Boy_ba;
        switch (next) {
        case 'e':
            id = DOUBUTSUGO_Boy_be;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_bi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_bo;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_bu;
            break;
        }
        break;
    case 'c':
        id = DOUBUTSUGO_Boy_ch;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ka;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_se;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ci;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_ko;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_ku;
            break;
        }
        break;
    case 'd':
        id = DOUBUTSUGO_Boy_de;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_da;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_di;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_do;
            break;
        }
        break;
    case 'e':
        id = DOUBUTSUGO_Boy_ei;
        switch (next) {
        case 'e':
            id = DOUBUTSUGO_Boy_ee;
            break;
        case 'f':
            id = DOUBUTSUGO_Boy_ef;
            break;
        case 'h':
            id = DOUBUTSUGO_Boy_0e;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ei;
            break;
        case 'm':
            id = DOUBUTSUGO_Boy_em;
            break;
        case 'n':
            id = DOUBUTSUGO_Boy_en;
            break;
        case 's':
            id = DOUBUTSUGO_Boy_es;
            break;
        case 'r':
            id = DOUBUTSUGO_Boy_0a;
            break;
        }
        break;
    case 'f':
        id = DOUBUTSUGO_Boy_fu;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ha;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_he;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_hi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_ho;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_fu;
            break;
        }
        break;
    case 'g':
        id = DOUBUTSUGO_Boy_gy;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ga;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_ge;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_gi;
            break;
        case 'n':
            id = DOUBUTSUGO_Boy_0n;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_go;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_gu;
            break;
        case 'y':
            id = DOUBUTSUGO_Boy_gy;
            break;
        }
        break;
    case 'h':
        id = DOUBUTSUGO_Boy_he;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ha;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_he;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_hi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_ho;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_fu;
            break;
        }
        break;
    case 'i':
        id = DOUBUTSUGO_Boy_ai;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ya;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_0i;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_yo;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_yu;
            break;
        case 'x':
            id = DOUBUTSUGO_Boy_ix;
            break;
        }
        break;
    case 'j':
        id = DOUBUTSUGO_Boy_je;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_za;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_je;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_zi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_zo;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_zu;
            break;
        }
        break;
    case 'k':
        id = DOUBUTSUGO_Boy_ka;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ka;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_ke;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ki;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_ko;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_ku;
            break;
        }
        break;
    case 'l':
        id = DOUBUTSUGO_Boy_ru;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ra;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_re;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ri;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_ro;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_ru;
            break;
        }
        break;
    case 'm':
        id = DOUBUTSUGO_Boy_mu;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ma;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_me;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_mi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_mo;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_mu;
            break;
        }
        break;
    case 'n':
        id = DOUBUTSUGO_Boy_0n;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_na;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_ne;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ni;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_no;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_nu;
            break;
        }
        break;
    case 'o':
        id = DOUBUTSUGO_Boy_0o;
        switch (next) {
        case 'o':
            id = DOUBUTSUGO_Boy_oo;
            break;
        }
        break;
    case 'p':
        id = DOUBUTSUGO_Boy_pi;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_pa;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_pe;
            break;
        case 'h':
            id = DOUBUTSUGO_Boy_fu;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_pi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_po;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_pu;
            break;
        }
        break;
    case 'q':
        id = DOUBUTSUGO_Boy_qu;
        switch (next) {
        case 'u':
            id = DOUBUTSUGO_Boy_qu;
            break;
        }
        break;
    case 'r':
        id = DOUBUTSUGO_Boy_ra;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ra;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_re;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ri;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_ro;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_ru;
            break;
        case 'y':
            id = DOUBUTSUGO_Boy_ry;
            break;
        }
        break;
    case 's':
        id = DOUBUTSUGO_Boy_su;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_sa;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_se;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_si;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_so;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_su;
            break;
        }
        break;
    case 't':
        id = DOUBUTSUGO_Boy_tu;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ta;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_te;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ti;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_to;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_tu;
            break;
        case 'y':
            id = DOUBUTSUGO_Boy_ty;
            break;
        }
        break;
    case 'u':
        id = DOUBUTSUGO_Boy_0u;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_wa;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_0u;
            break;
        }
        break;
    case 'v':
        id = DOUBUTSUGO_Boy_vi;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ba;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_be;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_vi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_bo;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_bu;
            break;
        }
        break;
    case 'w':
        id = DOUBUTSUGO_Boy_0u;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_wa;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_0e;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_0i;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_0o;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_0u;
            break;
        }
        break;
    case 'x':
        id = DOUBUTSUGO_Boy_ix;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_za;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_ze;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_ku;
            break;
        }
        break;
    case 'y':
        id = DOUBUTSUGO_Boy_ty;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ya;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_yo;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_yu;
            break;
        }
        break;
    case 'z':
        id = DOUBUTSUGO_Boy_zu;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_za;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_ze;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_zi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_zo;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_zu;
            break;
        }
        break;
    }
    if (id != 0) {
        id = base + id - DOUBUTSUGO_Boy_0a_l;
    }
    return id;
}

// 801DA1CC: Hangul syllables are 0xAC00 + (initial * 21 + vowel) * 28 + final.
int SoundVoice::getHangulKana(u16 c) {
    int kana = c;
    int initial = (c - 0xAC00) / (21 * 28);
    int vowel = (c - 0xAC00) % (21 * 28) / 28;
    switch (initial) {
    case 0:
        switch (vowel) {
        case 0:
        case 1:
        case 2:
        case 9:
            kana = K(ga);
            break;
        case 3:
        case 5:
        case 7:
        case 0xA:
        case 0xB:
        case 0xF:
            kana = K(ge);
            break;
        case 4:
        case 6:
        case 8:
        case 0xC:
        case 0xE:
            kana = K(go);
            break;
        case 0xD:
        case 0x11:
        case 0x12:
            kana = K(gu);
            break;
        case 0x10:
        case 0x13:
        case 0x14:
            kana = K(gi);
            break;
        }
        break;
    case 1:
    case 0xF:
        switch (vowel) {
        case 0:
        case 1:
        case 2:
        case 9:
            kana = K(ka);
            break;
        case 3:
        case 5:
        case 7:
        case 0xA:
        case 0xB:
        case 0xF:
            kana = K(ke);
            break;
        case 4:
        case 6:
        case 8:
        case 0xC:
        case 0xE:
            kana = K(ko);
            break;
        case 0xD:
        case 0x11:
        case 0x12:
            kana = K(ku);
            break;
        case 0x10:
        case 0x13:
        case 0x14:
            kana = K(ki);
            break;
        }
        break;
    case 2:
        switch (vowel) {
        case 0:
        case 1:
        case 2:
        case 9:
            kana = K(na);
            break;
        case 3:
        case 5:
        case 7:
        case 0xA:
        case 0xB:
        case 0xF:
            kana = K(ne);
            break;
        case 4:
        case 6:
        case 8:
        case 0xC:
        case 0xE:
            kana = K(no);
            break;
        case 0xD:
        case 0x11:
        case 0x12:
            kana = K(nu);
            break;
        case 0x10:
        case 0x13:
        case 0x14:
            kana = K(ni);
            break;
        }
        break;
    case 3:
        switch (vowel) {
        case 0:
        case 1:
        case 2:
        case 9:
            kana = K(da);
            break;
        case 3:
        case 5:
        case 7:
        case 0xA:
        case 0xB:
        case 0xF:
            kana = K(de);
            break;
        case 4:
        case 6:
        case 8:
        case 0xC:
        case 0xE:
            kana = K(do);
            break;
        case 0xD:
        case 0x11:
        case 0x12:
            kana = K(zu);
            break;
        case 0x10:
        case 0x13:
        case 0x14:
            kana = K(zi);
            break;
        }
        break;
    case 4:
    case 0xE:
    case 0x10:
        switch (vowel) {
        case 0:
        case 1:
        case 2:
        case 9:
            kana = K(ta);
            break;
        case 3:
        case 5:
        case 7:
        case 0xA:
        case 0xB:
        case 0xF:
            kana = K(te);
            break;
        case 4:
        case 6:
        case 8:
        case 0xC:
        case 0xE:
            kana = K(to);
            break;
        case 0xD:
        case 0x11:
        case 0x12:
            kana = K(tu);
            break;
        case 0x10:
        case 0x13:
        case 0x14:
            kana = K(ti);
            break;
        }
        break;
    case 5:
        switch (vowel) {
        case 0:
        case 1:
        case 2:
        case 9:
            kana = K(ra);
            break;
        case 3:
        case 5:
        case 7:
        case 0xA:
        case 0xB:
        case 0xF:
            kana = K(re);
            break;
        case 4:
        case 6:
        case 8:
        case 0xC:
        case 0xE:
            kana = K(ro);
            break;
        case 0xD:
        case 0x11:
        case 0x12:
            kana = K(ru);
            break;
        case 0x10:
        case 0x13:
        case 0x14:
            kana = K(ri);
            break;
        }
        break;
    case 6:
        switch (vowel) {
        case 0:
        case 1:
        case 2:
        case 9:
            kana = K(ma);
            break;
        case 3:
        case 5:
        case 7:
        case 0xA:
        case 0xB:
        case 0xF:
            kana = K(me);
            break;
        case 4:
        case 6:
        case 8:
        case 0xC:
        case 0xE:
            kana = K(mo);
            break;
        case 0xD:
        case 0x11:
        case 0x12:
            kana = K(mu);
            break;
        case 0x10:
        case 0x13:
        case 0x14:
            kana = K(mi);
            break;
        }
        break;
    case 7:
        switch (vowel) {
        case 0:
        case 1:
        case 2:
        case 9:
            kana = K(ba);
            break;
        case 3:
        case 5:
        case 7:
        case 0xA:
        case 0xB:
        case 0xF:
            kana = K(pe);
            break;
        case 4:
        case 6:
        case 8:
        case 0xC:
        case 0xE:
            kana = K(bo);
            break;
        case 0xD:
        case 0x11:
        case 0x12:
            kana = K(bu);
            break;
        case 0x10:
        case 0x13:
        case 0x14:
            kana = K(bi);
            break;
        }
        break;
    case 8:
    case 0x11:
        switch (vowel) {
        case 0:
        case 1:
        case 2:
        case 9:
            kana = K(pa);
            break;
        case 3:
        case 5:
        case 7:
        case 0xA:
        case 0xB:
        case 0xF:
            kana = K(be);
            break;
        case 4:
        case 6:
        case 8:
        case 0xC:
        case 0xE:
            kana = K(po);
            break;
        case 0xD:
        case 0x11:
        case 0x12:
            kana = K(pu);
            break;
        case 0x10:
        case 0x13:
        case 0x14:
            kana = K(pi);
            break;
        }
        break;
    case 9:
    case 0xA:
        switch (vowel) {
        case 0:
        case 1:
        case 2:
        case 9:
            kana = K(sa);
            break;
        case 3:
        case 5:
        case 7:
        case 0xA:
        case 0xB:
        case 0xF:
            kana = K(se);
            break;
        case 4:
        case 6:
        case 8:
        case 0xC:
        case 0xE:
            kana = K(so);
            break;
        case 0xD:
        case 0x11:
        case 0x12:
            kana = K(su);
            break;
        case 0x10:
        case 0x13:
        case 0x14:
            kana = K(si);
            break;
        }
        break;
    case 0xB:
        switch (vowel) {
        case 0:
        case 1:
            kana = K(0a);
            break;
        case 3:
        case 5:
        case 7:
        case 0xA:
        case 0xB:
        case 0xF:
            kana = K(0e);
            break;
        case 4:
        case 6:
        case 8:
        case 0xE:
            kana = K(0o);
            break;
        case 0xD:
        case 0x12:
            kana = K(0u);
            break;
        case 0x10:
        case 0x13:
        case 0x14:
            kana = K(0i);
            break;
        case 2:
            kana = K(ya);
            break;
        case 0x11:
            kana = K(yu);
            break;
        case 0xC:
            kana = K(yo);
            break;
        case 9:
            kana = K(wa);
            break;
        }
        break;
    case 0xC:
        switch (vowel) {
        case 0:
        case 1:
        case 2:
        case 9:
            kana = K(za);
            break;
        case 3:
        case 5:
        case 7:
        case 0xA:
        case 0xB:
        case 0xF:
            kana = K(ze);
            break;
        case 4:
        case 6:
        case 8:
        case 0xC:
        case 0xE:
            kana = K(zo);
            break;
        case 0xD:
        case 0x11:
        case 0x12:
            kana = K(zu);
            break;
        case 0x10:
        case 0x13:
        case 0x14:
            kana = K(zi);
            break;
        }
        break;
    case 0xD:
        switch (vowel) {
        case 0:
        case 1:
        case 2:
        case 9:
            kana = K(ta);
            break;
        case 3:
        case 5:
        case 7:
        case 0xA:
        case 0xB:
        case 0xF:
            kana = K(te);
            break;
        case 4:
        case 6:
        case 8:
        case 0xC:
        case 0xE:
            kana = K(to);
            break;
        case 0xD:
        case 0x11:
        case 0x12:
            kana = K(tu);
            break;
        case 0x10:
        case 0x13:
        case 0x14:
            kana = K(ti);
            break;
        }
        break;
    case 0x12:
        switch (vowel) {
        case 0:
        case 1:
        case 2:
        case 9:
            kana = K(ha);
            break;
        case 3:
        case 5:
        case 7:
        case 0xA:
        case 0xB:
        case 0xF:
            kana = K(he);
            break;
        case 4:
        case 6:
        case 8:
        case 0xC:
        case 0xE:
            kana = K(ho);
            break;
        case 0xD:
        case 0x11:
        case 0x12:
            kana = K(fu);
            break;
        case 0x10:
        case 0x13:
        case 0x14:
            kana = K(hi);
            break;
        }
        break;
    default:
        kana = 0;
        break;
    }
    return kana;
}

// 801DA658
int SoundVoice::getSyllableEs(u16 c, u16 next) {
    int id = 0;
    int base = getVoiceBase();
    switch (c) {
    case 0x57:
        id = DOUBUTSUGO_Boy_se;
        break;
    case 0x58:
        id = DOUBUTSUGO_Boy_no;
        break;
    case 0x59:
        id = DOUBUTSUGO_Boy_du;
        break;
    case 0x5A:
        id = DOUBUTSUGO_Boy_tu;
        break;
    case 0x5B:
        id = DOUBUTSUGO_Boy_0u;
        break;
    case 0x5C:
        id = DOUBUTSUGO_Boy_ti;
        break;
    case 0x5D:
        id = DOUBUTSUGO_Boy_te;
        break;
    case 0x5E:
        id = DOUBUTSUGO_Boy_ti;
        break;
    case 0x5F:
        id = DOUBUTSUGO_Boy_0u;
        break;
    case 0x60:
        id = DOUBUTSUGO_Boy_ne;
        break;
    case 'a':
        id = DOUBUTSUGO_Boy_0a;
        switch (next) {
        case 'i':
            id = DOUBUTSUGO_Boy_ai;
            break;
        case 'n':
            id = DOUBUTSUGO_Boy_an;
            break;
        }
        break;
    case 'b':
        id = DOUBUTSUGO_Boy_be;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ba;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_be;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_bi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_bo;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_bu;
            break;
        }
        break;
    case 'c':
        id = DOUBUTSUGO_Boy_si;
        switch (next) {
        case 'h':
            id = DOUBUTSUGO_Boy_ch;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ci;
            break;
        case 'a':
            id = DOUBUTSUGO_Boy_ka;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_ko;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_ku;
            break;
        }
        break;
    case 'd':
        id = DOUBUTSUGO_Boy_de;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_da;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_de;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_di;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_do;
            break;
        }
        break;
    case 'e':
        id = DOUBUTSUGO_Boy_ge;
        switch (next) {
        case 'e':
            id = DOUBUTSUGO_Boy_ee;
            break;
        case 'f':
            id = DOUBUTSUGO_Boy_ef;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ei;
            break;
        case 'm':
            id = DOUBUTSUGO_Boy_em;
            break;
        case 'n':
            id = DOUBUTSUGO_Boy_en;
            break;
        case 's':
            id = DOUBUTSUGO_Boy_es;
            break;
        }
        break;
    case 'f':
        id = DOUBUTSUGO_Boy_he;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ha;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_ho;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_fu;
            break;
        case 'y':
            id = DOUBUTSUGO_Boy_gy;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_he;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_hi;
            break;
        }
        break;
    case 'g':
        id = DOUBUTSUGO_Boy_ge;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ga;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_hi;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_he;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_go;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_gu;
            break;
        case 'y':
            id = DOUBUTSUGO_Boy_gy;
            break;
        }
        break;
    case 'h':
        id = DOUBUTSUGO_Boy_ha;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ha;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_hi;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_he;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_go;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_gu;
            break;
        case 'y':
            id = DOUBUTSUGO_Boy_gy;
            break;
        }
        break;
    case 'i':
        id = DOUBUTSUGO_Boy_yo;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ya;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_yo;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_yu;
            break;
        case 'x':
            id = DOUBUTSUGO_Boy_ix;
            break;
        }
        break;
    case 'j':
        id = DOUBUTSUGO_Boy_to;
        switch (next) {
        case 'e':
            id = DOUBUTSUGO_Boy_he;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_hi;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_fu;
            break;
        }
        break;
    case 'k':
        id = DOUBUTSUGO_Boy_ku;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ka;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_ke;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ki;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_ko;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_ku;
            break;
        }
        break;
    case 'l':
        id = DOUBUTSUGO_Boy_ru;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ra;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_re;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ri;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_ro;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_ru;
            break;
        }
        break;
    case 'm':
        id = DOUBUTSUGO_Boy_mu;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ma;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_me;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_mi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_mo;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_mu;
            break;
        }
        break;
    case 'n':
        id = DOUBUTSUGO_Boy_nu;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_na;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_ne;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ni;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_no;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_nu;
            break;
        }
        break;
    case 'o':
        id = DOUBUTSUGO_Boy_ro;
        switch (next) {
        case 'm':
            id = DOUBUTSUGO_Boy_0n;
            break;
        case 'h':
        case 'o':
            id = DOUBUTSUGO_Boy_oo;
            break;
        }
        break;
    case 'p':
        id = DOUBUTSUGO_Boy_pu;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_pa;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_pe;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_pi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_po;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_pu;
            break;
        }
        break;
    case 'q':
        id = DOUBUTSUGO_Boy_ku;
        switch (next) {
        case 'u':
            id = DOUBUTSUGO_Boy_qu;
            break;
        }
        break;
    case 'r':
        id = DOUBUTSUGO_Boy_ru;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ra;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_re;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ri;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_ro;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_ru;
            break;
        case 'y':
            id = DOUBUTSUGO_Boy_ry;
            break;
        }
        break;
    case 's':
        id = DOUBUTSUGO_Boy_su;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_sa;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_se;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_si;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_so;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_su;
            break;
        }
        break;
    case 't':
        id = DOUBUTSUGO_Boy_tu;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ta;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_te;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ti;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_to;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_tu;
            break;
        case 'y':
            id = DOUBUTSUGO_Boy_ty;
            break;
        }
        break;
    case 'u':
        id = DOUBUTSUGO_Boy_0u;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_wa;
            break;
        }
        break;
    case 'v':
        id = DOUBUTSUGO_Boy_be;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ba;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_be;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_vi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_bo;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_bu;
            break;
        }
        break;
    case 'w':
        id = DOUBUTSUGO_Boy_vi;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_wa;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_0o;
            break;
        }
        break;
    case 'x':
        id = DOUBUTSUGO_Boy_he;
        break;
    case 'y':
        id = DOUBUTSUGO_Boy_yu;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ha;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_ho;
            break;
        }
        break;
    case 'z':
        id = DOUBUTSUGO_Boy_se;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_sa;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_se;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_si;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_so;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_su;
            break;
        }
        break;
    }
    if (id != 0) {
        id = base + id - DOUBUTSUGO_Boy_0a_l;
    }
    return id;
}

// 801DAE14
int SoundVoice::getSyllableFr(u16 c, u16 next) {
    int id = 0;
    int base = getVoiceBase();
    switch (c) {
    case 0x57:
        id = DOUBUTSUGO_Boy_se;
        break;
    case 0x58:
        id = DOUBUTSUGO_Boy_an;
        break;
    case 0x59:
        id = DOUBUTSUGO_Boy_do;
        break;
    case 0x5A:
        id = DOUBUTSUGO_Boy_to;
        break;
    case 0x5B:
        id = DOUBUTSUGO_Boy_ku;
        break;
    case 0x5C:
        id = DOUBUTSUGO_Boy_sa;
        break;
    case 0x5D:
        id = DOUBUTSUGO_Boy_si;
        break;
    case 0x5E:
        id = DOUBUTSUGO_Boy_se;
        break;
    case 0x5F:
        id = DOUBUTSUGO_Boy_0u;
        break;
    case 0x60:
        id = DOUBUTSUGO_Boy_na;
        break;
    case 'a':
        id = DOUBUTSUGO_Boy_0a;
        switch (next) {
        case 'i':
            id = DOUBUTSUGO_Boy_0e;
            break;
        case 'n':
            id = DOUBUTSUGO_Boy_an;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_0o;
            break;
        }
        break;
    case 'b':
        id = DOUBUTSUGO_Boy_be;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_be;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_bi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_bo;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_bu;
            break;
        }
        break;
    case 'c':
        id = DOUBUTSUGO_Boy_se;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ke;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_so;
            break;
        case 'h':
            id = DOUBUTSUGO_Boy_ch;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ki;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_ko;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_ku;
            break;
        }
        break;
    case 'd':
        id = DOUBUTSUGO_Boy_de;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_de;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_da;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_di;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_do;
            break;
        }
        break;
    case 'e':
        id = DOUBUTSUGO_Boy_0e;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_0o;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_0e;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_ee;
            break;
        case 'f':
            id = DOUBUTSUGO_Boy_ef;
            break;
        case 'm':
            id = DOUBUTSUGO_Boy_em;
            break;
        case 'n':
            id = DOUBUTSUGO_Boy_en;
            break;
        case 's':
            id = DOUBUTSUGO_Boy_es;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_0u;
            break;
        }
        break;
    case 'f':
        id = DOUBUTSUGO_Boy_fu;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ha;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_he;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_hi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_ho;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_fu;
            break;
        }
        break;
    case 'g':
        id = DOUBUTSUGO_Boy_ga;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ge;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_ga;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_gi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_go;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_gu;
            break;
        case 'y':
            id = DOUBUTSUGO_Boy_gy;
            break;
        case 'n':
            id = DOUBUTSUGO_Boy_nu;
            break;
        }
        break;
    case 'h':
        id = DOUBUTSUGO_Boy_wi_dummy;
        break;
    case 'i':
        id = DOUBUTSUGO_Boy_0i;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ya;
            break;
        case 'm':
            id = DOUBUTSUGO_Boy_0a;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_yo;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_yu;
            break;
        case 'x':
            id = DOUBUTSUGO_Boy_ix;
            break;
        }
        break;
    case 'j':
        id = DOUBUTSUGO_Boy_ya;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ze;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_je;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_zi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_zo;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_zu;
            break;
        }
        break;
    case 'k':
        id = DOUBUTSUGO_Boy_ka;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ka;
            break;
        }
        break;
    case 'l':
        id = DOUBUTSUGO_Boy_ru;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ra;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_re;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ri;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_ro;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_ru;
            break;
        }
        break;
    case 'm':
        id = DOUBUTSUGO_Boy_me;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_me;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_ma;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_mi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_mo;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_mu;
            break;
        }
        break;
    case 'n':
        id = DOUBUTSUGO_Boy_ne;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ne;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_na;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ni;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_no;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_nu;
            break;
        }
        break;
    case 'o':
        id = DOUBUTSUGO_Boy_oo;
        switch (next) {
        case 'e':
        case 'u':
            id = DOUBUTSUGO_Boy_0u;
            break;
        case 'm':
            id = DOUBUTSUGO_Boy_0n;
            break;
        case 'h':
        case 'o':
            id = DOUBUTSUGO_Boy_oo;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_wa;
            break;
        }
        break;
    case 'p':
        id = DOUBUTSUGO_Boy_pa;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_pe;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_pa;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_pi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_po;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_pu;
            break;
        case 'h':
            id = DOUBUTSUGO_Boy_fu;
            break;
        }
        break;
    case 'q':
        id = DOUBUTSUGO_Boy_ku;
        switch (next) {
        case 'u':
            id = DOUBUTSUGO_Boy_qu;
            break;
        }
        break;
    case 'r':
        id = DOUBUTSUGO_Boy_re;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ra;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ri;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_ro;
            break;
        case 'e':
        case 'u':
            id = DOUBUTSUGO_Boy_ru;
            break;
        case 'y':
            id = DOUBUTSUGO_Boy_ry;
            break;
        }
        break;
    case 's':
        id = DOUBUTSUGO_Boy_si;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_sa;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_se;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_si;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_su;
            break;
        }
        break;
    case 't':
        id = DOUBUTSUGO_Boy_ti;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_te;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_ta;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ti;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_to;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_tu;
            break;
        case 'y':
            id = DOUBUTSUGO_Boy_ty;
            break;
        }
        break;
    case 'u':
        id = DOUBUTSUGO_Boy_0u;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_wa;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_0i;
            break;
        }
        break;
    case 'v':
        id = DOUBUTSUGO_Boy_ba;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_be;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_ba;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_vi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_bo;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_bu;
            break;
        }
        break;
    case 'w':
        id = DOUBUTSUGO_Boy_do;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_wa;
            break;
        }
        break;
    case 'x':
        id = DOUBUTSUGO_Boy_ix;
        break;
    case 'y':
        id = DOUBUTSUGO_Boy_0i;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ya;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_yo;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_yu;
            break;
        }
        break;
    case 'z':
        id = DOUBUTSUGO_Boy_se;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ze;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_za;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_zi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_zo;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_zu;
            break;
        }
        break;
    }
    if (id != 0) {
        id = base + id - DOUBUTSUGO_Boy_0a_l;
    }
    return id;
}

// 801DB5A0
int SoundVoice::getSyllableDe(u16 c, u16 next) {
    int id = 0;
    int base = getVoiceBase();
    switch (c) {
    case 0x57:
        id = DOUBUTSUGO_Boy_ze;
        break;
    case 0x58:
        id = DOUBUTSUGO_Boy_ai;
        break;
    case 0x59:
        id = DOUBUTSUGO_Boy_tu;
        break;
    case 0x5A:
        id = DOUBUTSUGO_Boy_do;
        break;
    case 0x5B:
        id = DOUBUTSUGO_Boy_bi;
        break;
    case 0x5C:
        id = DOUBUTSUGO_Boy_fu;
        break;
    case 0x5D:
        id = DOUBUTSUGO_Boy_ze;
        break;
    case 0x5E:
        id = DOUBUTSUGO_Boy_zi;
        break;
    case 0x5F:
        id = DOUBUTSUGO_Boy_0a;
        break;
    case 0x60:
        id = DOUBUTSUGO_Boy_no;
        break;
    case 'a':
        id = DOUBUTSUGO_Boy_0a;
        switch (next) {
        case 'n':
            id = DOUBUTSUGO_Boy_an;
            break;
        case 'i':
        case 'y':
            id = DOUBUTSUGO_Boy_ai;
            break;
        }
        break;
    case 'b':
        id = DOUBUTSUGO_Boy_be;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ba;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_be;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_bi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_bo;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_bu;
            break;
        }
        break;
    case 'c':
        id = DOUBUTSUGO_Boy_tu;
        switch (next) {
        case 'i':
            id = DOUBUTSUGO_Boy_ci;
            break;
        case 'h':
            id = DOUBUTSUGO_Boy_hi;
            break;
        case 'k':
            id = DOUBUTSUGO_Boy_ku;
            break;
        }
        break;
    case 'd':
        id = DOUBUTSUGO_Boy_de;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_da;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_de;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_di;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_do;
            break;
        case 't':
            id = DOUBUTSUGO_Boy_to;
            break;
        case 's':
            id = DOUBUTSUGO_Boy_tu;
            break;
        }
        break;
    case 'e':
        id = DOUBUTSUGO_Boy_0e;
        switch (next) {
        case 'e':
            id = DOUBUTSUGO_Boy_ee;
            break;
        case 'f':
            id = DOUBUTSUGO_Boy_ef;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ai;
            break;
        case 'm':
            id = DOUBUTSUGO_Boy_em;
            break;
        case 'n':
            id = DOUBUTSUGO_Boy_en;
            break;
        case 's':
            id = DOUBUTSUGO_Boy_es;
            break;
        case 'y':
            id = DOUBUTSUGO_Boy_ai;
            break;
        }
        break;
    case 'f':
        id = DOUBUTSUGO_Boy_ef;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ha;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_he;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_hi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_ho;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_fu;
            break;
        }
        break;
    case 'g':
        id = DOUBUTSUGO_Boy_ge;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ga;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_ge;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_gi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_go;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_gu;
            break;
        case 'y':
            id = DOUBUTSUGO_Boy_gy;
            break;
        }
        break;
    case 'h':
        id = DOUBUTSUGO_Boy_ha;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ha;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_he;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_hi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_ho;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_fu;
            break;
        }
        break;
    case 'i':
        id = DOUBUTSUGO_Boy_0i;
        switch (next) {
        case 'e':
            id = DOUBUTSUGO_Boy_0i;
            break;
        case 'x':
            id = DOUBUTSUGO_Boy_ix;
            break;
        }
        break;
    case 'j':
        id = DOUBUTSUGO_Boy_yo;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ya;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_je;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_zi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_yo;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_yu;
            break;
        }
        break;
    case 'k':
        id = DOUBUTSUGO_Boy_ka;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ka;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_ke;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ki;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_ko;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_ku;
            break;
        }
        break;
    case 'l':
        id = DOUBUTSUGO_Boy_ru;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ra;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_re;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ri;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_ro;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_ru;
            break;
        }
        break;
    case 'm':
        id = DOUBUTSUGO_Boy_nu;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ma;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_me;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_mi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_mo;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_mu;
            break;
        }
        break;
    case 'n':
        id = DOUBUTSUGO_Boy_ne;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_na;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_ne;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ni;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_no;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_nu;
            break;
        }
        break;
    case 'o':
        id = DOUBUTSUGO_Boy_oo;
        switch (next) {
        case 'h':
        case 'o':
            id = DOUBUTSUGO_Boy_oo;
            break;
        }
        break;
    case 'p':
        id = DOUBUTSUGO_Boy_pu;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_pa;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_pe;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_pi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_po;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_pu;
            break;
        }
        break;
    case 'q':
        id = DOUBUTSUGO_Boy_ku;
        switch (next) {
        case 'u':
            id = DOUBUTSUGO_Boy_qu;
            break;
        }
        break;
    case 'r':
        id = DOUBUTSUGO_Boy_ra;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ra;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_re;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ri;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_ro;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_ru;
            break;
        case 'y':
            id = DOUBUTSUGO_Boy_ry;
            break;
        }
        break;
    case 's':
        id = DOUBUTSUGO_Boy_ze;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_za;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_ze;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_zi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_zo;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_zu;
            break;
        }
        break;
    case 't':
        id = DOUBUTSUGO_Boy_te;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ta;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_te;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ti;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_to;
            break;
        case 's':
        case 'u':
        case 'z':
            id = DOUBUTSUGO_Boy_tu;
            break;
        case 'y':
            id = DOUBUTSUGO_Boy_ty;
            break;
        }
        break;
    case 'u':
        id = DOUBUTSUGO_Boy_yu;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_wa;
            break;
        }
        break;
    case 'v':
        id = DOUBUTSUGO_Boy_bi;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ha;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_vi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_ho;
            break;
        }
        break;
    case 'w':
        id = DOUBUTSUGO_Boy_be;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_wa;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_vi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_wo;
            break;
        }
        break;
    case 'x':
        id = DOUBUTSUGO_Boy_si;
        break;
    case 'y':
        id = DOUBUTSUGO_Boy_ti;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ya;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_yo;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_yu;
            break;
        }
        break;
    case 'z':
        id = DOUBUTSUGO_Boy_ze;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_za;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_ze;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_zi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_zo;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_zu;
            break;
        }
        break;
    case 0x9A:
        id = DOUBUTSUGO_Boy_su;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_sa;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_se;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_si;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_so;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_su;
            break;
        }
        break;
    }
    if (id != 0) {
        id = base + id - DOUBUTSUGO_Boy_0a_l;
    }
    return id;
}

// 801DBDAC
int SoundVoice::getSyllableIt(u16 c, u16 next) {
    int id = 0;
    int base = getVoiceBase();
    switch (c) {
    case 0x57:
        id = DOUBUTSUGO_Boy_ze;
        break;
    case 0x58:
        id = DOUBUTSUGO_Boy_no;
        break;
    case 0x59:
        id = DOUBUTSUGO_Boy_do;
        break;
    case 0x5A:
        id = DOUBUTSUGO_Boy_tu;
        break;
    case 0x5B:
        id = DOUBUTSUGO_Boy_0u;
        break;
    case 0x5C:
        id = DOUBUTSUGO_Boy_ti;
        break;
    case 0x5D:
        id = DOUBUTSUGO_Boy_te;
        break;
    case 0x5E:
        id = DOUBUTSUGO_Boy_he;
        break;
    case 0x5F:
        id = DOUBUTSUGO_Boy_to;
        break;
    case 0x60:
        id = DOUBUTSUGO_Boy_no;
        break;
    case 'a':
        id = DOUBUTSUGO_Boy_0a;
        switch (next) {
        case 'i':
            id = DOUBUTSUGO_Boy_ai;
            break;
        case 'n':
            id = DOUBUTSUGO_Boy_an;
            break;
        }
        break;
    case 'b':
        id = DOUBUTSUGO_Boy_bi;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ba;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_be;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_bi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_bo;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_bu;
            break;
        }
        break;
    case 'c':
        id = DOUBUTSUGO_Boy_si;
        switch (next) {
        case 'h':
            id = DOUBUTSUGO_Boy_ku;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ci;
            break;
        case 'a':
            id = DOUBUTSUGO_Boy_ka;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_ko;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_ku;
            break;
        }
        break;
    case 'd':
        id = DOUBUTSUGO_Boy_di;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_da;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_de;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_di;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_do;
            break;
        }
        break;
    case 'e':
        id = DOUBUTSUGO_Boy_de;
        switch (next) {
        case 'e':
            id = DOUBUTSUGO_Boy_ee;
            break;
        case 'f':
            id = DOUBUTSUGO_Boy_ef;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ai;
            break;
        case 'm':
            id = DOUBUTSUGO_Boy_em;
            break;
        case 'n':
            id = DOUBUTSUGO_Boy_en;
            break;
        case 's':
            id = DOUBUTSUGO_Boy_es;
            break;
        }
        break;
    case 'f':
        id = DOUBUTSUGO_Boy_he;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ha;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_he;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_hi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_ho;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_fu;
            break;
        }
        break;
    case 'g':
        id = DOUBUTSUGO_Boy_gi;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ga;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_he;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_hi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_go;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_gu;
            break;
        case 'y':
            id = DOUBUTSUGO_Boy_gy;
            break;
        }
        break;
    case 'h':
        id = DOUBUTSUGO_Boy_ha;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_0a;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_0e;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_0i;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_0o;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_0u;
            break;
        }
        break;
    case 'i':
        id = DOUBUTSUGO_Boy_0i;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ya;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_yo;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_yu;
            break;
        case 'x':
            id = DOUBUTSUGO_Boy_ix;
            break;
        }
        break;
    case 'j':
        id = DOUBUTSUGO_Boy_gi;
        switch (next) {
        case 'e':
            id = DOUBUTSUGO_Boy_he;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_hi;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_fu;
            break;
        }
        break;
    case 'k':
        id = DOUBUTSUGO_Boy_ku;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ka;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_ke;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ki;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_ko;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_ku;
            break;
        }
        break;
    case 'l':
        id = DOUBUTSUGO_Boy_ra;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ra;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_re;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ri;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_ro;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_ru;
            break;
        }
        break;
    case 'm':
        id = DOUBUTSUGO_Boy_ma;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ma;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_me;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_mi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_mo;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_mu;
            break;
        }
        break;
    case 'n':
        id = DOUBUTSUGO_Boy_ne;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_na;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_ne;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ni;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_no;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_nu;
            break;
        }
        break;
    case 'o':
        id = DOUBUTSUGO_Boy_no;
        switch (next) {
        case 'm':
            id = DOUBUTSUGO_Boy_0n;
            break;
        case 'h':
        case 'o':
            id = DOUBUTSUGO_Boy_oo;
            break;
        }
        break;
    case 'p':
        id = DOUBUTSUGO_Boy_pi;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_pa;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_pe;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_pi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_po;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_pu;
            break;
        }
        break;
    case 'q':
        id = DOUBUTSUGO_Boy_ku;
        switch (next) {
        case 'u':
            id = DOUBUTSUGO_Boy_qu;
            break;
        }
        break;
    case 'r':
        id = DOUBUTSUGO_Boy_ra;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ra;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_re;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ri;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_ro;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_ru;
            break;
        case 'y':
            id = DOUBUTSUGO_Boy_ry;
            break;
        }
        break;
    case 's':
        id = DOUBUTSUGO_Boy_si;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_sa;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_se;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_si;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_so;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_su;
            break;
        }
        break;
    case 't':
        id = DOUBUTSUGO_Boy_ti;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ta;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_te;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_ti;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_to;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_tu;
            break;
        case 'y':
            id = DOUBUTSUGO_Boy_ty;
            break;
        }
        break;
    case 'u':
        id = DOUBUTSUGO_Boy_0u;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_wa;
            break;
        }
        break;
    case 'v':
        id = DOUBUTSUGO_Boy_bu;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ba;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_be;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_vi;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_bo;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_bu;
            break;
        }
        break;
    case 'w':
        id = DOUBUTSUGO_Boy_vi;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_wa;
            break;
        }
        break;
    case 'x':
        id = DOUBUTSUGO_Boy_ku;
        break;
    case 'y':
        id = DOUBUTSUGO_Boy_ya;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_ha;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_ho;
            break;
        }
        break;
    case 'z':
        id = DOUBUTSUGO_Boy_zu;
        switch (next) {
        case 'a':
            id = DOUBUTSUGO_Boy_sa;
            break;
        case 'e':
            id = DOUBUTSUGO_Boy_se;
            break;
        case 'i':
            id = DOUBUTSUGO_Boy_si;
            break;
        case 'o':
            id = DOUBUTSUGO_Boy_so;
            break;
        case 'u':
            id = DOUBUTSUGO_Boy_su;
            break;
        }
        break;
    }
    if (id != 0) {
        id = base + id - DOUBUTSUGO_Boy_0a_l;
    }
    return id;
}
