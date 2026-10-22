#include <revolution/nand.h>

#include <string.h>
#include <wchar.h>

void NANDInitBanner(NANDBanner* bnr, u32 const flag, const u16* title, const u16* comment) {
    memset(bnr, 0, sizeof(NANDBanner));
    bnr->signature = 0x5749424E;
    bnr->flag = flag;

    if (wcscmp((wchar_t*)title, L"") == 0) {
        wcsncpy(bnr->comment[0], L" ", 32);
    } else {
        wcsncpy(bnr->comment[0], (wchar_t*)title, 32);
    }

    if (wcscmp((wchar_t*)comment, L"") == 0) {
        wcsncpy(bnr->comment[1], L" ", 32);
    } else {
        wcsncpy(bnr->comment[1], (wchar_t*)comment, 32);
    }
}
