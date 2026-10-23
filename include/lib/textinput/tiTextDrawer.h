#pragma once

// textinput::textdrawer: draws the edited text (see tiTextInputBase.h).

#include <lib/textinput/tiTextInputBase.h>
#include <nw4r/ut.h>
#include <nw4r/math.h>
#include <string.h>

namespace textinput {
namespace textdrawer {

extern s32 snWidthTable[0x180 - 0x20];

class Base : public nw4r::ut::CharWriter {
  public:
    struct ViewPort {
        /* 0x00 */ f32 xOrig;
        /* 0x04 */ f32 yOrig;
        /* 0x08 */ f32 wd;
        /* 0x0C */ f32 ht;
        /* 0x10 */ f32 nearZ;
        /* 0x14 */ f32 farZ;
    }; // size 0x18

    struct CursorPos {
        /* 0x00 */ u32 uCursorPos;
        /* 0x04 */ f32 fCursorX;
        /* 0x08 */ f32 fCursorY;
    }; // size 0xC

    struct DrawInfo {
        DrawInfo() {
        }

        /* 0x00 */ nw4r::ut::Rect rect;
        /* 0x10 */ wchar_t wc;
    }; // size 0x14

    virtual void create(MEMAllocator *allocator); // 0x08
    virtual void draw(CursorPos *cursorPos);      // 0x0C
    Base()
        : mpwcString(NULL), mfVIWidth(640.0f), mbAspectRatio4x3(false),
          mbUseCharSpace(false), miLineFeedNum(0), mfLineSpace(0.0f),
          mfCharSpace(0.0f), mfFontWidth(0.0f), mfFontHeight(0.0f),
          mfEndCursorX(0.0f), mfTextHeight(0.0f), muStartPos(0),
          muEndPos(0xFFFF), mbSecretMode(false), msDrawModifyStartLine(0),
          msDrawModifyEndLine(0), msDrawCacheStartLine(0),
          msDrawCacheEndLine(0), muDrawCacheStartPos(0), muDrawCacheEndPos(0),
          mbDrawCache(false), mfCursorCacheX(0.0f), mfCursorCacheY(0.0f),
          mfCursorCacheBaseY(0.0f), msCursorCacheLine(0), mbCursorCache(false),
          mbKeepDirtyCache(true) {
        memset(&mCursorCache, 0, sizeof(mCursorCache));
    }

    virtual void draw(); // 0x10
    virtual void setDrawString(const wchar_t *wcString, u32 startPos,
                               u32 endPos);      // 0x14
    virtual void setAspectRatio(bool aspect4x3); // 0x18
    virtual void setVIWidth(f32 viWidth) {
        mfVIWidth = viWidth;
    }                                                   // 0x1C
    virtual void beginDraw(const nw4r::ut::Rect &rect); // 0x20
    virtual void endDraw();                             // 0x24
    virtual f32 getLineHeight();                        // 0x28
    virtual void setFont(const nw4r::ut::Font &font);   // 0x2C
    virtual f32 getWidth(const wchar_t *wcString);      // 0x30
    virtual int getLine() {
        return miLineFeedNum + 1;
    }                                        // 0x34
    virtual nw4r::math::VEC2 getScale() = 0; // 0x38
    virtual void setSecretModeOn(bool secretMode) {
        mbSecretMode = secretMode;
    } // 0x3C
    virtual void doBeforeDrawProcess(const wchar_t *wcString, u32 pos,
                                     const DrawInfo &drawInfo); // 0x40
    virtual void doAfterDrawProcess(const wchar_t *wcString, u32 pos,
                                    const DrawInfo &drawInfo); // 0x44
    virtual void preDraw(u32 pos);                             // 0x48
    virtual void finishDraw(u32 pos);                          // 0x4C
    virtual void doLineFeed();                                 // 0x50
    virtual void put(wchar_t wc);                              // 0x54
    virtual void procCursor(CursorPos *cursorPos, s32 line);   // 0x58
    virtual bool onCursor(CursorPos *cursorPos);               // 0x5C
    virtual void makeUpCursorPos(CursorPos *cursorPos, u32 pos, s32 line,
                                 s32 endLine); // 0x60
    virtual void drawCursor(f32 x, f32 y);     // 0x64
    virtual void calcRect(DrawInfo &drawInfo); // 0x68
    virtual const u32 &getStartPos() const {
        return muStartPos;
    } // 0x6C
    virtual const u32 &getEndPos() const {
        return muEndPos;
    }                                                                // 0x70
    virtual void setDrawModifyScopeLine(s32 startLine, s32 endLine); // 0x74
    virtual void setDrawCacheScopeLine(s32 startLine, s32 endLine);  // 0x78
    virtual void modifyCursorCache(s32 line, u32 pos, f32 cursorX, f32 cursorY,
                                   f32 x, f32 y); // 0x7C
    virtual bool isEnableCursorCache() const {
        return mbCursorCache;
    } // 0x80
    virtual s32 getDrawModifyStartLine() const {
        return msDrawModifyStartLine;
    } // 0x84
    virtual s32 getDrawModifyEndLine() const {
        return msDrawModifyEndLine;
    }                                          // 0x88
    virtual u32 getDrawCacheStartPos() const { // 0x8C
        if (mbDrawCache) {
            return muDrawCacheStartPos;
        }
        return 0x7FFFFFFF;
    }
    virtual void dirtyDrawCache() {
        mbDrawCache = false;
    } // 0x90
    virtual void dirtyCursorCache() {
        mbCursorCache = false;
    } // 0x94

  protected:
    /* 0x050 */ const wchar_t *mpwcString;
    /* 0x054 */ ViewPort mViewPort;
    /* 0x06C */ f32 mafProjection[7];
    /* 0x088 */ nw4r::ut::Rect mProjectionRect;
    /* 0x098 */ f32 mfVIWidth;
    /* 0x09C */ bool mbAspectRatio4x3;
    /* 0x09D */ bool mbUseCharSpace;
    /* 0x0A0 */ int miLineFeedNum;
    /* 0x0A4 */ f32 mfLineSpace;
    /* 0x0A8 */ f32 mfCharSpace;
    /* 0x0AC */ f32 mfFontWidth;
    /* 0x0B0 */ f32 mfFontHeight;
    /* 0x0B4 */ f32 mfEndCursorX;
    /* 0x0B8 */ f32 mfTextHeight;
    /* 0x0BC */ u32 muStartPos;
    /* 0x0C0 */ u32 muEndPos;
    /* 0x0C4 */ u32 muDrawPos;
    /* 0x0C8 */ bool mbSecretMode;
    /* 0x0CC */ s32 msDrawModifyStartLine;
    /* 0x0D0 */ s32 msDrawModifyEndLine;
    /* 0x0D4 */ s32 msDrawCacheStartLine;
    /* 0x0D8 */ s32 msDrawCacheEndLine;
    /* 0x0DC */ u32 muDrawCacheStartPos;
    /* 0x0E0 */ u32 muDrawCacheEndPos;
    /* 0x0E4 */ bool mbDrawCache;
    /* 0x0E8 */ f32 mfCursorCacheX;
    /* 0x0EC */ f32 mfCursorCacheY;
    /* 0x0F0 */ f32 mfCursorCacheBaseY;
    /* 0x0F4 */ s32 msCursorCacheLine;
    /* 0x0F8 */ CursorPos mCursorCache;
    /* 0x104 */ bool mbCursorCache;
    /* 0x105 */ bool mbKeepDirtyCache;
}; // size 0x108

} // namespace textdrawer
} // namespace textinput
