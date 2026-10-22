#ifndef REVOLUTION_GX_REGS_H
#define REVOLUTION_GX_REGS_H

/*
 * Named bit fields of the GX hardware registers (BP/RAS, CP, PE, SU, TEV, TX, XF) and the
 * SC_*_SET_* shortcuts that insert a field into a register image. The raw FIFO write helpers
 * live in <private/gx.h>.
 */

#include <revolution/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <private/bp_reg.h>
#include <private/cp_reg.h>
#include <private/gen_reg.h>
#include <private/pe_reg.h>
#include <private/pi_reg.h>
#include <private/ra_gen.h>
#include <private/ras_reg.h>
#include <private/su_reg.h>
#include <private/tev_reg.h>
#include <private/tx_reg.h>
#include <private/xf_mem.h>
#include <revolution/gx/GXFDLShortcut.h>
#include <revolution/gx/shortcut_bp_reg.h>
#include <revolution/gx/shortcut_cp_reg.h>
#include <revolution/gx/shortcut_gen_reg.h>
#include <revolution/gx/shortcut_pe_reg.h>
#include <revolution/gx/shortcut_pi_cp_reg.h>
#include <revolution/gx/shortcut_ras_reg.h>
#include <revolution/gx/shortcut_su_reg.h>
#include <revolution/gx/shortcut_tev_reg.h>
#include <revolution/gx/shortcut_tx_reg.h>
#include <revolution/gx/shortcut_xf_mem.h>

#ifdef __MWERKS__
#define FAST_FLAG_SET(regOrg, newFlag, shift, size)                                                                                                      do {                                                                                                                                                     (regOrg) = (u32)__rlwimi((int)(regOrg), (int)(newFlag), (shift), (32 - (shift) - (size)), (31 - (shift)));                                       } while (0);
#else
#define FAST_FLAG_SET(regOrg, newFlag, shift, size)
#endif

#ifdef __cplusplus
}
#endif

#endif  // REVOLUTION_GX_REGS_H
