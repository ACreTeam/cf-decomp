// CF builds the platform socket, utility and memory sources as one translation unit: they share one
// .bss/.sdata block (memmanagercallbacks first), and -ipa file inlines gsimalloc/gsifree into the
// socket and utility code while every other GameSpy unit calls them.
#include "gsPlatformSocket.c"
#include "gsPlatformUtil.c"
#include "gsMemory.c"

#ifdef MUST_MATCH
// unreferenced .bss after getlocalhost's statics, layout only
#pragma force_active on
char lbl_806AB378[0xB0];
#pragma force_active reset
#endif
