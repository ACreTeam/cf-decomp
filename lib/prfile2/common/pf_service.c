#include <prfile2/pf_service.h>

u16 PF_GET_LE_U16(const u8* buf) {
    return buf[0] | buf[1] << 8;
}

void PF_LE16_TO_U16_STR(u8* sSrc, u32 num) {
    u32 cnt;
    s8 tmp_src;

    for (cnt = 0; cnt < num; cnt += sizeof(u16)) {
        tmp_src = ((s8*)sSrc)[cnt];
        sSrc[cnt] = sSrc[cnt + 1];
        sSrc[cnt + 1] = tmp_src;
    }
}
