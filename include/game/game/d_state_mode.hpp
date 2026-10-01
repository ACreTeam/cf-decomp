#pragma once

#include <game/game/d_state_base.hpp>

namespace dState {

template <class T>
class mode_c : public base_c<T> {
public:
    typedef typename base_c<T>::Method Method;

    struct Entry {
        const char *name; // 0x00
        Method initialize; // 0x04
        Method execute; // 0x10
    }; // sizeof = 0x1C

    mode_c() {}
    virtual ~mode_c() {}

    virtual const char *getStateName(Method method) {
        if (this->mInitialized) {
            const char *name = NULL;
            const Entry *entry = mpModes;
            for (unsigned int i = 0; i < mModeCount; i++, entry++) {
                if (entry->execute == method) {
                    name = entry->name;
                }
            }
            return name;
        }
        return NULL;
    }

    void initialize(const char *name, const Entry *modes, unsigned int count) {
        // The target requires a table containing an initial entry.
        mpModes = modes;
        mModeCount = count;
        this->mInInitialize = false;
        this->mpName = name;
        this->mInitialized = true;
        const Entry *first = mpModes;
        // Call the first initializer before installing the first execute method.
        if (first[0].initialize) {
            (static_cast<T *>(this)->*first[0].initialize)();
        }
        this->mCurrentMethod = first[0].execute;
    }

    void changeMode(Method method) {
        const Entry *entry = mpModes;
        Method next = NULL;
        Method initialize = NULL;
        for (unsigned int i = 0; i < mModeCount; i++, entry++) {
            if (entry->execute == method) {
                if (entry->initialize) {
                    initialize = entry->initialize;
                }
                next = method;
            }
        }
        if (next) {
            this->mCurrentMethod = next;
            if (initialize) {
                this->mInInitialize = true;
                (static_cast<T *>(this)->*initialize)();
                this->mInInitialize = false;
            }
        }
    }

protected:
    const Entry *mpModes; // 0x1C
    unsigned int mModeCount; // 0x20
}; // sizeof = 0x24

} // namespace dState
