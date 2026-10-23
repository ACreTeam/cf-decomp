#pragma once

// textinput::input: the USB keyboard input (see tiTextInputBase.h).

#include <lib/textinput/tiTextInputBase.h>
#include <revolution/kbd.h>
#include <revolution/os.h>

namespace textinput {
namespace input {

// Gets the key events of the USB keyboard; listeners are chained after HKBManager's root listener.
class KBDListener {
public:
    KBDListener() : mpPrev(NULL), mpNext(NULL) {}
    virtual ~KBDListener() {}                     // 0x08
    virtual void OnAttach(KBDDevEvent *event) {}   // 0x0C
    virtual void OnDetach(KBDDevEvent *event) {}   // 0x10
    virtual void OnKeyEvent(KBDKeyEvent *event) {} // 0x14

    void append(KBDListener *listener); // adds listener at the end of the chain
    void remove(KBDListener *listener);

    /* 0x04 */ KBDListener *mpPrev;
    /* 0x08 */ KBDListener *mpNext;
}; // size 0xC

class HKBManager {
public:
    enum {
        CHANNEL_NUM = 2,
        KBD_CHANNEL_NUM = 4,
        KEY_NUM = 8,
        LOCK_MASK = KBD_MS_NUM_LOCK | KBD_MS_CAPS_LOCK | KBD_MS_SCROLL_LOCK,
        REPEAT_DELAY = 30,
        REPEAT_INTERVAL = 8,
    };

    enum KeySetType {
        KST_HOLD,
        KST_TRIGGER,
        KST_RELEASE,
        KST_REPEAT,
    };

    class KeySet {
    public:
        KeySet(const HKBManager *manager, KeySetType type)
            : mpManager(manager), muType(type), msIndex(-1), muChannel(0), mwcVCode(0) {}

        KBDHidCode GetKey() const;
        wchar_t GetWChar() const;
        wchar_t GetVCode() const;
        bool IsValid() const;
        KeySet GetNext() const;

        /* 0x00 */ const HKBManager *mpManager;
        /* 0x04 */ u8 muType;
        /* 0x05 */ s8 msIndex;
        /* 0x06 */ u8 muChannel;
        /* 0x08 */ mutable wchar_t mwcVCode;
    }; // size 0xC

    class KeyState_ {
    public:
        void NotifyEvent(u8 down, KBDHidCode hid);
        void Update();
        void UpdateModState_();

        void Clear() {
            muHoldMask = 0;
            muTriggerMask = 0;
            muReleaseMask = 0;
            muRepeatMask = 0;
            muCurMask = 0;
            muNextMask = 0;
            muModState &= LOCK_MASK;
            muForceModMask = 0;
            muForceModState = 0;
            mbLedRetry = false;
        }

        u32 GetModState() const {
            u32 modState = muModState;
            modState &= ~muForceModMask;
            modState |= muForceModState & muForceModMask;
            return modState;
        }

        void SendModState() {
            KBDSetModState(muChannel, (KBDModState)muModState);
            u8 leds = 0;
            if (muModState & KBD_MS_NUM_LOCK) {
                leds |= KBD_LED_NUM_LOCK;
            }
            if (muModState & KBD_MS_CAPS_LOCK) {
                leds |= KBD_LED_CAPS_LOCK;
            }
            if (muModState & KBD_MS_SCROLL_LOCK) {
                leds |= KBD_LED_SCROLL_LOCK;
            }
            if (KBDSetLeds(muChannel, (KBDLedState)leds) == KBD_ERR_BUSY) {
                mbLedRetry = true;
            }
        }

        /* 0x00 */ KBDChannel muChannel;
        /* 0x04 */ u32 muHoldMask;
        /* 0x08 */ u32 muTriggerMask;
        /* 0x0C */ u32 muReleaseMask;
        /* 0x10 */ u32 muRepeatMask;
        /* 0x14 */ u32 muPrevMask;
        /* 0x18 */ u32 muCurMask;
        /* 0x1C */ u32 muNextMask;
        /* 0x20 */ u32 muModState;
        /* 0x24 */ u32 muForceModState;
        /* 0x28 */ u32 muForceModMask;
        /* 0x2C */ KBDCountryCode meCountry;
        /* 0x30 */ bool mbLedRetry;
        /* 0x31 */ KBDHidCode maPrevHid[KEY_NUM];
        /* 0x39 */ KBDHidCode maCurHid[KEY_NUM];
        /* 0x41 */ KBDHidCode maNextHid[KEY_NUM];
        /* 0x4C */ s32 maRepeatCount[KEY_NUM];
    }; // size 0x6C

    class KBDListenerOwn : public KBDListener {
    public:
        KBDListenerOwn(HKBManager *owner) : mpOwner(owner) {}
        virtual ~KBDListenerOwn() {}
        virtual void OnAttach(KBDDevEvent *event);
        virtual void OnDetach(KBDDevEvent *event);
        virtual void OnKeyEvent(KBDKeyEvent *event);

        /* 0x0C */ HKBManager *mpOwner;
    }; // size 0x10

    HKBManager();

    void Initialize();
    void ClearState();
    u32 GetModifierState() const;
    void SetCountry(u8 country);
    void SetModifierState(u32 state, u32 mask);
    void SetForceModifierState(u32 state, u32 mask);
    void Update();
    KeySet GetTriggeredKeySet() const;
    KeySet GetRepeatedKeySet() const;

    KBDListener *getListenerRoot() { return &mListener; }

    static wchar_t ConvertVCode(wchar_t wc) {
        if (wc >= 0xF130 && wc <= 0xF139) {
            wc = wc - 0xF100;
        } else if (wc == 0xF10D) {
            wc = 0xF1CD;
        } else if (wc >= 0xF100 && wc <= 0xF13F) {
            wc = wc - 0xF100;
        } else if (wc >= 0xF140 && wc <= 0xF17F) {
            wc = wc + 0x40;
        }
        if ((wc & 0xF000) == 0xF000) {
            wc = 0;
        } else if (wc == 0xEEEE) {
            wc = 0;
        } else if (wc < 0x20) {
            wc = 0;
        }
        return wc;
    }

    wchar_t FilterChar(wchar_t wc) const {
        if (mpwcFilter == NULL) {
            return wc;
        }
        if (muFilterNum == 0) {
            return wc;
        }
        for (u32 i = 0; i < muFilterNum; i++) {
            if (wc == mpwcFilter[i]) {
                return wc;
            }
        }
        return 0;
    }

    void DetachChannel(KBDChannel channel) {
        mbAttached[channel] = false;
        mKeyState[channel].Clear();
    }

    void SendLockState(u8 channel) {
        if (mbAttached[channel]) {
            mKeyState[channel].SendModState();
        }
    }

    static void SetLedCB(KBDLedEvent *ev, void *cbArg);
    static void AttachCB(KBDDevEvent *event);
    static void DetachCB(KBDDevEvent *event);
    static void KeyEventCB(KBDKeyEvent *event);

    void TurnOffLed(KBDChannel channel) {
        union {
            struct {
                KBDChannel channel;
                u32 leds;
            } led;
            void *userData;
        } arg;
        arg.led.channel = channel;
        arg.led.leds = 0;
        BOOL enabled = OSDisableInterrupts();
        muLedRetryMask &= ~(1 << channel);
        OSRestoreInterrupts(enabled);
        if (KBDSetLedsAsync(channel, KBD_LED_NULL, SetLedCB, arg.userData) == KBD_ERR_BUSY) {
            enabled = OSDisableInterrupts();
            muLedRetryMask |= 1 << channel;
            OSRestoreInterrupts(enabled);
        }
    }

    /* 0x00 */ bool mbInitialized;
    /* 0x01 */ bool mbAttached[CHANNEL_NUM];
    /* 0x04 */ u32 muLockState;
    /* 0x08 */ u32 muLedRetryMask;
    /* 0x0C */ const wchar_t *mpwcFilter;
    /* 0x10 */ u32 muFilterNum;
    /* 0x14 */ KeyState_ mKeyState[CHANNEL_NUM];
    /* 0xEC */ KBDListenerOwn mListener;

    static HKBManager sInstance;
}; // size 0xFC

} // namespace input
} // namespace textinput
