// Player-name script word. .text 800B953C..800B95E8.
#include <game/game/d_personal_id.hpp>

// 800B953C
dHmnName::Word_c::Word_c() {
    clear();
}

// 800B9580
dHmnName::Word_c::~Word_c() {}

// 800B95D8
u32 dHmnName::Word_c::getBufferSize() {
    return sizeof(mBuffer);
}

// 800B95E0
wchar_t *dHmnName::Word_c::getBuffer() {
    return mBuffer;
}
