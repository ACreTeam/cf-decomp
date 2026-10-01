#pragma once

#include <game/game/d_state_mode.hpp>

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

    void initialize(Owner *owner, const char *name, const Entry *steps,
                    unsigned int count) {
        mpOwner = owner;
        mpSteps = steps;
        mStepCount = count;
        this->mpName = name;
        this->mInitialized = true;
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
