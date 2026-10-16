// Play helpers next to the scene manager: weather time windows, actor / player list searches and
// the net sync of 8 shared records. .text 80160EA4..80161CC4, .bss 805FAF38..805FAF58,
// .sbss 8074E830..8074E848, .sdata2 80750E98..80750EB8. Names are inferred.
#include <game/game/d_play_util.hpp>
#include <game/game/d_date.hpp>
#include <game/game/d_scene.hpp>
#include <game/game/d_sky.hpp>
#include <game/game/d_fish_field.hpp>
#include <game/game/d_weather.hpp>
#include <game/cLib/c_lib.hpp>
#include <revolution/OS/OSTime.h>
#include <game/game/d_net.hpp>

dSky_c *g_sky; // 8074E830: the sky actor
u32 lbl_8074E834; // 8074E834
dPlayActorMng_c *g_playActorMng; // 8074E838: the actor manager (findActorIf, ...)
u8 g_insectRespawn; // 8074E83C
dFishField_c *g_fishField; // 8074E840: the fish field (the shared records' host)
u8 g_fishRespawn; // 8074E844: the fish field respawns on its next initSpawn

static u8 sSyncAcks[PLAY_SYNC_REC_NUM][4]; // 805FAF38: per record, per member

// 80160EA4
BOOL calcWeatherHoursA(f32 *hours) {
    if (dWeather_c::sInstance != NULL) {
        int weather = dWeather_c::sInstance->getCurrentPattern();
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
    if (dWeather_c::sInstance != NULL) {
        int weather = dWeather_c::sInstance->getCurrentPattern();
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
    if (dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_TOWN)) {
        if (dScene::isWinterSeasonType()) {
            check = TRUE;
        }
    } else if (dScene::isSnowSeason(&now)) {
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
    if (dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_TOWN)) {
        if (!dScene::isLateSeason()) {
            check = TRUE;
        }
    } else if (!dScene::isLateSeason(&now)) {
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
    if (rec->getMember() != dNet::getMyNo()) {
        rec->setState(PLAY_SYNC_STATE_CLAIMED);
        dPlaySyncMsg_c msg;
        msg.setType(0);
        msg.setIdx(idx);
        msg.setA(rec->getA());
        msg.setC(rec->getC());
        msg.setMember(rec->getMember());
        dNet::beginPacket();
        dNet::addPacket(&msg, sizeof(msg));
        dNet::sendPacket(NET_PACKET_PLAY_SYNC, NET_MEMBER_NONE);
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
    dNet::beginPacket();
    dNet::addPacket(&msg, sizeof(msg));
    dNet::sendPacket(NET_PACKET_PLAY_SYNC, NET_MEMBER_NONE);
    if (dNet::getMyNo() == 0) {
        recvSyncAck(&msg, 0);
    }
}

// 80161818
void claimSyncRec(dPlaySyncRec_c *rec, int idx) {
    if (!dNet::isSceneOwner()) {
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
        if (sSyncAcks[idx][i] == 0 && dNet::isMember(i)) {
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
        if (sSyncAcks[idx][i] == 0 && dNet::isMember(i)) {
            return;
        }
    }
    if (dScene::isSceneAttr(dScene::getCurrentScene(), dScene::SCENE_ATTR_TOWN) && dNet::isSceneOwner()) {
        dPlaySyncRecBuf_c buf;
        int id = msg->getIdx() + PLAY_SYNC_REC_ID;
        cLib::memCpy(&buf, dNet::getRecord(id), sizeof(dPlaySyncRec_c));
        buf.mRec.setState(PLAY_SYNC_STATE_NONE);
        dNet::setRecord(id, &buf, 0);
    } else {
        dPlaySyncMsg_c out;
        out.setType(4);
        out.setIdx(idx);
        out.setA(msg->getA());
        out.setC(msg->getC());
        out.setMember(msg->getMember());
        dNet::beginPacket();
        dNet::addPacket(&out, sizeof(out));
        dNet::sendPacket(NET_PACKET_PLAY_SYNC, NET_MEMBER_NONE);
    }
    for (u32 i = 0; i < 4; i++) {
        sSyncAcks[idx][i] = 0;
    }
}

// 80161B54
void releaseSyncRec(int idx) {
    int id = idx + PLAY_SYNC_REC_ID;
    void *src = dNet::getRecord(id);
    if (g_fishField != NULL) {
        dPlaySyncRec_c *rec = &g_fishField->mRecs[idx].mRec;
        if (rec->getMember() == dNet::getMyNo()) {
            rec->setState(PLAY_SYNC_STATE_NONE);
            dNet::setRecord(id, rec, 0);
        } else {
            dPlaySyncRecBuf_c buf;
            cLib::memCpy(&buf, src, sizeof(dPlaySyncRec_c));
            buf.mRec.setState(PLAY_SYNC_STATE_NONE);
            dNet::setRecord(id, &buf, 0);
        }
    } else {
        dPlaySyncRecBuf_c buf;
        cLib::memCpy(&buf, src, sizeof(dPlaySyncRec_c));
        buf.mRec.setMember(4);
        dNet::setRecord(id, &buf, 0);
        sendSyncRecRelease(&buf.mRec, idx);
    }
}
