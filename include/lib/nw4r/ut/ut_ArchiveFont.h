#ifndef NW4R_UT_ARCHIVE_FONT_H
#define NW4R_UT_ARCHIVE_FONT_H
// Destination: include/lib/nw4r/ut/ut_ArchiveFont.h (lib TUs ut_ArchiveFontBase.cpp 8022C784.., ut_ArchiveFont.cpp
// 8022D62C..8022E9D8, not decompiled). Only what the game uses; the private layout is opaque.
#include <nw4r/types_nw4r.h>

#include <nw4r/ut/ut_ResFontBase.h>

namespace nw4r {
namespace ut {
namespace detail {

class ArchiveFontBase : public ResFontBase {
public:
    ArchiveFontBase();          // 8022C784
    virtual ~ArchiveFontBase(); // 8022C7C8

private:
    u8 PADDING_0x1C[0x28 - 0x1C]; // at 0x1C
};

} // namespace detail

// A font streamed from a .brfna archive (glyph sheets decompressed into a caller buffer).
class ArchiveFont : public detail::ArchiveFontBase {
public:
    static const char LOAD_GLYPH_ALL[1]; // 807539F8 (.sbss2): "" = every glyph group

    ArchiveFont();          // 8022D62C
    virtual ~ArchiveFont(); // 8022D684

    // 8022D6DC: the buffer Construct needs for the archive's header and the given glyph groups.
    // The int is compared against 1 (meaning unknown).
    static u32 GetRequireBufferSize(const void *brfna, const char *glyphGroups, int arg2);
    // 8022DB60
    bool Construct(void *pBuffer, u32 bufferSize, const void *brfna, const char *glyphGroups);

private:
    u16 _28;     // at 0x28
    u16 _2A;     // at 0x2A
    u32 _2C;     // at 0x2C
    u32 _30;     // at 0x30
    u32 _34;     // at 0x34
    u32 _38;     // at 0x38
}; // size 0x3C

} // namespace ut
} // namespace nw4r

#endif
