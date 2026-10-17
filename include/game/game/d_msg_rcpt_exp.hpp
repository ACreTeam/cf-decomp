#ifndef D_MSG_RCPT_EXP_HPP
#define D_MSG_RCPT_EXP_HPP

#include <game/game/d_state.hpp>

// RTTI gives this wrapper's original name and global scope. Its unused storage
// is not yet identified; the mode table is the inherited dState table.
template <class T>
class dMsgRcptExp_c : public dState::mode_c<T> {
public:
    virtual ~dMsgRcptExp_c() {}

    void callMessageMethod(const typename dState::base_c<T>::Method &method) {
        (static_cast<T *>(this)->*method)();
    }

protected:
  // Run once by the receiver's onRequestEnd (d_a_npc_sp_player_select), cleared
  // unless it set a new one.
  /* 0x24 */ typename dState::base_c<T>::Method mRequestEndMethod;
  /* 0x30 */ u8 _0030[0xC];
}; // sizeof = 0x3C

#endif
