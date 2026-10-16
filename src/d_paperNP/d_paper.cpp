#include <game/game/d_item.hpp>
#include <game/game/d_item_def.hpp>
#include <game/game/d_mail.hpp>
#include <game/game/d_paper_view.hpp>
#include <game/mLib/m_heap.hpp>
#include <stdio.h>
#include <string.h>


namespace dPaper {
static char l_archivePath[] = "Layout/Paper/paper%03d.arc";
static char l_textArchivePath[] = "Layout/paper.arc";
static char l_layoutName[] = "paper%03d.brlyt";
static char l_textLayoutName[] = "ltr_txt_00.brlyt";
static char l_createPaneName[] = "T_msg_%02d";
static char l_rootPane[] = "N_all";
} // namespace dPaper

using namespace dPaper;

// .text:0x5C
dPaper_c::dPaper_c() {
    mTextWhite.mPacked = nw4r::ut::Color::WHITE;
    mTextBlack.mPacked = nw4r::ut::Color::WHITE;
    mNameWhite.mPacked = nw4r::ut::Color::WHITE;
    mNameBlack.mPacked = nw4r::ut::Color::WHITE;
    mSelectWhite.mPacked = nw4r::ut::Color::WHITE;
    mSelectBlack.mPacked = nw4r::ut::Color::WHITE;
}
// .text:0x108
dPaper_c::~dPaper_c() {
}
// .text:0x1A4
int dPaper_c::remove() {
    if (!mPaperRes.remove()) {
        return 0;
    }
    if (!(mFlags & FLAG_PAPER_ONLY) && !mTextRes.remove()) {
        return 0;
    }
    if (mpLayout != NULL) {
        EGG::Heap *heap = mHeap::setCurrentHeap(m2d::getAllocator()->mpHeap);
        delete mpLayout;
        mpLayout = NULL;
        mHeap::setCurrentHeap(heap);
    }
    return 1;
}
// .text:0x264
int dPaper_c::draw() {
    mpLayout->entry();
    if (!(mFlags & FLAG_PAPER_ONLY)) {
        mTextLayout.entry();
    }
    return 1;
}
// .text:0x2AC
void dPaper_c::execute() {
    mpLayout->calc();
    if (!(mFlags & FLAG_PAPER_ONLY)) {
        calcAnm();
    }
}
// .text:0x2FC
void dPaper_c::updateText(BOOL fixedSpaces, BOOL fixedHeader) {
    static const wchar_t *header = mpMail->mHeader;
    int namePos = mpMail->mNamePos;
    static const wchar_t *body = mpMail->mBody;
    static const wchar_t *footer = mpMail->mFooter;
    updateHeader(header, namePos, fixedHeader);
    updateBody(body, fixedSpaces);
    updateFooter(footer, fixedSpaces);
}
// .text:0x3D4
void dPaper_c::setPaper(int paper, BOOL paperOnly) {
    if (paperOnly) {
        mFlags |= FLAG_PAPER_ONLY;
    }
    mPaper = paper;
}
// .text:0x3F0
void dPaper_c::setMail(dMail_c *mail, BOOL nameColor) {
    mpMail = mail;
    mPaper = mail->mPaper;
    if (nameColor) {
        mFlags |= FLAG_NAME_COLOR;
    }
}
// .text:0x414
void dPaper_c::copyPos(m2d::SimpleEx_c *layout, const char *pane) {
    layout->copyPos(mpLayout, pane, "N_ltrBase_00");
    mpLayout->addPos("N_ltrBase_00", 0.0f, -7.0f);
}
// .text:0x474
void dPaper_c::setPosX(f32 x) {
    nw4r::lyt::Pane *pane = mpLayout->findPane(l_rootPane);
    mVec3_c pos = pane->GetTranslate();
    pos.x = x;
    pane->SetTranslate(*static_cast<nw4r::math::VEC3 *>(pos));
}
// .text:0x4D8
void dPaper_c::setPos(f32 x, f32 y) {
    nw4r::lyt::Pane *pane = mpLayout->findPane(l_rootPane);
    mVec3_c pos = pane->GetTranslate();
    pos.x = x;
    pos.y = y;
    pane->SetTranslate(*static_cast<nw4r::math::VEC3 *>(pos));
}
// .text:0x550
f32 dPaper_c::getLineY(int line) {
    nw4r::lyt::TextBox *box = mWords[line].mpTextBox;
    if (box == NULL) {
        return 0.0f;
    }
    mVec3_c pos;
    m2d::getScreenPos(&pos, box);
    return pos.y;
}
// .text:0x598
int dPaper_c::getBodyLine(int pos, int *linePos, BOOL nextLine) {
    int line = 0;
    while (pos >= mLineLengths[line] && line < BODY_LINES) {
        if (pos == mLineLengths[line]) {
            if (nextLine || !(mNewLineBits & (1 << line))) {
                break;
            }
        }
        pos -= mLineLengths[line];
        ++line;
    }
    if (line >= BODY_LINES) {
        return -1;
    }
    if (linePos != NULL) {
        *linePos = pos;
    }
    return line + 1;
}
// .text:0x60C
void dPaper_c::getHeaderPos(mVec3_c *out, int pos, BOOL afterName) {
    int index = pos;
    mVec3_c screen;
    m2d::getScreenPos(&screen, mWords[HEADER].mpTextBox);
    *out = screen;
    if (afterName) {
        index += mToNameLength;
    }
    out->x = mWords[HEADER].getScreenX(index);
}
// .text:0x69C
void dPaper_c::getBodyPos(mVec3_c *out, int pos, BOOL nextLine) {
    int linePos;
    int line = getBodyLine(pos, &linePos, nextLine);
    mVec3_c screen;
    m2d::getScreenPos(&screen, mWords[line].mpTextBox);
    *out = screen;
    out->x = mWords[line].getScreenX(linePos);
}
// .text:0x724
void dPaper_c::getFooterPos(mVec3_c *out, int pos) {
    mVec3_c screen;
    m2d::getScreenPos(&screen, mWords[FOOTER].mpTextBox);
    *out = screen;
    out->x = mWords[FOOTER].getScreenX(pos);
}
// .text:0x798
int dPaper_c::getBodyLineEnd(mVec3_c *out, int line) {
    if (mLineLengths[line] == 0 && line > 0 && !isNewLine(line)) {
        return 0;
    }
    mVec3_c screen;
    m2d::getScreenPos(&screen, mWords[line + 1].mpTextBox);
    *out = screen;
    out->x = mWords[line + 1].getScreenX(mLineLengths[line]);
    if (isNewLine(line + 1)) {
        return 1;
    }
    if (line == BODY_LINES - 1) {
        return 2;
    }
    if (line < BODY_LINES - 1 && mLineLengths[line + 1] == 0) {
        return 2;
    }
    return 0;
}
// .text:0x89C
int dPaper_c::hitCheck(const mVec2_c *pos) {
    for (int i = 0; i < WORD_COUNT; ++i) {
        if (m2d::isInPaneRect(mWords[i].mpTextBox, 5.0f, pos)) {
            return i;
        }
    }
    return -1;
}
// .text:0x91C
int dPaper_c::getPosAtScreenX(int line, f32 x) {
    if (line == HEADER) {
        return getHeaderPosAtScreenX(x);
    }
    if (line == FOOTER) {
        return getFooterPosAtScreenX(x);
    }
    int pos = mWords[line].getPosAtScreenX(x);
    for (int i = line - 1; i >= 1; --i) {
        pos += mLineLengths[i - 1];
    }
    return pos;
}
// .text:0xAC0
int dPaper_c::getTextPart(u32 *pos, int line) {
    if (line >= 1 && line <= BODY_LINES) {
        return PART_BODY;
    }
    if (line == FOOTER) {
        return PART_FOOTER;
    }
    if (mToNameLength == 0) {
        return PART_HEADER_AFTER_NAME;
    }
    u32 namePos = mpMail->mNamePos;
    if (*pos >= mToNameLength + namePos) {
        *pos -= mToNameLength;
        return PART_HEADER_AFTER_NAME;
    }
    if (*pos > mToNameLength / 2 + namePos) {
        *pos = namePos;
        return PART_HEADER_AFTER_NAME;
    }
    if (*pos > namePos) {
        *pos = namePos;
        return PART_HEADER;
    }
    return PART_HEADER;
}
// .text:0xB64
int dPaper_c::getHeaderPosAtScreenX(f32 x) {
    return mWords[HEADER].getPosAtScreenX(x);
}
// .text:0xB6C
int dPaper_c::getFooterPosAtScreenX(f32 x) {
    return mWords[FOOTER].getPosAtScreenX(x);
}
// .text:0xB74
void dPaper_c::setPaperOnly(BOOL paperOnly) {
    bool show = paperOnly != TRUE;
    mpLayout->setVisible("ltrBase_00", show);
    mpLayout->setVisible("ltrBaseSdw", show);
    mpLayout->setVisible("ltrBase_S_00", !show);
}
// .text:0xBF8
void dPaper_c::updateHeader(const wchar_t *text, int namePos,
                            BOOL fixedSpaces) {
    int length = m2d::Word_c::getLength(text);
    if (length < namePos) {
        namePos = length;
    }
    mWords[HEADER].clear();
    mWords[HEADER].set(text, 0);
    if (mToNameLength > 0) {
        dString::Word_c name;
        name.clear();
        name.set(mToName, 0);
        name.capitalize();
        if (mFlags & FLAG_NAME_COLOR) {
            mWords[HEADER].insertColored(name.getText(), namePos,
                                         mNameWhite.mBytes, mNameBlack.mBytes);
        } else {
            mWords[HEADER].insertColored(name.getText(), namePos,
                                         mTextWhite.mBytes, mTextBlack.mBytes);
        }
    }
    if (length > 0 && mSelectLength > 0 &&
        (mSelectPart == PART_HEADER || mSelectPart == PART_HEADER_AFTER_NAME)) {
        int start = mSelectStart;
        int end = mSelectStart + mSelectLength;
        if (mSelectPart == PART_HEADER_AFTER_NAME && mToNameLength > 0) {
            start += mToNameLength;
            end += mToNameLength;
        }
        mWords[HEADER].insertColor(start + 1, mSelectWhite.mBytes,
                                   mSelectBlack.mBytes, TRUE);
        mWords[HEADER].insertColor(end, mTextWhite.mBytes, mTextBlack.mBytes,
                                   FALSE);
    }
    if (fixedSpaces) {
        mWords[HEADER].toFixedSpaces();
    }
    mWords[HEADER].update();
    if (fixedSpaces) {
        mWords[HEADER].fromFixedSpaces();
    }
}
// .text:0xDA8
void dPaper_c::updateBody(const wchar_t *text, BOOL fixedSpaces) {
    int line;
    const wchar_t *cursor = text;
    static wchar_t lineText[LINE_BUFFER_LENGTH];
    for (line = 1; line <= BODY_LINES; ++line) {
        int index = line - 1;
        int length = mLineLengths[index];
        if (length > 0) {
            memcpy(lineText, cursor, length * sizeof(wchar_t));
        }
        lineText[length] = 0;
        mWords[line].clear();
        mWords[line].set(lineText, 0);
        cursor += length;
        if (length > 0 && mSelectPart == PART_BODY && mSelectLength > 0) {
            int colorStart;
            int start = mSelectStart;
            int end = start + mSelectLength;
            int before = 0;
            int i;
            for (i = 0; i < index; ++i) {
                before += mLineLengths[i];
            }
            if (start >= before && start < before + mLineLengths[index]) {
                colorStart = start - before;
            } else {
                colorStart = -1;
            }
            if (end > before && end <= before + mLineLengths[index]) {
                end -= before;
                if (colorStart == -1) {
                    colorStart = 0;
                }
            }
            if (colorStart > -1) {
                mWords[line].insertColor(colorStart + 1, mSelectWhite.mBytes,
                                         mSelectBlack.mBytes, TRUE);
            }
            if (end > -1) {
                mWords[line].insertColor(end, mTextWhite.mBytes,
                                         mTextBlack.mBytes, FALSE);
            }
        }
        if (fixedSpaces) {
            mWords[line].toFixedSpaces();
        }
        mWords[line].update();
        if (fixedSpaces) {
            mWords[line].fromFixedSpaces();
        }
    }
}
// .text:0xFFC
void dPaper_c::updateFooter(const wchar_t *text, BOOL fixedSpaces) {
    mWords[FOOTER].clear();
    mWords[FOOTER].set(text, 0);
    int start = mSelectStart;
    int end = start + mSelectLength;
    if (mSelectPart == PART_FOOTER && mSelectLength > 0) {
        mWords[FOOTER].insertColor(start + 1, mSelectWhite.mBytes,
                                   mSelectBlack.mBytes, TRUE);
        mWords[FOOTER].insertColor(end, mTextWhite.mBytes, mTextBlack.mBytes,
                                   FALSE);
    }
    if (fixedSpaces) {
        mWords[FOOTER].toFixedSpaces();
    }
    mWords[FOOTER].update();
    if (fixedSpaces) {
        mWords[FOOTER].fromFixedSpaces();
    }
}
// .text:0x10CC
BOOL dPaper_c::fitsHeader(const wchar_t *text) {
    int length = dScript::getLineLength((const u16 *)text, 0);
    if (text[length] != 0) {
        return FALSE;
    }
    nw4r::lyt::TextBox *box = mWords[HEADER].mpTextBox;
    f32 width = m2d::calcTextWidth(box, text, length);
    if (!mpMail->isToNone()) {
        width += 200.35f;
    }
    if (width > box->GetSize().width) {
        return FALSE;
    }
    return TRUE;
}
// .text:0x1188
BOOL dPaper_c::wrapBody(const wchar_t *text, BOOL) {
    int line;
    const wchar_t *cursor = text;
    int lengths[BODY_LINES];
    u32 newLines = 0;
    for (line = 1; line <= BODY_LINES; ++line) {
        int length;
        int index = line - 1;
        length = dScript::getLineLength((const u16 *)cursor, 0);
        if (length > MAX_LINE_LENGTH) {
            length = MAX_LINE_LENGTH;
        }
        if (length > 0) {
            nw4r::lyt::TextBox *box = mWords[line].mpTextBox;
            for (;;) {
                BOOL tooWide = m2d::calcTextWidth(box, cursor, length) >
                               box->GetSize().width;
                if (tooWide == FALSE) {
                    break;
                }
                if (line == BODY_LINES) {
                    return FALSE;
                }
                --length;
            }
        }
        cursor += length;
        lengths[index] = length;
        if (*cursor == L'\n') {
            newLines |= 1 << index;
            ++lengths[index];
            if (line != BODY_LINES) {
                ++cursor;
            }
        }
    }
    if (*cursor == 0) {
        for (int i = 0; i < BODY_LINES; ++i) {
            mLineLengths[i] = lengths[i];
        }
        mNewLineBits = newLines;
        return TRUE;
    }
    return FALSE;
}
// .text:0x12CC
BOOL dPaper_c::fitsFooter(const wchar_t *text) {
    int length = dScript::getLineLength((const u16 *)text, 0);
    if (text[length] != 0) {
        return FALSE;
    }
    nw4r::lyt::TextBox *box = mWords[FOOTER].mpTextBox;
    return !(m2d::calcTextWidth(box, text, length) > box->GetSize().width);
}
// .text:0x1350
BOOL dPaper_c::hasBodyLine(int line) {
    return mLineLengths[line - 1] != 0;
}
// .text:0x1370
int dPaper_c::getBodyLineCount() {
    for (int i = 1; i < BODY_LINES; ++i) {
        if (mLineLengths[i] == 0) {
            return i;
        }
    }
    return BODY_LINES;
}
// .text:0x13DC
int dPaper_c::getBodyLineLength(int line) {
    return mLineLengths[line - 1];
}
// .text:0x13F0
BOOL dPaper_c::isNewLine(int line) {
    return (mNewLineBits & (1 << (line - 1))) != 0;
}
// .text:0x1414
void dPaper_c::setFooterAlign(u8 align) {
    mTextLayout.findTextBox("T_msg_07")->SetTextPositionH(align);
}
// .text:0x1478
void dPaper_c::initColors() {
    mTextLayout.setAnm(&mColorAnm, "ltr_txt_00_T_msg_color.brlan", NULL);
    mColorAnm.setFrame(mColorFrame);
    mTextLayout.calc();
    getColors(0, mTextWhite.mBytes, mTextBlack.mBytes);
    getColors(1, mNameWhite.mBytes, mNameBlack.mBytes);
    getColors(2, mSelectWhite.mBytes, mSelectBlack.mBytes);
    mTextLayout.unbind(&mColorAnm);
    for (int i = 1; i < WORD_COUNT; ++i) {
        setColors(i, mTextWhite.mBytes, mTextBlack.mBytes);
    }
}
namespace dPaper {
static char l_colorPaneName[] = "T_msg_%02d";
} // namespace dPaper

// .text:0x156C
void dPaper_c::getColors(int line, u8 *white, u8 *black) {
    char pane[0x28];
    sprintf(pane, l_colorPaneName, line);
    m2d::getTevColors(mTextLayout.findTextBox(pane), white, black);
}
// .text:0x15E0
void dPaper_c::setColors(int line, const u8 *white, const u8 *black) {
    char pane[0x28];
    sprintf(pane, l_colorPaneName, line);
    m2d::setTevColors(mTextLayout.findTextBox(pane), white, black);
}
// .text:0x1654
dPaper_c::Word_c::Word_c() {
    clear();
}
// .text:0x1698
dPaper_c::Word_c::~Word_c() {
}
// .text:0x16F0
u32 dPaper_c::Word_c::getBufferSize() {
    return sizeof(mBuffer);
}
// .text:0x16F8
wchar_t *dPaper_c::Word_c::getBuffer() {
    return mBuffer;
}
// .text:0x1700
int dPaper_c::load() {
    char path[0x30];
    sprintf(path, l_archivePath, mPaper);
    if (!mPaperRes.load(path)) {
        return 0;
    }
    if (!(mFlags & FLAG_PAPER_ONLY) && !mTextRes.load(l_textArchivePath)) {
        return 0;
    }
    return 1;
}
// .text:0x178C
void dPaper_c::create() {
    char name[0x30];
    if (mpLayout == NULL) {
        EGG::Heap *heap = mHeap::setCurrentHeap(m2d::getAllocator()->mpHeap);
        mpLayout = new m2d::SimpleEx_c;
        mHeap::setCurrentHeap(heap);
    }
    sprintf(name, l_layoutName, mPaper);
    mpLayout->build(&mPaperRes, name);
    if (!(mFlags & FLAG_PAPER_ONLY)) {
        mTextLayout.build(&mTextRes, l_textLayoutName);
        for (int i = 0; i < WORD_COUNT; ++i) {
            sprintf(name, l_createPaneName, i);
            mWords[i].setTextBox(mTextLayout.findTextBox(name));
        }
        mpMail->getToName(mToName);
        mToNameLength = m2d::Word_c::getLength(mToName);
        wrapBody(mpMail->mBody, TRUE);
        setPaperOnly(FALSE);
        dItem::Item item;
        item.setFromIndex(dItem::ITEM_IDX_BUTTERFLY_PAPER, mPaper, 0);
        mColorFrame = dItem::infoBank_c::get()->getBITM(item)->m_textPltt;
        if (mColorFrame > 0) {
            --mColorFrame;
        }
        initColors();
        updateText(FALSE, FALSE);
        mTextLayout.calc();
    } else {
        setPaperOnly(TRUE);
    }
    mSelectPart = PART_NONE;
    mSelectStart = 0;
    mSelectLength = 0;
}
// .text:0x1978
void dPaper_c::calcAnm() {
    mpLayout->copyPos(&mTextLayout, l_rootPane, l_rootPane);
    mTextLayout.calc();
}
