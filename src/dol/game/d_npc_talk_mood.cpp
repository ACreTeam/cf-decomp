// The npc's mood (dNpcMood_c, a member of dAcNpcNml_c at +0x1FB0): happy / angry / sad animations
// and effects on a timer. Started by message tags (talk_c::setMood1..4 via startMood) and by the
// npc actor RELs. .text 800493EC..80049FDC. A TU of its own (file name inferred): its .rodata and .data
// start 8-aligned right after d_npc_talk_birthday's.
#include <game/game/d_npc_talk_birthday.hpp>
#include <game/game/d_animal.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_npc.hpp>
#include <game/game/d_player_mgr.hpp>

// 8046C838: wait anm of each mood (0 = normal)
static const int l_moodWaitAnm[5] = {0, 0xC4, 0xC6, 0xC8, 0xC6};
// 8046C84C: walk anm of each mood
static const int l_moodWalkAnm[5] = {1, 0xC5, 0xC7, 0xC9, 0xC7};
// 8046C860.. effect names of the moods
static const char l_happyWalkL[] = "afm_mnp_happy_walk_L";
static const char l_happyWalkR[] = "afm_mnp_happy_walk_R";
static const char l_pun[] = "afm_mnp_pun_S";
static const char l_sad[] = "afm_mnp_sad_S";

// The wait and walk anm of mood idx.
static inline void setMoodAnm(dAcNpc_c *npc, int idx) {
    dAcNpc_c::anmSet_c *anmSet = &npc->mAnmSet;
    anmSet->setAnm0(l_moodWaitAnm[idx]);
    anmSet->setAnm1(l_moodWalkAnm[idx]);
}

// 800493EC
dNpcMood_c::dNpcMood_c() {
    mTimer.reset();
    mTimerPending = 0;
    mState = 5;
    mProc = NULL;
    mShowEffects = 1;
    mDisabled = 0;
}

// 800494E8
dNpcMood_c::~dNpcMood_c() {}

// 800495CC
BOOL dNpcMood_c::isEnabled() {
    if (mDisabled) {
        return FALSE;
    }
    if (fn_800DCEDC() && fn_800DCF30() > 1) {
        return FALSE;
    }
    return TRUE;
}

// 80049620
void dNpcMood_c::setAnm(dAcNpcNml_c *npc, u32 mood) {
    int idx = mood;
    if (mood >= 5) {
        idx = 0;
    }
    if (!isEnabled()) {
        idx = 0;
    }
    if (idx == 4) {
        u8 looks = npc->mpAnimal != NULL ? npc->mpAnimal->mID.getLooks(1) : 6;
        switch (looks) {
        case 0:
        case 3:
            idx = 3;
            break;
        default:
            idx = 2;
            break;
        }
    }
    setMoodAnm(npc, idx);
}

// 800496F0
void dNpcMood_c::startTimer(int mood, int hours) {
    if (!isEnabled()) {
        mood = 0;
    }
    mTimer.set(mood, hours * 3600);
}

// 80049754
BOOL dNpcMood_c::isMoodAnm(dAcNpcNml_c *npc) {
    int anm = npc->getModel()->mAnm.mAnmId;
    for (int i = 0; i < 5; i++) {
        if (i == 0) {
            continue;
        }
        if (anm == l_moodWaitAnm[i] || anm == l_moodWalkAnm[i]) {
            return TRUE;
        }
    }
    return FALSE;
}

// 80049830
BOOL dNpcMood_c::isMoodWaitAnm(dAcNpcNml_c *npc) {
    int anm = npc->getModel()->mAnm.mAnmId;
    for (int i = 0; i < 5; i++) {
        if (anm == l_moodWaitAnm[i]) {
            return TRUE;
        }
    }
    return FALSE;
}

// 800498CC
BOOL dNpcMood_c::execute(dAcNpcNml_c *npc) {
    if (!isEnabled()) {
        return FALSE;
    }
    if (npc->mpEntry == NULL) {
        return FALSE;
    }
    dNpcTimer_c *timer = &npc->mpEntry->mTimer;
    int mode = timer->getMode();
    dAcNpc_c::action_c *action;
    int anm = npc->getModel()->mAnm.mAnmId;
    action = &npc->mAction;
    if (npc->mTalk.isBusy()) {
        mode = 0;
        if (anm != 0 && !action->hasRequest() && isMoodWaitAnm(npc)) {
            setAnm(npc, 0);
            action->requestWait(action->mCurPrio);
        }
    } else {
        if (mTimerPending) {
            if ((u32)mTimer.getMode() < 5) {
                timer->copy(&mTimer);
            }
            mTimerPending = 0;
            mTimer.reset();
        }
        timer->tick();
        mode = timer->getMode();
        setAnm(npc, mode);
        if (anm != npc->mAnmSet.getAnm(0) && !action->hasRequest() && isMoodWaitAnm(npc)) {
            action->requestWait(action->mCurPrio);
        }
    }
    updateState(npc, mode);
    return TRUE;
}

// 80049A64
BOOL dNpcMood_c::enterState(dAcNpcNml_c *npc, u32 state) {
    static enterFunc l_enter[5] = {
        NULL,
        &dNpcMood_c::enterHappy,
        &dNpcMood_c::enterAngry,
        &dNpcMood_c::enterSad,
        NULL,
    };
    if (state < 5 && l_enter[state]) {
        return (this->*l_enter[state])(npc);
    }
    return FALSE;
}

// 80049B30
void dNpcMood_c::updateState(dAcNpcNml_c *npc, int state) {
    if (state != mState && mState != 5) {
        mProc = NULL;
        mState = 5;
    } else if (!mProc) {
        switch (npc->getModel()->mAnm.mAnmId) {
        case 0xC4:
        case 0xC5:
            if (state == 1 && enterState(npc, 1)) {
                mState = state;
            }
            break;
        case 0xC6:
        case 0xC7:
            if (state == 2 || state == 4) {
                if (enterState(npc, 2)) {
                    mState = state;
                }
            }
            break;
        case 0xC8:
        case 0xC9:
            if (state == 3 || state == 4) {
                if (enterState(npc, 3)) {
                    mState = state;
                }
            }
            break;
        }
    }
    if (mProc) {
        (this->*mProc)(npc);
    }
}

// 80049CA0
BOOL dNpcMood_c::enterHappy(dAcNpcNml_c *npc) {
    mProc = &dNpcMood_c::procHappy;
    mPunOn = 0;
    return TRUE;
}

// 80049CCC
void dNpcMood_c::procHappy(dAcNpcNml_c *npc) {
    if (isMoodAnm(npc)) {
        if ((u8)npc->getModel()->mAnm.getFrame() == 0) {
            npc->mAudioObj.startSound(0x18CF);
        }
        if (mShowEffects) {
            const mMtx_c *mtx = &npc->mNodeMtx12;
            fn_80087B40(&mEffectL, l_happyWalkL, mtx, 0);
            fn_80087B40(&mEffectR, l_happyWalkR, mtx, 0);
        }
    }
}

// 80049D98
BOOL dNpcMood_c::enterAngry(dAcNpcNml_c *npc) {
    mProc = &dNpcMood_c::procAngry;
    mPunOn = 0;
    return TRUE;
}

// 80049DC4
void dNpcMood_c::procAngry(dAcNpcNml_c *npc) {
    if (isMoodAnm(npc)) {
        if ((u8)npc->getModel()->mAnm.getFrame() == 0) {
            npc->mAudioObj.startSound(0x18D1);
        }
        if (mShowEffects) {
            const mMtx_c *mtx = &npc->mNodeMtx12;
            if (!mPunOn) {
                if ((u8)npc->getModel()->mAnm.getFrame() == 0 && mEffectPun.create(l_pun, mtx)) {
                    mPunOn = 1;
                }
            } else if (!mEffectPun.followEffect(mtx)) {
                mPunOn = 0;
            }
        } else if (mPunOn) {
            mPunOn = 0;
        }
    }
}

// 80049F00
BOOL dNpcMood_c::enterSad(dAcNpcNml_c *npc) {
    mProc = &dNpcMood_c::procSad;
    mPunOn = 0;
    return TRUE;
}

// 80049F2C
void dNpcMood_c::procSad(dAcNpcNml_c *npc) {
    if (isMoodAnm(npc)) {
        if ((u8)npc->getModel()->mAnm.getFrame() == 0) {
            npc->mAudioObj.startSound(0x18D0);
        }
        if (mShowEffects) {
            fn_80087B40(&mEffectL, l_sad, &npc->mNodeMtx12, 0);
        }
    }
}
