#ifndef D_A_RMOBJ_CHECKP_HPP
#define D_A_RMOBJ_CHECKP_HPP

#include <types.h>

// The gate of the check point (REL d_a_rmobj_checkpNP, profile
// A_RMOBJ_CHECKP). Only the gate controls the gatekeeper calls are known: they
// act on the gate instance (NULL: FALSE) and set or test its state (0 closed,
// 1 opening, 2 open, 3 closing).
namespace dARmobjCheckp {
BOOL isGateClosed(); // fn_85_C6C: state 0
BOOL isGateOpen();   // fn_85_C94: state 2
BOOL openGate();     // fn_85_CC0: state 1
BOOL closeGate();    // fn_85_CE0: state 3
} // namespace dARmobjCheckp

#endif
