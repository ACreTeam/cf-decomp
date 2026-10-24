#include <private/mp.h>
#include <private/wdp.h>

u32 WDPi_GetBeaconCheckSum(const u8 *data) {
    u32 sum = 0;
    int i;
    for (i = 0; i < 51; ++i) {
        sum += data[0] | (data[1] << 8);
        data += 2;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum = (sum >> 16) + (sum & 0xffff);
    return (~sum) & 0xffff;
}

int WDP_CompareMemory(const void *a, const void *b, u32 size) {
    const u8 *p = (const u8 *)a;
    const u8 *q = (const u8 *)b;
    int result = 0;
    u32 i;
    for (i = 0; !result && i < size; ++i) {
        result = p[i] - q[i];
    }
    return result;
}

u32 WDP_CopyString(u16 *dst, u32 size, const u16 *src) {
    u32 i = 0;
    u32 count;

    size >>= 1;

    for (; i < size; i++) {
        u16 value = *src++;
        if (value == 0) {
            break;
        }
        
        MI_SetLE16(dst, value);
        dst++;
    }
    
    count = i;
    for (; i < size; i++) {
        MI_SetLE16(dst, 0);
        dst++;
    }
    return count;
}

static inline BOOL IsRangeInside(u32 address, u32 size, u32 start, u32 end) {
    return address >= start && address + size <= end;
}
BOOL WDPi_IsValidSegmentRegion(int type, u32 address, u32 size, BOOL extended,
                               BOOL *relocate) {
    BOOL valid = FALSE;
    if (relocate) {
        *relocate = FALSE;
    }
    if (type < WDP_SEG_TYPE_NUM && !((address | size) & 0x80000000)) {
        switch (type) {
        case WDP_SEG_TYPE_0:
            valid = address >= 0x027ffe00 && address + size <= 0x027fff60;
            break;
        case WDP_SEG_TYPE_1:
            valid = address >= 0x02000000 && address + size <= 0x022c0000;
            break;
        case WDP_SEG_TYPE_2:
            if (IsRangeInside(address, size, 0x02000000, 0x02300000)) {
                valid = TRUE;
            } else if (extended) {
                if (IsRangeInside(address, size, 0x02300000, 0x023fe800) &&
                    size <= 0x40000) {
                    valid = TRUE;
                    if (relocate) {
                        *relocate = TRUE;
                    }
                } else if (IsRangeInside(address, size, 0x037f8000,
                                         0x0380f000)) {
                    valid = TRUE;
                    if (relocate) {
                        *relocate = TRUE;
                    }
                }
            }
            break;
        }
    }
    return valid;
}

u32 WDPi_InitSegmentMapper(WDPSegmentMapper *mapper, const WDPSegmentInfo *header,
                           u32 packetSize) {
    WDPSegType i;
    u32 count = 0;
    u32 blockSize = packetSize - 6;
    for (i = 0; i < WDP_SEG_TYPE_NUM; i++) {
        u32 addr = MI_GetLE32((void*)&header->data[i].laddr);
        u32 size = MI_GetLE32((void*)&header->data[i].size);
        if (!WDPi_IsValidSegmentRegion((int)(i), addr, size,
                                       (BOOL)(TRUE), (BOOL *)(NULL))) {
            count = 0;
            break;
        }
        mapper->firstBlock[(u32)i] = (u16)count;
        count += (size + blockSize - 1) / blockSize;
    }
    if (count >= 0x3000) {
        count = 0;
    }
    mapper->blockCount = (u16)count;
    mapper->blockSize = (u16)blockSize;
    MI_CpuCopy8((const void *)(header), &mapper->info, sizeof(mapper->info));
    return count;
}

BOOL WDPi_GetSegmentRegion(const WDPSegmentMapper *mapper, u32 block,
                           WDPSegmentRegion *region) {
    BOOL valid = FALSE;
    if (block < mapper->blockCount) {
        WDPSegType i =  WDP_SEG_TYPE_NUM;
        while (--i >= 0) {
            s32 index = block - mapper->firstBlock[i];
            if (index >= 0) {
                u32 size = MI_GetLE32((void*)&mapper->info.data[i].size);
                region->type = i;
                region->offset = index * mapper->blockSize;
                region->size = size - region->offset;
                if (region->size > mapper->blockSize) {
                    region->size = mapper->blockSize;
                }
                valid = TRUE;
                break;
            }
        }
    }
    return valid;
}

BOOL WDP_MergeBitset(u32 *bits, u32 count, u32 index) {
    BOOL changed = bits == (void *)0;
    if (!changed && index < count) {
        u32 mask = 1 << (index & 31);
        changed = (bits[index >> 5] & mask) == 0;
        if (changed) {
            bits[index >> 5] = bits[index >> 5] | mask;
        }
    }
    return changed;
}
