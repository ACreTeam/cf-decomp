#pragma once

#include <lib/textinput/tiTextInputBase.h>
#include <lib/textinput/tiHKBManager.h>

namespace textinput {

class Manager;

namespace keyboard {
namespace hwkey {

class HWKeyboard : public CommandSender {
  public:
    HWKeyboard(Manager *manager);

    virtual ~HWKeyboard() {
    }
    virtual void init();
    virtual bool updateInput(input::HKBManager &hkbManager); // 0x20

    void updateShift(input::HKBManager &hkbManager);
    void setLanguage(Destination destination, Language language);

    Manager *mgr() {
        return mpManager;
    }

    /* 0x10 */ Manager *mpManager;
}; // size 0x14

} // namespace hwkey
} // namespace keyboard
} // namespace textinput
