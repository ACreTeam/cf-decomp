#include <dwc/dwc_memfunc.h>
#include <dwc/dwc_nonport.h>
#include <dwc/dwc_report.h>

#include <revolution/nwc24/NWC24Config.h>
#include <revolution/vf.h>

#include <string.h>

static u64 dwci_np_consoleid;
static int dwci_np_try_get_consoleid;
static int s_initialized;

void DWCi_Np_SetInitFlag(int initialzied)
{
    s_initialized = initialzied;
}

void DWCi_Np_CPUCopyFast(void* dst, const void* src, u32 n)
{
    int count;
    u8* pSrc;
    u8* pDst;
    u16* pwSrc;
    u16* pwDst;
    u32* pdwSrc;
    u32* pdwDst;
    u64* pqwSrc;
    u64* pqwDst;

    pDst = (u8*)dst;
    pSrc = (u8*)src;

    if (n > 32)
    {
        count = ((u32)dst & 0xF) ^ ((u32)src & 0xF);
        if ((count & 1) != 0)
        {
            goto copy_mem;
        }
        if ((count & 2) != 0)
        {
            count = 2 - ((u32)dst & 1);
            if (count != 0)
            {
                *pDst++ = *pSrc++;
                src = pSrc;
                dst = pDst;
                n--;
            }
            goto copy_half;
        }
        if ((count & 4) != 0)
        {
            count = 4 - ((u32)dst & 3);
            switch (count)
            {
            case 4:
                break;
            case 3:
                *pDst++ = *pSrc++;
            case 2:
                *pDst++ = *pSrc++;
            case 1:
                *pDst++ = *pSrc++;
            }
            src = pSrc;
            dst = pDst;
            n -= count;
            goto copy_word;
        }
        count = 8 - ((u32)dst & 7);
        switch (count)
        {
        case 8:
            break;
        case 7:
            *pDst++ = *pSrc++;
        case 6:
            *pDst++ = *pSrc++;
        case 5:
            *pDst++ = *pSrc++;
        case 4:
            *pDst++ = *pSrc++;
        case 3:
            *pDst++ = *pSrc++;
        case 2:
            *pDst++ = *pSrc++;
        case 1:
            *pDst++ = *pSrc++;
        }
        src = pSrc;
        dst = pDst;
        n -= count;
        goto copy_qword;
    }
    else
    {
        if (((u32)dst & 7) == 0 && ((u32)src & 7) == 0)
        {
            goto copy_qword;
        }
        if (((u32)dst & 3) == 0 && ((u32)src & 3) == 0)
        {
            goto copy_word;
        }
        if (((u32)dst & 1) == 0 && ((u32)src & 1) == 0)
        {
            goto copy_half;
        }
    }

copy_mem:
    memcpy(dst, src, n);
    return;

copy_half:
    pwSrc = (u16*)src;
    pwDst = (u16*)dst;
    count = n >> 1;
    if (count != 0)
    {
        count >>= 4;
        for (; count > 0; count--)
        {
            *pwDst++ = *pwSrc++;
            *pwDst++ = *pwSrc++;
            *pwDst++ = *pwSrc++;
            *pwDst++ = *pwSrc++;
            *pwDst++ = *pwSrc++;
            *pwDst++ = *pwSrc++;
            *pwDst++ = *pwSrc++;
            *pwDst++ = *pwSrc++;
            *pwDst++ = *pwSrc++;
            *pwDst++ = *pwSrc++;
            *pwDst++ = *pwSrc++;
            *pwDst++ = *pwSrc++;
            *pwDst++ = *pwSrc++;
            *pwDst++ = *pwSrc++;
            *pwDst++ = *pwSrc++;
            *pwDst++ = *pwSrc++;
        }
        switch ((n >> 1) & 15)
        {
        case 15:
            *pwDst++ = *pwSrc++;
        case 14:
            *pwDst++ = *pwSrc++;
        case 13:
            *pwDst++ = *pwSrc++;
        case 12:
            *pwDst++ = *pwSrc++;
        case 11:
            *pwDst++ = *pwSrc++;
        case 10:
            *pwDst++ = *pwSrc++;
        case 9:
            *pwDst++ = *pwSrc++;
        case 8:
            *pwDst++ = *pwSrc++;
        case 7:
            *pwDst++ = *pwSrc++;
        case 6:
            *pwDst++ = *pwSrc++;
        case 5:
            *pwDst++ = *pwSrc++;
        case 4:
            *pwDst++ = *pwSrc++;
        case 3:
            *pwDst++ = *pwSrc++;
        case 2:
            *pwDst++ = *pwSrc++;
        case 1:
            *pwDst++ = *pwSrc++;
        }
    }
    pSrc = (u8*)pwSrc;
    pDst = (u8*)pwDst;
    switch (n & 1)
    {
    case 1:
        *pDst++ = *pSrc++;
    }
    return;

copy_word:
    pdwSrc = (u32*)src;
    pdwDst = (u32*)dst;
    count = n >> 2;
    if (count != 0)
    {
        count >>= 3;
        for (; count > 0; count--)
        {
            *pdwDst++ = *pdwSrc++;
            *pdwDst++ = *pdwSrc++;
            *pdwDst++ = *pdwSrc++;
            *pdwDst++ = *pdwSrc++;
            *pdwDst++ = *pdwSrc++;
            *pdwDst++ = *pdwSrc++;
            *pdwDst++ = *pdwSrc++;
            *pdwDst++ = *pdwSrc++;
        }
        switch ((n >> 2) & 7)
        {
        case 7:
            *pdwDst++ = *pdwSrc++;
        case 6:
            *pdwDst++ = *pdwSrc++;
        case 5:
            *pdwDst++ = *pdwSrc++;
        case 4:
            *pdwDst++ = *pdwSrc++;
        case 3:
            *pdwDst++ = *pdwSrc++;
        case 2:
            *pdwDst++ = *pdwSrc++;
        case 1:
            *pdwDst++ = *pdwSrc++;
        }
    }
    pSrc = (u8*)pdwSrc;
    pDst = (u8*)pdwDst;
    switch (n & 3)
    {
    case 3:
        *pDst++ = *pSrc++;
    case 2:
        *pDst++ = *pSrc++;
    case 1:
        *pDst++ = *pSrc++;
    }
    return;

copy_qword:
    pqwSrc = (u64*)src;
    pqwDst = (u64*)dst;
    count = n >> 3;
    if (count != 0)
    {
        count >>= 3;
        for (; count > 0; count--)
        {
            *pqwDst++ = *pqwSrc++;
            *pqwDst++ = *pqwSrc++;
            *pqwDst++ = *pqwSrc++;
            *pqwDst++ = *pqwSrc++;
            *pqwDst++ = *pqwSrc++;
            *pqwDst++ = *pqwSrc++;
            *pqwDst++ = *pqwSrc++;
            *pqwDst++ = *pqwSrc++;
        }
        switch ((n >> 3) & 7)
        {
        case 7:
            *pqwDst++ = *pqwSrc++;
        case 6:
            *pqwDst++ = *pqwSrc++;
        case 5:
            *pqwDst++ = *pqwSrc++;
        case 4:
            *pqwDst++ = *pqwSrc++;
        case 3:
            *pqwDst++ = *pqwSrc++;
        case 2:
            *pqwDst++ = *pqwSrc++;
        case 1:
            *pqwDst++ = *pqwSrc++;
        }
    }
    pSrc = (u8*)pqwSrc;
    pDst = (u8*)pqwDst;
    switch (n & 7)
    {
    case 7:
        *pDst++ = *pSrc++;
    case 6:
        *pDst++ = *pSrc++;
    case 5:
        *pDst++ = *pSrc++;
    case 4:
        *pDst++ = *pSrc++;
    case 3:
        *pDst++ = *pSrc++;
    case 2:
        *pDst++ = *pSrc++;
    case 1:
        *pDst++ = *pSrc++;
    }
}

u64 DWCi_Np_GetConsoleId(void)
{
    if (!dwci_np_try_get_consoleid)
    {
        BOOL vfAvailable = VFIsAvailable();
        // 0x4000 bytes, allocated and freed but never used (older DWC: the NWC24 work area)
        void* work = DWC_AllocEx(DWC_ALLOCTYPE_UTIL, 0x4000, 32);
        // a second 0x8000-byte area is the VF work area
        void* vfWork = DWC_AllocEx(DWC_ALLOCTYPE_UTIL, 0x8000, 32);

        dwci_np_try_get_consoleid = TRUE;
        if (vfAvailable != TRUE)
        {
            // VFInitEx with that area (older DWC: VFInit())
            VFInitEx(vfWork, 0x8000);
        }

        // no NWC24OpenLib/NWC24CloseLib around it (older DWC opens the library first)
        if (NWC24GetMyUserId(&dwci_np_consoleid) == 0)
        {
            DWC_Printf(0x8000000, " get console friend code = %016lld\n", dwci_np_consoleid);
        }
        else
        {
            DWC_Printf(0x8000000, " failed to get console friend code.\n");
            dwci_np_consoleid = 0;
        }

        if (vfAvailable != TRUE)
        {
            VFFinalize();
        }
        // size 0 in both frees (older DWC: NWC24_WORK_PUBLIC_SIZE)
        DWC_Free(DWC_ALLOCTYPE_UTIL, work, 0);
        DWC_Free(DWC_ALLOCTYPE_UTIL, vfWork, 0);
    }

    return dwci_np_consoleid;
}

char* DWCi_Np_GetConsoleName(void)
{
    return "Wii";
}

char* DWCi_Np_GetAppGameName(void)
{
    return (char*)OSGetAppGamename();
}
