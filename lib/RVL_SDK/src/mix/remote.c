#include <revolution/mix/MIXPrivate.h>

MIXRmtChannel __s_MIXRmtChannel[AX_VOICE_MAX];
MIXRmtChannel* __MIXRmtChannel;

#if 0 // not linked in City Folk
void MIXRmtSetVolumes(AXVPB* p, u32 mode, int main0, int main1, int main2, int main3,
    int aux0, int aux1, int aux2, int aux3)
{
    MIXRmtChannel* channel = &__MIXRmtChannel[p->index];
    u32 value = channel->mode | (mode & 0xF);

    channel->main[0] = main0;
    channel->main[1] = main1;
    channel->main[2] = main2;
    channel->main[3] = main3;
    channel->aux[0] = aux0;
    channel->aux[1] = aux1;
    channel->aux[2] = aux2;
    channel->aux[3] = aux3;
    channel->mode = value | 0x40000000;
}
#endif

#if 0 // not linked in City Folk
void MIXRmtSetFader(AXVPB* p, int channelIndex, int fader)
{
    MIXRmtChannel* channel = &__MIXRmtChannel[p->index];

    switch (channelIndex)
    {
    case 0:
        channel->main[0] = fader;
        break;
    case 1:
        channel->main[1] = fader;
        break;
    case 2:
        channel->main[2] = fader;
        break;
    case 3:
        channel->main[3] = fader;
        break;
    }

    channel->mode |= 0x40000000;
}
#endif

#if 0 // not linked in City Folk
void __MIXRmtUpdateSettings(u32 index, AXVPB* axvpb)
{
    MIXRmtChannel* channel = &__MIXRmtChannel[index];
    u16 mixerCtrl;
    u16* p;

    if (channel->mode & 0xC0000000)
    {
        if (channel->mode & 0x80000000)
        {
            channel->vMain0 = channel->vMain0Target;
            channel->vAux0 = channel->vAux0Target;
            channel->vMain1 = channel->vMain1Target;
            channel->vAux1 = channel->vAux1Target;
            channel->vMain2 = channel->vMain2Target;
            channel->vAux2 = channel->vAux2Target;
            channel->vMain3 = channel->vMain3Target;
            channel->vAux3 = channel->vAux3Target;
            channel->mode &= ~0x80000000;
        }

        if (channel->mode & 0x40000000)
        {
            channel->vMain0Target = __MIXGetVolume(channel->main[0]);
            channel->vMain1Target = __MIXGetVolume(channel->main[1]);
            channel->vMain2Target = __MIXGetVolume(channel->main[2]);
            channel->vMain3Target = __MIXGetVolume(channel->main[3]);

            if (channel->mode & 1)
            {
                channel->vAux0Target = __MIXGetVolume(channel->aux[0]);
            }
            else
            {
                channel->vAux0Target = __MIXGetVolume(channel->main[0] + channel->aux[0]);
            }

            if (channel->mode & 2)
            {
                channel->vAux1Target = __MIXGetVolume(channel->aux[1]);
            }
            else
            {
                channel->vAux1Target = __MIXGetVolume(channel->main[1] + channel->aux[1]);
            }

            if (channel->mode & 4)
            {
                channel->vAux2Target = __MIXGetVolume(channel->aux[2]);
            }
            else
            {
                channel->vAux2Target = __MIXGetVolume(channel->main[2] + channel->aux[2]);
            }

            if (channel->mode & 8)
            {
                channel->vAux3Target = __MIXGetVolume(channel->aux[3]);
            }
            else
            {
                channel->vAux3Target = __MIXGetVolume(channel->main[3] + channel->aux[3]);
            }

            channel->mode &= ~0x40000000;
            channel->mode |= 0x80000000;
        }

        mixerCtrl = 0;
        p = (u16*)&axvpb->pb.rmtMix;

        if ((*p++ = channel->vMain0))
            mixerCtrl |= AX_MIXER_CTRL_RMT_M0;
        if ((*p++ = (channel->vMain0Target - channel->vMain0) / 18))
            mixerCtrl |= AX_MIXER_CTRL_RMT_DELTA_M0;
        if ((*p++ = channel->vAux0))
            mixerCtrl |= AX_MIXER_CTRL_RMT_A0;
        if ((*p++ = (channel->vAux0Target - channel->vAux0) / 18))
            mixerCtrl |= AX_MIXER_CTRL_RMT_DELTA_A0;
        if ((*p++ = channel->vMain1))
            mixerCtrl |= AX_MIXER_CTRL_RMT_M1;
        if ((*p++ = (channel->vMain1Target - channel->vMain1) / 18))
            mixerCtrl |= AX_MIXER_CTRL_RMT_DELTA_M1;
        if ((*p++ = channel->vAux1))
            mixerCtrl |= AX_MIXER_CTRL_RMT_A1;
        if ((*p++ = (channel->vAux1Target - channel->vAux1) / 18))
            mixerCtrl |= AX_MIXER_CTRL_RMT_DELTA_A1;
        if ((*p++ = channel->vMain2))
            mixerCtrl |= AX_MIXER_CTRL_RMT_M2;
        if ((*p++ = (channel->vMain2Target - channel->vMain2) / 18))
            mixerCtrl |= AX_MIXER_CTRL_RMT_DELTA_M2;
        if ((*p++ = channel->vAux2))
            mixerCtrl |= AX_MIXER_CTRL_RMT_A2;
        if ((*p++ = (channel->vAux2Target - channel->vAux2) / 18))
            mixerCtrl |= AX_MIXER_CTRL_RMT_DELTA_A2;
        if ((*p++ = channel->vMain3))
            mixerCtrl |= AX_MIXER_CTRL_RMT_M3;
        if ((*p++ = (channel->vMain3Target - channel->vMain3) / 18))
            mixerCtrl |= AX_MIXER_CTRL_RMT_DELTA_M3;
        if ((*p++ = channel->vAux3))
            mixerCtrl |= AX_MIXER_CTRL_RMT_A3;
        if ((*p++ = (channel->vAux3Target - channel->vAux3) / 18))
            mixerCtrl |= AX_MIXER_CTRL_RMT_DELTA_A3;

        axvpb->pb.rmtMixerCtrl = mixerCtrl;
        axvpb->sync |= AX_PBSYNC_RMT_MIXER_CTRL | AX_PBSYNC_RMTMIX;
    }
}
#endif

void __MIXRmtResetChannel(u32 index)
{
    MIXRmtChannel* channel = &__MIXRmtChannel[index];

    channel->mode = 0;
    channel->main[0] = 0;
    channel->main[1] = 0;
    channel->main[2] = 0;
    channel->main[3] = 0;
    channel->aux[0] = -0x3C0;
    channel->aux[1] = -0x3C0;
    channel->aux[2] = -0x3C0;
    channel->aux[3] = -0x3C0;
    channel->vMain0 = channel->vAux0 = channel->vMain1 = channel->vAux1 = channel->vMain2 = channel->vAux2 = channel->vMain3 = channel->vAux3 = 0;
}
