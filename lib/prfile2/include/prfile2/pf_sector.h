#ifndef PRFILE2_PF_SECTOR_H
#define PRFILE2_PF_SECTOR_H

#include <prfile2/pf_types.h>

#ifdef __cplusplus
extern "C" {
#endif

s32 PFSEC_ReadFAT(struct PF_VOLUME* p_vol, u8* p_buf, u32 sector, u16 offset, u16 size);
s32 PFSEC_ReadData(struct PF_VOLUME* p_vol, u8* p_buf, u32 sector, u16 offset, u32 size, u32* p_success_size, u32 set_sig);
s32 PFSEC_WriteFAT(struct PF_VOLUME* p_vol, const u8* p_buf, u32 sector, u16 offset, u16 size);
s32 PFSEC_WriteData(struct PF_VOLUME* p_vol, const u8* p_buf, u32 sector, u16 offset, u32 size, u32* p_success_size, u32 set_sig);

#ifdef __cplusplus
}
#endif
#endif
