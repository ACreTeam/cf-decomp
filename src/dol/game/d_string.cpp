// dString: BMG string loading, the global word slots and the string words.
// .text 8016AE68..8016B894, compiled with -sym on. The static initializer
// (8016B7A8) is followed by dString::Comp_c's destructor. It has to be
// strong (an inline one makes Comp_c's vtable weak, which moves it from the
// start of .data to the end) and come from another source file to land after
// the initializer, so it lives in include/game/game/d_string_comp.inc.
// Notes: notes/d_string.txt.
#include <game/game/d_string.hpp>
#include <game/game/d_land.hpp>
#include <game/game/d_personal_id.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_private_data.hpp>
#include <game/cLib/c_math.hpp>
#include <cstring>

namespace dScript {
Bank_c *getBank();
void *getBmgFile(Bank_c *bank, const char *name);
const u8 *getTagParams(const wchar_t *tag);
} // namespace dScript

// Callers still use these names.
extern "C" {
BOOL fn_8016AE68(dScript::Word_c *word, u16 index, const char *group);
u16 fn_8016AF68(const char *group);
u16 fn_8016AFD4(const char *group);
void fn_8016B050(int slot, const dPersonalID_c *pid);
void fn_8016B0B0(int slot, dLandID_c *land);
void fn_8016B15C(int slot, dScript::Word_c *word);
dScript::Word_c *fn_8016B1B8(int slot);
}

// 805FF398 (static initializer 8016B7A8)
dString::Comp_c lbl_805FF398;

// 8016AE68: loads string `index` of a BMG group into word
BOOL fn_8016AE68(dScript::Word_c *word, u16 index, const char *group) {
    void *bmg = dScript::getBmgFile(dScript::getBank(), group);
    if (bmg == NULL) {
        return FALSE;
    }
    u32 max = word->getCapacity() - 1;
    dScript::Res_c res(bmg);
    if (index >= res.getCount()) {
        return FALSE;
    }
    const wchar_t *msg = res.getMessage(index);
    if (dScript::getStringLength(msg, 0, 0) > max) {
        return FALSE;
    }
    word->set(msg, 0);
    return TRUE;
}

// 8016AF68: number of strings in a BMG group
u16 fn_8016AF68(const char *group) {
    void *bmg = dScript::getBmgFile(dScript::getBank(), group);
    if (bmg == NULL) {
        return 0;
    }
    dScript::Res_c res(bmg);
    return res.getCount();
}

// 8016AFD4: random string index (1-based) in a BMG group
u16 fn_8016AFD4(const char *group) {
    void *bmg = dScript::getBmgFile(dScript::getBank(), group);
    if (bmg == NULL) {
        return 0;
    }
    dScript::Res_c res(bmg);
    int index = cM::rndInt(res.getCount() - 1);
    return index + 1;
}

// 8016B050: puts a player name into a word slot
void fn_8016B050(int slot, const dPersonalID_c *pid) {
    dHmnName::Word_c name;
    pid->setWord(&name);
    fn_8016B15C(slot, &name);
}

// 8016B0B0: puts a town name into a word slot
void fn_8016B0B0(int slot, dLandID_c *land) {
    dLandNameWord_c name;
    land->setWord(&name);
    fn_8016B15C(slot, &name);
}

namespace dString {

// 8016B110
int WordBase_c::procPlayer(int pos) {
    return procPlayerName(pos, &dPlayerMgr_c::getCurrentPlayer()->mPID);
}

} // namespace dString

// 8016B15C: copies word into a slot
void fn_8016B15C(int slot, dScript::Word_c *word) {
    dScript::Word_c *dst = &lbl_805FF398.mWords[slot];
    dst->clear();
    dst->copy(word, 0);
}

// 8016B1B8: word slot
dScript::Word_c *fn_8016B1B8(int slot) {
    return &lbl_805FF398.mWords[slot];
}

namespace dString {

// 8016B1D0
WordBase_c::WordBase_c() {}

// 8016B20C
WordBase_c::~WordBase_c() {}

// 8016B264
int WordBase_c::procWord(int pos) {
    const u8 *params = dScript::getTagParams(getBuffer() + pos);
    u8 slot = 0;
    memcpy(&slot, params, 1);
    dScript::Word_c *word = fn_8016B1B8(slot);
    _18 = 0;
    return setTagWord(pos, word);
}

// 8016B2FC
int WordBase_c::procPlayerChar(int pos, int index) {
    wchar_t c = 0;
    if (!lbl_805FF398.mPlayerName.getChar(&c, index)) {
        lbl_805FF398.mNoChar = 1;
        return removeTag(pos);
    }
    removeTag(pos);
    return insertWord(pos, &c, 1, FALSE);
}

// 8016B398
int WordBase_c::procPlayerChar0(int pos) {
    return procPlayerChar(pos, 0);
}

// 8016B3A0
int WordBase_c::procPlayerChar1(int pos) {
    return procPlayerChar(pos, 1);
}

// 8016B3A8
int WordBase_c::procPlayerChar2(int pos) {
    return procPlayerChar(pos, 2);
}

// 8016B3B0
int WordBase_c::setInflect(int pos) {
    const u8 *params = dScript::getTagParams(getBuffer() + pos);
    u8 gender = 0;
    u8 indef = 0;
    u8 def = 0;
    memcpy(&gender, params, 1);
    memcpy(&indef, params + 1, 1);
    memcpy(&def, params + 2, 1);
    dScript::Inflect_c *inflect = &mInflect;
    inflect->setGender(gender);
    inflect->setIndefArticle(indef);
    inflect->setDefArticle(def);
    return dScript::Word_c::setInflect(pos);
}

// 8016B480
int WordBase_c::procPlayerGender(int pos) {
    return procByPlayerGender(pos, &dPlayerMgr_c::getCurrentPlayer()->mPID);
}

// 8016B4CC
int WordBase_c::procWordGender(int pos) {
    const u8 *params = dScript::getTagParams(getBuffer() + pos);
    u8 slot = 0;
    memcpy(&slot, params, 1);
    return procByWordGender(pos, fn_8016B1B8(slot));
}

// 8016B54C
int WordBase_c::procPlayerElision(int pos) {
    return procElisionPlayer(pos, &dPlayerMgr_c::getCurrentPlayer()->mPID);
}

// 8016B598
int WordBase_c::procWordElision(int pos) {
    const u8 *params = dScript::getTagParams(getBuffer() + pos);
    u8 slot = 0;
    memcpy(&slot, params, 1);
    return procElision(pos, 1, *fn_8016B1B8(slot)->getBuffer());
}

// 8016B630
Word_c::Word_c() {
    clear();
}

// 8016B674
Word_c::Word_c(const wchar_t *str) {
    clear();
    set(str, 0);
}

// 8016B6D4
Word_c::Word_c(u16 index, const char *group) {
    clear();
    fn_8016AE68(this, index, group);
}

// 8016B740
Word_c::~Word_c() {}

// 8016B798
u32 Word_c::getBufferSize() {
    return sizeof(mBuffer);
}

// 8016B7A0
wchar_t *Word_c::getBuffer() {
    return mBuffer;
}

} // namespace dString

// TODO: fix the weak function order. Comp_c's dtor should be an inline (weak)
// dtor in d_string.hpp, with a second virtual (stripped; vtable +0x0C is NULL)
// as the key function. That layout compiles right, but our link keeps the
// stripped function, so the dtor is defined out of line in this .inc instead.
#include <game/game/d_string_comp.inc>
