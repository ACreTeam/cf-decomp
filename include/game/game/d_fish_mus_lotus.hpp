#pragma once

// The lotus leaf decoration, the frog's seat.

#include <game/game/d_fish_museum.hpp>

class dFishMusObjLotus_c : public dFishMusBaseObj_c {
public:
    dFishMusObjLotus_c();
    virtual ~dFishMusObjLotus_c();
    virtual BOOL create(nw4r::g3d::ResFile res);
    virtual BOOL execute();

    static int sBaseNodeID;
    static mMtx_c sBaseMtx; // world matrix of the "base" node (dFishMusFrog_c sits on it)

    /* 0x90 */ const char *mBaseNodeName;
}; // size 0x94
