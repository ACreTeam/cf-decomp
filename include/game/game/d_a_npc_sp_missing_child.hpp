#ifndef D_A_NPC_SP_MISSING_CHILD_HPP
#define D_A_NPC_SP_MISSING_CHILD_HPP

#include <game/game/d_a_npc_sp.hpp>
#include <game/game/d_state.hpp>

// Katie, the lost child (REL d_a_npc_sp_missing_childNP, profile
// A_NPC_SP_MISSING_CHILD, item key 0x8019). Class name from RTTI
// ("dAcNpcSpMissingChild_c"); mode names from its CP932 mode labels. Only what
// the gatekeeper uses is declared.
class dAcNpcSpMissingChild_c : public dAcNpcSp_c,
                               public dState::mode_c<dAcNpcSpMissingChild_c> {
  public:
    typedef dState::base_c<dAcNpcSpMissingChild_c>::Method Method;

    void executeFollowRun(); // fn_29_25F0 "プレーヤー追従走り" (follow the
                             // player, running)
    void executeFall(); // fn_29_2EE8 "転ぶ" (falls over)

    // Used by the gatekeeper when the player leaves with Katie.
    void startFollowRun() {
        changeMode(&dAcNpcSpMissingChild_c::executeFollowRun);
    }
    void startFall() {
        changeMode(&dAcNpcSpMissingChild_c::executeFall);
    }
};

#endif
