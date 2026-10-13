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
    /* 0x24 */ u8 _0024[0x18];
}; // sizeof = 0x3C

#endif
