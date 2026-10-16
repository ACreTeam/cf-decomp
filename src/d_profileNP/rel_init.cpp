#include <game/game/d_profile.hpp>
#include <runtime/rel.h>

extern "C" {

__declspec(section ".ctors") extern voidFctPtr _ctors[];
__declspec(section ".dtors") extern voidFctPtr _dtors[];

// .text:0x0
void _prolog() {
    ModuleConstructorsX(_ctors);
    dProfile::install();
}

// .text:0x2C
void _epilog() {
    dProfile::remove();
    ModuleDestructorsX(_dtors);
}

// .text:0x58
void _unresolved() {
    ModuleUnresolved();
}

} // extern "C"
