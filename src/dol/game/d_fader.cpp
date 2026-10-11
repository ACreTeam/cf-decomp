// The scene change faders. .text 8008B91C..8008BD2C, .ctors 80465668, .sbss 8074E2E8..8074E300,
// .sdata2 80750590..807505A0. See include/game/game/d_fader.hpp.
#include <game/game/d_fader.hpp>
#include <game/game/d_side_wipe_fader.hpp>
#include <game/game/d_wipe_fader.hpp>
#include <game/mLib/m_color_fader.hpp>
#include <game/mLib/m_fader.hpp>

namespace dFader {

// The forced fade types (8074E2E8).
struct Types_s {
    Types_s() : mOut(0), mIn(0) {}

    int mOut;
    int mIn;
};

static Types_s sTypes;
static mColorFader_c *sColorFader;       // 8074E2F0
static dWipeFader_c *sWipeFader;         // 8074E2F4
static dSideWipeFader_c *sSideWipeFader; // 8074E2F8
static u8 sFlag;                         // 8074E2FC

// 8008B91C
int getOutType() {
    return sTypes.mOut;
}

// 8008B924
int getInType() {
    return sTypes.mIn;
}

// 8008B930
BOOL create() {
    sColorFader = new mColorFader_c(mColor(0, 0, 0, 0x80), mFaderBase_c::OPAQUE);
    if (sColorFader == NULL) {
        return FALSE;
    }
    sWipeFader = new dWipeFader_c(mColor(0, 0, 0, 0xFF), mFaderBase_c::OPAQUE);
    if (sWipeFader == NULL) {
        return FALSE;
    }
    sWipeFader->init();
    sSideWipeFader = new dSideWipeFader_c(mColor(0, 0, 0, 0xFF), mFaderBase_c::OPAQUE);
    if (sSideWipeFader == NULL) {
        return FALSE;
    }
    sSideWipeFader->setDefaultTexture();
    return TRUE;
}

// 8008BA48
void fn_8008BA48() {
    if (!sFlag) {
        sFlag = TRUE;
    }
}

// 8008BA60
BOOL setFader(int type) {
    switch (type) {
    case TYPE_BLACK:
        if (mFader_c::setFader(sColorFader)) {
            if (sColorFader->getStatus() == mFaderBase_c::HIDDEN) {
                sColorFader->setColor(mColor(0, 0, 0, 0xFF));
            }
            return TRUE;
        }
        break;
    case TYPE_WHITE:
        if (mFader_c::setFader(sColorFader)) {
            if (sColorFader->getStatus() == mFaderBase_c::HIDDEN) {
                sColorFader->setColor(mColor(0xFF, 0xFF, 0xFF, 0xFF));
            }
            return TRUE;
        }
        break;
    case TYPE_CIRCLE:
    case TYPE_CIRCLE2:
        if (mFader_c::setFader(sWipeFader)) {
            if (sColorFader->getStatus() == mFaderBase_c::HIDDEN) {
                sColorFader->setColor(mColor(0, 0, 0, 0xFF));
            }
            return TRUE;
        }
        break;
    case TYPE_SIDE_LEFT:
        if (mFader_c::setFader(sSideWipeFader)) {
            sSideWipeFader->setCenter(0.0f, 0.5f);
            if (sColorFader->getStatus() == mFaderBase_c::HIDDEN) {
                sColorFader->setColor(mColor(0, 0, 0, 0xFF));
            }
            return TRUE;
        }
        break;
    case TYPE_SIDE_RIGHT:
        if (mFader_c::setFader(sSideWipeFader)) {
            sSideWipeFader->setCenter(1.0f, 0.5f);
            if (sColorFader->getStatus() == mFaderBase_c::HIDDEN) {
                sColorFader->setColor(mColor(0, 0, 0, 0xFF));
            }
            return TRUE;
        }
        break;
    }
    return FALSE;
}

} // namespace dFader
