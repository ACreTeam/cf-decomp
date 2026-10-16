#ifndef GAME_D_PAPER_VIEW_HPP
#define GAME_D_PAPER_VIEW_HPP

#include <types.h>
#include <game/game/d_m2d.hpp>

class dMail_c;

// RTTI establishes both class names. Other names are inferred from behavior.
class dPaper_c {
public:
    enum { BODY_LINES = 6, WORD_COUNT = 8, HEADER = 0, FOOTER = 7,
           MAX_LINE_LENGTH = 64, WORD_BUFFER_LENGTH = 93, LINE_BUFFER_LENGTH = 94 };
    enum { FLAG_PAPER_ONLY = 1, FLAG_NAME_COLOR = 2 };
    enum TextPart_e { PART_HEADER, PART_HEADER_AFTER_NAME, PART_BODY, PART_FOOTER, PART_NONE };

    // Packed RGBA storage, passed as bytes to the m2d color-tag helpers.
    union Color_c {
        u32 mPacked;
        u8 mBytes[4];
    };

    class Word_c : public m2d::Word_c {
    public:
        Word_c();                          // .text:0x1654
        virtual ~Word_c();                 // 0x1698
        virtual u32 getBufferSize();        // 0x16F0
        virtual wchar_t *getBuffer();       // 0x16F8
        /* 0x28 */ wchar_t mBuffer[WORD_BUFFER_LENGTH];
    }; // size 0xE4

    dPaper_c();                             // .text:0x5C
    virtual ~dPaper_c();                    // 0x108
    int remove();                          // 0x1A4
    int draw();                           // 0x264
    void execute();                        // 0x2AC
    void updateText(BOOL fixedSpaces, BOOL fixedHeader); // 0x2FC
    void setPaper(int paper, BOOL paperOnly); // 0x3D4
    void setMail(dMail_c *mail, BOOL nameColor); // 0x3F0
    void copyPos(m2d::SimpleEx_c *layout, const char *pane); // 0x414
    void setPosX(f32 x);                    // 0x474
    void setPos(f32 x, f32 y);              // 0x4D8
    f32 getLineY(int line);                 // 0x550
    int getBodyLine(int pos, int *linePos, BOOL nextLine); // 0x598
    void getHeaderPos(mVec3_c *out, int pos, BOOL afterName); // 0x60C
    void getBodyPos(mVec3_c *out, int pos, BOOL nextLine); // 0x69C
    void getFooterPos(mVec3_c *out, int pos); // 0x724
    int getBodyLineEnd(mVec3_c *out, int line); // 0x798
    int hitCheck(const mVec2_c *pos);        // 0x89C
    int getPosAtScreenX(int line, f32 x);    // 0x91C
    int getTextPart(u32 *pos, int line);     // 0xAC0
    int getHeaderPosAtScreenX(f32 x);        // 0xB64
    int getFooterPosAtScreenX(f32 x);        // 0xB6C
    void setPaperOnly(BOOL paperOnly);      // 0xB74
    void updateHeader(const wchar_t *text, int namePos, BOOL fixedSpaces); // 0xBF8
    void updateBody(const wchar_t *text, BOOL fixedSpaces); // 0xDA8
    void updateFooter(const wchar_t *text, BOOL fixedSpaces); // 0xFFC
    BOOL fitsHeader(const wchar_t *text);   // 0x10CC
    BOOL wrapBody(const wchar_t *text, BOOL); // 0x1188
    BOOL fitsFooter(const wchar_t *text);   // 0x12CC
    BOOL hasBodyLine(int line);             // 0x1350
    int getBodyLineCount();                 // 0x1370
    int getBodyLineLength(int line);        // 0x13DC
    BOOL isNewLine(int line);               // 0x13F0
    void setFooterAlign(u8 align);          // 0x1414
    void initColors();                      // 0x1478
    void getColors(int line, u8 *white, u8 *black); // 0x156C
    void setColors(int line, const u8 *white, const u8 *black); // 0x15E0
    int load();                            // 0x1700
    void create();                         // 0x178C
    void calcAnm();                        // 0x1978

    /* 0x004 */ m2d::ResAccLoader_c mPaperRes;
    /* 0x0D8 */ m2d::SimpleEx_c *mpLayout;
    /* 0x0DC */ m2d::ResAccLoader_c mTextRes;
    /* 0x1B0 */ m2d::SimpleEx_c mTextLayout;
    /* 0x248 */ m2d::Anm_c mColorAnm;
    /* 0x26C */ int mPaper;
    /* 0x270 */ int mLineLengths[BODY_LINES];
    /* 0x288 */ u32 mNewLineBits;
    /* 0x28C */ Word_c mWords[WORD_COUNT];
    /* 0x9AC */ dMail_c *mpMail;
    /* 0x9B0 */ int mSelectPart;
    /* 0x9B4 */ int mSelectLength;
    /* 0x9B8 */ int mSelectStart;
    /* 0x9BC */ wchar_t mToName[10];
    /* 0x9D0 */ int mToNameLength;
    /* 0x9D4 */ Color_c mTextWhite;
    /* 0x9D8 */ Color_c mTextBlack;
    /* 0x9DC */ Color_c mNameWhite;
    /* 0x9E0 */ Color_c mNameBlack;
    /* 0x9E4 */ Color_c mSelectWhite;
    /* 0x9E8 */ Color_c mSelectBlack;
    /* 0x9EC */ int mColorFrame;
    /* 0x9F0 */ u16 mFlags;
}; // size 0x9F4

#endif
