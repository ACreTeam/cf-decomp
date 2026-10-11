#pragma once

// d_root (DOL TU d_root.cpp, .text 801B96A4..801B97B4): dRoot_c (named by its RTTI), the root process: it creates
// profile 0x129 as its child. Member names inferred.

#include <types.h>
#include <game/framework/f_base.hpp>

class dSceneChange_c;

class dRoot_c : public fBase_c {
public:
    virtual ~dRoot_c();     // 801B974C
    virtual int create();   // 801B96F0
    virtual int doDelete(); // 801B972C
    virtual int execute();  // 801B973C
    virtual int draw();     // 801B9744

    static dSceneChange_c *getSceneChange(); // 801B97A8: &gSceneChange (d_sv_demo)

    static dRoot_c *sInstance; // 8074EB88
}; // size 0x64
