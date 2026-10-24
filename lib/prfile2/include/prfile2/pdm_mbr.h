#ifndef PRFILE2_PDM_MBR_H
#define PRFILE2_PDM_MBR_H

#define PDM_MBR_SECTOR 0  // physical sector of the master boot record
#define PDM_MBR_PARTITION_COUNT 4
#define PDM_MBR_PARTITION_TBL 0x1BE
#define PDM_MBR_PARTITION_TBL_LEN 0x10
#define PDM_MBR_PART_ENTRY(i) (PDM_MBR_PARTITION_TBL + (i) * PDM_MBR_PARTITION_TBL_LEN)
#define PDM_EBR_ENTRY_COUNT 2  // logical partition, link to the next EBR
#define PDM_EBR_ENTRY_LOGICAL 0  // EBR entry of the logical partition
#define PDM_EBR_ENTRY_NEXT 1  // EBR entry linking to the next EBR

// partition table entry field offsets
enum {
    PDM_PART_TBL_BOOT_FLAG = 0x0,
    PDM_PART_TBL_S_HEAD = 0x1,
    PDM_PART_TBL_S_CYL_SECTOR = 0x2,
    PDM_PART_TBL_PARTITION_TYPE = 0x4,
    PDM_PART_TBL_E_HEAD = 0x5,
    PDM_PART_TBL_E_CYL_SECTOR = 0x6,
    PDM_PART_TBL_LBA_START_SECTOR = 0x8,
    PDM_PART_TBL_LBA_NUM_SECTORS = 0xC,
};

// partition_type
enum {
    PDM_PART_TYPE_FAT12 = 0x01,
    PDM_PART_TYPE_FAT16_SMALL = 0x04,  // FAT16 below 32 MB
    PDM_PART_TYPE_FAT16 = 0x06,
};

#endif
