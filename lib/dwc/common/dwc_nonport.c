#include <dwc/dwc_memfunc.h>
#include <dwc/dwc_nonport.h>
#include <dwc/dwc_report.h>

#include <revolution/nwc24/NWC24Config.h>
#include <revolution/vf.h>

#include <string.h>

u64 dwci_np_consoleid;
u32 dwci_np_try_get_consoleid;

// Copy with the largest unit both pointers allow: words, halfwords, else memcpy
void DWCi_Np_CPUCopyFast(void* dst, const void* src, u32 size)
{
    u32* dstw;
    const u32* srcw;
    u16* dsth;
    const u16* srch;
    u8* dstb;
    const u8* srcb;
    int count;
    int blocks;

    if (size & 0x80000000)
    {
        // no return here; the copy below runs as well
        memcpy(dst, src, size);
    }

    if (((u32)dst & 3) != 0 || ((u32)src & 3) != 0)
    {
        if (((u32)dst & 1) != 0 || ((u32)src & 1) != 0)
        {
            memcpy(dst, src, size);
        }
        else
        {
            dsth = (u16*)dst;
            srch = (const u16*)src;
            count = size >> 1;
            if (count != 0)
            {
                for (blocks = count >> 4; blocks > 0; blocks--)
                {
                    dsth[0] = srch[0];
                    dsth[1] = srch[1];
                    dsth[2] = srch[2];
                    dsth[3] = srch[3];
                    dsth[4] = srch[4];
                    dsth[5] = srch[5];
                    dsth[6] = srch[6];
                    dsth[7] = srch[7];
                    dsth[8] = srch[8];
                    dsth[9] = srch[9];
                    dsth[10] = srch[10];
                    dsth[11] = srch[11];
                    dsth[12] = srch[12];
                    dsth[13] = srch[13];
                    dsth[14] = srch[14];
                    dsth[15] = srch[15];
                    dsth += 16;
                    srch += 16;
                }
                switch (count & 15)
                {
                case 15:
                    *dsth++ = *srch++;
                case 14:
                    *dsth++ = *srch++;
                case 13:
                    *dsth++ = *srch++;
                case 12:
                    *dsth++ = *srch++;
                case 11:
                    *dsth++ = *srch++;
                case 10:
                    *dsth++ = *srch++;
                case 9:
                    *dsth++ = *srch++;
                case 8:
                    *dsth++ = *srch++;
                case 7:
                    *dsth++ = *srch++;
                case 6:
                    *dsth++ = *srch++;
                case 5:
                    *dsth++ = *srch++;
                case 4:
                    *dsth++ = *srch++;
                case 3:
                    *dsth++ = *srch++;
                case 2:
                    *dsth++ = *srch++;
                case 1:
                    *dsth++ = *srch++;
                }
            }
            switch (size & 1)
            {
            case 1:
                *(u8*)dsth = *(const u8*)srch;
            }
        }
    }
    else
    {
        dstw = (u32*)dst;
        srcw = (const u32*)src;
        count = size >> 2;
        if (count != 0)
        {
            for (blocks = count >> 3; blocks > 0; blocks--)
            {
                dstw[0] = srcw[0];
                dstw[1] = srcw[1];
                dstw[2] = srcw[2];
                dstw[3] = srcw[3];
                dstw[4] = srcw[4];
                dstw[5] = srcw[5];
                dstw[6] = srcw[6];
                dstw[7] = srcw[7];
                dstw += 8;
                srcw += 8;
            }
            switch (count & 7)
            {
            case 7:
                *dstw++ = *srcw++;
            case 6:
                *dstw++ = *srcw++;
            case 5:
                *dstw++ = *srcw++;
            case 4:
                *dstw++ = *srcw++;
            case 3:
                *dstw++ = *srcw++;
            case 2:
                *dstw++ = *srcw++;
            case 1:
                *dstw++ = *srcw++;
            }
        }
        dstb = (u8*)dstw;
        srcb = (const u8*)srcw;
        switch (size & 3)
        {
        case 3:
            *dstb++ = *srcb++;
        case 2:
            *dstb++ = *srcb++;
        case 1:
            *dstb = *srcb;
        }
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
