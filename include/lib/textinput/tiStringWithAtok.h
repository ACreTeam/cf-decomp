#pragma once

// textinput::string::WithAtok: the string with ATOK (Japanese) prediction (see tiTextInputBase.h).

#include <lib/textinput/tiString.h>

namespace textinput {
namespace string {

class WithAtok : public Decolated {
public:
    virtual void vt_0xCC();
    virtual void vt_0xD0();
    virtual void vt_0xD4();
    virtual void vt_0xD8();
    virtual void vt_0xDC();
    virtual void vt_0xE0();
    virtual void vt_0xE4();
    virtual void vt_0xE8();
    virtual void vt_0xEC();
    virtual void vt_0xF0();
    virtual void vt_0xF4();
    virtual void vt_0xF8();
    virtual void vt_0xFC();
    virtual void vt_0x100();
    virtual void vt_0x104();
    virtual void vt_0x108();
    virtual void vt_0x10C();
    virtual void vt_0x110();
    virtual void vt_0x114();
    virtual void vt_0x118();
    virtual void vt_0x11C();
    virtual void vt_0x120();
    virtual void vt_0x124();
    virtual void vt_0x128();
    virtual void setFixedPredictionJP(int num, const char **predicts) {}   // 0x12C
    virtual void setDefaultPredictionJP(int num, const char **predicts) {} // 0x130
};

} // namespace string
} // namespace textinput
