#ifndef GAME_D_PAD_HPP
#define GAME_D_PAD_HPP

#include <types.h>
#include <game/mLib/m_vec.hpp>
#include <game/mLib/m_pad.hpp>

// Game input, pointer coordinates and synthesized stick/button state.
// Names inferred from behavior.
namespace dPad {
enum InputMode {
    INPUT_STICK,
    INPUT_REMOTE,
    INPUT_NONE
};

enum Button {
    BUTTON_LEFT = 0x001,
    BUTTON_RIGHT = 0x002,
    BUTTON_DOWN = 0x004,
    BUTTON_UP = 0x008,
    BUTTON_CONFIRM = 0x010,
    BUTTON_CANCEL = 0x020,
    BUTTON_A = 0x040,
    BUTTON_B = 0x080,
    BUTTON_C = 0x100,
    BUTTON_SHAKE = 0x200,
    BUTTON_DIRECTION = BUTTON_LEFT | BUTTON_RIGHT | BUTTON_DOWN | BUTTON_UP
};

void disconnectOtherRemotes(const s32 *status); // 800FA08C
void init(); // 800FA0E0
void updateConnection(); // 800FA0F0
void begin(); // 800FA150
BOOL updateButtons(BOOL suppressA); // 800FA4C4
void end(); // 800FA5B4
BOOL isRemoteUnavailable(); // 800FA5D8
BOOL hasUnsupportedExtension(); // 800FA640
mVec2_c getRawPointer(); // 800FA6AC
mVec2_c getPointerPosition(); // 800FA6C4
mVec2_c getPointerDirection(); // 800FA6DC
int getInputMode(); // 800FA6F4
f32 getStickX(); // 800FA6FC
f32 getStickY(); // 800FA704
u16 getTriggered(); // 800FA70C
u16 getHeld(); // 800FA714
u16 getReleased(); // 800FA71C
BOOL isATriggered(); // 800FA724
BOOL isAHeld(); // 800FA730
BOOL isCTriggered(); // 800FA73C
BOOL isConfirmTriggered(); // 800FA748
BOOL isConfirmHeld(); // 800FA754
BOOL isCancelTriggered(); // 800FA760
BOOL isCancelHeld(); // 800FA76C
BOOL isShakeTriggered(); // 800FA778
BOOL isShakeHeld(); // 800FA784
BOOL isUpTriggered(); // 800FA790
BOOL isDownTriggered(); // 800FA7DC
BOOL isLeftTriggered(); // 800FA828
BOOL isRightTriggered(); // 800FA874
BOOL isUpHeld(); // 800FA8C0
BOOL isDownHeld(); // 800FA90C
BOOL isLeftHeld(); // 800FA958
BOOL isRightHeld(); // 800FA9A4

// 800FAA6C: emitted after the static initializer with -sym on.
inline mVec2_c readPointer() {
    EGG::Vector2f pos = mPad::getCore()->getDpdRawPos();
    return mVec2_c(pos);
}

// 800FAAB0
inline mVec2_c readHorizon() {
    EGG::Vector2f dir = mPad::getCore()->getCoreStatus()->getHorizon();
    return mVec2_c(dir);
}
} // namespace dPad

#endif
