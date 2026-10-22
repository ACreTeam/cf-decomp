#ifndef REVOLUTION_NET_H
#define REVOLUTION_NET_H

#include <revolution/types.h>

#include <revolution/os.h>

#include <revolution/net/NETAES.h>
#include <revolution/net/NETDigest.h>
#include <revolution/net/NETMem.h>
#include <revolution/net/NETStartup.h>
#include <revolution/net/NETVersion.h>

#ifdef __cplusplus
extern "C" {
#endif

int NETGetWirelessMacAddress(void* data);

BOOL NETGetUniversalCalendar(OSCalendarTime* time);

#ifdef __cplusplus
}
#endif

#endif // REVOLUTION_NET_H
