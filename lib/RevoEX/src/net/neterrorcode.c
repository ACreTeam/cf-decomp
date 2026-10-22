#include <revolution/os.h>

#include <revolution/ncd.h>
#include <revolution/net/NETStartup.h>

#include <private/net.h>

static inline s32 CountTrailingZeros(u32 val);

s32 NETiGetConnectionTypeFromConfigList(u32 wired, u32 wireless, u32 usbap)
{
    s32 result = 99;

    if (wired != 0)
    {
        if (wireless == 0 && usbap == 0)
        {
            result = CountTrailingZeros(wired) + 20;
        }
    }
    else if (wireless != 0)
    {
        if (usbap == 0)
        {
            result = CountTrailingZeros(wireless) + 30;
        }
    }
    else if (usbap != 0)
    {
        result = CountTrailingZeros(usbap) + 40;
    }

    return result;
}

s32 NETGetStartupErrorCode(s32 soErr)
{
    u32 list2;
    u32 list1;
    u32 list0;
    s32 connType = 99;

    if (NCDiGetEnabledConfigList(&list0, &list1, &list2) >= 0)
    {
        connType = NETiGetConnectionTypeFromConfigList(list0, list1, list2);
    }
    if (connType < 0)
    {
        soErr = 0x80000000;
        connType = 99;
    }

    return GetStartupErrorCode(soErr, connType) - connType;
}

s32 GetStartupErrorCode(s32 soErr, s32 connType)
{
    if (soErr >= 0)
    {
        return 0;
    }

    switch (soErr)
    {
    case -45:
        return -50200;
    case -28:
        return -50300;
    case -62:
        return -50400;
    case -111:
        return -52700;
    case -121:
        if (connType >= 20 && connType < 30)
        {
            return -51400;
        }
        return -51000;
    case -112:
    case -76:
    case -48:
    case -39:
        if (connType >= 20 && connType < 30)
        {
            return -51400;
        }
        return -51300;
    case -102:
    case -101:
    case -100:
        return -52000;
    case (s32)0x80000000:
        return -50100;
    default:
        OSReport("Unknown SOStartup Error: %d\n", soErr);
        return -50100;
    }
}

static inline s32 CountTrailingZeros(u32 val)
{
    s32 i;
    u32 b;

    for (i = 0, b = 1; i < 32; i++, b <<= 1)
    {
        if (val & b)
        {
            return i;
        }
    }

    return -1;
}
