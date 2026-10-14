// main() and the game thread, the process stop reasons (dMain). .text 800CDD2C..800CDEA0, .bss 805A20F8..805A2410,
// .sbss 8074E4D8..8074E4E0. See include/game/game/d_main.hpp.
#include <game/game/d_main.hpp>
#include <game/framework/f_manager.hpp>
#include <game/game/d_sys.hpp>
#include <lib/revolution/OS/OSThread.h>

namespace dMain {

static OSThread sThread;
u32 sStopReason;

// 800CDD2C
BOOL isStopped(u32 mask) {
    return (sStopReason & mask) != 0;
}

// 800CDD44
void addStop(u32 mask) {
    u32 reason = sStopReason;
    if (reason == 0) {
        fManager_c::m_StopProcInf |= fManager_c::PROC_FLAG_CONNECT | fManager_c::PROC_FLAG_CREATE |
                                     fManager_c::PROC_FLAG_EXECUTE | fManager_c::PROC_FLAG_DELETE;
    }
    sStopReason = reason | mask;
}

// 800CDD68
void removeStop(u32 mask) {
    sStopReason &= ~mask;
    if (sStopReason == 0) {
        fManager_c::m_StopProcInf &= fManager_c::PROC_FLAG_DRAW;
    }
}

// 800CDD88
void init() {
    dSys::init();
    dSys::setBlack(0);
}

// 800CDDB0
void loop() {
    while (true) {
        dSys::runFrame();
    }
}

// 800CDDC4
void *threadMain(void *arg) {
    init();
    loop();
    return NULL;
}

} // namespace dMain

// 800CDDEC
void main() {
    u8 stack[0xF000] ALIGN(32);
    OSThread *thread = OSGetCurrentThread();
    OSCreateThread(&dMain::sThread, dMain::threadMain, NULL, stack + sizeof(stack), sizeof(stack), 16, 0);
    OSResumeThread(&dMain::sThread);
    OSSetThreadPriority(thread, 31);
    OSSuspendThread(thread);
}
