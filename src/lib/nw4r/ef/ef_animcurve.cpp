#include <nw4r/ef.h>
#include <nw4r/math.h>
#include <nw4r/ef/ef_animcurve.h>
#include <nw4r/ef/ef_creationqueue.h>
#include <cmath>

namespace nw4r {
namespace ef {

static u8 gCountStandBit[256] = {
    0x00, 0x01, 0x01, 0x02, 0x01, 0x02, 0x02, 0x03, 0x01, 0x02, 0x02, 0x03,
    0x02, 0x03, 0x03, 0x04, 0x01, 0x02, 0x02, 0x03, 0x02, 0x03, 0x03, 0x04,
    0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05, 0x01, 0x02, 0x02, 0x03,
    0x02, 0x03, 0x03, 0x04, 0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05,
    0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05, 0x03, 0x04, 0x04, 0x05,
    0x04, 0x05, 0x05, 0x06, 0x01, 0x02, 0x02, 0x03, 0x02, 0x03, 0x03, 0x04,
    0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05, 0x02, 0x03, 0x03, 0x04,
    0x03, 0x04, 0x04, 0x05, 0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06,
    0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05, 0x03, 0x04, 0x04, 0x05,
    0x04, 0x05, 0x05, 0x06, 0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06,
    0x04, 0x05, 0x05, 0x06, 0x05, 0x06, 0x06, 0x07, 0x01, 0x02, 0x02, 0x03,
    0x02, 0x03, 0x03, 0x04, 0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05,
    0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05, 0x03, 0x04, 0x04, 0x05,
    0x04, 0x05, 0x05, 0x06, 0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05,
    0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06, 0x03, 0x04, 0x04, 0x05,
    0x04, 0x05, 0x05, 0x06, 0x04, 0x05, 0x05, 0x06, 0x05, 0x06, 0x06, 0x07,
    0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05, 0x03, 0x04, 0x04, 0x05,
    0x04, 0x05, 0x05, 0x06, 0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06,
    0x04, 0x05, 0x05, 0x06, 0x05, 0x06, 0x06, 0x07, 0x03, 0x04, 0x04, 0x05,
    0x04, 0x05, 0x05, 0x06, 0x04, 0x05, 0x05, 0x06, 0x05, 0x06, 0x06, 0x07,
    0x04, 0x05, 0x05, 0x06, 0x05, 0x06, 0x06, 0x07, 0x05, 0x06, 0x06, 0x07,
    0x06, 0x07, 0x07, 0x08,
};

static u8 gMSB[256] = {
    0x00, 0x01, 0x02, 0x02, 0x04, 0x04, 0x04, 0x04, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80,
};

enum CurveInterpType {
    CURVETYPE_LINEAR = 0,
    CURVETYPE_HERMITE = 1,
    CURVETYPE_STEP = 2,
};

enum KeyFlag {
    KEYFLAG_RANDOM = (1 << 0),
    KEYFLAG_RANDOMTABLE = (1 << 1),
};

enum ChildType {
    CHILDTYPE_PARTICLE = 0,
    CHILDTYPE_EMITTER = 1,
};

static const bool SEARCHKEYINDEX_FRONT = true;
static const bool SEARCHKEYINDEX_BACK = false;

enum KeyOffset {
    KEY_FLAG = 2,
    KEY_CURVETYPE = 4,
    KEY_PARAM1 = 12,
};

#define U8RANDCLAMP(val, tbl, rand)                                            \
    val = *tbl++;                                                              \
    val += *tbl++ * static_cast<s16>(rand >> 16) / 32768;                      \
    if (val < 0) {                                                             \
        val = 0;                                                               \
    }                                                                          \
    if (val > 255) {                                                           \
        val = 255;                                                             \
    }

#define HAS_SYNCRAND(header)                                                   \
    (((header)->processFlag & AnimCurveHeader::PROC_FLAG_SYNC) != 0)

static inline u8 CntBit1(u8 x) {
    return gCountStandBit[x];
}

static inline u8 GetMSB(u8 x) {
    return gMSB[x];
}

static inline void getAnimHeaders(u8 *pHead, AnimCurveHeader **header,
                                  AnimCurveKey **key, AnimCurveRandom **random,
                                  AnimCurveRandomTable **randomTable,
                                  AnimCurveNameTable **nameTable) {
    *header = reinterpret_cast<AnimCurveHeader *>(pHead);
    *key = reinterpret_cast<AnimCurveKey *>(reinterpret_cast<u8 *>(*header) +
                                            sizeof(AnimCurveHeader));
    *random = reinterpret_cast<AnimCurveRandom *>(reinterpret_cast<u8 *>(*key) +
                                                  (*header)->keyTable);
    *randomTable = reinterpret_cast<AnimCurveRandomTable *>(
        reinterpret_cast<u8 *>(*random) + (*header)->rangeTable);
    *nameTable = reinterpret_cast<AnimCurveNameTable *>(
        reinterpret_cast<u8 *>(*randomTable) + (*header)->randomTable);
}

static inline void calcTick(const u32 life, const AnimCurveHeader *header,
                            const u32 origTick, u16 *tick, f32 *tickF32,
                            u32 *resDiv) {
    *resDiv = 0;

    if (header->frameLength <= 1) {
        *tick = 0;
        *tickF32 = 0;
        *resDiv = origTick;
        return;
    }

    if ((header->processFlag & AnimCurveHeader::PROC_FLAG_INFLOOP) == 0 &&
        header->loopCount <= 1) {
        if ((header->processFlag & AnimCurveHeader::PROC_FLAG_FITTING) == 0) {
            if (origTick >= header->frameLength - 1) {
                *tick = static_cast<u16>(header->frameLength - 1);
            } else {
                *tick = static_cast<u16>(origTick);
            }

            *tickF32 = *tick;
        } else {
            if (life == 1) {
                *tickF32 = header->frameLength - 1;
            } else {
                *tickF32 = static_cast<f32>(header->frameLength - 1) /
                           (life - 1) * origTick;

                if (*tickF32 > static_cast<f32>(header->frameLength - 1)) {
                    *tickF32 = header->frameLength - 1;
                }
            }

            *tick = static_cast<u16>(*tickF32);
        }
    } else {
        if ((header->processFlag & AnimCurveHeader::PROC_FLAG_FITTING) == 0) {
            u32 div = origTick / (header->frameLength - 1);
            *resDiv = div;

            if ((header->processFlag & AnimCurveHeader::PROC_FLAG_TURN) == 0) {
                if ((header->processFlag &
                     AnimCurveHeader::PROC_FLAG_INFLOOP) == 0 &&
                    div >= header->loopCount) {
                    *tick = static_cast<u16>(header->frameLength - 1);
                    *resDiv = static_cast<u8>(header->loopCount - 1);
                } else {
                    *tick = static_cast<u16>(origTick -
                                             div * (header->frameLength - 1));
                }
            } else {
                if ((header->processFlag &
                     AnimCurveHeader::PROC_FLAG_INFLOOP) == 0 &&
                    div >= header->loopCount) {
                    if ((header->loopCount % 2) == 0) {
                        *tick = 0;
                    } else {
                        *tick = static_cast<u16>(header->frameLength - 1);
                    }

                    *resDiv = static_cast<u8>(header->loopCount - 1);
                } else {
                    if ((div & 1) == 0) {
                        *tick = static_cast<u16>(
                            origTick - div * (header->frameLength - 1));
                    } else {
                        *tick = static_cast<u16>(
                            header->frameLength - 1 -
                            (origTick - div * (header->frameLength - 1)));
                    }
                }
            }

            *tickF32 = *tick;
        } else {
            if (origTick >= life - 1) {
                if ((header->processFlag & AnimCurveHeader::PROC_FLAG_TURN) ==
                        0 ||
                    (header->loopCount % 2) != 0) {
                    *tick = static_cast<u16>(header->frameLength - 1);
                } else {
                    *tick = 0;
                }

                *tickF32 = *tick;
                *resDiv = static_cast<u8>(header->loopCount - 1);
            } else {
                f32 rate = static_cast<f32>(header->frameLength - 1) /
                           (life - 1) * header->loopCount;
                u32 div = static_cast<u32>(origTick * rate /
                                           (header->frameLength - 1));
                *resDiv = div;
                f32 pos = origTick * rate - div * (header->frameLength - 1);

                if ((header->processFlag & AnimCurveHeader::PROC_FLAG_TURN) ==
                    0) {
                    *tickF32 = pos;
                } else {
                    if ((div & 1) == 0) {
                        *tickF32 = pos;
                    } else {
                        *tickF32 = header->frameLength - 1 - pos;
                    }
                }

                *tick = static_cast<u16>(*tickF32);
            }
        }
    }
}

static inline void calcTickStep(const u32 life, const AnimCurveHeader *header,
                                const u32 origTick, u16 *tick, f32 *tickF32,
                                u32 *resDiv, bool *final) {
    *resDiv = 0;
    *final = false;

    if ((header->processFlag & AnimCurveHeader::PROC_FLAG_INFLOOP) == 0 &&
        header->loopCount <= 1) {
        if ((header->processFlag & AnimCurveHeader::PROC_FLAG_FITTING) == 0) {
            if (origTick >= header->frameLength) {
                *tick = static_cast<u16>(header->frameLength);
            } else {
                *tick = static_cast<u16>(origTick);
            }
        } else {
            *tick = static_cast<u16>(header->frameLength * origTick / life);
        }
    } else {
        if ((header->processFlag & AnimCurveHeader::PROC_FLAG_FITTING) == 0) {
            if (header->frameLength <= 1 ||
                (header->processFlag & AnimCurveHeader::PROC_FLAG_TURN) == 0) {
                u32 div = origTick / header->frameLength;

                if ((header->processFlag &
                     AnimCurveHeader::PROC_FLAG_INFLOOP) == 0 &&
                    div >= header->loopCount) {
                    *tick = header->frameLength;
                    *resDiv = static_cast<u8>(header->loopCount - 1);
                } else {
                    *tick =
                        static_cast<u16>(origTick - div * header->frameLength);
                    *resDiv = div;
                }
            } else {
                u32 div = origTick / (header->frameLength - 1);
                *resDiv = div;

                if ((header->processFlag &
                     AnimCurveHeader::PROC_FLAG_INFLOOP) == 0 &&
                    div >= header->loopCount) {
                    if ((header->loopCount % 2) == 0) {
                        if (div == header->loopCount &&
                            origTick - div * (header->frameLength - 1) == 0) {
                            *final = true;
                        }

                        *tick = 0;
                    } else {
                        *tick = static_cast<u16>(header->frameLength - 1);
                    }

                    *resDiv = static_cast<u8>(header->loopCount - 1);
                } else {
                    if ((div & 1) == 0) {
                        *tick = static_cast<u16>(
                            origTick - div * (header->frameLength - 1));
                    } else {
                        *tick = static_cast<u16>(
                            (header->frameLength - 1) -
                            (origTick - div * (header->frameLength - 1)));
                    }
                }
            }
        } else {
            if (origTick >= life) {
                if ((header->processFlag & AnimCurveHeader::PROC_FLAG_TURN) ==
                        0 ||
                    (header->loopCount % 2) != 0) {
                    *tick = header->frameLength;
                } else {
                    *tick = 0;
                }

                *resDiv = static_cast<u8>(header->loopCount - 1);
            } else {
                if (header->frameLength <= 1 ||
                    (header->processFlag & AnimCurveHeader::PROC_FLAG_TURN) ==
                        0) {
                    f32 rate = static_cast<f32>(header->frameLength) / life *
                               header->loopCount;
                    u32 div =
                        static_cast<u32>(origTick * rate / header->frameLength);
                    *resDiv = div;
                    *tick = static_cast<u16>(origTick * rate -
                                             div * header->frameLength);
                } else {
                    f32 rate = (static_cast<f32>(header->frameLength - 1) *
                                    header->loopCount +
                                1) /
                               life;
                    u32 normTick = static_cast<u32>(origTick * rate);
                    u32 div = normTick / (header->frameLength - 1);
                    *resDiv = div;

                    if (div >= header->loopCount) {
                        if ((header->loopCount % 2) == 0) {
                            if (origTick == life - 1) {
                                *final = true;
                            }

                            *tick = 0;
                        } else {
                            *tick = static_cast<u16>(header->frameLength - 1);
                        }

                        *resDiv = static_cast<u8>(header->loopCount - 1);
                    } else {
                        if ((div & 1) == 0) {
                            *tick = static_cast<u16>(
                                normTick - div * (header->frameLength - 1));
                        } else {
                            *tick = static_cast<u16>(
                                (header->frameLength - 1) -
                                (normTick - div * (header->frameLength - 1)));
                        }
                    }
                }
            }
        }
    }

    *tickF32 = *tick;
}

void TestCalcTick(const u32 life, const AnimCurveHeader *header,
                  const u32 origTick, u16 *tick, f32 *tickF32, u32 *resDiv) {
    calcTick(life, header, origTick, tick, tickF32, resDiv);
}

void TestCalcTickStep(const u32 life, const AnimCurveHeader *header,
                      const u32 origTick, u16 *tick, f32 *tickF32,
                      u32 *resDiv) {
    bool final;
    calcTickStep(life, header, origTick, tick, tickF32, resDiv, &final);
}

inline void searchKey(int &frameIdx, bool &onKey, u16 &loFrame, u16 &hiFrame,
                      int tick, f32 tickF32, u8 *ptrBase, int framesize,
                      int start, int end) {
    int lo = start;
    int hi = end;
    int md = (lo + hi) / 2;
    bool isInt = std::fabs(tick - tickF32) < FLT_EPSILON;

    loFrame = *reinterpret_cast<u16 *>(ptrBase + lo * framesize);

    if (tick < loFrame) {
        frameIdx = lo;
        onKey = true;
        return;
    }

    if (tick == loFrame) {
        frameIdx = lo;

        if (lo == end) {
            onKey = true;
        } else {
            onKey = isInt;

            if (!isInt) {
                hiFrame =
                    *reinterpret_cast<u16 *>(ptrBase + (lo + 1) * framesize);
            }
        }

        return;
    }

    hiFrame = *reinterpret_cast<u16 *>(ptrBase + hi * framesize);

    if (hiFrame <= tick) {
        frameIdx = hi;
        onKey = true;
        return;
    }

    u16 mdFrame = *reinterpret_cast<u16 *>(ptrBase + md * framesize);

    while (lo < md) {
        if (tick == mdFrame) {
            frameIdx = md;
            onKey = isInt;

            if (!isInt) {
                loFrame = mdFrame;
                hiFrame = *reinterpret_cast<u16 *>(ptrBase +
                                                   (frameIdx + 1) * framesize);
            }

            return;
        }

        if (mdFrame < tick) {
            lo = md;
            loFrame = mdFrame;
        } else {
            hi = md;
            hiFrame = mdFrame;
        }

        md = (lo + hi) / 2;
        mdFrame = *reinterpret_cast<u16 *>(ptrBase + md * framesize);
    }

    frameIdx = lo;
    onKey = false;
}

inline void searchKeyIndex(int &frameIdx, int tick, u8 *ptrBase, int framesize,
                           int start, int end, bool searchFront) {
    int lo = start;
    int hi = end;
    int md = (lo + hi) / 2;

    u16 loFrame = *reinterpret_cast<u16 *>(ptrBase + lo * framesize);

    if (tick < loFrame || (tick == loFrame && searchFront)) {
        frameIdx = lo;
        return;
    }

    u16 hiFrame = *reinterpret_cast<u16 *>(ptrBase + hi * framesize);

    if (hiFrame < tick || (tick == hiFrame && !searchFront)) {
        frameIdx = hi;
        return;
    }

    u16 mdFrame = *reinterpret_cast<u16 *>(ptrBase + md * framesize);

    while (lo < md) {
        if (tick == mdFrame) {
            if (searchFront) {
                hi = md;
                hiFrame = mdFrame;
            } else {
                lo = md;
                loFrame = mdFrame;
            }
        } else if (mdFrame < tick) {
            lo = md;
            loFrame = mdFrame;
        } else {
            hi = md;
            hiFrame = mdFrame;
        }

        md = (lo + hi) / 2;
        mdFrame = *reinterpret_cast<u16 *>(ptrBase + md * framesize);
    }

    frameIdx = lo;
}

static inline u8 AnimCurveInterpolate(u32 posF, u8 valL, u8 valH, u8 type) {
    switch (type & 3) {
    case CURVETYPE_LINEAR: {
        return static_cast<u8>(
            ((posF * (static_cast<int>(valH) - valL)) >> 16) + valL);
    }

    case CURVETYPE_HERMITE: {
        f32 p = posF / 65536.0f;

        if (type == CURVETYPE_HERMITE) {
            return static_cast<u8>(valL +
                                   (valH - valL) * (3.0f - 2.0f * p) * p * p);
        }

        f32 p1;
        f32 p2;

        if (((type >> 2) & 1) == 0) {
            p2 = 0;
        } else {
            p2 = 1.5;
        }

        if (((type >> 3) & 1) == 0) {
            p1 = 0;
        } else {
            p1 = 1.5;
        }

        f32 result =
            valL + (valH - valL) * p *
                       ((p1 + p2 - 2) * p * p + (-2.0f * p1 - p2 + 3) * p + p1);

        if (result < 0) {
            return 0;
        }

        if (result > 255) {
            return 255;
        }

        return static_cast<u8>(result);
    }

    case CURVETYPE_STEP: {
        return valL;
    }
    }

    return 0;
}

static inline f32 AnimCurveInterpolate(f32 posF, f32 valL, f32 valH, u8 type) {
    switch (type & 3) {
    case CURVETYPE_LINEAR: {
        return valL + (valH - valL) * posF;
    }

    case CURVETYPE_HERMITE: {
        if (type == CURVETYPE_HERMITE) {
            return valL + (valH - valL) * (3.0f - 2.0f * posF) * posF * posF;
        }

        f32 p1 = ((type >> 3) & 1) == 0 ? 0.0f : 1.5f;
        f32 p2 = ((type >> 2) & 1) == 0 ? 0.0f : 1.5f;

        f32 result = valL + (valH - valL) * posF *
                                ((p1 + p2 - 2) * posF * posF +
                                 (-2.0f * p1 - p2 + 3) * posF + p1);
        return result;
    }

    case CURVETYPE_STEP: {
        return valL;
    }
    }

    return 0;
}

static inline void CalcTurnDiv(const AnimCurveHeader *header,
                               const AnimCurveKey *key, int frameIdx, u32 *divL,
                               u32 *divH) {
    *divH = *divL;

    if ((header->processFlag & AnimCurveHeader::PROC_FLAG_TURN) != 0 &&
        ((header->processFlag & AnimCurveHeader::PROC_FLAG_INFLOOP) != 0 ||
         header->loopCount > 1)) {
        if ((*divL % 2) == 0 && frameIdx + 1 >= key->count - 1 &&
            ((header->processFlag & AnimCurveHeader::PROC_FLAG_INFLOOP) != 0 ||
             *divL < header->loopCount - 1)) {
            ++*divH;
        }

        if ((*divL % 2) != 0 && frameIdx == 0 && *divL > 0) {
            ++*divL;
        }
    }
}

void AnimCurveExecuteU8(u8 *mCmdList, u8 *target, u32 origTick, u16 randomSeed,
                        u32 life) {
    if (*mCmdList == NW4R_EF_MAGIC_ANIMBAKED) {
        return AnimBakedExecuteU8(mCmdList, target, origTick, life);
    }

    u8 enableFlag = mCmdList[3];
    u8 endBitmap = GetMSB(enableFlag);
    int elemCount = CntBit1(enableFlag);

    if (elemCount == 0) {
        return;
    }

    const int KEY_END = (KEY_PARAM1 + elemCount + 1) & ~1;
    const int framesize = KEY_END;

    AnimCurveHeader *header;
    AnimCurveKey *key;
    AnimCurveRandom *random;
    AnimCurveRandomTable *randomTable;
    AnimCurveNameTable *nameTable;
    getAnimHeaders(mCmdList, &header, &key, &random, &randomTable, &nameTable);

    const int RNDRANGE_SIZE = elemCount * 2;
    const int RNDSET_SIZE = elemCount * 2;

    f32 tickF32;
    u16 tick;
    u32 divL, divH;
    calcTick(life, header, origTick, &tick, &tickF32, &divL);
    divH = divL;

    u8 *ptrBase = reinterpret_cast<u8 *>(key) + 4;
    u8 *ptrFixed;
    int frameIdx;
    bool onKey;
    u16 loFrame;
    u16 hiFrame;
    searchKey(frameIdx, onKey, loFrame, hiFrame, tick, tickF32, ptrBase,
              framesize, 0, key->count - 1);

    if (onKey) {
        ptrFixed = ptrBase + frameIdx * framesize + KEY_PARAM1;

        if (*(ptrFixed - KEY_PARAM1 + KEY_FLAG) == 0) {
            for (u16 b = 1; b <= endBitmap; b <<= 1) {
                if ((enableFlag & b) != 0) {
                    *target = *ptrFixed++;
                }

                ++target;
            }

            return;
        } else {
            u16 randIdx = *reinterpret_cast<u16 *>(ptrFixed);
            u32 rand =
                RANDOM_INIT(randomSeed, header->randomSeed, randIdx, divL);
            u8 *tbl;

            if ((*(ptrFixed - KEY_PARAM1 + KEY_FLAG) & KEYFLAG_RANDOMTABLE) ==
                0) {
                tbl = random->datas + randIdx * RNDRANGE_SIZE;
            } else {
                tbl = randomTable->datas +
                      ((rand >> 16) % randomTable->count) * RNDSET_SIZE;
                rand = RANDOM_NEXT(rand);
            }

            int val;
            for (u16 b = 1; b <= endBitmap; b <<= 1) {
                if ((enableFlag & b) != 0) {
                    U8RANDCLAMP(val, tbl, rand);
                    *target = static_cast<u8>(val);
                }

                if (!HAS_SYNCRAND(header)) {
                    rand = RANDOM_NEXT(rand);
                }

                ++target;
            }
        }

        return;
    }

    CalcTurnDiv(header, key, frameIdx, &divL, &divH);

    u32 posF = static_cast<u32>((tickF32 - loFrame) * 65536UL);
    posF /= (hiFrame - loFrame);

    u8 *ptrL = ptrBase + frameIdx * framesize + KEY_PARAM1;
    u8 *ptrH = ptrBase + (frameIdx + 1) * framesize + KEY_PARAM1;
    u8 *type = ptrBase + frameIdx * framesize + KEY_CURVETYPE;
    const bool isRandL = *(ptrL - KEY_PARAM1 + KEY_FLAG) != 0;
    const bool isRandH = *(ptrH - KEY_PARAM1 + KEY_FLAG) != 0;

    if (!isRandL && !isRandH) {
        for (u16 b = 1; b <= endBitmap; b <<= 1) {
            if ((enableFlag & b) != 0) {
                if (*ptrL == *ptrH) {
                    *target = *ptrL;
                } else {
                    *target = AnimCurveInterpolate(posF, *ptrL, *ptrH, *type);
                }

                ptrL++;
                ptrH++;
            }

            ++type;
            ++target;
        }
    } else if (isRandL && !isRandH) {
        u16 randIdx = *reinterpret_cast<u16 *>(ptrL);
        u32 rand = RANDOM_INIT(randomSeed, header->randomSeed, randIdx, divL);
        u8 *tbl;

        if ((*(ptrL - KEY_PARAM1 + KEY_FLAG) & KEYFLAG_RANDOMTABLE) == 0) {
            tbl = random->datas + randIdx * RNDRANGE_SIZE;
        } else {
            tbl = randomTable->datas +
                  ((rand >> 16) % randomTable->count) * RNDSET_SIZE;
            rand = RANDOM_NEXT(rand);
        }

        int val;
        for (u16 b = 1; b <= endBitmap; b <<= 1) {
            if ((enableFlag & b) != 0) {
                U8RANDCLAMP(val, tbl, rand);
                *target = AnimCurveInterpolate(posF, static_cast<u8>(val),
                                               *ptrH, *type);
                ptrH++;
            }

            if (!HAS_SYNCRAND(header)) {
                rand = RANDOM_NEXT(rand);
            }

            ++type;
            ++target;
        }
    } else if (!isRandL && isRandH) {
        u16 randIdx = *reinterpret_cast<u16 *>(ptrH);
        u32 rand = RANDOM_INIT(randomSeed, header->randomSeed, randIdx, divH);
        u8 *tbl;

        if ((*(ptrH - KEY_PARAM1 + KEY_FLAG) & KEYFLAG_RANDOMTABLE) == 0) {
            tbl = random->datas + randIdx * RNDRANGE_SIZE;
        } else {
            tbl = randomTable->datas +
                  ((rand >> 16) % randomTable->count) * RNDSET_SIZE;
            rand = RANDOM_NEXT(rand);
        }

        int val;
        for (u16 b = 1; b <= endBitmap; b <<= 1) {
            if ((enableFlag & b) != 0) {
                U8RANDCLAMP(val, tbl, rand);
                *target = AnimCurveInterpolate(posF, *ptrL,
                                               static_cast<u8>(val), *type);
                ptrL++;
            }

            if (!HAS_SYNCRAND(header)) {
                rand = RANDOM_NEXT(rand);
            }

            ++type;
            ++target;
        }
    } else {
        u16 randIdx = *reinterpret_cast<u16 *>(ptrL);
        u32 randL = RANDOM_INIT(randomSeed, header->randomSeed, randIdx, divL);
        u8 *tblL;

        if ((*(ptrL - KEY_PARAM1 + KEY_FLAG) & KEYFLAG_RANDOMTABLE) == 0) {
            tblL = random->datas + randIdx * RNDRANGE_SIZE;
        } else {
            tblL = randomTable->datas +
                   ((randL >> 16) % randomTable->count) * RNDSET_SIZE;
            randL = RANDOM_NEXT(randL);
        }

        randIdx = *reinterpret_cast<u16 *>(ptrH);
        u32 randH = RANDOM_INIT(randomSeed, header->randomSeed, randIdx, divH);
        u8 *tblH;

        if ((*(ptrH - KEY_PARAM1 + KEY_FLAG) & KEYFLAG_RANDOMTABLE) == 0) {
            tblH = random->datas + randIdx * RNDRANGE_SIZE;
        } else {
            tblH = randomTable->datas +
                   ((randH >> 16) % randomTable->count) * RNDSET_SIZE;
            randH = RANDOM_NEXT(randH);
        }

        int valL;
        int valH;
        for (u16 b = 1; b <= endBitmap; b <<= 1) {
            if ((enableFlag & b) != 0) {
                U8RANDCLAMP(valL, tblL, randL);

                if (!HAS_SYNCRAND(header)) {
                    randL = RANDOM_NEXT(randL);
                }

                U8RANDCLAMP(valH, tblH, randH);

                if (!HAS_SYNCRAND(header)) {
                    randH = RANDOM_NEXT(randH);
                }

                *target = AnimCurveInterpolate(posF, static_cast<u8>(valL),
                                               static_cast<u8>(valH), *type);
            } else {
                if (!HAS_SYNCRAND(header)) {
                    randL = RANDOM_NEXT(randL);
                    randH = RANDOM_NEXT(randH);
                }
            }

            ++type;
            ++target;
        }
    }
}

void AnimCurveExecuteF32(u8 *mCmdList, f32 *target, u32 origTick,
                         u16 randomSeed, u32 life) {
    if (*mCmdList == NW4R_EF_MAGIC_ANIMBAKED) {
        return AnimBakedExecuteF32(mCmdList, target, origTick, life);
    }

    u8 enableFlag = mCmdList[3];

    AnimCurveHeader *header;
    AnimCurveKey *key;
    AnimCurveRandom *random;
    AnimCurveRandomTable *randomTable;
    AnimCurveNameTable *nameTable;
    getAnimHeaders(mCmdList, &header, &key, &random, &randomTable, &nameTable);

    f32 tickF32;
    u16 tick;
    u32 divL, divH;
    calcTick(life, header, origTick, &tick, &tickF32, &divL);
    divH = divL;

    u8 endBitmap = GetMSB(enableFlag);
    int elemCount = CntBit1(enableFlag);

    if (elemCount == 0) {
        return;
    }

    const int KEY_END = 4 * elemCount + KEY_PARAM1;
    const int RNDRANGE_SIZE = 4 * elemCount * 2;
    const int RNDSET_SIZE = 4 * elemCount * 2;
    const int framesize = KEY_END;

    u8 *ptrBase = reinterpret_cast<u8 *>(key) + 4;
    u8 *ptrFixed;
    int frameIdx;
    bool onKey;
    u16 loFrame;
    u16 hiFrame;
    searchKey(frameIdx, onKey, loFrame, hiFrame, tick, tickF32, ptrBase,
              framesize, 0, key->count - 1);

    if (onKey) {
        ptrFixed = ptrBase + frameIdx * framesize + KEY_PARAM1;

        if (*(ptrFixed - KEY_PARAM1 + KEY_FLAG) == 0) {
            f32 *fp = reinterpret_cast<f32 *>(ptrFixed);

            for (u16 b = 1; b <= endBitmap; b <<= 1) {
                if ((enableFlag & b) != 0) {
                    *target = *fp++;
                }

                ++target;
            }

            return;
        } else {
            u16 randIdx = *reinterpret_cast<u16 *>(ptrFixed);
            u32 rand =
                RANDOM_INIT(randomSeed, header->randomSeed, randIdx, divL);
            f32 *tbl;

            if ((*(ptrFixed - KEY_PARAM1 + KEY_FLAG) & KEYFLAG_RANDOMTABLE) ==
                0) {
                tbl = reinterpret_cast<f32 *>(random->datas +
                                              randIdx * RNDRANGE_SIZE);
            } else {
                tbl = reinterpret_cast<f32 *>(
                    randomTable->datas +
                    ((rand >> 16) % randomTable->count) * RNDSET_SIZE);
                rand = RANDOM_NEXT(rand);
            }

            f32 val;
            for (u16 b = 1; b <= endBitmap; b <<= 1) {
                if ((enableFlag & b) != 0) {
                    val = *tbl++;
                    *target = *tbl++ * (rand >> 16) + val;

                    if (!HAS_SYNCRAND(header)) {
                        rand = RANDOM_NEXT(rand);
                    }
                }

                ++target;
            }
        }

        return;
    }

    CalcTurnDiv(header, key, frameIdx, &divL, &divH);

    f32 posF = static_cast<f32>(tickF32 - loFrame) / (hiFrame - loFrame);
    u8 *ptrLu8 = ptrBase + frameIdx * framesize + KEY_PARAM1;
    u8 *ptrHu8 = ptrBase + (frameIdx + 1) * framesize + KEY_PARAM1;
    f32 *ptrL = reinterpret_cast<f32 *>(ptrLu8);
    f32 *ptrH = reinterpret_cast<f32 *>(ptrHu8);
    u8 *type = ptrBase + frameIdx * framesize + KEY_CURVETYPE;
    const bool isRandL = *(ptrLu8 - KEY_PARAM1 + KEY_FLAG) != 0;
    const bool isRandH = *(ptrHu8 - KEY_PARAM1 + KEY_FLAG) != 0;

    if (!isRandL && !isRandH) {
        f32 val;
        for (u16 b = 1; b <= endBitmap; b <<= 1) {
            if ((enableFlag & b) != 0) {
                if (*ptrL == *ptrH) {
                    val = *ptrL;
                } else {
                    val = AnimCurveInterpolate(posF, *ptrL, *ptrH, *type);
                }

                *target = val;
                ptrL++;
                ptrH++;
            }

            ++type;
            ++target;
        }
    } else if (isRandL && !isRandH) {
        u16 randIdx = *reinterpret_cast<u16 *>(ptrLu8);
        u32 rand = RANDOM_INIT(randomSeed, header->randomSeed, randIdx, divL);
        f32 *tbl;

        if ((*(ptrLu8 - KEY_PARAM1 + KEY_FLAG) & KEYFLAG_RANDOMTABLE) == 0) {
            tbl = reinterpret_cast<f32 *>(random->datas +
                                          randIdx * RNDRANGE_SIZE);
        } else {
            tbl = reinterpret_cast<f32 *>(randomTable->datas +
                                          ((rand >> 16) % randomTable->count) *
                                              RNDSET_SIZE);
            rand = RANDOM_NEXT(rand);
        }

        f32 val;
        for (u16 b = 1; b <= endBitmap; b <<= 1) {
            if ((enableFlag & b) != 0) {
                val = *tbl++;
                val = *tbl++ * (rand >> 16) + val;

                if (!HAS_SYNCRAND(header)) {
                    rand = RANDOM_NEXT(rand);
                }

                *target = AnimCurveInterpolate(posF, val, *ptrH, *type);
                ptrH++;
            }

            ++type;
            ++target;
        }
    } else if (!isRandL && isRandH) {
        u16 randIdx = *reinterpret_cast<u16 *>(ptrHu8);
        u32 rand = RANDOM_INIT(randomSeed, header->randomSeed, randIdx, divH);
        f32 *tbl;

        if ((*(ptrHu8 - KEY_PARAM1 + KEY_FLAG) & KEYFLAG_RANDOMTABLE) == 0) {
            tbl = reinterpret_cast<f32 *>(random->datas +
                                          randIdx * RNDRANGE_SIZE);
        } else {
            tbl = reinterpret_cast<f32 *>(randomTable->datas +
                                          ((rand >> 16) % randomTable->count) *
                                              RNDSET_SIZE);
            rand = RANDOM_NEXT(rand);
        }

        f32 val;
        for (u16 b = 1; b <= endBitmap; b <<= 1) {
            if ((enableFlag & b) != 0) {
                val = *tbl++;
                val = *tbl++ * (rand >> 16) + val;

                if (!HAS_SYNCRAND(header)) {
                    rand = RANDOM_NEXT(rand);
                }

                *target = AnimCurveInterpolate(posF, *ptrL, val, *type);
                ptrL++;
            }

            ++type;
            ++target;
        }
    } else {
        u16 randIdx = *reinterpret_cast<u16 *>(ptrLu8);
        u32 randL = RANDOM_INIT(randomSeed, header->randomSeed, randIdx, divL);
        f32 *tblL;

        if ((*(ptrLu8 - KEY_PARAM1 + KEY_FLAG) & KEYFLAG_RANDOMTABLE) == 0) {
            tblL = reinterpret_cast<f32 *>(random->datas +
                                           randIdx * RNDRANGE_SIZE);
        } else {
            tblL = reinterpret_cast<f32 *>(
                randomTable->datas +
                ((randL >> 16) % randomTable->count) * RNDSET_SIZE);
            randL = RANDOM_NEXT(randL);
        }

        randIdx = *reinterpret_cast<u16 *>(ptrHu8);
        u32 randH = RANDOM_INIT(randomSeed, header->randomSeed, randIdx, divH);
        f32 *tblH;

        if ((*(ptrHu8 - KEY_PARAM1 + KEY_FLAG) & KEYFLAG_RANDOMTABLE) == 0) {
            tblH = reinterpret_cast<f32 *>(random->datas +
                                           randIdx * RNDRANGE_SIZE);
        } else {
            tblH = reinterpret_cast<f32 *>(
                randomTable->datas +
                ((randH >> 16) % randomTable->count) * RNDSET_SIZE);
            randH = RANDOM_NEXT(randH);
        }

        f32 valL;
        f32 valH;
        for (u16 b = 1; b <= endBitmap; b <<= 1) {
            if ((enableFlag & b) != 0) {
                valL = *tblL++;
                valL = *tblL++ * (randL >> 16) + valL;

                if (!HAS_SYNCRAND(header)) {
                    randL = RANDOM_NEXT(randL);
                }

                valH = *tblH++;
                valH = *tblH++ * (randH >> 16) + valH;

                if (!HAS_SYNCRAND(header)) {
                    randH = RANDOM_NEXT(randH);
                }

                *target = AnimCurveInterpolate(posF, valL, valH, *type);
            }

            ++type;
            ++target;
        }
    }
}

void AnimCurveExecuteRotate(u8 *mCmdList, f32 *target, u32 origTick,
                            u16 randomSeed, u32 life) {
    u8 enableFlag = mCmdList[3];

    AnimCurveHeader *header;
    AnimCurveKey *key;
    AnimCurveRandom *random;
    AnimCurveRandomTable *randomTable;
    AnimCurveNameTable *nameTable;
    getAnimHeaders(mCmdList, &header, &key, &random, &randomTable, &nameTable);

    f32 tickF32;
    u16 tick;
    u32 divL, divH;
    calcTick(life, header, origTick, &tick, &tickF32, &divL);
    divH = divL;

    u8 endBitmap = GetMSB(enableFlag);
    int elemCount = CntBit1(enableFlag);

    if (elemCount == 0) {
        return;
    }

    const int KEY_END = 4 * elemCount + KEY_PARAM1;
    const int RANDKEY_DIRECTION = 4 * elemCount * 2;
    const int RNDRANGE_SIZE = 4 * elemCount * 2 + 4;
    const int RNDSET_SIZE = 4 * elemCount * 2 + 4;
    const int framesize = KEY_END;

    u8 *ptrBase = reinterpret_cast<u8 *>(key) + 4;
    u8 *ptrFixed;
    int frameIdx;
    bool onKey;
    u16 loFrame;
    u16 hiFrame;
    searchKey(frameIdx, onKey, loFrame, hiFrame, tick, tickF32, ptrBase,
              framesize, 0, key->count - 1);

    if (onKey) {
        ptrFixed = ptrBase + frameIdx * framesize + KEY_PARAM1;

        if (*(ptrFixed - KEY_PARAM1 + KEY_FLAG) == 0) {
            f32 *fp = reinterpret_cast<f32 *>(ptrFixed);

            for (u16 b = 1; b <= endBitmap; b <<= 1) {
                if ((enableFlag & b) != 0) {
                    *target = *fp++;
                }

                ++target;
            }

            return;
        } else {
            u16 randIdx = *reinterpret_cast<u16 *>(ptrFixed);
            u32 rand =
                RANDOM_INIT(randomSeed, header->randomSeed, randIdx, divL);
            f32 *tbl;

            if ((*(ptrFixed - KEY_PARAM1 + KEY_FLAG) & KEYFLAG_RANDOMTABLE) ==
                0) {
                tbl = reinterpret_cast<f32 *>(random->datas +
                                              randIdx * RNDRANGE_SIZE);
            } else {
                tbl = reinterpret_cast<f32 *>(
                    randomTable->datas +
                    ((rand >> 16) % randomTable->count) * RNDSET_SIZE);
                rand = RANDOM_NEXT(rand);
            }

            bool dir = false;

            if (*(reinterpret_cast<u8 *>(tbl) + RANDKEY_DIRECTION)) {
                if ((rand & 0x10000) == 0) {
                    dir = true;
                }

                rand = RANDOM_NEXT(rand);
            }

            f32 val;
            for (u16 b = 1; b <= endBitmap; b <<= 1) {
                if ((enableFlag & b) != 0) {
                    val = *tbl++;
                    val = *tbl++ * (rand >> 16) + val;

                    if (dir) {
                        val = -val;
                    }

                    *target = val;
                }

                ++target;

                if (!HAS_SYNCRAND(header)) {
                    rand = RANDOM_NEXT(rand);
                }
            }
        }

        return;
    }

    CalcTurnDiv(header, key, frameIdx, &divL, &divH);

    f32 posF = static_cast<f32>(tickF32 - loFrame) / (hiFrame - loFrame);
    u8 *ptrLu8 = ptrBase + frameIdx * framesize + KEY_PARAM1;
    u8 *ptrHu8 = ptrBase + (frameIdx + 1) * framesize + KEY_PARAM1;
    f32 *ptrL = reinterpret_cast<f32 *>(ptrLu8);
    f32 *ptrH = reinterpret_cast<f32 *>(ptrHu8);
    u8 *type = ptrBase + frameIdx * framesize + KEY_CURVETYPE;
    const bool isRandL = *(ptrLu8 - KEY_PARAM1 + KEY_FLAG) != 0;
    const bool isRandH = *(ptrHu8 - KEY_PARAM1 + KEY_FLAG) != 0;

    if (!isRandL && !isRandH) {
        f32 val;
        for (u16 b = 1; b <= endBitmap; b <<= 1) {
            if ((enableFlag & b) != 0) {
                if (*ptrL == *ptrH) {
                    val = *ptrL;
                } else {
                    val = AnimCurveInterpolate(posF, *ptrL, *ptrH, *type);
                }

                *target = val;
                ++ptrL;
                ++ptrH;
            }

            ++type;
            ++target;
        }
    } else if (isRandL && !isRandH) {
        u16 randIdx = *reinterpret_cast<u16 *>(ptrLu8);
        u32 rand = RANDOM_INIT(randomSeed, header->randomSeed, randIdx, divL);
        f32 *tbl;

        if ((*(ptrLu8 - KEY_PARAM1 + KEY_FLAG) & KEYFLAG_RANDOMTABLE) == 0) {
            tbl = reinterpret_cast<f32 *>(random->datas +
                                          randIdx * RNDRANGE_SIZE);
        } else {
            tbl = reinterpret_cast<f32 *>(randomTable->datas +
                                          ((rand >> 16) % randomTable->count) *
                                              RNDSET_SIZE);
            rand = RANDOM_NEXT(rand);
        }

        f32 val;
        bool dir = false;

        if (*(reinterpret_cast<u8 *>(tbl) + RANDKEY_DIRECTION)) {
            if ((rand & 0x10000) == 0) {
                dir = true;
            }

            rand = RANDOM_NEXT(rand);
        }

        for (u16 b = 1; b <= endBitmap; b <<= 1) {
            if ((enableFlag & b) != 0) {
                val = *tbl++;
                val = *tbl++ * (rand >> 16) + val;

                if (dir) {
                    val = -val;
                }

                *target = AnimCurveInterpolate(posF, val, *ptrH, *type);
                ptrH++;
            }

            ++type;
            ++target;

            if (!HAS_SYNCRAND(header)) {
                rand = RANDOM_NEXT(rand);
            }
        }
    } else if (!isRandL && isRandH) {
        u16 randIdx = *reinterpret_cast<u16 *>(ptrHu8);
        u32 rand = RANDOM_INIT(randomSeed, header->randomSeed, randIdx, divH);
        f32 *tbl;

        if ((*(ptrHu8 - KEY_PARAM1 + KEY_FLAG) & KEYFLAG_RANDOMTABLE) == 0) {
            tbl = reinterpret_cast<f32 *>(random->datas +
                                          randIdx * RNDRANGE_SIZE);
        } else {
            tbl = reinterpret_cast<f32 *>(randomTable->datas +
                                          ((rand >> 16) % randomTable->count) *
                                              RNDSET_SIZE);
            rand = RANDOM_NEXT(rand);
        }

        bool dir = false;

        if (*(reinterpret_cast<u8 *>(tbl) + RANDKEY_DIRECTION)) {
            if ((rand & 0x10000) == 0) {
                dir = true;
            }

            rand = RANDOM_NEXT(rand);
        }

        f32 val;
        for (u16 b = 1; b <= endBitmap; b <<= 1) {
            if ((enableFlag & b) != 0) {
                val = *tbl++;
                val = *tbl++ * (rand >> 16) + val;

                if (dir) {
                    val = -val;
                }

                *target = AnimCurveInterpolate(posF, *ptrL, val, *type);
                ptrL++;
            }

            ++type;
            ++target;

            if (!HAS_SYNCRAND(header)) {
                rand = RANDOM_NEXT(rand);
            }
        }
    } else {
        u16 randIdx = *reinterpret_cast<u16 *>(ptrLu8);
        u32 randL = RANDOM_INIT(randomSeed, header->randomSeed, randIdx, divL);
        f32 *tblL;

        if ((*(ptrLu8 - KEY_PARAM1 + KEY_FLAG) & KEYFLAG_RANDOMTABLE) == 0) {
            tblL = reinterpret_cast<f32 *>(random->datas +
                                           randIdx * RNDRANGE_SIZE);
        } else {
            tblL = reinterpret_cast<f32 *>(
                randomTable->datas +
                ((randL >> 16) % randomTable->count) * RNDSET_SIZE);
            randL = RANDOM_NEXT(randL);
        }

        randIdx = *reinterpret_cast<u16 *>(ptrHu8);
        u32 randH = RANDOM_INIT(randomSeed, header->randomSeed, randIdx, divH);
        f32 *tblH;

        if ((*(ptrHu8 - KEY_PARAM1 + KEY_FLAG) & KEYFLAG_RANDOMTABLE) == 0) {
            tblH = reinterpret_cast<f32 *>(random->datas +
                                           randIdx * RNDRANGE_SIZE);
        } else {
            tblH = reinterpret_cast<f32 *>(
                randomTable->datas +
                ((randH >> 16) % randomTable->count) * RNDSET_SIZE);
            randH = RANDOM_NEXT(randH);
        }

        bool dirL = false;

        if (*(reinterpret_cast<u8 *>(tblL) + RANDKEY_DIRECTION)) {
            if ((randL & 0x10000) == 0) {
                dirL = true;
            }

            randL = RANDOM_NEXT(randL);
        }

        bool dirH = false;

        if (*(reinterpret_cast<u8 *>(tblH) + RANDKEY_DIRECTION)) {
            if ((randH & 0x10000) == 0) {
                dirH = true;
            }

            randH = RANDOM_NEXT(randH);
        }

        f32 valL;
        f32 valH;
        for (u16 b = 1; b <= endBitmap; b <<= 1) {
            if ((enableFlag & b) != 0) {
                valL = *tblL++;
                valL = *tblL++ * (randL >> 16) + valL;

                if (dirL) {
                    valL = -valL;
                }

                valH = *tblH++;
                valH = *tblH++ * (randH >> 16) + valH;

                if (dirH) {
                    valH = -valH;
                }

                *target = AnimCurveInterpolate(posF, valL, valH, *type);
            }

            ++type;
            ++target;

            if (!HAS_SYNCRAND(header)) {
                randL = RANDOM_NEXT(randL);
                randH = RANDOM_NEXT(randH);
            }
        }
    }
}

struct KeyTexture {
    u8 wrap;     // at 0x0
    u8 reverse;  // at 0x1
    u16 nameIdx; // at 0x2
};

struct RndKey {
    KeyTexture base; // at 0x0
    u8 flipRandom;   // at 0x4
    u8 padding[3];   // at 0x5
};

void AnimCurveExecuteTexture(u8 *mCmdList, Particle *pp, u32 origTick,
                             u16 randomSeed, u32 life) {
    AnimCurveHeader *header;
    AnimCurveKey *key;
    AnimCurveRandom *random;
    AnimCurveRandomTable *randomTable;
    AnimCurveNameTable *nameTable;
    getAnimHeaders(mCmdList, &header, &key, &random, &randomTable, &nameTable);

    f32 tickF32;
    u16 tick;
    u32 div;
    bool final;
    calcTickStep(life, header, origTick, &tick, &tickF32, &div, &final);

    int layer = 0;
    TextureData **target;

    switch (header->kindType) {
    case 0x68: {
        target = &pp->mParameter.mTexture[0];
        layer = 0;
        break;
    }

    case 0x6C: {
        target = &pp->mParameter.mTexture[1];
        layer = 1;
        break;
    }

    case 0x70: {
        target = &pp->mParameter.mTexture[2];
        layer = 2;
        break;
    }

    default: {
        return;
    }
    }

    enum {
        KEY_END = 16,
        RNDSET_SIZE = KEY_END - KEY_PARAM1,
        framesize = KEY_END,
    };

    u8 *ptrBase = reinterpret_cast<u8 *>(key) + 4;
    u8 *ptrFixed;
    int frameIdx;
    searchKeyIndex(frameIdx, tick, ptrBase, framesize, 0, key->count - 1,
                   SEARCHKEYINDEX_BACK);
    ptrFixed = ptrBase + frameIdx * framesize;

    {
        KeyTexture key;

        if (*(ptrFixed + KEY_FLAG) == 0) {
            key = *reinterpret_cast<KeyTexture *>(ptrFixed + KEY_PARAM1);
        } else {
            u16 randIdx = *reinterpret_cast<u16 *>(ptrFixed + KEY_PARAM1);
            u32 rand =
                RANDOM_INIT(randomSeed, header->randomSeed, randIdx, div);

            if ((*(ptrFixed + KEY_FLAG) & KEYFLAG_RANDOMTABLE) == 0) {
                const RndKey *rndKey = reinterpret_cast<RndKey *>(
                    random->datas + randIdx * sizeof(RndKey));
                key = rndKey->base;

                switch (rndKey->flipRandom) {
                case 1: {
                    key.reverse =
                        static_cast<u8>((key.reverse & 2) | ((rand >> 16) & 1));
                    break;
                }

                case 2: {
                    key.reverse =
                        static_cast<u8>((key.reverse & 1) | ((rand >> 16) & 2));
                    break;
                }

                case 3: {
                    key.reverse = static_cast<u8>((rand >> 16) & 3);
                    break;
                }
                }
            } else {
                key = *reinterpret_cast<KeyTexture *>(
                    randomTable->datas +
                    ((rand >> 16) % randomTable->count) * RNDSET_SIZE);
            }
        }

        *target =
            reinterpret_cast<TextureData *>(nameTable->datas[key.nameIdx].work);

        pp->mParameter.mTextureWrap &= ~(0xF << (layer * 4));
        pp->mParameter.mTextureWrap |= (key.wrap & 0xF) << (layer * 4);

        pp->mParameter.mTextureReverse &= ~(3 << (layer * 2));
        pp->mParameter.mTextureReverse |= (key.reverse & 3) << (layer * 2);
    }
}

struct ChildData {
    EmitterInheritSetting inheritSetting; // at 0x0
    u16 name;                             // at 0xA
};

static void createChild(u8 *ptrFixed, u16 randomSeed, AnimCurveHeader *header,
                        AnimCurveNameTable *nameTable,
                        AnimCurveRandomTable *randomTable, Particle *pp,
                        u32 div) {
    enum {
        KEY_END = 24,
        RNDSET_SIZE = KEY_END - KEY_PARAM1,
    };

    ChildData *childData;

    if (*(ptrFixed + KEY_FLAG) == 0) {
        childData = reinterpret_cast<ChildData *>(ptrFixed + KEY_PARAM1);
    } else {
        u16 randIdx = *reinterpret_cast<u16 *>(ptrFixed + KEY_PARAM1);
        u32 rand = RANDOM_INIT(randomSeed, header->randomSeed, randIdx, div);

        if ((*(ptrFixed + KEY_FLAG) & KEYFLAG_RANDOMTABLE) == 0) {
        } else {
            if (randomTable->count == 0) {
                return;
            }

            childData = reinterpret_cast<ChildData *>(
                randomTable->datas +
                ((rand >> 16) % randomTable->count) * RNDSET_SIZE);
        }
    }

    EmitterResource *eh = reinterpret_cast<EmitterResource *>(
        nameTable->datas[childData->name].work);

    if (eh != NULL) {
        if (childData->inheritSetting.type == CHILDTYPE_PARTICLE) {
            pp->mParticleManager->mManagerEM->mManagerEF->mManagerES
                ->mCreationQueue.AddParticleCreation(&childData->inheritSetting,
                                                     pp, eh, pp->mCalcRemain,
                                                     NULL, NULL);
        } else {
            pp->mParticleManager->mManagerEM->mManagerEF->mManagerES
                ->mCreationQueue.AddEmitterCreation(&childData->inheritSetting,
                                                    pp, eh, pp->mCalcRemain,
                                                    NULL, NULL);
        }
    }
}

void AnimCurveExecuteChild(u8 *mCmdList, Particle *pp, u32 origTick,
                           u16 randomSeed, u32 life) {
    AnimCurveHeader *header;
    AnimCurveKey *key;
    AnimCurveRandom *random;
    AnimCurveRandomTable *randomTable;
    AnimCurveNameTable *nameTable;
    getAnimHeaders(mCmdList, &header, &key, &random, &randomTable, &nameTable);

    f32 dummy;
    u16 tick;
    u32 div;
    bool final;
    calcTickStep(life, header, origTick, &tick, &dummy, &div, &final);

    u16 nextTick;
    u32 nextDiv;
    bool dummybool;
    calcTickStep(life, header, origTick + 1, &nextTick, &dummy, &nextDiv,
                 &dummybool);

    enum {
        KEY_END = 24,
        framesize = KEY_END,
    };

    u8 *ptrBase = reinterpret_cast<u8 *>(key) + 4;
    u8 *ptrFixed;
    int frameIdx;

    if (div == nextDiv) {
        if (tick == nextTick) {
            if (!final) {
                return;
            }

            nextDiv = div + 1;
            nextTick = 1;
        }

        if (tick < nextTick) {
            searchKeyIndex(frameIdx, tick, ptrBase, framesize, 0,
                           key->count - 1, SEARCHKEYINDEX_FRONT);

            for (ptrFixed = ptrBase + frameIdx * framesize;
                 frameIdx < key->count; ptrFixed += framesize, ++frameIdx) {
                u16 frame = *reinterpret_cast<u16 *>(ptrFixed);

                if (frame < tick) {
                    continue;
                }

                if (nextTick <= frame) {
                    break;
                }

                createChild(ptrFixed, randomSeed, header, nameTable,
                            randomTable, pp, div);
            }
        } else {
            searchKeyIndex(frameIdx, tick, ptrBase, framesize, 0,
                           key->count - 1, SEARCHKEYINDEX_BACK);

            for (ptrFixed = ptrBase + frameIdx * framesize; frameIdx >= 0;
                 ptrFixed -= framesize, --frameIdx) {
                u16 frame = *reinterpret_cast<u16 *>(ptrFixed);

                if (tick < frame) {
                    continue;
                }

                if (frame <= nextTick) {
                    break;
                }

                createChild(ptrFixed, randomSeed, header, nameTable,
                            randomTable, pp, div);
            }
        }
    } else {
        if ((header->processFlag & AnimCurveHeader::PROC_FLAG_TURN) != 0) {
            if (div % 2 == 0) {
                searchKeyIndex(frameIdx, tick, ptrBase, framesize, 0,
                               key->count - 1, SEARCHKEYINDEX_FRONT);

                for (ptrFixed = ptrBase + frameIdx * framesize;
                     frameIdx < key->count; ptrFixed += framesize, ++frameIdx) {
                    u16 frame = *reinterpret_cast<u16 *>(ptrFixed);

                    if (frame < tick) {
                        continue;
                    }

                    if (frame == header->frameLength - 1) {
                        break;
                    }

                    createChild(ptrFixed, randomSeed, header, nameTable,
                                randomTable, pp, div);
                }
            } else {
                searchKeyIndex(frameIdx, tick, ptrBase, framesize, 0,
                               key->count - 1, SEARCHKEYINDEX_BACK);

                for (ptrFixed = ptrBase + frameIdx * framesize; frameIdx >= 0;
                     ptrFixed -= framesize, --frameIdx) {
                    u16 frame = *reinterpret_cast<u16 *>(ptrFixed);

                    if (tick < frame) {
                        continue;
                    }

                    if (frame == 0) {
                        break;
                    }

                    createChild(ptrFixed, randomSeed, header, nameTable,
                                randomTable, pp, div);
                }
            }

            for (u32 d = div + 1; d < nextDiv; d++) {
                if (div % 2 == 0) {
                    frameIdx = 0;

                    for (ptrFixed = ptrBase + frameIdx * framesize;
                         frameIdx < key->count;
                         ptrFixed += framesize, ++frameIdx) {
                        u16 frame = *reinterpret_cast<u16 *>(ptrFixed);

                        if (frame == header->frameLength - 1) {
                            break;
                        }

                        createChild(ptrFixed, randomSeed, header, nameTable,
                                    randomTable, pp, div);
                    }
                } else {
                    frameIdx = key->count - 1;

                    for (ptrFixed = ptrBase + frameIdx * framesize;
                         frameIdx >= 0; ptrFixed -= framesize, --frameIdx) {
                        u16 frame = *reinterpret_cast<u16 *>(ptrFixed);

                        if (frame == 0) {
                            break;
                        }

                        createChild(ptrFixed, randomSeed, header, nameTable,
                                    randomTable, pp, div);
                    }
                }
            }

            if (nextDiv % 2 == 0) {
                frameIdx = 0;

                for (ptrFixed = ptrBase + frameIdx * framesize;
                     frameIdx < key->count; ptrFixed += framesize, ++frameIdx) {
                    u16 frame = *reinterpret_cast<u16 *>(ptrFixed);

                    if (nextTick <= frame) {
                        break;
                    }

                    createChild(ptrFixed, randomSeed, header, nameTable,
                                randomTable, pp, div);
                }
            } else {
                frameIdx = key->count - 1;

                for (ptrFixed = ptrBase + frameIdx * framesize; frameIdx >= 0;
                     ptrFixed -= framesize, --frameIdx) {
                    u16 frame = *reinterpret_cast<u16 *>(ptrFixed);

                    if (nextTick >= frame) {
                        break;
                    }

                    createChild(ptrFixed, randomSeed, header, nameTable,
                                randomTable, pp, div);
                }
            }
        } else {
            searchKeyIndex(frameIdx, tick, ptrBase, framesize, 0,
                           key->count - 1, SEARCHKEYINDEX_FRONT);

            for (ptrFixed = ptrBase + frameIdx * framesize;
                 frameIdx < key->count; ptrFixed += framesize, ++frameIdx) {
                u16 frame = *reinterpret_cast<u16 *>(ptrFixed);

                if (frame < tick) {
                    continue;
                }

                createChild(ptrFixed, randomSeed, header, nameTable,
                            randomTable, pp, div);
            }

            for (u32 d = div + 1; d < nextDiv; d++) {
                frameIdx = 0;

                for (ptrFixed = ptrBase + frameIdx * framesize;
                     frameIdx < key->count; ptrFixed += framesize, ++frameIdx) {
                    createChild(ptrFixed, randomSeed, header, nameTable,
                                randomTable, pp, div);
                }
            }

            frameIdx = 0;

            for (ptrFixed = ptrBase + frameIdx * framesize;
                 frameIdx < key->count; ptrFixed += framesize, ++frameIdx) {
                u16 frame = *reinterpret_cast<u16 *>(ptrFixed);

                if (nextTick <= frame) {
                    break;
                }

                createChild(ptrFixed, randomSeed, header, nameTable,
                            randomTable, pp, div);
            }
        }
    }
}

static inline void calcTickBaked(const AnimCurveHeader *header,
                                 const u32 origTick, u16 *tick,
                                 const u32 life) {
    if (header->frameLength <= 1) {
        *tick = 0;
        return;
    }

    if ((header->processFlag & AnimCurveHeader::PROC_FLAG_INFLOOP) == 0 &&
        header->loopCount <= 1) {
        if ((header->processFlag & AnimCurveHeader::PROC_FLAG_FITTING) == 0) {
            if (origTick >= header->frameLength - 1) {
                *tick = static_cast<u16>(header->frameLength - 1);
            } else {
                *tick = static_cast<u16>(origTick);
            }
        } else {
            if (life == 1) {
                *tick = static_cast<u16>(header->frameLength - 1);
            } else {
                f32 tickF32 = static_cast<f32>(header->frameLength - 1) /
                              (life - 1) * origTick;

                if (tickF32 > static_cast<f32>(header->frameLength - 1)) {
                    tickF32 = header->frameLength - 1;
                }

                *tick = static_cast<u16>(tickF32);
            }
        }
    } else {
        if ((header->processFlag & AnimCurveHeader::PROC_FLAG_FITTING) == 0) {
            u32 div = origTick / (header->frameLength - 1);

            if ((header->processFlag & AnimCurveHeader::PROC_FLAG_TURN) == 0) {
                if ((header->processFlag &
                     AnimCurveHeader::PROC_FLAG_INFLOOP) == 0 &&
                    div >= header->loopCount) {
                    *tick = static_cast<u16>(header->frameLength - 1);
                } else {
                    *tick = static_cast<u16>(origTick -
                                             div * (header->frameLength - 1));
                }
            } else {
                if ((header->processFlag &
                     AnimCurveHeader::PROC_FLAG_INFLOOP) == 0 &&
                    div >= header->loopCount) {
                    if ((header->loopCount % 2) == 0) {
                        *tick = 0;
                    } else {
                        *tick = static_cast<u16>(header->frameLength - 1);
                    }
                } else {
                    if ((div & 1) == 0) {
                        *tick = static_cast<u16>(
                            origTick - div * (header->frameLength - 1));
                    } else {
                        *tick = static_cast<u16>(
                            header->frameLength - 1 -
                            (origTick - div * (header->frameLength - 1)));
                    }
                }
            }
        } else {
            if (origTick >= life - 1) {
                if ((header->processFlag & AnimCurveHeader::PROC_FLAG_TURN) ==
                        0 ||
                    (header->loopCount % 2) != 0) {
                    *tick = static_cast<u16>(header->frameLength - 1);
                } else {
                    *tick = 0;
                }
            } else {
                f32 rate = static_cast<f32>(header->frameLength - 1) /
                           (life - 1) * header->loopCount;
                u32 div = static_cast<u32>(origTick * rate /
                                           (header->frameLength - 1));
                f32 pos = origTick * rate - div * (header->frameLength - 1);

                if ((header->processFlag & AnimCurveHeader::PROC_FLAG_TURN) ==
                    0) {
                    *tick = static_cast<u16>(pos);
                } else {
                    if ((div & 1) == 0) {
                        *tick = static_cast<u16>(pos);
                    } else {
                        *tick = static_cast<u16>(header->frameLength - 1 - pos);
                    }
                }
            }
        }
    }
}

void AnimBakedExecuteU8(u8 *mCmdList, u8 *target, u32 origTick, u32 life) {
    u8 enableFlag = mCmdList[3];
    u8 endBitmap = GetMSB(enableFlag);
    int elemCount = CntBit1(enableFlag);

    if (elemCount == 0) {
        return;
    }

    AnimCurveHeader *header;
    AnimCurveKey *key;
    AnimCurveRandom *random;
    AnimCurveRandomTable *randomTable;
    AnimCurveNameTable *nameTable;
    getAnimHeaders(mCmdList, &header, &key, &random, &randomTable, &nameTable);

    u16 tick;
    calcTickBaked(header, origTick, &tick, life);

    u8 *ptr = reinterpret_cast<u8 *>(key) + elemCount * tick;

    for (u16 b = 1; b <= endBitmap; b <<= 1) {
        if ((enableFlag & b) != 0) {
            *target = *ptr++;
        }

        ++target;
    }
}

void AnimBakedExecuteF32(u8 *mCmdList, f32 *target, u32 origTick, u32 life) {
    u8 enableFlag = mCmdList[3];
    u8 endBitmap = GetMSB(enableFlag);
    int elemCount = CntBit1(enableFlag);

    if (elemCount == 0) {
        return;
    }

    AnimCurveHeader *header;
    AnimCurveKey *key;
    AnimCurveRandom *random;
    AnimCurveRandomTable *randomTable;
    AnimCurveNameTable *nameTable;
    getAnimHeaders(mCmdList, &header, &key, &random, &randomTable, &nameTable);

    u16 tick;
    calcTickBaked(header, origTick, &tick, life);

    f32 *ptr = reinterpret_cast<f32 *>(key) + elemCount * tick;

    for (u16 b = 1; b <= endBitmap; b <<= 1) {
        if ((enableFlag & b) != 0) {
            *target = *ptr++;
        }

        ++target;
    }
}

} // namespace ef
} // namespace nw4r
