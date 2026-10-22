#ifndef VCT_SSP_H
#define VCT_SSP_H

#include "vct.h"
#include "vct_main.h"

BOOL vct_init_ssp(VCTConfig* config);
void vct_finish_ssp(void);
void vct_check_timeout(void);
BOOL vct_handle_ssp(u8 aid, u8* buffer, int size, VCTResult* result);

#endif
