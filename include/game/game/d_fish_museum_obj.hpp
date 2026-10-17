#pragma once

// The fish tank decorations (wakame, water weed, sea anemone, octopus pod).

#include <game/game/d_fish_museum.hpp>

class dFishMusObjWakame00_c : public dFishMusBaseObj_c {
public:
    dFishMusObjWakame00_c();
    virtual ~dFishMusObjWakame00_c();
    virtual BOOL create(nw4r::g3d::ResFile res);
    virtual BOOL execute();
}; // size 0x90

class dFishMusObjWakame01_c : public dFishMusBaseObj_c {
public:
    dFishMusObjWakame01_c();
    virtual ~dFishMusObjWakame01_c();
    virtual BOOL create(nw4r::g3d::ResFile res);
    virtual BOOL execute();
}; // size 0x90

class dFishMusObjWaterWeed00_c : public dFishMusBaseObj_c {
public:
    dFishMusObjWaterWeed00_c();
    virtual ~dFishMusObjWaterWeed00_c();
    virtual BOOL create(nw4r::g3d::ResFile res);
    virtual BOOL execute();
}; // size 0x90

class dFishMusObjWaterWeed01_c : public dFishMusBaseObj_c {
public:
    dFishMusObjWaterWeed01_c();
    virtual ~dFishMusObjWaterWeed01_c();
    virtual BOOL create(nw4r::g3d::ResFile res);
    virtual BOOL execute();
}; // size 0x90

class dFishMusObjSeaAnemone_c : public dFishMusBaseObj_c {
public:
    dFishMusObjSeaAnemone_c();
    virtual ~dFishMusObjSeaAnemone_c();
    virtual BOOL create(nw4r::g3d::ResFile res);
    virtual BOOL execute();
}; // size 0x90

class dFishMusObjOctopusPod_c : public dFishMusBaseObj_c {
public:
    dFishMusObjOctopusPod_c();
    virtual ~dFishMusObjOctopusPod_c();
    virtual BOOL create(nw4r::g3d::ResFile res);
    virtual BOOL execute();
}; // size 0x90
