// The game's main font (dFont). .text 800A75B0..800A773C. See include/game/game/d_font.hpp.
#include <game/game/d_font.hpp>
#include <game/game/d_heap.hpp>
#include <lib/egg/core/eggHeap.h>

namespace dFont {

font_c g_mainFont;

// 800A75B0
BOOL load() {
    if (g_mainFont.mpBuffer == nullptr) {
        EGG::FrmHeap *heap = dHeap::saveHeap_p;
        void *data = g_mainFont.mLoader.request("/Font/Anml_HummingStd_B_28_I4.brfna", 0, heap);
        if (data == nullptr) {
            return FALSE;
        }
#if VERSION == VERSION_RUUK01_01
        u32 size = nw4r::ut::ArchiveFont::GetRequireBufferSize(data, nw4r::ut::ArchiveFont::LOAD_GLYPH_ALL, 70);
#else
        u32 size = nw4r::ut::ArchiveFont::GetRequireBufferSize(data, nw4r::ut::ArchiveFont::LOAD_GLYPH_ALL, 50);
#endif
        g_mainFont.mpBuffer = EGG::Heap::alloc(size, 0x20, dHeap::fontHeap_p);
        g_mainFont.mFont.Construct(g_mainFont.mpBuffer, size, data, nw4r::ut::ArchiveFont::LOAD_GLYPH_ALL);
        heap->free(MEM_FRM_HEAP_FREE_ALL);
    }
    return TRUE;
}

// 800A7678
u32 getHeapSize() {
    return FONT_HEAP_SIZE;
}

} // namespace dFont
