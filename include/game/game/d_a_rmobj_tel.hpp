#pragma once

// d_a_rmobj_tel (REL 95, the house phone): functions the player REL calls directly (STATE_PHONE_*).
// They play the phone model's animation on the single phone instance (.bss 8).
extern "C" void fn_95_1B8(); // .text 1B8: picked up (model animation 1, once)
extern "C" void fn_95_1E4(); // .text 1E4: talking (animation 2, loop)
extern "C" void fn_95_210(); // .text 210: hung up (animation 1, reversed)
