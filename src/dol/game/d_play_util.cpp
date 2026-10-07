// Play helpers next to the scene manager: weather time windows, actor / player list searches and
// the net sync of 8 shared records. .text 80160EA4..80161CC4, .bss 805FAF38..805FAF58,
// .sbss 8074E830..8074E848, .sdata2 80750E98..80750EB8. Names are inferred.
#include <game/game/d_play_util.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_fish_field.hpp>
#include <game/game/d_weather.hpp>
#include <game/cLib/c_lib.hpp>
#include <revolution/OS/OSTime.h>

// Not split yet (C linkage keeps the target names).
extern "C" {
BOOL fn_800DCF2C(int player); // net: the member is present
int fn_800DCF58(); // net: own member index
BOOL fn_800DD960();
void fn_800DD4C8();
void fn_800DD518(const void *data, u32 size);
void fn_800DD588(int type, int dst);
void *fn_800DD64C(int id); // net: a shared record
void fn_800DD5F8(int id, void *data, int arg); // net: set a shared record
}

void *lbl_8074E830; // 8074E830: the sky light (d_fireworks, d_star)
u32 lbl_8074E834; // 8074E834
dPlayActorMng_c *lbl_8074E838; // 8074E838: the actor manager (findActorIf, ...)
u8 lbl_8074E83C; // 8074E83C
dFishField_c *lbl_8074E840; // 8074E840: the fish field (the shared records' host)
u8 lbl_8074E844; // 8074E844: the fish field respawns on its next initSpawn

static u8 sSyncAcks[PLAY_SYNC_REC_NUM][4]; // 805FAF38: per record, per member

// 80160EA4
BOOL calcWeatherHoursA(f32 *hours) {
    if (lbl_8074EBE8 != NULL) {
        int weather = fn_801C98CC(lbl_8074EBE8);
        BOOL found = FALSE;
        dTime_c now = *dTime_c::getCurrent();
        dTime_c end = now;
        end.add(0, -TIME_DAY_START_HOUR, 0, 0);
        switch (weather) {
        case 1:
            found = TRUE;
            end.set(end.year, end.month, end.mday, 23, 0, 0);
            break;
        case 2:
            found = TRUE;
            end.set(end.year, end.month, end.mday, 22, 15, 0);
            break;
        case 3:
            found = TRUE;
            end.set(end.year, end.month, end.mday, 21, 30, 0);
            break;
        case 4:
            found = TRUE;
            end.set(end.year, end.month, end.mday, 20, 45, 0);
            break;
        }
        if (found) {
            *hours = (int)OS_TICKS_TO_SEC(dTime_c::diffTicks(&now, &end, FALSE)) / 3600.0f;
            return TRUE;
        }
    }
    return FALSE;
}

// 801610A8
BOOL calcWeatherHoursB(f32 *hours) {
    if (lbl_8074EBE8 != NULL) {
        int weather = fn_801C98CC(lbl_8074EBE8);
        BOOL found = FALSE;
        dTime_c now = *dTime_c::getCurrent();
        dTime_c start = now;
        start.add(0, -TIME_DAY_START_HOUR, 0, 0);
        switch (weather) {
        case 0x13:
            found = TRUE;
            start.set(start.year, start.month, start.mday, 15, 0, 0);
            break;
        case 0x14:
            found = TRUE;
            start.set(start.year, start.month, start.mday, 14, 0, 0);
            break;
        case 0x15:
            found = TRUE;
            start.set(start.year, start.month, start.mday, 13, 0, 0);
            break;
        case 0x16:
            found = TRUE;
            start.set(start.year, start.month, start.mday, 14, 0, 0);
            break;
        case 0x17:
            found = TRUE;
            start.set(start.year, start.month, start.mday, 13, 0, 0);
            break;
        }
        if (found) {
            *hours = (int)OS_TICKS_TO_SEC(dTime_c::diffTicks(&now, &start, FALSE)) / 3600.0f;
            return TRUE;
        }
    }
    return FALSE;
}

// 801612D8
int getWeatherPhaseA() {
    BOOL check = FALSE;
    dTime_c now = *dTime_c::getCurrent();
    if (isSceneAttr(getCurrentScene(), SCENE_ATTR_TOWN)) {
        if (isWinterSeasonType()) {
            check = TRUE;
        }
    } else if (isSnowSeason(&now)) {
        check = TRUE;
    }
    if (check) {
        f32 hours;
        if (calcWeatherHoursA(&hours)) {
            if (hours > 1.0f) {
                return PLAY_WEATHER_PHASE_FAR;
            }
            return hours < 0.0f ? PLAY_WEATHER_PHASE_PAST : PLAY_WEATHER_PHASE_SOON;
        }
    }
    return PLAY_WEATHER_PHASE_NONE;
}

// 801613E4
int getWeatherPhaseB() {
    BOOL check = FALSE;
    dTime_c now = *dTime_c::getCurrent();
    if (isSceneAttr(getCurrentScene(), SCENE_ATTR_TOWN)) {
        if (!isLateSeason()) {
            check = TRUE;
        }
    } else if (!isLateSeason(&now)) {
        check = TRUE;
    }
    if (check) {
        f32 hours;
        if (calcWeatherHoursB(&hours)) {
            if (hours > 2.0f / 3.0f) {
                return PLAY_WEATHER_PHASE_FAR;
            }
            return hours < 1.0f / 6.0f ? PLAY_WEATHER_PHASE_PAST : PLAY_WEATHER_PHASE_SOON;
        }
    }
    return PLAY_WEATHER_PHASE_NONE;
}

// 801614F0
BOOL dPlayActorMng_c::findActorIf(dPlayActorFunc func, void *arg) {
    dPlayActor_c *actor;
    dPlayActor_c **link = &mActors;
    while ((actor = *link) != NULL) {
        if (func(actor, arg)) {
            return TRUE;
        }
        link = &actor->mNext;
    }
    return FALSE;
}

// 8016156C
BOOL dPlayActorMng_c::forEachActiveActor(dPlayActorFunc func, void *arg) {
    dPlayActor_c *actor;
    BOOL found = FALSE;
    dPlayActor_c **link = &mActors;
    while ((actor = *link) != NULL) {
        if (actor->isActive() && func(actor, arg)) {
            found = TRUE;
        }
        link = &actor->mNext;
    }
    return found;
}

// 8016160C
BOOL dPlayActorMng_c::findPlayerIf(dPlayActorFunc func, void *arg) {
    for (int i = 0; i < PLAY_PLAYER_NUM; i++) {
        dPlayActor_c *player = mPlayers[i];
        if (player != NULL && !player->m23B && func(player, arg)) {
            return TRUE;
        }
    }
    return FALSE;
}

// 801616A4
void sendSyncRecClaim(dPlaySyncRec_c *rec, int idx) {
    if (rec->getMember() != fn_800DCF58()) {
        rec->setState(PLAY_SYNC_STATE_CLAIMED);
        dPlaySyncMsg_c msg;
        msg.setIdx(idx);
        msg.setA(rec->getA());
        msg.setC(rec->getC());
        msg.setMember(rec->getMember());
        fn_800DD4C8();
        fn_800DD518(&msg, sizeof(msg));
        fn_800DD588(0x41, 4);
    }
}

// 8016177C
void sendSyncRecRelease(const dPlaySyncRec_c *rec, int idx) {
    dPlaySyncMsg_c msg;
    msg.setType(2);
    msg.setIdx(idx);
    msg.setA(rec->getA());
    msg.setC(rec->getC());
    msg.setMember(rec->getMember());
    fn_800DD4C8();
    fn_800DD518(&msg, sizeof(msg));
    fn_800DD588(0x41, 4);
    if (fn_800DCF58() == 0) {
        recvSyncAck(&msg, 0);
    }
}

// 80161818
void claimSyncRec(dPlaySyncRec_c *rec, int idx) {
    if (!fn_800DD960()) {
        rec->setState(PLAY_SYNC_STATE_CLAIMED);
        sendSyncRecRelease(rec, idx);
    }
}

// 80161874
void clearSyncAcks() {
    for (int i = 0; i < PLAY_SYNC_REC_NUM; i++) {
        for (int j = 0; j < 4; j++) {
            sSyncAcks[i][j] = 0;
        }
    }
}

// 80161900
void recvSyncRelease(const dPlaySyncMsg_c *msg, int member) {
    u32 idx = msg->getIdx();
    sSyncAcks[idx][member] = 1;
    for (u32 i = 0; i < 4; i++) {
        if (sSyncAcks[idx][i] == 0 && fn_800DCF2C(i)) {
            return;
        }
    }
    for (u32 i = 0; i < 4; i++) {
        sSyncAcks[idx][i] = 0;
    }
    releaseSyncRec(idx);
}

// 801619AC
void recvSyncAck(const dPlaySyncMsg_c *msg, int member) {
    u32 idx = msg->getIdx();
    sSyncAcks[idx][member] = 1;
    for (u32 i = 0; i < 4; i++) {
        if (sSyncAcks[idx][i] == 0 && fn_800DCF2C(i)) {
            return;
        }
    }
    if (isSceneAttr(getCurrentScene(), SCENE_ATTR_TOWN) && fn_800DD960()) {
        dPlaySyncRecBuf_c buf;
        int id = msg->getIdx() + PLAY_SYNC_REC_ID;
        cLib::memCpy(&buf, fn_800DD64C(id), sizeof(dPlaySyncRec_c));
        buf.mRec.setState(PLAY_SYNC_STATE_NONE);
        fn_800DD5F8(id, &buf, 0);
    } else {
        dPlaySyncMsg_c out;
        out.setType(4);
        out.setIdx(idx);
        out.setA(msg->getA());
        out.setC(msg->getC());
        out.setMember(msg->getMember());
        fn_800DD4C8();
        fn_800DD518(&out, sizeof(out));
        fn_800DD588(0x41, 4);
    }
    for (u32 i = 0; i < 4; i++) {
        sSyncAcks[idx][i] = 0;
    }
}

// 80161B54
void releaseSyncRec(int idx) {
    int id = idx + PLAY_SYNC_REC_ID;
    void *src = fn_800DD64C(id);
    if (lbl_8074E840 != NULL) {
        dPlaySyncRec_c *rec = &lbl_8074E840->mRecs[idx].mRec;
        if (rec->getMember() == fn_800DCF58()) {
            rec->setState(PLAY_SYNC_STATE_NONE);
            fn_800DD5F8(id, rec, 0);
        } else {
            dPlaySyncRecBuf_c buf;
            cLib::memCpy(&buf, src, sizeof(dPlaySyncRec_c));
            buf.mRec.setState(PLAY_SYNC_STATE_NONE);
            fn_800DD5F8(id, &buf, 0);
        }
    } else {
        dPlaySyncRecBuf_c buf;
        cLib::memCpy(&buf, src, sizeof(dPlaySyncRec_c));
        buf.mRec.setMember(4);
        fn_800DD5F8(id, &buf, 0);
        sendSyncRecRelease(&buf.mRec, idx);
    }
}
