#ifndef GAME_D_PROFILE_HPP
#define GAME_D_PROFILE_HPP

#include <game/framework/f_profile.hpp>

// The City's profile table, installed while d_profileNP is linked.
// Names are inferred from the two functions' behavior.
namespace dProfile {
void install(); // .text:0x5C
void remove();  // .text:0x70
} // namespace dProfile

#endif
