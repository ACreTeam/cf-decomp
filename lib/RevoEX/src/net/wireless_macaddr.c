#include <revolution/ncd.h>
#include <revolution/net.h>

int NETGetWirelessMacAddress(void* data)
{
    return NCDiGetWirelessMacAddress(data);
}
