#ifndef NW4R_UT_RES_FONT_H
#define NW4R_UT_RES_FONT_H
#include <nw4r/types_nw4r.h>

#include <nw4r/ut/ut_ResFontBase.h>

namespace nw4r {
namespace ut {

struct BinaryFileHeader;

class ResFont : public detail::ResFontBase {
public:
    ResFont();
    virtual ~ResFont(); // at 0x8

    bool SetResource(void* pBuffer);
    void* RemoveResource();

private:
    static FontInformation* Rebuild(BinaryFileHeader* pHeader);

private:
    static const ulong SIGNATURE = 'RFNT';
    static const ulong SIGNATURE_UNPACKED = 'RFNU';

    static const ulong SIGNATURE_FONTINFO = 'FINF';
    static const ulong SIGNATURE_TEXGLYPH = 'TGLP';
    static const ulong SIGNATURE_CHARWIDTH = 'CWDH';
    static const ulong SIGNATURE_CHARMAP = 'CMAP';

    static const ulong SIGNATURE_GLGR = 'GLGR';
};

} // namespace ut
} // namespace nw4r

#endif
