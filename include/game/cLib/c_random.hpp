#pragma once
#include <types.h>

/// @brief Random number generation helper class.
/// @ingroup clib
class cRandom_c {
public:

    /// @brief Initializes the class with the given seed.
    cRandom_c(u32 seed) {
        setSeed(seed);
    }
    ~cRandom_c() {}

    void setSeed(u32 seed);
    u32 getRandom(); ///< Generates an integer between 0 and ULONG_MAX.
    float getRandomF(); ///< Generates a floating point number between 0 and 1.

    
private:
    u32 mSeed; ///< The current seed.
};
