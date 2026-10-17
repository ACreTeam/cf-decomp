#pragma once

// The octopus: sits in its pot and plays its animation.

#include <game/game/d_fish_museum.hpp>

class dFishMusOctopus_c : public dFishMusBase_c {
public:
    dFishMusOctopus_c();
    virtual ~dFishMusOctopus_c();
    virtual BOOL create(nw4r::g3d::ResFile res);
    virtual BOOL execute();
    virtual BOOL draw();
    virtual BOOL doDelete();

    void init();
}; // size 0x360
