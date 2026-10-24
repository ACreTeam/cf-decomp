#include <lib/textinput/tiHwKeyboard.h>
#include <lib/textinput/tiManager.h>
#include <lib/textinput/tiLayoutGather.h>

namespace textinput {
namespace keyboard {
namespace hwkey {

HWKeyboard::HWKeyboard(Manager *manager) : mpManager(manager) {
}

void HWKeyboard::init() {
    LayoutGather::Singleton::getInstance().setHWPressedShift(false);
    u32 state = input::HKBManager::sInstance.GetModifierState();
    input::HKBManager::sInstance.SetModifierState(state | 0x100, 0x700);
}

void HWKeyboard::updateShift(input::HKBManager &hkbManager) {
    LayoutGather &gather = LayoutGather::Singleton::getInstance();
    u32 state = hkbManager.GetModifierState();
    if (!mgr()->getToolBar()->isQwerty() &&
        !mgr()->getToolBar()->isHWKBMode()) {
        return;
    }
    bool previousShift = gather.isHoldingShift();
    if (state & 2) {
        gather.setHWPressedShift(true);
        if (!previousShift) {
            mgr()->getPCKeyboard()->onPressedShift(true);
        }
    } else {
        gather.setHWPressedShift(false);
        if (previousShift && !gather.isHoldingShift()) {
            mgr()->getPCKeyboard()->onReleasedShift();
        }
    }
}

bool HWKeyboard::updateInput(input::HKBManager &hkbManager) {
    if (!mgr()->getSignKeyboard()->isActive()) {
        u32 modifier = hkbManager.GetModifierState();
        input::HKBManager::KeySet keySet;
        for (keySet = hkbManager.GetTriggeredKeySet(); keySet.IsValid();
             keySet = keySet.GetNext()) {
            switch ((u8)keySet.GetKey()) {
            case 0x28:
            case 0x58:
                if (hkbManager.GetModifierState() & 4) {
                    mgr()->getToolBar()->onOK();
                    goto end;
                }
                break;
            case 0x29:
            case 0x2A:
                if (hkbManager.GetModifierState() & 4) {
                    mgr()->getToolBar()->onCancel();
                    goto end;
                }
                break;
            }
        }

        keySet = hkbManager.GetRepeatedKeySet();
        if (keySet.IsValid()) {
            if (!mgr()->getToolBar()->isQwerty() &&
                mgr()->getToolBar()->isHWKBMode()) {
                goto end;
            }
            for (; keySet.IsValid(); keySet = keySet.GetNext()) {
                HVKCode code;
                switch ((u8)keySet.GetKey()) {
                case 0x4C:
                    sendCommand(2, NULL);
                    break;
                case 0x2A:
                    sendCommand(1, NULL);
                    break;
                case 0x4F:
                    code = (HVKCode)0x20;
                    goto arrow;
                case 0x50:
                    code = (HVKCode)0x1F;
                    goto arrow;
                case 0x51:
                    code = (HVKCode)0x1E;
                    goto arrow;
                case 0x52:
                    code = (HVKCode)0x1D;
                arrow: {
                    inputform::Base::HKBInput input;
                    input.code = code;
                    input.flags = modifier;
                    sendCommand(0x26, &input);
                    break;
                }
                case 0x2C:
                    if (mgr()->getToolBar()->isQwerty() ||
                        !mgr()->getCellPhoneKeyboard()->isNumeric()) {
                        inputform::Base::HKBInput space = {(HVKCode)8, 0};
                        space.flags = modifier;
                        sendCommand(0x26, &space);
                    }
                    break;
                }
            }
        }

        keySet = hkbManager.GetTriggeredKeySet();
        if (keySet.IsValid()) {
            if (!mgr()->getToolBar()->isQwerty() &&
                mgr()->getToolBar()->isHWKBMode()) {
                goto end;
            }
            if (!mgr()->getPCKeyboard()->isABC()) {
                if (mgr()->getToolBar()->isQwerty() &&
                    mgr()->getEventObserver() != NULL) {
                    mgr()->getEventObserver()->onSE(
                        sound::SE_KETAI_MODE_SWITCHING);
                }
                mgr()->getPCKeyboard()->setABC(true);
            }
            for (; keySet.IsValid(); keySet = keySet.GetNext()) {
                wchar_t wc = keySet.GetWChar();
                u8 key = keySet.GetKey();
                if (wc != L' ') {
                    if (wc != L'\0') {
                        bool send = true;
                        if (!mgr()->getToolBar()->isQwerty() &&
                            mgr()->getCellPhoneKeyboard()->isNumeric()) {
                            switch (wc) {
                            case L'.':
                                if (mgr()
                                        ->getCellPhoneKeyboard()
                                        ->isNumericWithDot()) {
                                    break;
                                }
                            case L'/':
                            default:
                                send = false;
                                break;
                            case L'0':
                            case L'1':
                            case L'2':
                            case L'3':
                            case L'4':
                            case L'5':
                            case L'6':
                            case L'7':
                            case L'8':
                            case L'9':
                                break;
                            }
                        }
                        if (send) {
                            bool kana = false;
                            if (mgr()->getToolBar()->isQwerty()) {
                                kana = mgr()
                                           ->getPCKeyboard()
                                           ->getTranslateMode() !=
                                       pctype::Base::TM_Direct;
                            }
                            if (kana) {
                                switch (wc) {
                                case L',':
                                    wc = 0x3001;
                                    goto highlight;
                                case L'.':
                                    wc = 0x3002;
                                    goto highlight;
                                case L'-':
                                    wc = 0x30FC;
                                    goto highlight;
                                case L'[':
                                    wc = 0x300C;
                                    goto highlight;
                                case L']':
                                    wc = 0x300D;
                                highlight: {
                                    nw4rmanager::AnmPane *anmPane =
                                        mgr()->getPCKeyboard()->searchAnmPane(
                                            wc);
                                    if (anmPane != NULL) {
                                        anmPane->onAnmEvent(
                                            nw4rmanager::AnmPane::PE_0);
                                    }
                                    break;
                                }
                                }
                            }
                            modifier = 0;
                            if (hkbManager.GetModifierState() & 2) {
                                modifier |= 1;
                            }
                            if (hkbManager.GetModifierState() & 0x200) {
                                modifier |= 2;
                            }
                            if (!(modifier & 1)) {
                                pctype::Base::KeyState &state =
                                    mgr()->getPCKeyboard()->mKeyState;
                                state.setABCFlag(state.muFlag & ~0x8F);
                            }
                            inputform::Base::CharInput input = {L'\0', 0, true,
                                                                false};
                            input.ch = wc;
                            input.flags = modifier;
                            sendCommand(0, &input);
                        }
                    } else {
                        static const u8 csHidToControlCode[] = {
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x09, 0x01, 0x0A, 0x03, 0x08, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
                            0x00, 0x04, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
                            0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x19, 0x1A,
                            0x1B, 0x13, 0x15, 0x17, 0x14, 0x16, 0x18, 0x00,
                            0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00,
                            0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x10, 0x00, 0x0F, 0x0E, 0x00, 0x00, 0x00, 0x00,
                            0x11, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x06, 0x05, 0x07, 0x0B, 0x06, 0x05, 0x07, 0x0B,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        };
                        bool send = true;
                        HVKCode code;
                        if (key < 0x20) {
                            code = (HVKCode)0;
                        } else if (key < 0x98) {
                            code = (HVKCode)csHidToControlCode[key - 0x20];
                        } else if (key < 0xE0) {
                            code = (HVKCode)0;
                        } else if (key < 0xF0) {
                            code = (HVKCode)csHidToControlCode[key - 0x68];
                        } else {
                            code = (HVKCode)0;
                        }
                        if (code == 0x1C) {
                            bool caps =
                                !(hkbManager.GetModifierState() & 0x100);
                            if (!mgr()->getToolBar()->isQwerty() &&
                                mgr()->getCellPhoneKeyboard()->isNumeric()) {
                                caps = true;
                            }
                            hkbManager.SetModifierState(caps ? 0x100 : 0,
                                                        0x100);
                        }
                        if (!mgr()->getToolBar()->isQwerty() &&
                            mgr()->getCellPhoneKeyboard()->isNumeric()) {
                            switch (code) {
                            case 0x15:
                            case 0x16:
                            case 0x1D:
                            case 0x1E:
                            case 0x1F:
                            case 0x20:
                                break;
                            default:
                                send = false;
                                break;
                            }
                        }
                        if (send) {
                            inputform::Base::HKBInput input;
                            input.code = code;
                            input.flags = modifier;
                            sendCommand(0x26, &input);
                        }
                    }
                }
                switch ((u8)keySet.GetKey()) {
                case 0x39:
                    if (mgr()->getToolBar()->isQwerty() ||
                        mgr()->getToolBar()->isHWKBMode()) {
                        mgr()->getPCKeyboard()->onPressedCaps();
                    }
                    break;
                }
            }
        }
    }
end:
    updateShift(hkbManager);
    return false;
}

void HWKeyboard::setLanguage(Destination destination, Language language) {
    static const u8 csCountryEU[] = {9, 0x11, 0x10, 5, 6, 8, 0xE, 0};
    static const u8 csCountry[] = {9, 0x11, 0x10, 2, 6, 8, 0xA, 0};
    if (destination == DST_EU) {
        input::HKBManager::sInstance.SetCountry(csCountryEU[language]);
    } else {
        input::HKBManager::sInstance.SetCountry(csCountry[language]);
    }
}

} // namespace hwkey
} // namespace keyboard
} // namespace textinput
