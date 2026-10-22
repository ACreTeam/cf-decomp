#ifndef RVL_SDK_MIX_PRIVATE_H
#define RVL_SDK_MIX_PRIVATE_H

#include <revolution/mix.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MIXRmtChannel
{
    u32 mode;
    int main[4];
    int aux[4];
    u16 vMain0;
    u16 vMain0Target;
    u16 vAux0;
    u16 vAux0Target;
    u16 vMain1;
    u16 vMain1Target;
    u16 vAux1;
    u16 vAux1Target;
    u16 vMain2;
    u16 vMain2Target;
    u16 vAux2;
    u16 vAux2Target;
    u16 vMain3;
    u16 vMain3Target;
    u16 vAux3;
    u16 vAux3Target;
} MIXRmtChannel;

extern MIXRmtChannel __s_MIXRmtChannel[AX_VOICE_MAX];
extern MIXRmtChannel* __MIXRmtChannel;

void __MIXRmtResetChannel(u32 index);
void __MIXRmtUpdateSettings(u32 index, AXVPB* axvpb);

#ifdef __cplusplus
}
#endif

#endif // RVL_SDK_MIX_PRIVATE_H
