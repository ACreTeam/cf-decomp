#include <lib/textinput/tiHKBManager.h>

namespace textinput {
namespace input {

HKBManager HKBManager::sInstance;

void KBDListener::append(KBDListener *listener) {
    KBDListener *last = this;
    while (last->mpNext != NULL) {
        last = last->mpNext;
    }
    last->mpNext = listener;
    listener->mpPrev = last;
}

void KBDListener::remove(KBDListener *listener) {
    for (KBDListener *it = mpNext; it != NULL; it = it->mpNext) {
        if (it == listener) {
            KBDListener *prev = it->mpPrev;
            KBDListener *next = it->mpNext;
            if (prev != NULL) {
                prev->mpNext = next;
            }
            if (next != NULL) {
                next->mpPrev = prev;
            }
            listener->mpPrev = NULL;
            listener->mpNext = NULL;
            return;
        }
    }
    OSPanic("tiHKBManager.cpp", 146, "specified listener not found!");
}

void HKBManager::SetLedCB(KBDLedEvent *ev, void *cbArg) {
    if (ev->rc == KBD_ERR_BUSY) {
        sInstance.TurnOffLed(ev->channel);
    }
}

void HKBManager::KBDListenerOwn::OnAttach(KBDDevEvent *event) {
    KBDChannel channel = event->channel;
    if (channel >= CHANNEL_NUM) {
        KBDSetLockProcessing(channel, FALSE);
        mpOwner->TurnOffLed(event->channel);
    } else {
        mpOwner->mbAttached[channel] = true;
        KBDSetLockProcessing(event->channel, FALSE);
        mpOwner->mKeyState[event->channel].mbLedRetry = true;
    }
}

void HKBManager::KBDListenerOwn::OnDetach(KBDDevEvent *event) {
    if (event->channel >= CHANNEL_NUM) {
        return;
    }
    mpOwner->DetachChannel(event->channel);
}

void HKBManager::KBDListenerOwn::OnKeyEvent(KBDKeyEvent *event) {
    KBDChannel channel = event->channel;
    if (channel < CHANNEL_NUM && !(event->mode & KBD_KM_REPEAT)) {
        mpOwner->mKeyState[channel].NotifyEvent(event->mode & KBD_KM_DOWN,
                                                event->hid);
    }
}

void HKBManager::AttachCB(KBDDevEvent *event) {
    KBDListener *listener = &sInstance.mListener;
    while (listener != NULL) {
        listener->OnAttach(event);
        listener = listener->mpNext;
        if (listener == &sInstance.mListener) {
            break;
        }
    }
}

void HKBManager::DetachCB(KBDDevEvent *event) {
    KBDListener *listener = &sInstance.mListener;
    while (listener != NULL) {
        listener->OnDetach(event);
        listener = listener->mpNext;
        if (listener == &sInstance.mListener) {
            break;
        }
    }
}

void HKBManager::KeyEventCB(KBDKeyEvent *event) {
    KBDListener *listener = &sInstance.mListener;
    while (listener != NULL) {
        listener->OnKeyEvent(event);
        listener = listener->mpNext;
        if (listener == &sInstance.mListener) {
            break;
        }
    }
}

HKBManager::HKBManager()
    : mbInitialized(false), muLockState(0), muLedRetryMask(0), mpwcFilter(NULL),
      muFilterNum(0), mListener(this) {
    for (u8 i = 0; i < CHANNEL_NUM; i++) {
        mbAttached[i] = false;
        mKeyState[i].Clear();
        mKeyState[i].muChannel = i;
        mKeyState[i].meCountry = KBD_CC_JAPANESE;
        mKeyState[i].muModState = 0;
    }
}

void HKBManager::Initialize() {
    if (mbInitialized) {
        return;
    }
    mbInitialized = true;
    KBDInitRegionUS();
    KBDInit();
    if (mListener.mpPrev == NULL) {
        KBDSetAttachCallback(AttachCB);
        KBDSetDetachCallback(DetachCB);
        KBDSetKeyCallback(KeyEventCB);
    }
    for (u8 i = 0; i < CHANNEL_NUM; i++) {
        KBDSetCountry(i, mKeyState[i].meCountry);
    }
}

void HKBManager::ClearState() {
    for (u8 i = 0; i < CHANNEL_NUM; i++) {
        mKeyState[i].Clear();
    }
}

u32 HKBManager::GetModifierState() const {
    u32 modState = 0;
    for (u32 i = 0; i < CHANNEL_NUM; i++) {
        modState |= mKeyState[i].GetModState();
    }
    return modState;
}

void HKBManager::SetCountry(u8 country) {
    for (u8 i = 0; i < CHANNEL_NUM; i++) {
        KeyState_ &keyState = mKeyState[i];
        keyState.meCountry = (KBDCountryCode)country;
        KBDSetCountry(i, (KBDCountryCode)country);
        keyState.SendModState();
    }
}

void HKBManager::SetModifierState(u32 state, u32 mask) {
    muLockState = (muLockState & ~mask) | (state & mask);
    for (u8 i = 0; i < CHANNEL_NUM; i++) {
        bool lockChanged =
            (mKeyState[i].muModState & LOCK_MASK) == (muLockState & LOCK_MASK)
                ? false
                : true;
        mKeyState[i].muModState &= ~LOCK_MASK;
        mKeyState[i].muModState |= muLockState;
        if (lockChanged) {
            SendLockState(i);
        }
    }
}

void HKBManager::SetForceModifierState(u32 state, u32 mask) {
    for (int i = 0; i < CHANNEL_NUM; i++) {
        mKeyState[i].muForceModState = state;
        mKeyState[i].muForceModMask = mask;
    }
}

void HKBManager::Update() {
    if (!mbInitialized) {
        return;
    }
    for (u8 channel = CHANNEL_NUM; channel < KBD_CHANNEL_NUM; channel++) {
        if (muLedRetryMask & (1 << channel)) {
            TurnOffLed(channel);
        }
    }
    for (u8 i = 0; i < CHANNEL_NUM; i++) {
        mKeyState[i].Update();
    }
}

HKBManager::KeySet HKBManager::GetTriggeredKeySet() const {
    KeySet keySet(this, KST_TRIGGER);
    return keySet.GetNext();
}

HKBManager::KeySet HKBManager::GetRepeatedKeySet() const {
    KeySet keySet(this, KST_REPEAT);
    return keySet.GetNext();
}

void HKBManager::KeyState_::NotifyEvent(u8 down, KBDHidCode hid) {
    int i;
    int slot = -1;
    if (down == 1) {
        for (i = 0; i < KEY_NUM; i++) {
            if (muNextMask & (1 << i)) {
                if (maNextHid[i] == hid) {
                    slot = i;
                    break;
                }
            } else {
                slot = i;
                break;
            }
        }
        if (slot >= 0) {
            maNextHid[slot] = hid;
            muNextMask |= 1 << slot;
        }
    } else {
        for (i = 0; i < KEY_NUM; i++) {
            if ((muNextMask & (1 << i)) && maNextHid[i] == hid) {
                slot = i;
                break;
            }
        }
        if (slot >= 0) {
            muNextMask &= ~(1 << slot);
        }
    }
}

void HKBManager::KeyState_::Update() {
    u32 i;
    BOOL enabled = OSDisableInterrupts();
    muPrevMask = muCurMask;
    muCurMask = muNextMask;
    for (i = 0; i < KEY_NUM; i++) {
        maPrevHid[i] = maCurHid[i];
        maCurHid[i] = maNextHid[i];
    }
    OSRestoreInterrupts(enabled);

    UpdateModState_();

    muHoldMask = muCurMask;
    muTriggerMask = 0;
    for (i = 0; i < KEY_NUM; i++) {
        if ((muCurMask & (1 << i)) &&
            (!(muPrevMask & (1 << i)) || maPrevHid[i] != maCurHid[i])) {
            muTriggerMask |= 1 << i;
        }
    }

    muReleaseMask = 0;
    for (i = 0; i < KEY_NUM; i++) {
        if ((muPrevMask & (1 << i)) &&
            (!(muCurMask & (1 << i)) || maPrevHid[i] != maCurHid[i])) {
            muReleaseMask |= 1 << i;
        }
    }

    muRepeatMask = 0;
    for (i = 0; i < KEY_NUM; i++) {
        if (muCurMask & (1 << i)) {
            if (muTriggerMask & (1 << i)) {
                maRepeatCount[i] = REPEAT_DELAY;
                muRepeatMask |= 1 << i;
            }
            if (--maRepeatCount[i] == 0) {
                maRepeatCount[i] = REPEAT_INTERVAL;
                muRepeatMask |= 1 << i;
            }
        }
    }
}

void HKBManager::KeyState_::UpdateModState_() {
    u32 prevModState = muModState;
    muModState &=
        ~(KBD_MS_CTRL | KBD_MS_SHIFT | KBD_MS_ALT | KBD_MS_GUI | KBD_MS_ALTGR);
    for (u8 i = 0; i < KEY_NUM; i++) {
        if (!(muCurMask & (1 << i))) {
            continue;
        }
        KBDHidCode hid = maCurHid[i];
        if (hid == 0xE5 || hid == 0xE1) {
            muModState |= KBD_MS_SHIFT;
        } else if (hid == 0xE4 || hid == 0xE0) {
            muModState |= KBD_MS_CTRL;
        } else if (hid == 0xE7 || hid == 0xE3) {
            muModState |= KBD_MS_GUI;
        } else if (hid == 0xE2) {
            muModState |= KBD_MS_ALT;
        } else if (hid == 0xE6) {
            if (meCountry == KBD_CC_JAPANESE ||
                meCountry == KBD_CC_UNITED_STATES) {
                muModState |= KBD_MS_ALT;
            } else {
                muModState |= KBD_MS_ALTGR;
            }
        }
    }
    if ((prevModState & LOCK_MASK) != (muModState & LOCK_MASK) || mbLedRetry) {
        mbLedRetry = false;
        SendModState();
    }
}

KBDHidCode HKBManager::KeySet::GetKey() const {
    if (!IsValid()) {
        return 0;
    }
    switch (muType) {
    case KST_HOLD:
        return mpManager->mKeyState[muChannel].maCurHid[msIndex];
    case KST_TRIGGER:
        return mpManager->mKeyState[muChannel].maCurHid[msIndex];
    case KST_RELEASE:
        return mpManager->mKeyState[muChannel].maPrevHid[msIndex];
    case KST_REPEAT:
        return mpManager->mKeyState[muChannel].maCurHid[msIndex];
    }
    return 0;
}

wchar_t HKBManager::KeySet::GetWChar() const {
    wchar_t wc = GetVCode();
    wc = ConvertVCode(wc);
    wc = mpManager->FilterChar(wc);
    return wc;
}

wchar_t HKBManager::KeySet::GetVCode() const {
    if (!IsValid()) {
        return 0;
    }
    if (mwcVCode != 0) {
        return mwcVCode;
    }
    const HKBManager *manager = mpManager;
    u32 modState = manager->GetModifierState();
    if ((modState & (KBD_MS_CTRL | KBD_MS_ALT | KBD_MS_GUI)) != 0 &&
        (modState & (KBD_MS_CTRL | KBD_MS_ALT)) != (KBD_MS_CTRL | KBD_MS_ALT)) {
        return 0;
    }
    switch (muType) {
    case KST_HOLD:
    case KST_TRIGGER:
    case KST_REPEAT:
        if ((1 << msIndex) & manager->mKeyState[muChannel].muCurMask) {
            mwcVCode = KBDTranslateHidCode(
                manager->mKeyState[muChannel].maCurHid[msIndex],
                (KBDModState)modState, manager->mKeyState[muChannel].meCountry);
        }
        break;
    case KST_RELEASE:
        if ((1 << msIndex) & manager->mKeyState[muChannel].muPrevMask) {
            mwcVCode = KBDTranslateHidCode(
                manager->mKeyState[muChannel].maPrevHid[msIndex],
                (KBDModState)modState, manager->mKeyState[muChannel].meCountry);
        }
        break;
    }
    return mwcVCode;
}

bool HKBManager::KeySet::IsValid() const {
    if (mpManager == NULL) {
        return false;
    }
    if (msIndex < 0 || msIndex >= KEY_NUM) {
        return false;
    }
    if (muChannel >= CHANNEL_NUM) {
        return false;
    }
    switch (muType) {
    case KST_HOLD:
        return ((1 << msIndex) & mpManager->mKeyState[muChannel].muHoldMask) !=
               0;
    case KST_TRIGGER:
        return ((1 << msIndex) &
                mpManager->mKeyState[muChannel].muTriggerMask) != 0;
    case KST_RELEASE:
        return ((1 << msIndex) &
                mpManager->mKeyState[muChannel].muReleaseMask) != 0;
    case KST_REPEAT:
        return ((1 << msIndex) &
                mpManager->mKeyState[muChannel].muRepeatMask) != 0;
    }
    return false;
}

HKBManager::KeySet HKBManager::KeySet::GetNext() const {
    KeySet next(mpManager, (KeySetType)muType);
    if (mpManager == NULL) {
        return next;
    }
    for (next.muChannel = muChannel; next.muChannel < CHANNEL_NUM;
         next.muChannel++) {
        u32 mask = 0;
        switch (muType) {
        case KST_HOLD:
            mask = mpManager->mKeyState[next.muChannel].muHoldMask;
            break;
        case KST_TRIGGER:
            mask = mpManager->mKeyState[next.muChannel].muTriggerMask;
            break;
        case KST_RELEASE:
            mask = mpManager->mKeyState[next.muChannel].muReleaseMask;
            break;
        case KST_REPEAT:
            mask = mpManager->mKeyState[next.muChannel].muRepeatMask;
            break;
        }
        for (next.msIndex = msIndex + 1; next.msIndex < KEY_NUM;
             next.msIndex++) {
            if (mask & (1 << next.msIndex)) {
                return next;
            }
        }
        next.msIndex = -1;
    }
    return next;
}

} // namespace input
} // namespace textinput
