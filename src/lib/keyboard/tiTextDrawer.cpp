#include <lib/textinput/tiTextDrawer.h>
#include <lib/textinput/tiUtil.h>
#include <revolution/gx.h>
#include <revolution/mtx.h>

namespace textinput {
namespace textdrawer {

s32 snWidthTable[0x180 - 0x20];

void Base::create(MEMAllocator *allocator) {
    for (int i = 0; i < 0x180 - 0x20; i++) {
        snWidthTable[i] = -1;
    }
    mfVIWidth = 640.0f;
}

void Base::setAspectRatio(bool aspect4x3) {
    mbAspectRatio4x3 = aspect4x3;
    if (aspect4x3) {
        util::getProjectionRect4x3(&mProjectionRect);
    } else {
        util::getProjectionRect16x9(&mProjectionRect);
    }
}

void Base::setDrawString(const wchar_t *wcString, u32 startPos, u32 endPos) {
    mpwcString = wcString;
    muStartPos = startPos;
    muEndPos = endPos;
}

void Base::modifyCursorCache(s32 line, u32 pos, f32 cursorX, f32 cursorY, f32 x,
                             f32 y) {
    msCursorCacheLine = line;
    mCursorCache.uCursorPos = pos;
    mCursorCache.fCursorX = cursorX;
    mCursorCache.fCursorY = cursorY - mfCursorCacheBaseY;
    mfCursorCacheX = x;
    mfCursorCacheY = y - mfCursorCacheBaseY;
    mbCursorCache = true;
}

void Base::setDrawModifyScopeLine(s32 startLine, s32 endLine) {
    msDrawModifyStartLine = startLine;
    msDrawModifyEndLine = endLine;
}

void Base::setDrawCacheScopeLine(s32 startLine, s32 endLine) {
    if (startLine < 0) {
        startLine = 0;
    }
    if (msDrawCacheStartLine != startLine) {
        dirtyDrawCache();
    }
    msDrawCacheStartLine = startLine;
    msDrawCacheEndLine = endLine;
}

void Base::procCursor(CursorPos *cursorPos, s32 line) {
    if (!onCursor(cursorPos)) {
        cursorPos->fCursorX = GetCursorX();
        cursorPos->fCursorY = GetCursorY();
    }
}

bool Base::onCursor(CursorPos *cursorPos) {
    return false;
}

void Base::makeUpCursorPos(CursorPos *cursorPos, u32 pos, s32 line,
                           s32 endLine) {
    const wchar_t *wcString = mpwcString;
    if (wcString != NULL) {
        const wchar_t *wc = &wcString[pos];
        s32 lineNum = line;
        f32 prevY = GetCursorY();
        while (*wc != 0) {
            DrawInfo drawInfo;
            drawInfo.wc = *wc;
            calcRect(drawInfo);
            doBeforeDrawProcess(wcString, pos, drawInfo);
            if (cursorPos != NULL && cursorPos->uCursorPos == pos) {
                procCursor(cursorPos, lineNum + (prevY != GetCursorY()));
            }
            if (*wc == L'\n') {
                doLineFeed();
                break;
            }
            MoveCursorX((drawInfo.rect.right - drawInfo.rect.left) *
                        getScale().x);
            doAfterDrawProcess(wcString, pos, drawInfo);
            if (prevY != GetCursorY()) {
                lineNum++;
            }
            prevY = GetCursorY();
            wc++;
            pos++;
        }

        if (cursorPos != NULL && *wc == 0 && cursorPos->uCursorPos == pos) {
            procCursor(cursorPos, lineNum);
        }
    }
}

void Base::doBeforeDrawProcess(const wchar_t *wcString, u32 pos,
                               const DrawInfo &drawInfo) {
}

void Base::doLineFeed() {
    miLineFeedNum++;
}

void Base::doAfterDrawProcess(const wchar_t *wcString, u32 pos,
                              const DrawInfo &drawInfo) {
}

void Base::put(wchar_t wc) {
    if (!mbSecretMode) {
        Print(wc);
    } else {
        Print(L'*');
    }
}

void Base::draw(CursorPos *cursorPos) {
    const wchar_t *wc = mpwcString;
    if (GetFont() == NULL) {
        return;
    }

    u32 startPos = muStartPos;
    f32 prevY = GetCursorY();
    muDrawCacheStartPos = startPos;
    u32 endPos = muEndPos;
    muDrawPos = startPos;
    preDraw(startPos);

    wc += startPos;
    SetFontSize(mfFontWidth * getScale().x, mfFontHeight * getScale().y);
    SetupGX();

    while (*wc != 0 && muDrawPos < endPos) {
        DrawInfo drawInfo;
        drawInfo.wc = *wc;
        calcRect(drawInfo);
        doBeforeDrawProcess(mpwcString, muDrawPos, drawInfo);
        if (cursorPos != NULL && cursorPos->uCursorPos == muDrawPos) {
            procCursor(cursorPos, prevY != GetCursorY());
        }
        if (*wc == L'\n') {
            nw4r::ut::Color color = GetTextColor();
            SetTextColor(nw4r::ut::Color(200, 200, 200, color.a));
            Print(0xE056);
            SetTextColor(color);
            doLineFeed();
        } else if (*wc != 0xFFFF) {
            put(*wc);
            MoveCursorX(mfCharSpace * getScale().x);
        }
        doAfterDrawProcess(mpwcString, muDrawPos, drawInfo);
        wc++;
        muDrawPos++;
    }

    if (cursorPos != NULL && cursorPos->uCursorPos == muDrawPos) {
        procCursor(cursorPos, 0);
    }
    finishDraw(muDrawPos);
}

void Base::preDraw(u32 pos) {
}

void Base::finishDraw(u32 pos) {
}

f32 Base::getLineHeight() {
    if (getScale().y == 0.0f) {
        return 1.0f;
    }
    f32 height = (mfFontHeight + mfLineSpace) * getScale().y;
    return (height >= 1.0f) ? height : 1.0f;
}

void Base::setFont(const nw4r::ut::Font &font) {
    SetFont(font);
    for (int i = 0; i < 0x180 - 0x20; i++) {
        snWidthTable[i] = -1;
    }
    for (wchar_t wc = 0x20; wc <= 0x17F; wc++) {
        DrawInfo drawInfo;
        drawInfo.wc = wc;
        calcRect(drawInfo);
        snWidthTable[wc - 0x20] =
            (s32)(drawInfo.rect.right - drawInfo.rect.left);
    }
}

void Base::calcRect(DrawInfo &drawInfo) {
    drawInfo.rect.left = 0.0f;
    wchar_t wc = drawInfo.wc;
    drawInfo.rect.right = 0.0f;
    drawInfo.rect.top = 0.0f;
    drawInfo.rect.bottom = getLineHeight();

    f32 right = 0.0f;
    right += mfCharSpace;
    if (IsWidthFixed()) {
        right += GetFixedWidth();
    } else {
        f32 scale = getScale().x;
        if (scale == 0.0f) {
            scale = 0.0001f;
        }
        right += GetFont()->GetCharWidth(wc) * GetScaleH() / scale;
    }
    drawInfo.rect.right = nw4r::ut::Max(drawInfo.rect.right, right);
}

f32 Base::getWidth(const wchar_t *wcString) {
    f32 width = 0.0f;
    DrawInfo drawInfo;
    while (*wcString != 0) {
        drawInfo.wc = *wcString;
        calcRect(drawInfo);
        wcString++;
        width += drawInfo.rect.right - drawInfo.rect.left;
    }
    return width;
}

void Base::beginDraw(const nw4r::ut::Rect &rect) {
    muDrawPos = 0;
    GXGetViewportv(&mViewPort.xOrig);
    GXGetProjectionv(mafProjection);

    Mtx44 projection;
    C_MTXOrtho(projection, 0.0f, rect.top - rect.bottom, -50.0f,
               50.0f + (rect.right - rect.left), -1.0f, 1.0f);
    GXSetProjection(projection, GX_ORTHOGRAPHIC);

    f32 scale = mfVIWidth / (mProjectionRect.right - mProjectionRect.left);
    f32 x =
        rect.left + (mProjectionRect.left - mProjectionRect.right) / 2 - 50.0f;
    ViewPort viewPort;
    viewPort.xOrig = x + x * (scale - 1.0f);
    viewPort.yOrig =
        rect.bottom + (mProjectionRect.bottom - mProjectionRect.top) / 2;
    viewPort.wd = scale * (100.0f + (rect.right - rect.left));
    viewPort.ht = rect.top - rect.bottom;
    viewPort.nearZ = mViewPort.nearZ;
    viewPort.farZ = mViewPort.farZ;
    GXSetViewport(viewPort.xOrig, viewPort.yOrig, viewPort.wd, viewPort.ht,
                  viewPort.nearZ, viewPort.farZ);

    u32 left = viewPort.xOrig;
    if (left >= 640) {
        left = 639;
    }
    u32 top = viewPort.yOrig;
    if (top >= 480) {
        top = 479;
    }
    u32 wd = viewPort.wd;
    if (left + wd >= 640) {
        wd = 639 - left;
    }
    u32 ht = viewPort.ht;
    if (top + ht >= 480) {
        ht = 479 - top;
    }
    GXSetScissor(left, top, wd, ht);

    Mtx mtx;
    PSMTXIdentity(mtx);
    GXLoadPosMtxImm(mtx, GX_PNMTX0);

    miLineFeedNum = 0;
    mfTextHeight = GetCursorY();
}

void Base::endDraw() {
    GXSetViewport(mViewPort.xOrig, mViewPort.yOrig, mViewPort.wd, mViewPort.ht,
                  mViewPort.nearZ, mViewPort.farZ);
    GXSetScissor(mViewPort.xOrig, mViewPort.yOrig, mViewPort.wd, mViewPort.ht);
    mfEndCursorX = GetCursorX();
    f32 y = GetCursorY();
    mfTextHeight = mfTextHeight - y + getLineHeight();
}

void Base::drawCursor(f32 x, f32 y) {
}

void Base::draw() {
}

} // namespace textdrawer
} // namespace textinput
