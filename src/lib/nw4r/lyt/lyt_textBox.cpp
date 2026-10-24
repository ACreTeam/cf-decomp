#include <nw4r/lyt.h>
#include <nw4r/ut.h>

#include <cstring>
#include <cwchar>

/******************************************************************************
 *
 * Utility functions
 *
 ******************************************************************************/
namespace {

using namespace nw4r;
using namespace nw4r::lyt;

void ReverseYAxis(math::MTX34* pMtx) {
    pMtx->_01 = -pMtx->_01;
    pMtx->_11 = -pMtx->_11;
    pMtx->_21 = -pMtx->_21;
}

u8 ClampColor(s16 colVal) {
    return colVal < 0 ? 0 : (colVal > 255 ? 255 : colVal);
}

ut::Color GetColor(const GXColorS10& src) {
    GXColor dst;
    dst.r = ClampColor(src.r);
    dst.g = ClampColor(src.g);
    dst.b = ClampColor(src.b);
    dst.a = ClampColor(src.a);
    return dst;
}

template <typename T>
int CalcLineRectImpl(ut::Rect* pRect, ut::TextWriterBase<T>* pTextWriter,
                     const T* str, int length, f32 maxWidth, bool* pbOver) {
    ut::PrintContext<T> context = {pTextWriter, str, 0.0f, 0.0f, 0};
    const ut::Font* font = pTextWriter->GetFont();
    f32 x = 0.0f;
    bool bCharSpace = false;

    ut::CharStrmReader reader = font->GetCharStrmReader();
    const T* prStrPos = static_cast<const T*>(reader.GetCurrentPos());

    pRect->left = 0.0f;
    pRect->right = 0.0f;
    pRect->top = ut::Min(0.0f, pTextWriter->GetLineHeight());
    pRect->bottom = ut::Max(0.0f, pTextWriter->GetLineHeight());

    *pbOver = false;

    reader.Set(str);
    ut::Rect prMaxRect = *pRect;
    u16 code = reader.Next();

    while (static_cast<const T*>(reader.GetCurrentPos()) - str <= length) {
        if (code < ' ') {
            typename ut::TagProcessorBase<T>::Operation operation;
            ut::Rect rect(x, 0.0f, 0.0f, 0.0f);

            context.str = static_cast<const T*>(reader.GetCurrentPos());
            context.flags = !bCharSpace;

            pTextWriter->SetCursorX(x);
            operation =
                pTextWriter->GetTagProcessor()->CalcRect(&rect, code, &context);
            reader.Set(context.str);

            pRect->left = ut::Min(pRect->left, rect.left);
            pRect->top = ut::Min(pRect->top, rect.top);
            pRect->right = ut::Max(pRect->right, rect.right);
            pRect->bottom = ut::Max(pRect->bottom, rect.bottom);

            x = pTextWriter->GetCursorX();

            if (pRect->GetWidth() > maxWidth) {
                *pbOver = true;
                break;
            }

            if (operation == ut::TagProcessorBase<T>::OPERATION_END_DRAW) {
                return length;
            } else if (operation == ut::TagProcessorBase<T>::OPERATION_NO_CHAR_SPACE) {
                bCharSpace = false;
            } else if (operation == ut::TagProcessorBase<T>::OPERATION_CHAR_SPACE) {
                bCharSpace = true;
            } else if (operation == ut::TagProcessorBase<T>::OPERATION_NEXT_LINE) {
                break;
            }
        } else {
            if (bCharSpace) {
                x += pTextWriter->GetCharSpace();
            }

            bCharSpace = true;

            if (pTextWriter->IsWidthFixed()) {
                x += pTextWriter->GetFixedWidth();
            } else {
                x += pTextWriter->GetFont()->GetCharWidth(code) *
                     pTextWriter->GetScaleH();
            }

            pRect->left = ut::Min(pRect->left, x);
            pRect->right = ut::Max(pRect->right, x);

            if (pRect->GetWidth() > maxWidth) {
                *pbOver = true;
                break;
            }
        }

        prStrPos = static_cast<const T*>(reader.GetCurrentPos());
        code = reader.Next();
        prMaxRect = *pRect;
    }

    if (*pbOver && prStrPos != NULL) {
        *pRect = prMaxRect;
        return prStrPos - str;
    }

    return static_cast<const T*>(reader.GetCurrentPos()) - str;
}

template <typename T>
int CalcLineStrNum(f32* pWidth, ut::TextWriterBase<T>* pTextWriter,
                   const T* str, int length, f32 maxWidth, bool* pbOver) {
    ut::Rect rect;
    ut::TextWriterBase<T> myCopy = *pTextWriter;

    myCopy.SetCursor(0.0f, 0.0f);
    int ret = CalcLineRectImpl(&rect, &myCopy, str, length, maxWidth, pbOver);

    *pWidth = rect.GetWidth();
    return ret;
}

template <typename T>
void CalcStringRectImpl(ut::Rect* pRect, ut::TextWriterBase<T>* pTextWriter,
                        const T* str, int length, f32 maxWidth) {
    pRect->left = 0.0f;
    pRect->right = 0.0f;
    pRect->top = 0.0f;
    pRect->bottom = 0.0f;

    pTextWriter->SetCursor(0.0f, 0.0f);

    int remain = length;
    const T* pos = str;

    do {
        ut::Rect rect;
        bool bOver;

        int read =
            CalcLineRectImpl(&rect, pTextWriter, pos, remain, maxWidth, &bOver);

        pos += read;
        remain -= read;

        pRect->left = ut::Min(pRect->left, rect.left);
        pRect->top = ut::Min(pRect->top, rect.top);
        pRect->right = ut::Max(pRect->right, rect.right);
        pRect->bottom = ut::Max(pRect->bottom, rect.bottom);

        if (bOver) {
            CalcLineRectImpl(&rect, pTextWriter, L"\n", 1, maxWidth, &bOver);

            pRect->left = ut::Min(pRect->left, rect.left);
            pRect->top = ut::Min(pRect->top, rect.top);
            pRect->right = ut::Max(pRect->right, rect.right);
            pRect->bottom = ut::Max(pRect->bottom, rect.bottom);
        }
    } while (remain > 0);
}

template <typename T>
void CalcStringRect(ut::Rect* pRect, ut::TextWriterBase<T>* pTextWriter,
                    const T* str, int length, f32 maxWidth) {
    ut::TextWriterBase<T> myCopy = *pTextWriter;
    CalcStringRectImpl(pRect, &myCopy, str, length, maxWidth);
}

} // namespace

namespace nw4r {
namespace lyt {

NW4R_UT_RTTI_DEF_DERIVED(TextBox, Pane);

/******************************************************************************
 *
 * TextBox
 *
 ******************************************************************************/
TextBox::TextBox(const res::TextBox* pBlock, const ResBlockSet& resBlockSet)
    : Pane(pBlock) {
    u16 allocStrBufLen = pBlock->textBufBytes / sizeof(wchar_t);
    if (allocStrBufLen > 0) {
        allocStrBufLen--;
    }

    Init(allocStrBufLen);

    if (pBlock->textStrBytes >= sizeof(wchar_t) && mTextBuf != NULL) {
        const wchar_t* const pBlockText =
            detail::ConvertOffsToPtr<wchar_t>(pBlock, pBlock->textStrOffset);

        u16 resStrLen = pBlock->textStrBytes / sizeof(wchar_t) - 1;
        SetString(pBlockText, 0, resStrLen);
    }

    for (int i = 0; i < TEXTCOLOR_MAX; i++) {
        mTextColors[i] = pBlock->textCols[i];
    }

    mFontSize = pBlock->fontSize;
    mTextPosition = pBlock->textPosition;
    mBits.textAlignment = pBlock->textAlignment;
    mCharSpace = pBlock->charSpace;
    mLineSpace = pBlock->lineSpace;

    const res::Font* const fonts = detail::ConvertOffsToPtr<res::Font>(
        resBlockSet.pFontList, sizeof(res::FontList));

    const char* const fontName = detail::ConvertOffsToPtr<char>(
        fonts, fonts[pBlock->fontIdx].nameStrOffset);

    ut::Font* pFont = resBlockSet.pResAccessor->GetFont(fontName);

    if (pFont != NULL) {
        mpFont = pFont;
    } else {
        void* fontRes = resBlockSet.pResAccessor->GetResource(
            ArcResourceAccessor::RES_TYPE_FONT, fontName, NULL);

        if (fontRes != NULL) {
            ut::ResFont* pResFont = Layout::NewObj<ut::ResFont>();

            if (pResFont != NULL) {
                bool bSuccess = pResFont->SetResource(fontRes);
#pragma unused(bSuccess)

                mpFont = pResFont;
                mBits.bAllocFont = true;
            }
        }
    }

    const u32* const matOffsTbl = detail::ConvertOffsToPtr<u32>(
        resBlockSet.pMaterialList, sizeof(res::MaterialList));

    const res::Material* const pResMaterial =
        detail::ConvertOffsToPtr<res::Material>(
            resBlockSet.pMaterialList, matOffsTbl[pBlock->materialIdx]);

    mpMaterial = Layout::NewObj<Material>(pResMaterial, resBlockSet);
}

void TextBox::Init(u16 allocStrLen) {
    mTextBuf = NULL;
    mTextBufBytes = 0;
    mTextLen = 0;

    mpFont = NULL;
    mFontSize = Size(0.0f, 0.0f);

    mLineSpace = 0.0f;
    mCharSpace = 0.0f;

    mpTagProcessor = NULL;

    SetTextPositionH(HORIZONTALPOSITION_CENTER);
    SetTextPositionV(VERTICALPOSITION_CENTER);

    std::memset(&mBits, 0, sizeof(mBits));

    if (allocStrLen != 0) {
        AllocStringBuffer(allocStrLen);
    }
}

TextBox::~TextBox() {
    SetFont(NULL);

    if (mpMaterial != NULL && !mpMaterial->IsUserAllocated()) {
        Layout::DeleteObj(mpMaterial);
        mpMaterial = NULL;
    }

    FreeStringBuffer();
}

ut::Color TextBox::GetVtxColor(ulong idx) const {
    return GetTextColor(idx / 2);
}

ut::Color TextBox::GetTextColor(ulong type) const {
    return mTextColors[type];
}

void TextBox::SetVtxColor(ulong idx, ut::Color value) {
    SetTextColor(idx / 2, value);
}

void TextBox::SetTextColor(ulong type, ut::Color value) {
    mTextColors[type] = value;
}

u8 TextBox::GetVtxColorElement(ulong idx) const {
    return reinterpret_cast<const u8*>(
        mTextColors)[((idx / 2) & ~3) + (idx & 3)];
}

void TextBox::SetVtxColorElement(ulong idx, u8 value) {
    reinterpret_cast<u8*>(mTextColors)[((idx / 2) & ~3) + (idx & 3)] = value;
}

ut::Rect TextBox::GetTextDrawRect(const DrawInfo& drawInfo) const {
#pragma unused(drawInfo)

    ut::WideTextWriter writer;
    return GetTextDrawRect(&writer);
}

void TextBox::DrawSelf(const DrawInfo& drawInfo) {
    if (mTextBuf == NULL || mpFont == NULL || mpMaterial == NULL) {
        return;
    }

    LoadMtx(drawInfo);

    ut::WideTextWriter writer;
    ut::Rect textRect = GetTextDrawRect(&writer);

    ut::Color topCol =
        detail::MultipleAlpha(mTextColors[TEXTCOLOR_TOP], mGlbAlpha);
    ut::Color btmCol =
        detail::MultipleAlpha(mTextColors[TEXTCOLOR_BOTTOM], mGlbAlpha);

    writer.SetGradationMode(topCol != btmCol ? ut::CharWriter::GRADMODE_V
                                             : ut::CharWriter::GRADMODE_NONE);
    writer.SetTextColor(topCol, btmCol);

    ut::Color minCol = GetColor(mpMaterial->GetTevColor(TEVCOLOR_REG0));
    ut::Color maxCol = GetColor(mpMaterial->GetTevColor(TEVCOLOR_REG1));
    writer.SetColorMapping(minCol, maxCol);

    writer.SetupGX();
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);

    f32 hMag = GetTextAlignMag();
    const wchar_t* strPos = mTextBuf;

    writer.SetCursor(textRect.left, -textRect.top);

    f32 textWidth = textRect.GetWidth();
    int remain = mTextLen;

    while (remain > 0) {
        f32 lineWidth;
        bool bOver;

        int lineStrNum = CalcLineStrNum(&lineWidth, &writer, strPos, remain,
                                        mSize.width, &bOver);

        f32 textPosX = textRect.left + hMag * (textWidth - lineWidth);
        writer.SetCursorX(textPosX);
        writer.PrintMutable(strPos, lineStrNum);

        if (bOver) {
            writer.PrintMutable(L"\n", 1);
        }

        strPos += lineStrNum;
        remain -= lineStrNum;
    }
}

u16 TextBox::GetStringBufferLength() const {
    if (mTextBufBytes == 0) {
        return 0;
    }

    return mTextBufBytes / sizeof(wchar_t) - 1;
}

void TextBox::AllocStringBuffer(u16 minLen) {
    if (minLen == 0) {
        return;
    }

    ulong allocLen = minLen;
    ulong textBufBytes = (minLen + 1) * sizeof(wchar_t);

    if (textBufBytes > mTextBufBytes) {
        FreeStringBuffer();

        mTextBuf = Layout::NewArray<wchar_t>(allocLen + 1);

        if (mTextBuf != NULL) {
            mTextBufBytes = textBufBytes;
        }
    }
}

void TextBox::FreeStringBuffer() {
    if (mTextBuf != NULL) {
        Layout::DeletePrimArray(mTextBuf);
        mTextBuf = NULL;
        mTextBufBytes = 0;
    }
}

u16 TextBox::SetString(const wchar_t* str, u16 dstIdx) {
    return SetStringImpl(str, dstIdx, std::wcslen(str));
}

u16 TextBox::SetString(const wchar_t* str, u16 dstIdx, u16 strLen) {
    return SetStringImpl(str, dstIdx, strLen);
}

u16 TextBox::SetStringImpl(const wchar_t* str, u16 dstIdx, ulong strLen) {
    if (mTextBuf == NULL) {
        return 0;
    }

    u16 bufLen = GetStringBufferLength();

    if (dstIdx >= bufLen) {
        return 0;
    }

    ulong cpLen = ut::Min<ulong>(strLen, bufLen - dstIdx);

    std::memcpy(&mTextBuf[dstIdx], str, cpLen * sizeof(wchar_t));
    mTextLen = dstIdx + cpLen;
    mTextBuf[mTextLen] = L'\0';

    return cpLen;
}

const ut::Font* TextBox::GetFont() const {
    return mpFont;
}

void TextBox::SetFont(const ut::Font* pFont) {
    if (mBits.bAllocFont) {
        Layout::DeleteObj(const_cast<ut::Font*>(mpFont));
        mBits.bAllocFont = false;
    }

    mpFont = pFont;

    if (mpFont != NULL) {
        SetFontSize(Size(mpFont->GetWidth(), mpFont->GetHeight()));
    } else {
        SetFontSize(Size(0.0f, 0.0f));
    }
}

void TextBox::LoadMtx(const DrawInfo& drawInfo) {
    math::MTX34 mtx;

    if (drawInfo.IsMultipleViewMtxOnDraw()) {
        PSMTXConcat(drawInfo.GetViewMtx(), mGlbMtx, mtx);
    } else {
        PSMTXCopy(mGlbMtx, mtx);
    }

    ReverseYAxis(&mtx);

    GXLoadPosMtxImm(mtx, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);
}

ut::Rect TextBox::GetTextDrawRect(ut::WideTextWriter* pWriter) const {
    ut::Rect textRect;

    pWriter->SetCursor(0.0f, 0.0f);
    pWriter->SetFont(*mpFont);
    pWriter->SetFontSize(mFontSize.width, mFontSize.height);
    pWriter->SetLineSpace(mLineSpace);
    pWriter->SetCharSpace(mCharSpace);

    if (mpTagProcessor != NULL) {
        pWriter->SetTagProcessor(mpTagProcessor);
    }

    CalcStringRect(&textRect, pWriter, mTextBuf, mTextLen, mSize.width);

    Size textSize(textRect.GetWidth(), textRect.GetHeight());
    math::VEC2 ltPos = GetVtxPos();

    ltPos.x += (mSize.width - textSize.width) * GetTextMagH();
    ltPos.y -= (mSize.height - textSize.height) * GetTextMagV();

    textRect.left = ltPos.x;
    textRect.top = ltPos.y;
    textRect.right = ltPos.x + textSize.width;
    textRect.bottom = ltPos.y - textSize.height;

    return textRect;
}

f32 TextBox::GetTextMagH() const {
    switch (GetTextPositionH()) {
    default:
    case HORIZONTALPOSITION_LEFT: {
        return 0.0f;
    }

    case HORIZONTALPOSITION_CENTER: {
        return 0.5f;
    }

    case HORIZONTALPOSITION_RIGHT: {
        return 1.0f;
    }
    }
}

f32 TextBox::GetTextMagV() const {
    switch (GetTextPositionV()) {
    default:
    case VERTICALPOSITION_TOP: {
        return 0.0f;
    }

    case VERTICALPOSITION_CENTER: {
        return 0.5f;
    }

    case VERTICALPOSITION_BOTTOM: {
        return 1.0f;
    }
    }
}

f32 TextBox::GetTextAlignMag() const {
    switch (GetTextAlignment()) {
    default:
    case 0: {
        return GetTextMagH();
    }

    case 1: {
        return 0.0f;
    }

    case 2: {
        return 0.5f;
    }

    case 3: {
        return 1.0f;
    }
    }
}

} // namespace lyt
} // namespace nw4r
