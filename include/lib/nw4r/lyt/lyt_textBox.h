#ifndef NW4R_LYT_TEXTBOX_H
#define NW4R_LYT_TEXTBOX_H
#include <nw4r/types_nw4r.h>

#include <nw4r/lyt/lyt_common.h>
#include <nw4r/lyt/lyt_pane.h>
#include <nw4r/lyt/lyt_types.h>

#include <nw4r/ut.h>

namespace nw4r {
namespace lyt {

// Forward declarations
struct ResBlockSet;

/******************************************************************************
 *
 * TextColor
 *
 ******************************************************************************/
enum TextColor {
    TEXTCOLOR_TOP,
    TEXTCOLOR_BOTTOM,

    TEXTCOLOR_MAX
};

namespace res {

/******************************************************************************
 *
 * TXT1 binary layout
 *
 ******************************************************************************/
struct TextBox : Pane {
    static const ulong SIGNATURE = 'txt1';

    u16 textBufBytes;             // at 0x4C
    u16 textStrBytes;             // at 0x4E
    u16 materialIdx;              // at 0x50
    u16 fontIdx;                  // at 0x52
    u8 textPosition;              // at 0x54
    u8 textAlignment;             // at 0x55
    u8 PADDING_0x56[0x58 - 0x56]; // at 0x56
    ulong textStrOffset;            // at 0x58
    ulong textCols[TEXTCOLOR_MAX];  // at 0x5C
    Size fontSize;                // at 0x64
    f32 charSpace;                // at 0x6C
    f32 lineSpace;                // at 0x70
};

} // namespace res

/******************************************************************************
 *
 * TextBox
 *
 ******************************************************************************/
class TextBox : public Pane {
public:
    NW4R_UT_RTTI_DECL(TextBox);

public:
    TextBox(const res::TextBox* pRes, const ResBlockSet& rBlockSet);
    virtual ~TextBox(); // at 0x8

    virtual void DrawSelf(const DrawInfo& drawInfo); // at 0x18

    virtual ut::Color GetVtxColor(ulong idx) const;       // at 0x24
    virtual void SetVtxColor(ulong idx, ut::Color color); // at 0x28

    virtual u8 GetVtxColorElement(ulong idx) const;       // at 0x34
    virtual void SetVtxColorElement(ulong idx, u8 value); // at 0x38

    virtual void LoadMtx(const DrawInfo& drawInfo); // at 0x70

    virtual void AllocStringBuffer(u16 minLen); // at 0x74
    virtual void FreeStringBuffer();            // at 0x78
    u16 GetStringBufferLength() const;

    virtual u16 SetString(const wchar_t* str, u16 dstIdx); // at 0x7C
    virtual u16 SetString(const wchar_t* str, u16 dstIdx,
                          u16 strLen); // at 0x80

    ut::Rect GetTextDrawRect(const DrawInfo& drawInfo) const;
    ut::Rect GetTextDrawRect(ut::WideTextWriter* pWriter) const;

    const wchar_t* GetString() const {
        return mTextBuf;
    }
    u16 GetStringLength() const {
        return mTextLen;
    }

    const wchar_t* GetStringBuffer() const {
        return mTextBuf;
    }

    const ut::Font* GetFont() const;
    void SetFont(const ut::Font* pFont);

    ut::Color GetTextColor(ulong type) const;
    void SetTextColor(ulong type, ut::Color value);

    const Size& GetFontSize() const {
        return mFontSize;
    }
    void SetFontSize(const Size& rFontSize) {
        mFontSize = rFontSize;
    }

    f32 GetLineSpace() const {
        return mLineSpace;
    }
    void SetLineSpace(f32 space) {
        mLineSpace = space;
    }

    f32 GetCharSpace() const {
        return mCharSpace;
    }
    void SetCharSpace(f32 space) {
        mCharSpace = space;
    }

    ut::WideTagProcessor* GetTagProcessor() const {
        return mpTagProcessor;
    }
    void SetTagProcessor(ut::WideTagProcessor* pProcessor) {
        mpTagProcessor = pProcessor;
    }

    u8 GetTextPositionH() const {
        return detail::GetHorizontalPosition(mTextPosition);
    }
    void SetTextPositionH(u8 value) {
        detail::SetHorizontalPosition(&mTextPosition, value);
    }

    u8 GetTextPositionV() const {
        return detail::GetVerticalPosition(mTextPosition);
    }
    void SetTextPositionV(u8 value) {
        detail::SetVerticalPosition(&mTextPosition, value);
    }

    u8 GetTextAlignment() const {
        return mBits.textAlignment;
    }

    f32 GetTextMagH() const;
    f32 GetTextMagV() const;
    f32 GetTextAlignMag() const;

protected:
    wchar_t* mTextBuf;                    // at 0xD8
    ut::Color mTextColors[TEXTCOLOR_MAX]; // at 0xDC

    const ut::Font* mpFont; // at 0xE4
    Size mFontSize;         // at 0xE8
    f32 mLineSpace;         // at 0xF0
    f32 mCharSpace;         // at 0xF4

    ut::WideTagProcessor* mpTagProcessor; // at 0xF8
    u16 mTextBufBytes;                    // at 0xFC
    u16 mTextLen;                         // at 0xFE
    u8 mTextPosition;                     // at 0x100

    struct {
        u8 bAllocFont : 1;
        u8 textAlignment : 2;
    } mBits; // at 0x101

private:
    void Init(u16 allocStrLen);
    u16 SetStringImpl(const wchar_t* str, u16 dstIdx, ulong strLen);
};

} // namespace lyt
} // namespace nw4r

#endif
