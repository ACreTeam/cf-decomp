#pragma once

#include <types.h>
#include <nw4r/ut.h>

namespace textinput {
namespace util {

wchar_t toWLower(wchar_t ch);
wchar_t toWUpper(wchar_t ch);
wchar_t HankakuToZenkaku(wchar_t ch);

wchar_t KBD_ConvertDakuten(wchar_t ch);
wchar_t KBD_ConvertHandaku(wchar_t ch);
wchar_t KBD_ConvertSmall(wchar_t ch);
wchar_t KBD_ConvertAll(wchar_t ch);

BOOL KBD_IsDakuten(wchar_t ch);
BOOL KBD_IsHandaku(wchar_t ch);
BOOL KBD_IsSmall(wchar_t ch);

bool strcmp(const char *s1, const char *s2);
void replaceChar(char *dest, u32 destLen, const char *src, int replaceIdx, char newCh);
bool isAlphabet(wchar_t ch);

void getProjectionRect4x3(nw4r::ut::Rect *rect);
void getProjectionRect16x9(nw4r::ut::Rect *rect);

f32 hermiteInterporation(f32 x, f32 arg1, f32 arg2, f32 arg3, f32 arg4, f32 arg5, f32 arg6);

class AnimObserver {
public:
    enum AnmEvent {
        AE_START,
        AE_END,
    };

    virtual void onAnmEvent(AnmEvent event, void *data) = 0; // 0x08
};

class Animation {
public:
    Animation() : mfAnimationTime(0.0f), mbInAnimation(false), mbSE(false), mpAnimObserver(NULL) {}

    virtual void startAnm(f32 start, f32 end, f32 time, AnimObserver *observer, void *data) { // 0x08
        mfStartPoint = start;
        mfEndPoint = end;
        mfAnimationTime = time;
        mfCurrentFrame = 0.0f;
        mbInAnimation = true;
        mpAnimObserver = observer;
        mpData = data;
        mbSE = false;
        if (observer != NULL) {
            observer->onAnmEvent(AnimObserver::AE_START, data);
        }
    }
    virtual void calc() { // 0x0C
        if (mbInAnimation) {
            if (mfCurrentFrame < mfAnimationTime) {
                mfCurrentFrame = 1.0f + mfCurrentFrame;
            } else {
                if (mbInAnimation && mpAnimObserver != NULL) {
                    mpAnimObserver->onAnmEvent(AnimObserver::AE_END, mpData);
                }
                mbInAnimation = false;
            }
        }
    }
    virtual f32 getValue() { // 0x10
        return hermiteInterporation(mfCurrentFrame, 0.0f, mfStartPoint, 0.0f, mfAnimationTime, mfEndPoint, 0.0f);
    }
    virtual bool isActive() { return mbInAnimation; }   // 0x14
    virtual void setSEFlag(bool flag) { mbSE = flag; }  // 0x18
    virtual bool isSEFlag() { return mbSE; }            // 0x1C
    virtual void stop() { mbInAnimation = false; }      // 0x20

    /* 0x04 */ f32 mfStartPoint;
    /* 0x08 */ f32 mfEndPoint;
    /* 0x0C */ f32 mfCurrentFrame;
    /* 0x10 */ f32 mfAnimationTime;
    /* 0x14 */ bool mbInAnimation;
    /* 0x15 */ bool mbSE;
    /* 0x18 */ AnimObserver *mpAnimObserver;
    /* 0x1C */ void *mpData;
}; // size 0x20

} // namespace util
} // namespace textinput
