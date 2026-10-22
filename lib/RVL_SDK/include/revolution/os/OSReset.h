#ifndef REVOLUTION_OS_RESET_H
#define REVOLUTION_OS_RESET_H

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef BOOL (*OSShutdownFunction)(BOOL, u32);

typedef struct OSShutdownFunctionInfo OSShutdownFunctionInfo;
struct OSShutdownFunctionInfo {
    OSShutdownFunction func;  // 0x00

    u32 priority;  // 0x04

    OSShutdownFunctionInfo* next;  // 0x08
    OSShutdownFunctionInfo* prev;  // 0x0C
};

typedef struct OSShutdownFunctionQueue {
    OSShutdownFunctionInfo* head;  // 0x00
    OSShutdownFunctionInfo* tail;  // 0x04
} OSShutdownFunctionQueue;

void OSRegisterShutdownFunction(OSShutdownFunctionInfo* info);
void OSUnregisterShutdownFunction(OSShutdownFunctionInfo* info);

u32 OSGetResetCode();
#define OSIsRestart() ((OSGetResetCode() & 0x80000000) ? TRUE : FALSE)

void OSRebootSystem();
void OSReturnToMenu();
void OSShutdownSystem();

void OSRestart(u32 resetCode);
void OSReturnToDataManager();
void OSReturnToSetting(u8 setting);
void OSResetSystem(int, u32, int);

#ifdef __cplusplus
}
#endif

#endif  // REVOLUTION_OS_RESET_H
