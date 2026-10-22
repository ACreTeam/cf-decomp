#ifndef RVL_SDK_VF_ALL_H
#define RVL_SDK_VF_ALL_H
#ifdef __cplusplus
extern "C" {
#endif

#define SWAP16(x) (((x) >> 8) & 0xFF) | (((x) << 8) & 0xFF00)
#define SWAP32(x) ((x) >> 24) | (((x) >> 8) & 0xFF00) | (((x) << 8) & 0xFF0000) | ((x) << 24)

#include <revolution/vf/vf_struct.h>

#include <revolution/vf/pf_clib.h>
#include <revolution/vf/pf_code.h>
#include <revolution/vf/pf_service.h>
#include <revolution/vf/pf_str.h>
#include <revolution/vf/pf_w_clib.h>
#include <revolution/vf/pf_driver.h>
#include <revolution/vf/pdm_bpb.h>
#include <revolution/vf/pdm_disk.h>
#include <revolution/vf/pdm_partition.h>
#include <revolution/vf/pdm_mbr.h>
#include <revolution/vf/pdm_dskmng.h>
#include <revolution/vf/pf_cache.h>
#include <revolution/vf/pf_cluster.h>
#include <revolution/vf/pf_dir.h>
#include <revolution/vf/pf_entry.h>
#include <revolution/vf/pf_entry_iterator.h>
#include <revolution/vf/pf_fat.h>
#include <revolution/vf/pf_fat12.h>
#include <revolution/vf/pf_fat16.h>
#include <revolution/vf/pf_fat32.h>
#include <revolution/vf/pf_fatfs.h>
#include <revolution/vf/pf_file.h>
#include <revolution/vf/pf_path.h>
#include <revolution/vf/pf_sector.h>
#include <revolution/vf/pf_volume.h>
#include <revolution/vf/pf_cp932.h>
#include <revolution/vf/pf_api_util.h>
#include <revolution/vf/pf_attach.h>
#include <revolution/vf/pf_detach.h>
#include <revolution/vf/pf_errnum.h>
#include <revolution/vf/pf_fclose.h>
#include <revolution/vf/pf_finfo.h>
#include <revolution/vf/pf_fopen.h>
#include <revolution/vf/pf_fread.h>
#include <revolution/vf/pf_fseek.h>
#include <revolution/vf/pf_fwrite.h>
#include <revolution/vf/pf_getdev.h>
#include <revolution/vf/pf_init_prfile2.h>
#include <revolution/vf/pf_remove.h>
#include <revolution/vf/pf_unmount.h>
#include <revolution/vf/pf_filelock.h>
#include <revolution/vf/pf_system.h>
#include <revolution/vf/d_vf.h>
#include <revolution/vf/d_vf_sys.h>
#include <revolution/vf/d_hash.h>
#include <revolution/vf/d_time.h>
#include <revolution/vf/d_common.h>
#include <revolution/vf/nand_drv.h>
#include <revolution/vf/sd_drv.h>

#ifdef __cplusplus
}
#endif
#endif
