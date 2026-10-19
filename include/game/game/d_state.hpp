#pragma once

// dState: base_c / mode_c / step_c in one header. With -sym on all their inline functions share one weak
// .text section: d_msg's tail interleaves them (base_c dtor, mode_c dtor, mode_c::getStateName,
// base_c::getStateName), and so does d_sv_mgr's (with step_c's). d_state_base.hpp, d_state_mode.hpp and
// d_state_step.hpp only include this file. See notes/d_state.txt.

#include <types.h>

namespace dState {

// This framework is independent of the sLib state-ID framework.
// Method and member names are inferred from the save-manager instantiations.
template <class T>
class base_c {
public:
    typedef void (T::*Method)();

    // Target constructors only install the vtable; initialize() fills storage.
    base_c() {}
    virtual ~base_c() {}

    // Second virtual slot, after the destructor. The base has no name table.
    virtual const char *getStateName(Method method) { return NULL; }

    void initialize(const char *name, const Method &method) {
        mpName = name;
        mInitialized = true;
        mCurrentMethod = method;
    }

    void execute() {
        if (mCurrentMethod) {
            (static_cast<T *>(this)->*mCurrentMethod)();
        }
    }

    const char *getName() const { return mpName; }
    Method getCurrentMethod() const { return mCurrentMethod; }
    bool isCurrentMethod(Method method) const { return mCurrentMethod == method; }
    bool isInitialized() const { return mInitialized; }
    bool isInitializing() const { return mInInitialize; }

protected:
    const char *mpName; // 0x04
    bool mInitialized; // 0x08; gates state-name lookup
    Method mCurrentMethod; // 0x0C, 12-byte Metrowerks member pointer
    bool mInInitialize; // 0x18; set around mode-change initialization
}; // sizeof = 0x1C

} // namespace dState

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

    // d_msg (dMsg::Comp_c) passes nameLookup = false (its create stores 0 there).
    void initialize(const char *name, const Entry *modes, unsigned int count, bool nameLookup = true) {
        // The target requires a table containing an initial entry.
        mpModes = modes;
        mModeCount = count;
        this->mInInitialize = false;
        this->mpName = name;
        this->mInitialized = nameLookup;
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

namespace dState {

// The callbacks belong to Step; Owner is a separate object stored at +0x24.
// API names are inferred from the save-manager step instantiations.
template <class Owner, class Step>
class step_c : public mode_c<Step> {
public:
    typedef typename base_c<Step>::Method Method;

    struct Entry {
        const char *name; // 0x00
        Method execute; // 0x04
    }; // sizeof = 0x10

    step_c() {}
    virtual ~step_c() {}

    virtual const char *getStateName(Method method) {
        if (this->mInitialized) {
            const char *name = NULL;
            const Entry *entry = mpSteps;
            for (unsigned int i = 0; i < mStepCount; i++, entry++) {
                if (entry->execute == method) {
                    name = entry->name;
                }
            }
            return name;
        }
        return NULL;
    }

    // d_a_npc_sp_missing_child passes nameLookup = false.
    void initialize(Owner *owner, const char *name, const Entry *steps,
                    unsigned int count, bool nameLookup = true) {
        mpOwner = owner;
        mpSteps = steps;
        mStepCount = count;
        this->mpName = name;
        this->mInitialized = nameLookup;
        this->mCurrentMethod = steps[0].execute;
    }

    void nextStep() {
        const Entry *entry = mpSteps;
        for (unsigned int i = 0; i < mStepCount; i++, entry++) {
            if (entry->execute == this->mCurrentMethod && i + 1 < mStepCount) {
                this->mCurrentMethod = entry[1].execute;
                return;
            }
        }
    }

    Owner *getOwner() const { return mpOwner; }

protected:
    // The inherited mode table/count and in-initialize flag are untouched by
    // step initialization. The recovered save steps use their own table only.
    Owner *mpOwner; // 0x24
    const Entry *mpSteps; // 0x28
    unsigned int mStepCount; // 0x2C
}; // sizeof = 0x30

} // namespace dState
