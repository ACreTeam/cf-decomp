#ifndef _REVOLUTION_MIX_H_
#define _REVOLUTION_MIX_H_

#include <revolution/types.h>
#include <revolution/ax.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define MIX_SOUND_MODE_MONO     0
#define MIX_SOUND_MODE_STEREO   1
#define MIX_SOUND_MODE_SURROUND 2
#define MIX_SOUND_MODE_DPL2     3

    typedef struct MIXChannel
    {
        /* 0x00 */ AXVPB* axvpb;
        /* 0x04 */ u32 mode;
        /* 0x08 */ int input;
        /* 0x0C */ int auxA;
        /* 0x10 */ int auxB;
        /* 0x14 */ int auxC;
        /* 0x18 */ int pan;
        /* 0x1C */ int span;
        /* 0x20 */ int fader;
        /* 0x24 */ int l;
        /* 0x28 */ int r;
        /* 0x2C */ int f;
        /* 0x30 */ int b;
        /* 0x34 */ int l1;
        /* 0x38 */ int r1;
        /* 0x3C */ u16 v;
        /* 0x3E */ u16 v1;
        /* 0x40 */ u16 vL;
        /* 0x42 */ u16 vL1;
        /* 0x44 */ u16 vR;
        /* 0x46 */ u16 vR1;
        /* 0x48 */ u16 vS;
        /* 0x4A */ u16 vS1;
        /* 0x4C */ u16 vAL;
        /* 0x4E */ u16 vAL1;
        /* 0x50 */ u16 vAR;
        /* 0x52 */ u16 vAR1;
        /* 0x54 */ u16 vAS;
        /* 0x56 */ u16 vAS1;
        /* 0x58 */ u16 vBL;
        /* 0x5A */ u16 vBL1;
        /* 0x5C */ u16 vBR;
        /* 0x5E */ u16 vBR1;
        /* 0x60 */ u16 vBS;
        /* 0x62 */ u16 vBS1;
        /* 0x64 */ u16 vCL;
        /* 0x66 */ u16 vCL1;
        /* 0x68 */ u16 vCR;
        /* 0x6A */ u16 vCR1;
        /* 0x6C */ u16 vCS;
        /* 0x6E */ u16 vCS1;
    } MIXChannel;

    void MIXInit(void);
    void MIXSetSoundMode(u32 mode);
    void MIXInitChannel(AXVPB* axvpb, u32 mode, int input, int auxA, int auxB, int auxC,
        int pan, int span, int fader);
    void MIXReleaseChannel(AXVPB* axvpb);
    void MIXSetInput(AXVPB* p, int dB);
    void MIXSetAuxA(AXVPB* p, int dB);
    void MIXSetAuxB(AXVPB* p, int dB);
    void MIXSetPan(AXVPB* p, int pan);
    void MIXSetSPan(AXVPB* p, int span);
    void MIXUpdateSettings(void);

    void MIXRmtSetVolumes(AXVPB* p, u32 mode, int main0, int main1, int main2, int main3,
        int aux0, int aux1, int aux2, int aux3);
    void MIXRmtSetFader(AXVPB* p, int channelIndex, int fader);

#ifdef __cplusplus
}
#endif

#endif
