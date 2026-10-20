#include <dwc/dwc_common.h>

#include <nitro/math/rand.h>
#include <revolution/net_fwd.h>
#include <revolution/os/OSTime.h>
#include <stdio.h>
#include <string.h>

static MATHRandContext32 stRandContext32 = { 0, 0, 0 };


int DWC_SetCommonKeyValueString(const char* key, const char* value,
    char* string, char separator)
{
    snprintf(string, 4096, "%c%s%c%s", separator, key, separator, value);
    return strlen(string);
}

int DWC_AddCommonKeyValueString(const char* key, const char* value,
    char* string, char separator)
{
    DWC_SetCommonKeyValueString(key, value, strchr(string, '\0'), separator);
    return strlen(string);
}

int DWC_GetCommonValueString(const char* key, char* value, const char* string,
    char separator)
{
    const char* cursor;
    const char* end;
    int length;

    if (value == NULL)
    {
        return -1;
    }

    cursor = strchr(string, separator);
    if (cursor == NULL)
    {
        return -1;
    }

    for (;;)
    {
        if (strncmp(cursor + 1, key, strlen(key)) == 0 && cursor[strlen(key) + 1] == separator)
        {
            break;
        }
        cursor = strchr(cursor + 1, separator);
        if (cursor == NULL)
        {
            return -1;
        }
        cursor = strchr(cursor + 1, separator);
        if (cursor == NULL)
        {
            return -1;
        }
    }

    cursor = strchr(cursor + 1, separator);
    if (cursor == NULL)
    {
        return -1;
    }

    end = strchr(cursor + 1, separator);
    if (end != NULL)
    {
        length = end - (cursor + 1);
    }
    else
    {
        length = strlen(cursor + 1);
    }

    strncpy(value, cursor + 1, length);
    value[length] = '\0';
    return length;
}

u32 DWCi_GetMathRand32(u32 max)
{
    u64 seed;
    OSTime time;

    if (!stRandContext32.x && !stRandContext32.mul && !stRandContext32.add)
    {
        NETGetWirelessMacAddress((u8*)&seed);
        time = OSGetTime();
        seed = ((seed >> 24) & 0xFFFFFF) | (time << 24);
        MATH_InitRand32(&stRandContext32, seed);
    }

    return MATH_Rand32(&stRandContext32, max);
}

u32 DWCi_WStrLen(const u16* string)
{
    u32 length = 0;

    while (string[length] != 0)
    {
        length++;
    }
    return length;
}
