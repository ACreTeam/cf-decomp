#include <MetroTRK/MWCriticalSection_gc.h>
#include <revolution/os/OSInterrupt.h>

void MWInitializeCriticalSection(unsigned int* section)
{
}

void MWEnterCriticalSection(unsigned int* section)
{
    *section = OSDisableInterrupts();
}

void MWExitCriticalSection(unsigned int* section)
{
    OSRestoreInterrupts(*section);
}
