// Game-side m2d extensions: m2d::ResAccLoader_c (disc-loaded layout archives), m2d::SimpleEx_c (menu
// layout helpers), m2d::tagProcessor_c and the m2d words (text shown in text boxes), m2d::AnmEx_c /
// AnmEx2_c (animation tables), the post-draw lists and m2d::moveCalc_c. City Folk only.
// .text 800075C0..8000AD84. See include/game/game/d_m2d.hpp and notes/d_m2d.txt.
#include <game/game/d_m2d.hpp>
#include <game/game/d_script.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/mLib/m_color.hpp>
#include <game/mLib/m_vec.hpp>
#include <lib/egg/core/eggExpHeap.h>
#include <lib/egg/core/eggHeap.h>
#include <lib/egg/math/eggMath.h>
#include <lib/egg/math/eggMatrix.h>
#include <lib/egg/math/eggVector.h>
#include <nw4r/lyt.h>
#include <nw4r/math.h>
#include <nw4r/ut.h>
#include <revolution/MTX/mtx.h>
#include <revolution/MTX/mtxvec.h>
#include <revolution/OS.h>
#include <revolution/SC/scapi.h>
#include <string.h>

// Dependencies whose owners are not recovered yet.
extern "C" {
extern nw4r::ut::ResFont lbl_8058B6FC; // the system font
BOOL fn_8016AE68(dScript::Word_c *word, u16 index, const char *group); // loads a BMG string
u32 fn_8008B90C(); // the current capture source
BOOL fn_802C406C(u16 id); // id is a valid capture texture
void fn_802C2538(void *buffer, int format, u32 source, u16 id, int, void *params); // copies the frame buffer
}

namespace m2d {

static mAllocator_c *l_resAllocator;
static tagProcessor_c l_tagProcessor;
static dLineMg_c l_callbackList;
static dLineMg_c l_captureList;

// 800075C0
ResAccLoader_c::ResAccLoader_c() : mpArc(nullptr) {}

// 8000760C
ResAccLoader_c::~ResAccLoader_c() {}

// 80007674
bool ResAccLoader_c::load(const char *path) {
    if (mpArc != nullptr) {
        return true;
    }
    mpArc = mLoader.request(path, 0, l_resAllocator->mpHeap);
    if (mpArc != nullptr) {
        attach(mpArc, "arc");
        return true;
    }
    return false;
}

// 800076EC
void ResAccLoader_c::remove() {
    if (mpArc != nullptr) {
        detach();
        l_resAllocator->free(mpArc);
        mpArc = nullptr;
    }
    mLoader.release();
}

// 8000774C
bool ResAccLoader_c::create(EGG::Heap *heap, size_t size) {
    EGG::ExpHeap *expHeap = EGG::ExpHeap::create(size, heap, 4);
    expHeap->setName("\x82\x51\x82\x63\x83\x8A\x83\x5C\x81\x5B\x83\x58\x97\x70\x83\x71\x81\x5B\x83\x76"
                     "(d2d::ResAccLoader_c::create)");
    l_resAllocator = new (expHeap, 4) mAllocator_c();
    l_resAllocator->attach(expHeap, 0x20);
    return true;
}

// 800077C4
mAllocator_c *ResAccLoader_c::getAllocator() {
    return l_resAllocator;
}

// 800077CC
int getNameNumber(const char *name, int digits) {
    int len = strlen(name);
    if (len == 0) {
        return -1;
    }
    if (len < digits) {
        digits = len;
    }
    int num = 0;
    for (int i = len - digits; i < len; i++) {
        s32 c = name[i];
        num *= 10;
        if (c >= '0' && c <= '9') {
            num += c - '0';
        } else {
            return -1;
        }
    }
    return num;
}

struct hitArg_c {
    mVec2_c mPos;
    f32 mScale;
    nw4r::lyt::Pane *mpHit;
};

// 80007878
int isInPane(nw4r::lyt::Pane *pane, void *arg) {
    hitArg_c *hit = (hitArg_c *)arg;
    if (!isVisible(pane)) {
        return 0;
    }
    if (pane->GetName()[0] != 'B') {
        return 0;
    }
    const nw4r::math::MTX34 &g = pane->GetGlobalMtx();
    EGG::Matrix34f mtx(g._00, g._01, g._02, g._03, g._10, g._11, g._12, g._13, g._20, g._21, g._22, g._23);
    PSMTXInverse(mtx.m, mtx.m);
    mVec3_c pos(hit->mPos.x, hit->mPos.y, 0.0f);
    fn_803911A4(mtx.m, (Vec *)&pos, (Vec *)&pos);
    char c = pane->GetName()[1];
    if (c == 'C' || c == 'c') {
        f32 r2 = hit->mScale * (pane->GetSize().width * 0.5f);
        r2 *= r2;
        if (pos.x * pos.x + pos.y * pos.y < r2) {
            hit->mpHit = pane;
            return 1;
        }
    } else {
        nw4r::lyt::DrawInfo drawInfo;
        nw4r::ut::Rect rect = pane->GetPaneRect(drawInfo);
        if (pos.x >= rect.left * hit->mScale && pos.x <= rect.right * hit->mScale && pos.y >= rect.bottom * hit->mScale &&
            pos.y <= rect.top * hit->mScale) {
            hit->mpHit = pane;
            return 1;
        }
    }
    return 0;
}

// 80007A44
bool isCircleButton(nw4r::lyt::Pane *pane) {
    nw4r::lyt::Bounding *bounding = nw4r::ut::DynamicCast<nw4r::lyt::Bounding *>(pane);
    if (bounding == nullptr) {
        return false;
    }
    bool ret = false;
    if (pane->GetName()[0] == 'B') {
        bool circle = true;
        char c = pane->GetName()[1];
        if (c != 'C' && c != 'c') {
            circle = false;
        }
        if (circle) {
            ret = true;
        }
    }
    return ret;
}

// 80007B14
bool isRectButton(nw4r::lyt::Pane *pane) {
    nw4r::lyt::Bounding *bounding = nw4r::ut::DynamicCast<nw4r::lyt::Bounding *>(pane);
    if (bounding == nullptr) {
        return false;
    }
    bool ret = false;
    if (pane->GetName()[0] == 'B') {
        bool circle = true;
        char c = pane->GetName()[1];
        if (c != 'C' && c != 'c') {
            circle = false;
        }
        if (!circle) {
            ret = true;
        }
    }
    return ret;
}

// 80007BE4
bool SimpleEx_c::build(const char *lytName, ResAccIf_c *resAcc) {
    return Simple_c::build(lytName, resAcc);
}

// 80007BE8
void SimpleEx_c::setVisible(const char *name, bool visible) {
    nw4r::lyt::Pane *pane = findPane(name);
    if (pane != nullptr) {
        pane->SetVisible(visible);
        if (visible) {
            pane->CalculateMtx(mDrawInfo);
        }
    }
}

// 80007C54
void SimpleEx_c::setAlpha(const char *name, u8 alpha) {
    nw4r::lyt::Pane *pane = findPane(name);
    if (pane != nullptr) {
        pane->SetAlpha(alpha);
        pane->CalculateMtx(mDrawInfo);
    }
}

// 80007CAC
bool SimpleEx_c::setAnm(Anm_c *anm, const char *name, ResAccIf_c *resAcc) {
    if (anm->isBound()) {
        if (!changeAnm(anm, name, resAcc)) {
            return false;
        }
    } else {
        if (!createAnm(anm, name, resAcc)) {
            return false;
        }
        bind(anm);
    }
    anm->setFrame(0.0f);
    return true;
}

// 80007D30
bool SimpleEx_c::build(ResAccIf_c *resAcc, const char *lytName) {
    mpResAcc = resAcc;
    if (!build(lytName, nullptr)) {
        return false;
    }
    setFont(&lbl_8058B6FC);
    return true;
}

// 80007D9C
void SimpleEx_c::setNumber(const char *name, int value, int digits, dScript::NumberFormat_e format) {
    nw4r::lyt::TextBox *textBox = findTextBox(name);
    dString::Word_c word;
    dScript::formatNumber(&word, value, digits, format, 0);
    dScript::Word_c *w = &word;
    textBox->SetString(w->getBuffer(), 0);
}

// 80007E48
void SimpleEx_c::setNumberSep(const char *name, int value, int digits, dScript::NumberFormat_e format) {
    wchar_t *buf;
    nw4r::lyt::TextBox *textBox = findTextBox(name);
    dString::Word_c word;
    dScript::setNumber(&word, value, digits, format);
    dScript::Word_c *w = &word;
    buf = w->getBuffer();
    textBox->AllocStringBuffer(digits + digits / 3);
    textBox->SetString(buf, 0);
}

// 80007F24
void SimpleEx_c::setMessage(const char *name, u16 index, const char *group) {
    stringWordLL_c word;
    word.setTextBox(findTextBox(name));
    if (group == nullptr) {
        group = "sys_2D/SYS_2D_menu";
    }
    fn_8016AE68(&word, index, group);
    word.update();
}

// 80007FC4
void SimpleEx_c::setWord(const char *name, const dScript::Word_c *src) {
    stringWordLL_c word;
    word.setTextBox(findTextBox(name));
    word.copy(src, 0);
    word.update();
}

// 80008048
void SimpleEx_c::setPosZ(const char *name, f32 z) {
    nw4r::lyt::Pane *pane = findPane(name);
    mVec3_c pos = pane->GetTranslate();
    pos.z = z;
    pane->SetTranslate(pos);
}

// 800080A0
void SimpleEx_c::addPos(const char *name, f32 x, f32 y) {
    nw4r::lyt::Pane *pane = findPane(name);
    mVec3_c pos = pane->GetTranslate();
    pos.x += x;
    pos.y += y;
    pane->SetTranslate(pos);
}

// 80008114
const mVec3_c &SimpleEx_c::getPos(const char *name) {
    mVec3_c pos = findPane(name)->GetTranslate();
    return pos;
}

// 80008150
void SimpleEx_c::setPos(const char *name, const mVec3_c *pos) {
    findPane(name)->SetTranslate(*pos);
}

// 80008194
f32 calcTextWidth(nw4r::lyt::TextBox *textBox, const wchar_t *str, int len) {
    if (str == nullptr) {
        str = textBox->GetStringBuffer();
    }
    if (len == 0) {
        len = textBox->GetStringLength();
    }
    EGG::Heap *heap = EGG::Heap::getCurrentHeap();
    wchar_t *buf = (wchar_t *)EGG::Heap::alloc(len * sizeof(wchar_t), 0x20, heap);
    memset(buf, 0, len * sizeof(wchar_t));
    wchar_t *dst = buf;
    int i = 0;
    int num = 0;
    while (i < len) {
        if (str[i] == 0x1A) {
            i += dScript::getTagLength(&str[i]);
        } else {
            if (str[i] == 0) {
                break;
            }
            *dst = str[i];
            i++;
            dst++;
            num++;
        }
    }
    const nw4r::ut::Font *font = textBox->GetFont();
    nw4r::lyt::Size size = textBox->GetFontSize();
    nw4r::ut::TextWriterBase<wchar_t> writer;
    writer.SetFont(*font);
    f32 sx = size.width / writer.GetFontWidth();
    f32 sy = size.height / writer.GetFontHeight();
    writer.SetScale(sx, sy);
    writer.SetCharSpace(textBox->GetCharSpace());
    f32 width = writer.CalcStringWidth(buf, num);
    if (buf != nullptr) {
        EGG::Heap::free(buf, heap);
    }
    width += 0.0001f;
    return width;
}

// 80008308
mVec3_c getTotalPos(nw4r::lyt::Pane *pane) {
    mVec3_c pos = pane->GetTranslate();
    for (nw4r::lyt::Pane *parent = pane->GetParent(); parent != nullptr; parent = parent->GetParent()) {
        mVec3_c t = parent->GetTranslate();
        pos += t;
    }
    return pos;
}

// 8000837C
void setNameNumber(char *name, void *arg) {
    const char *num = (const char *)arg;
    int i = strlen(name);
    for (; i > 0;) {
        i--;
        if (name[i] >= '0' && name[i] <= '9') {
            name[i - 1] = num[0];
            name[i] = num[1];
            return;
        }
    }
}

// 800083F8
void setAnmNumber(Anm_c *anm, int number) {
    char num[3];
    num[0] = number / 10 + '0';
    num[1] = number % 10 + '0';
    num[2] = '\0';
    anm->patrolTargetName(setNameNumber, num);
}

// 80008468
nw4r::lyt::Material *getMaterial(nw4r::lyt::Pane *pane) {
    return pane->GetMaterial();
}

// 80008478
void fn_80008478(nw4r::lyt::Pane *pane) {
    getMaterial(pane)->GetTexMapAry();
}

// 8000849C
void fn_8000849C(nw4r::lyt::Pane *pane) {
    getMaterial(pane)->GetTexSRTAry();
}

// 800084C0
void getTevColors(nw4r::lyt::Pane *pane, u8 *white, u8 *black) {
    nw4r::lyt::Material *material = getMaterial(pane);
    GXColorS10 c0 = material->GetTevColor(0);
    toColor(&c0, black);
    GXColorS10 c1 = material->GetTevColor(1);
    toColor(&c1, white);
}

// 8000855C
void setTevColors(nw4r::lyt::Pane *pane, const u8 *white, const u8 *black) {
    nw4r::lyt::Material *material = getMaterial(pane);
    GXColorS10 c1 = {white[0], white[1], white[2], white[3]};
    material->SetTevColor(1, c1);
    GXColorS10 c0 = {black[0], black[1], black[2], black[3]};
    material->SetTevColor(0, c0);
}

// 800085D4
void SimpleEx_c::copyPos(SimpleEx_c *other, const char *srcName, const char *dstName) {
    nw4r::lyt::Pane *src = findPane(srcName);
    nw4r::lyt::Pane *dst = other->findPane(dstName);
    if (src != nullptr && dst != nullptr) {
        mVec3_c pos;
        getScreenPos(&pos, src);
        if (SCGetAspectRatio() == SC_ASPECT_WIDE && !dst->IsLocationAdjust()) {
            pos.x *= 1.368421f;
        }
        dst->SetTranslate(pos);
    }
}

// 8000868C
void SimpleEx_c::copyPos(SimpleEx_c *other, const char *name) {
    copyPos(other, name, name);
}

// Inferred, unreferenced helper: its code is stripped, but 1.0f must enter the
// constant pool here. The original function's name and body are not recovered.
f32 getDefaultScale() {
    return 1.0f;
}

// 80008694
void getScreenPos(mVec3_c *pos, nw4r::lyt::Pane *pane) {
    const nw4r::math::MTX34 &g = pane->GetGlobalMtx();
    EGG::Matrix34f mtx(g._00, g._01, g._02, g._03, g._10, g._11, g._12, g._13, g._20, g._21, g._22, g._23);
    pos->x = 0.0f;
    pos->y = 0.0f;
    pos->z = 0.0f;
    fn_803911A4(mtx.m, (Vec *)pos, (Vec *)pos);
}

// 80008724
void getScreenLeftTop(mVec3_c *pos, nw4r::lyt::Pane *pane) {
    nw4r::lyt::DrawInfo drawInfo;
    nw4r::ut::Rect rect = pane->GetPaneRect(drawInfo);
    const nw4r::math::MTX34 &g = pane->GetGlobalMtx();
    EGG::Matrix34f mtx(g._00, g._01, g._02, g._03, g._10, g._11, g._12, g._13, g._20, g._21, g._22, g._23);
    pos->x = 0.0f;
    pos->y = 0.0f;
    pos->z = 0.0f;
    pos->x = rect.left;
    pos->y = rect.top;
    fn_803911A4(mtx.m, (Vec *)pos, (Vec *)pos);
}

// 800087F4
void getScreenRightBottom(mVec3_c *pos, nw4r::lyt::Pane *pane) {
    nw4r::lyt::DrawInfo drawInfo;
    nw4r::ut::Rect rect = pane->GetPaneRect(drawInfo);
    const nw4r::math::MTX34 &g = pane->GetGlobalMtx();
    EGG::Matrix34f mtx(g._00, g._01, g._02, g._03, g._10, g._11, g._12, g._13, g._20, g._21, g._22, g._23);
    pos->x = 0.0f;
    pos->y = 0.0f;
    pos->z = 0.0f;
    pos->x = rect.right;
    pos->y = rect.bottom;
    fn_803911A4(mtx.m, (Vec *)pos, (Vec *)pos);
}

// 800088C4
bool isInPaneRect(nw4r::lyt::Pane *pane, f32 margin, const mVec2_c *pos) {
    mVec3_c lt;
    getScreenLeftTop(&lt, pane);
    mVec3_c rb;
    getScreenRightBottom(&rb, pane);
    return lt.x - margin <= pos->x && rb.x + margin >= pos->x && lt.y >= pos->y && rb.y <= pos->y;
}

// 8000897C
void SimpleEx_c::setPosFrom(const char *srcName, const char *dstName, f32 x, f32 y) {
    nw4r::lyt::Pane *src = findPane(srcName);
    nw4r::lyt::Pane *dst = findPane(dstName);
    mVec3_c pos = src->GetTranslate();
    pos.x += x;
    pos.y += y;
    dst->SetTranslate(pos);
}

// 80008A20
void rgb565ToColor(u16 color, GXColorS10 *out) {
    out->r = ((color >> 8) & 0xF8) | ((color >> 13) & 7);
    out->g = ((color >> 3) & 0xFC) | ((color >> 9) & 3);
    out->b = ((color << 3) & 0xF8) | ((color >> 2) & 7);
    out->a = 0xFF;
}

// 80008A50
u8 clampColor(int value) {
    int v;
    if (value < 0) {
        v = 0;
    } else {
        v = 0xFF;
        if (value <= 0xFF) {
            v = value;
        }
    }
    return v;
}

// 80008A78
void toColor(const GXColorS10 *in, u8 *out) {
    out[0] = clampColor(in->r);
    out[1] = clampColor(in->g);
    out[2] = clampColor(in->b);
    out[3] = clampColor(in->a);
}

// 80008ADC
int hitButton(const mVec2_c *pos, Simple_c *lyt, int rectBase, int rectMax, int circleBase, int circleMax, f32 scale) {
    hitArg_c hit;
    hit.mpHit = nullptr;
    hit.mScale = scale;
    hit.mPos = *pos;
    if (circleBase != -1) {
        lyt->patrolPane(isInPane, isCircleButton, &hit);
        if (hit.mpHit != nullptr) {
            int idx = circleBase + getNameNumber(hit.mpHit->GetName(), 2);
            if (idx >= circleBase && idx <= circleMax) {
                return idx;
            } else {
                return -1;
            }
        }
    }
    if (rectBase != -1) {
        lyt->patrolPane(isInPane, isRectButton, &hit);
        if (hit.mpHit != nullptr) {
            int idx = rectBase + getNameNumber(hit.mpHit->GetName(), 2);
            if (idx >= rectBase && idx <= rectMax) {
                return idx;
            } else {
                return -1;
            }
        }
    }
    return -1;
}

// 80008BF4
void setVisible(nw4r::lyt::Pane *root, const char **names, int num, bool visible) {
    for (int i = 0; i < num; i++) {
        root->FindPaneByName(names[i], true)->SetVisible(visible);
    }
}

// 80008C74
tagProcessor_c::tagProcessor_c() {}

// 80008CB0
tagProcessor_c::~tagProcessor_c() {}

// 80008D0C
tagProcessor_c::Operation tagProcessor_c::Process(u16 ch, ContextType *pCtx) {
    if (ch == 0x1A) {
        u8 size = 0;
        u32 cmd = 0;
        void *data = nullptr;
        EGG::MsgRes::analyzeTag(ch, pCtx->str, &size, &cmd, &data);
        switch (cmd) {
            case 0xFF0000:
                fn_8016C9FC(0, pCtx, size, data);
                break;
            case 0xFE0000:
                setColor(pCtx, (u8 *)data);
                break;
        }
        pCtx->str = (const wchar_t *)((const u8 *)pCtx->str + (size & ~1));
        return OPERATION_DEFAULT;
    }
    return nw4r::ut::TagProcessorBase<wchar_t>::Process(ch, pCtx);
}

// 80008DE0
tagProcessor_c::Operation tagProcessor_c::CalcRect(nw4r::ut::Rect *pRect, u16 ch, ContextType *pCtx) {
    if (ch == 0x1A) {
        u8 size = 0;
        u32 cmd = 0;
        void *data = nullptr;
        EGG::MsgRes::analyzeTag(ch, pCtx->str, &size, &cmd, &data);
        pCtx->str = (const wchar_t *)((const u8 *)pCtx->str + (size & ~1));
        return OPERATION_DEFAULT;
    }
    return nw4r::ut::TagProcessorBase<wchar_t>::CalcRect(pRect, ch, pCtx);
}

// 80008E58
void tagProcessor_c::setColor(ContextType *pCtx, u8 *data) {
    mColor white;
    white.r = data[0];
    white.g = data[1];
    white.b = data[2];
    white.a = data[3];
    mColor black;
    black.r = data[4];
    black.g = data[5];
    black.b = data[6];
    black.a = data[7];
    pCtx->writer->SetColorMapping(black, white);
    pCtx->writer->SetupGX();
}

// 80008EF4
int writeColorTag(wchar_t *buf, const u8 *color0, const u8 *color1) {
    u8 *p = (u8 *)buf;
    *buf = 0x1A;
    p[2] = 0xE;
    p[3] = 0xFE;
    p[4] = 0;
    p[5] = 0;
    p[6] = color0[0];
    p[7] = color0[1];
    p[8] = color0[2];
    p[9] = color0[3];
    p[10] = color1[0];
    p[11] = color1[1];
    p[12] = color1[2];
    p[13] = color1[3];
    return 0xE;
}

// 80008F80
Word_c::Word_c() : mpTextBox(nullptr) {}

// 80008FC4
Word_c::~Word_c() {}

// 8000901C
void Word_c::setTextBox(nw4r::lyt::TextBox *textBox) {
    mpTextBox = textBox;
    u32 capacity = getCapacity();
    mpTextBox->AllocStringBuffer(capacity);
    mpTextBox->SetTagProcessor(&l_tagProcessor);
    clear();
    update();
}

// 80009084
void Word_c::toFixedSpaces() {
    wchar_t *buf = getBuffer();
    int capacity = getCapacity();
    int i = 0;
    while (i < capacity) {
        wchar_t *p = &buf[i];
        if (buf[i] == 0x1A) {
            i += dScript::getTagLength(p);
        } else {
            if (buf[i] == L' ') {
                *p = 0xE057;
            }
            if (*p == 0x3000) {
                *p = 0xE020;
            }
            i++;
        }
    }
}

// 80009134
void Word_c::fromFixedSpaces() {
    wchar_t *buf = getBuffer();
    int capacity = getCapacity();
    int i = 0;
    while (i < capacity) {
        wchar_t *p = &buf[i];
        if (buf[i] == 0x1A) {
            i += dScript::getTagLength(p);
        } else {
            if (buf[i] == 0xE057) {
                *p = L' ';
            }
            if (*p == 0xE020) {
                *p = 0x3000;
            }
            i++;
        }
    }
}

// 800091E0
void Word_c::update() {
    u32 capacity = getCapacity();
    mpTextBox->SetString(getBuffer(), 0, capacity);
}

// 8000924C
f32 Word_c::calcWidth(const wchar_t *str, int len) {
    if (str == nullptr) {
        str = getBuffer();
    }
    if (len == 0) {
        len = getCapacity();
    }
    return calcTextWidth(mpTextBox, str, len);
}

// 800092CC
int Word_c::getLength(const wchar_t *str) {
    const wchar_t *p = str;
    int len = 0;
    while (true) {
        if (*p == 0x1A) {
            p += dScript::getTagLength(p);
        } else if (*p != 0) {
            p++;
            len++;
        } else {
            break;
        }
    }
    return len;
}

// 80009338
int Word_c::getSize(const wchar_t *str) {
    const wchar_t *p = str;
    int size = 0;
    while (true) {
        if (*p == 0x1A) {
            u8 len = dScript::getTagLength(p);
            p += len;
            size += len;
        } else if (*p != 0) {
            p++;
            size++;
        } else {
            break;
        }
    }
    return size;
}

// 800093AC
int Word_c::getSizeAt(const wchar_t *str, int pos) {
    const wchar_t *p = str;
    int size = 0;
    int i = 0;
    while (i < pos) {
        if (*p == 0x1A) {
            u8 len = dScript::getTagLength(p);
            p += len;
            size += len;
        } else if (*p != 0) {
            p++;
            i++;
            size++;
        } else {
            break;
        }
    }
    return size;
}

// 80009444
int Word_c::procColor(int pos) {
    return dScript::getTagLength(getBuffer() + pos);
}

// 8000948C
int Word_c::procRubyTag(int pos) {
    return procRuby(pos);
}

// 80009490
int Word_c::getPosAt(f32 x) {
    const wchar_t *str = getBuffer();
    f32 prev = 0.0f;
    f32 cur = prev;
    int pos = 0;
    while (x > cur) {
        if (*str == 0x1A) {
            str += dScript::getTagLength(str);
        } else {
            if (*str == 0 || *str == L'\n') {
                return pos;
            }
            prev = cur;
            pos++;
            str++;
            cur = calcWidth(nullptr, getSizeAt(getBuffer(), pos));
        }
    }
    if (pos != 0 && x - prev < cur - x) {
        pos--;
    }
    return pos;
}

// 800095BC
int Word_c::getPosAtScreenX(f32 x) {
    mVec3_c pos;
    f32 dx;
    switch (mpTextBox->GetTextPositionH()) {
        case 2: {
            mVec3_c rb;
            getScreenRightBottom(&rb, mpTextBox);
            pos = rb;
            f32 width = calcWidth(nullptr, 0);
            if (SCGetAspectRatio() == SC_ASPECT_WIDE) {
                width *= 0.730769f;
            }
            dx = x - (pos.x - width);
            break;
        }
        default: {
            mVec3_c lt;
            getScreenLeftTop(&lt, mpTextBox);
            pos = lt;
            dx = x - pos.x;
            break;
        }
    }
    if (SCGetAspectRatio() == SC_ASPECT_WIDE) {
        dx *= 1.368421f;
    }
    return getPosAt(dx);
}

// 800096E0
f32 Word_c::getScreenXLeft(int pos) {
    mVec3_c lt;
    getScreenLeftTop(&lt, mpTextBox);
    if (pos == 0) {
        return lt.x;
    }
    f32 width = calcWidth(nullptr, getSizeAt(getBuffer(), pos));
    if (SCGetAspectRatio() == SC_ASPECT_WIDE) {
        width *= 0.730769f;
    }
    return lt.x + width;
}

// 80009790
f32 Word_c::getScreenXRight(int pos) {
    mVec3_c rb;
    getScreenRightBottom(&rb, mpTextBox);
    int size = getSizeAt(getBuffer(), pos);
    f32 width;
    if (pos == 0) {
        width = 0.0f;
    } else {
        width = calcWidth(nullptr, size);
    }
    f32 total = calcWidth(nullptr, 0);
    if (SCGetAspectRatio() == SC_ASPECT_WIDE) {
        width *= 0.730769f;
        total *= 0.730769f;
    }
    return rb.x + width - total;
}

// 8000986C
f32 Word_c::getScreenX(int pos) {
    switch (mpTextBox->GetTextPositionH()) {
        case 2:
            return getScreenXRight(pos);
        default:
            return getScreenXLeft(pos);
    }
}

// 800098A8
BOOL Word_c::insertColored(const wchar_t *str, int pos, const u8 *color0, const u8 *color1) {
    wchar_t *buf = getBuffer();
    int size = getSize(buf);
    int strSize = getSize(str);
    if (size < pos) {
        return FALSE;
    }
    if (strSize == 0) {
        return FALSE;
    }
    if (strSize + size > getCapacity()) {
        return FALSE;
    }
    EGG::Heap *heap = EGG::Heap::getCurrentHeap();
    wchar_t *tmp = (wchar_t *)EGG::Heap::alloc(getBufferSize(), 0x20, heap);
    memset(tmp, 0, getBufferSize());
    int n = 0;
    if (pos > 0) {
        memcpy(tmp, buf, pos * sizeof(wchar_t));
        n = pos;
    }
    u8 white[4];
    u8 black[4];
    *(u32 *)white = 0xFFFFFFFF;
    *(u32 *)black = 0xFFFFFFFF;
    if (color0 != nullptr) {
        getTevColors(mpTextBox, white, black);
        if (color1 == nullptr) {
            color1 = black;
        }
        n += writeColorTag(&tmp[n], color0, color1) / sizeof(wchar_t);
    }
    memcpy(&tmp[n], str, strSize * sizeof(wchar_t));
    n += strSize;
    if (color0 != nullptr) {
        n += writeColorTag(&tmp[n], white, black) / sizeof(wchar_t);
    }
    if (size > pos) {
        memcpy(&tmp[n], &buf[pos], (size - pos) * sizeof(wchar_t));
    }
    memcpy(buf, tmp, getBufferSize());
    if (tmp != nullptr) {
        EGG::Heap::free(tmp, heap);
    }
    return TRUE;
}

// 80009AA0
BOOL Word_c::insertColor(int pos, const u8 *color0, const u8 *color1, BOOL before) {
    wchar_t *buf = getBuffer();
    int size = getSize(buf);
    int at = getSizeAt(buf, pos);
    if (before && at > 0) {
        at--;
    }
    if (size < pos) {
        return FALSE;
    }
    if (size + 7 > getCapacity()) {
        return FALSE;
    }
    EGG::Heap *heap = EGG::Heap::getCurrentHeap();
    wchar_t *tmp = (wchar_t *)EGG::Heap::alloc(getBufferSize(), 0x20, heap);
    memset(tmp, 0, getBufferSize());
    int n = 0;
    if (at > 0) {
        memcpy(tmp, buf, at * sizeof(wchar_t));
        n = at;
    }
    n += writeColorTag(&tmp[n], color0, color1) / sizeof(wchar_t);
    if (size > at) {
        memcpy(&tmp[n], &buf[at], (size - at) * sizeof(wchar_t));
    }
    memcpy(buf, tmp, getBufferSize());
    if (tmp != nullptr) {
        EGG::Heap::free(tmp, heap);
    }
    return TRUE;
}

// 80009C34
int Word_c::getLineNum() {
    const wchar_t *str = getBuffer();
    int capacity = getCapacity();
    int i = 0;
    int lines = 0;
    int chars = 0;
    while (i < capacity && *str != 0) {
        if (*str == 0x1A) {
            str += dScript::getTagLength(str);
        } else {
            if (*str == 0) {
                break;
            }
            if (*str == L'\n') {
                chars = 0;
                lines++;
            } else if (*str != L' ') {
                chars = 1;
            }
            str++;
            i++;
        }
    }
    return lines + chars;
}

// 80009CF8
itemWord_c::itemWord_c() {
    clear();
}

// 80009D3C
itemWord_c::~itemWord_c() {}

// 80009D94
u32 itemWord_c::getBufferSize() {
    return sizeof(mBuffer);
}

// 80009D9C
wchar_t *itemWord_c::getBuffer() {
    return mBuffer;
}

// 80009DA4
stringWord_c::stringWord_c() {
    clear();
}

// 80009DE8
stringWord_c::~stringWord_c() {}

// 80009E40
u32 stringWord_c::getBufferSize() {
    return sizeof(mBuffer);
}

// 80009E48
wchar_t *stringWord_c::getBuffer() {
    return mBuffer;
}

// 80009E50
stringWordL_c::stringWordL_c() {
    clear();
}

// 80009E94
stringWordL_c::~stringWordL_c() {}

// 80009EEC
u32 stringWordL_c::getBufferSize() {
    return sizeof(mBuffer);
}

// 80009EF4
wchar_t *stringWordL_c::getBuffer() {
    return mBuffer;
}

// 80009EFC
stringWordLL_c::stringWordLL_c() {
    clear();
}

// 80009F40
stringWordLL_c::~stringWordLL_c() {}

// 80009F98
u32 stringWordLL_c::getBufferSize() {
    return sizeof(mBuffer);
}

// 80009FA0
wchar_t *stringWordLL_c::getBuffer() {
    return mBuffer;
}

// 80009FA8
AnmEx_c::AnmEx_c() : mpLyt(nullptr), mpNames(nullptr), mpResAcc(nullptr), mNum(0), mCurIdx(-1), mNextIdx(-1) {}

// 8000A004
AnmEx_c::~AnmEx_c() {}

// 8000A05C
void AnmEx_c::init(SimpleEx_c *lyt, ResAccIf_c *resAcc, const char **names, int num) {
    mpLyt = lyt;
    mpResAcc = resAcc;
    mpNames = names;
    mNum = num;
    mNextIdx = -1;
    mCurIdx = -1;
}

// 8000A07C
bool AnmEx_c::play() {
    if (mCurIdx == -1) {
        if (mNextIdx != -1) {
            change(mNextIdx);
            return true;
        }
        return false;
    }
    if (mFrameCtrl.isStop()) {
        if (mNextIdx != -1 && mNextIdx != mCurIdx) {
            change(mNextIdx);
        } else {
            return false;
        }
    }
    Anm_c::play();
    return true;
}

// 8000A130
bool AnmEx_c::setNext(int idx) {
    if (mNextIdx != idx) {
        mNextIdx = idx;
        return mCurIdx != idx;
    }
    return false;
}

// 8000A160
BOOL AnmEx_c::isEnd() {
    BOOL end = TRUE;
    StopState_e stopped;
    if (mNextIdx != -1) {
        stopped = RUNNING;
        if (mCurIdx == mNextIdx && mFrameCtrl.isStop()) {
            stopped = STOPPED;
        }
        if (stopped == RUNNING) {
            end = FALSE;
        }
    }
    return end;
}

// 8000A1D0
void AnmEx_c::start(int idx) {
    mNextIdx = idx;
    change(idx);
}

// 8000A1E4
void AnmEx_c::startEnd(int idx) {
    mNextIdx = idx;
    change(idx);
    setToEnd();
}

// 8000A228
void AnmEx_c::stop() {
    mNextIdx = -1;
    if (mCurIdx != -1) {
        mpLyt->unbind(this);
        mCurIdx = -1;
    }
}

// 8000A27C
void AnmEx_c::change(int idx) {
    if (mpNames[idx] != nullptr) {
        mpLyt->setAnm(this, mpNames[idx], mpResAcc);
        mCurIdx = idx;
    }
}

// 8000A2D8
AnmEx2_c::AnmEx2_c() : mNumber(0) {}

// 8000A31C
AnmEx2_c::~AnmEx2_c() {}

// 8000A374
void AnmEx2_c::setNumber(int number) {
    mNumber = number;
    onFlag(FLAG_NUMBER);
}

// 8000A380
void AnmEx2_c::stop() {
    if (isFlag(FLAG_NUMBER)) {
        setAnmNumber(this, mNumber);
    }
    AnmEx_c::stop();
}

// 8000A3CC
void AnmEx2_c::onFlag(u32 flag) {
    mFlags |= flag;
}

// 8000A3DC
BOOL AnmEx2_c::isFlag(u32 flag) {
    return (mFlags & flag) != 0;
}

// 8000A3F4
void AnmEx2_c::change(int idx) {
    if (mpNames[idx] != nullptr) {
        if (isBound()) {
            if (isFlag(FLAG_NUMBER)) {
                setAnmNumber(this, mNumber);
            }
            mpLyt->unbind(this);
        }
        create(mpNames[idx], mpResAcc);
        if (isFlag(FLAG_NUMBER)) {
            setAnmNumber(this, mNumber);
        }
        mpLyt->bind(this);
        if (isFlag(FLAG_END)) {
            setToEnd();
        } else {
            setFrame(0.0f);
        }
        mCurIdx = idx;
    }
}

// 8000A4E8
void initCallbackList() {
    l_callbackList.init();
}

// 8000A4F0
void addCallback(callbackNode_c *node) {
    l_callbackList.insertByPriority(node);
}

// 8000A4FC
void executeCallbacks() {
    callbackNode_c *node = (callbackNode_c *)l_callbackList.getFirst();
    while (node != nullptr) {
        node->execute();
        callbackNode_c *cur = node;
        node = (callbackNode_c *)node->getNext();
        l_callbackList.removeLineNode(cur);
    }
    l_callbackList.init();
}

// 8000A55C
void initCaptureList() {
    l_captureList.init();
}

// 8000A564
void addCapture(captureNode_c *node) {
    l_captureList.insertByPriority(node);
}

// 8000A570
void removeCapture(captureNode_c *node) {
    l_captureList.removeLineNode(node);
}

// 8000A57C
void executeCaptures() {
    captureNode_c *node = (captureNode_c *)l_captureList.getFirst();
    while (node != nullptr) {
        node->execute();
        captureNode_c *cur = node;
        node = (captureNode_c *)node->getNext();
        l_captureList.removeLineNode(cur);
    }
    l_captureList.init();
}

struct captureParams_c {
    u16 mWidth;
    u16 mHeight;
    int _4;
    GXColor mColor;
    int _C;
};

// 8000A5D0
void captureNode_c::execute() {
    if (mID == 0xFFFE || fn_802C406C(mID)) {
        captureParams_c params;
        params._4 = 1;
        params.mColor = nw4r::ut::Color(0);
        params._C = 0;
        params.mWidth = mWidth;
        params.mHeight = mHeight;
        if (mID == 0xFFFE) {
            fn_802C2538(mpBuffer, 7, fn_8008B90C(), 0, 0, &params);
        } else {
            fn_802C2538(mpBuffer, 0, 0, mID, 0, &params);
        }
        DCFlushRange(mpBuffer, mWidth * mHeight * 2);
    }
}

// 8000A6B8
void captureNode_c::clear() {
    mID = 0xFFFF;
    mpBuffer = nullptr;
}

// 8000A6D0
void captureNode_c::set(u16 id, int width, int height, void *buffer) {
    clear();
    mID = id;
    mpBuffer = buffer;
    mWidth = width;
    mHeight = height;
}

// 8000A724
void captureNode_c::entry() {
    addCapture(this);
}

// 8000A728
void captureNode_c::remove() {
    removeCapture(this);
}

// 8000A72C
void moveCalc_c::reset() {
    mCount = -1;
}

// 8000A738
bool moveCalc_c::calc() {
    int count = mCount;
    if (count <= 0) {
        mCount = -1;
        return true;
    }
    if (count <= mDecel) {
        f32 rate = (f32)count / (f32)(count + 1);
        mStep.x *= rate;
        mStep.y *= rate;
    }
    mPos.x += mStep.x;
    mPos.y += mStep.y;
    mCount--;
    return false;
}

// 8000A7EC
void moveCalc_c::set(const mVec2_c *from, const mVec2_c *to, int maxSteps, int minSteps, int decel, f32 speed) {
    mPos.x = from->x;
    mPos.y = from->y;
    mDecel = decel;
    mVec2_c diff = *to - *from;
    int count = EGG::Mathf::sqrt(diff.x * diff.x + diff.y * diff.y) / speed;
    if (count > maxSteps) {
        count = maxSteps;
    }
    if (count < minSteps) {
        count = minSteps;
    }
    mCount = count;
    f32 steps = 0.0f;
    for (int i = decel; i > 0; i--) {
        steps += i;
    }
    steps /= decel + 1;
    steps += mCount - decel;
    mVec2_c step = diff / steps;
    mStep.x = step.x;
    mStep.y = step.y;
}

// 8000AA5C
SimpleEx_c::~SimpleEx_c() {}

} // namespace m2d
