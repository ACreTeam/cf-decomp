#ifndef GAME_D_A_SAMPLE_HPP
#define GAME_D_A_SAMPLE_HPP

// An empty sample actor (REL d_a_sampleNP, profile A_SAMPLE). A leftover
// template actor: create, delete, execute and draw all just succeed. Class name
// from the RTTI ("dAcSample_c").
#include <game/game/d_actor.hpp>

class dAcSample_c : public dActor_c {
  public:
    virtual int create();   // .text:0xA8
    virtual int doDelete(); // .text:0xB0
    virtual int execute();  // .text:0xB8
    virtual int draw();     // .text:0xC0
    virtual ~dAcSample_c(); // .text:0xC8
}; // size 0xA0

#endif
