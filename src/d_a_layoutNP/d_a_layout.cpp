// A layout test actor (REL d_a_layoutNP). See include/game/game/d_a_layout.hpp.
#include <game/game/d_a_layout.hpp>
#include <game/framework/f_profile.hpp>
#include <game/game/d_font.hpp>

namespace dALayout {
// .text:0x5C
BASE_PROFILE(A_LAYOUT, dAcLayout_c);
} // namespace dALayout

// .text:0xC0
int dAcLayout_c::create() {
    if (!mResAcc.load("Layout/bal.arc")) {
        return NOT_READY;
    }
    mLyt.mpResAcc = &mResAcc;
    mLyt.build("balloon_00.brlyt", nullptr);
    mAnm.create("balloon_00_blnIn.brlan", &mResAcc);
    mLyt.bind(&mAnm);
    mLyt.setFont(&dFont::g_mainFont.mFont);
    return SUCCEEDED;
}

// .text:0x168
int dAcLayout_c::doDelete() {
    return mResAcc.remove() != 0;
}

// .text:0x198
int dAcLayout_c::execute() {
    mAnm.play();
    mLyt.calc();
    return SUCCEEDED;
}

// .text:0x1E0
int dAcLayout_c::draw() {
    mLyt.entry();
    return SUCCEEDED;
}

// .text:0x208
dAcLayout_c::~dAcLayout_c() {
}
