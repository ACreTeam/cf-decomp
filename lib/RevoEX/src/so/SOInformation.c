#include <revolution/so.h>

#include <private/ios.h>
#include <private/ipc.h>
#include <revolution/net/NETMem.h>
#include <revolution/os.h>

#include <string.h>

long SOGetHostID(void)
{
    int result;
    s32 rmId;
    long hostId = 0;

    result = SOiPrepare(NULL, &rmId);
    if (result == SO_SUCCESS)
    {
        hostId = IOS_Ioctl(rmId, 16, NULL, 0, NULL, 0);
        SOiConclude(NULL, result);
    }
    return hostId;
}

SOHostEnt* SOGetHostByName(const char* name)
{
    int result;
    s32 rmId;
    int nameLength;
    int bufferSize;
    SOHostEnt* resultHost = NULL;
    u32* alias;
    s32 offset;

    result = SOiPrepare(NULL, &rmId);
    if (result == SO_SUCCESS)
    {
        if (name == NULL)
        {
            result = SO_EINVAL;
        }
        else
        {
            nameLength = strlen(name);
            bufferSize = OSRoundUp32B(nameLength + 1);
            {
                char* nameBuffer = SOiAlloc(12, bufferSize);
                SOHostEnt* host = (SOHostEnt*)SOiGetSysWork()->_unk10;

                if (nameBuffer == NULL)
                {
                    result = SO_ENOMEM;
                }
                else
                {
                    strcpy(nameBuffer, name);
                    result = IOS_Ioctl(rmId, 17, nameBuffer, nameLength + 1, host, 0x460);
                    if (result >= 0)
                    {
                        offset = (s32)((u8*)host + sizeof(SOHostEnt)) - (s32)host->name;
                        alias = (u32*)((u8*)host + 0x340);
                        while (*alias != 0)
                        {
                            *alias += offset;
                            alias++;
                        }
                        host->aliases = (char**)((u8*)host->aliases + offset);
                        resultHost = host;
                        host->name = (char*)((u8*)host->name + offset);
                        host->addrList = (u8**)((u8*)host->addrList + offset);
                    }
                    SOiFree(12, nameBuffer, bufferSize);
                }
            }
        }
        SOiConclude(NULL, result);
    }
    return resultHost;
}

int SOGetAddrInfo(const char* nodeName, const char* servName,
    const SOAddrInfo* hints, SOAddrInfo** res)
{
    int result;
    s32 rmId;
    int nodeNameLength;
    int servNameLength;
    int bufferSize;
    char* nodeNameBuffer;
    char* servNameBuffer;
    SOAddrInfo* hintsBuffer;
    SOAddrInfo* current;
    u8* address;

    result = SOiPrepare(NULL, &rmId);
    if (result == SO_SUCCESS)
    {
        SOAddrInfo* resultBuffer;
        IOSIoVector* vectors;

        servNameLength = servName == NULL ? 0 : strlen(nodeName) + 1;
        nodeNameLength = nodeName == NULL ? 0 : strlen(nodeName) + 1;
        bufferSize = OSRoundUp32B(OSRoundUp32B(nodeNameLength)
                                  + OSRoundUp32B(servNameLength) + 64);
        vectors = SOiAlloc(12, bufferSize);
        if (vectors == NULL)
        {
            result = SO_ENOMEM;
        }
        else
        {
            resultBuffer = SOiAlloc(10, 0x840);
            if (resultBuffer == NULL)
            {
                SOiFree(12, vectors, bufferSize);
                result = SO_ENOMEM;
            }
            else
            {
                nodeNameBuffer = (char*)((u8*)vectors + 32);
                servNameBuffer = nodeNameBuffer
                               + OSRoundUp32B(nodeName == NULL ? 0 : strlen(nodeName) + 1);
                hintsBuffer = (SOAddrInfo*)((u8*)servNameBuffer
                                            + OSRoundUp32B(servName == NULL ? 0 : strlen(nodeName) + 1));

                if (nodeName != NULL)
                {
                    strcpy(nodeNameBuffer, nodeName);
                }
                vectors[0].base = (u8*)(nodeName != NULL ? nodeNameBuffer : NULL);
                vectors[0].length = nodeName == NULL ? 0 : strlen(nodeName);

                if (servName != NULL)
                {
                    strcpy(servNameBuffer, servName);
                }
                vectors[1].base = (u8*)(servName != NULL ? servNameBuffer : NULL);
                vectors[1].length = servName == NULL ? 0 : strlen(servName);

                if (hints != NULL)
                {
                    NETMemCpy(hintsBuffer, hints, sizeof(SOAddrInfo));
                }
                else
                {
                    NETMemSet(hintsBuffer, 0, sizeof(SOAddrInfo));
                }
                if (hintsBuffer->family == 0)
                {
                    hintsBuffer->family = SO_PF_INET;
                }
                if (hintsBuffer->family == SO_PF_INET6)
                {
                    *res = NULL;
                    result = SO_EPROTONOSUPPORT;
                    SOiFree(10, resultBuffer, 0x840);
                }
                else
                {
                    vectors[2].base = (u8*)hintsBuffer;
                    vectors[2].length = sizeof(SOAddrInfo);
                    vectors[3].base = (u8*)resultBuffer;
                    vectors[3].length = 0x834;
                    result = IOS_Ioctlv(rmId, 24, 3, 1, vectors);
                    if (result >= 0)
                    {
                        *res = resultBuffer;
                        address = (u8*)resultBuffer + 0x460;
                        current = resultBuffer;
                        while (current != NULL)
                        {
                            current->addr = address;
                            if (current->next != NULL)
                            {
                                current->next = current + 1;
                            }
                            current = current->next;
                            address += 28;
                        }
                    }
                    else
                    {
                        *res = NULL;
                        SOiFree(10, resultBuffer, 0x840);
                    }
                }
                SOiFree(12, vectors, bufferSize);
            }
        }
        result = SOiConclude(NULL, result);
    }
    return result;
}

void SOFreeAddrInfo(SOAddrInfo* head)
{
    BOOL enabled = OSDisableInterrupts();

    if (SOiIsInitialized() == TRUE && head != NULL)
    {
        SOiFree(10, head, 0x840);
    }
    OSRestoreInterrupts(enabled);
}
