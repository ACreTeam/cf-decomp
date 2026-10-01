#pragma once

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
    bool isInitialized() const { return mInitialized; }
    bool isInitializing() const { return mInInitialize; }

protected:
    const char *mpName; // 0x04
    bool mInitialized; // 0x08; gates state-name lookup
    Method mCurrentMethod; // 0x0C, 12-byte Metrowerks member pointer
    bool mInInitialize; // 0x18; set around mode-change initialization
}; // sizeof = 0x1C

} // namespace dState
