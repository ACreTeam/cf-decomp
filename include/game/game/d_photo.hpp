#pragma once

// d_photo.cpp: the in-game photo. A capture copies the EFB into a texture (the menus show it); an encode
// shrinks that texture to 640 x 480 (896 x 480 on 16:9) on a worker thread and compresses it to a JPEG.

#include <types.h>
#include <lib/egg/core/eggHeap.h>

namespace dPhoto {

enum Mode_e {
    MODE_NONE = 0,
    MODE_CAPTURE = 1,
    MODE_ENCODE = 2,
};

enum Step_e {
    STEP_NONE = 0,
    STEP_CAPTURE = 1,  // capture() copies the EFB
    STEP_CAPTURED = 2, // finished on the next capture()
    STEP_ENCODE = 3,   // execute() resumes the thread
    STEP_ENCODING = 4,
};

int getMode();                 // 800FACC0
void request(int mode);        // 800FACC8: ignored unless the mode is MODE_NONE
void requestCapture();         // 800FAD20
void requestEncode(bool small); // 800FAD28: small: at most 70 KB instead of 100 KB
void *getTexBuffer();          // 800FAD5C
int getTexWidth();             // 800FAD6C
int getTexHeight();            // 800FAD9C
u8 *getJpeg();                 // 800FADCC
u32 getJpegSize();             // 800FADD4
BOOL isCaptured();             // 800FADDC
void clear();                  // 800FADE4
void create(EGG::Heap *heap);   // 800FB868
void capture();                // 800FB9A0: after the frame is drawn
void execute();                // 800FBA18
BOOL tryClear();               // 800FBAA0: clears the photo unless a request is running

} // namespace dPhoto
