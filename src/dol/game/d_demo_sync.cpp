// Demo sync between the consoles of a session (dDemoSync). .text 8008205C..8008278C, .bss 805839D8..80583A08,
// .sdata 80749E18. See include/game/game/d_demo_sync.hpp.
#include <game/game/d_demo_sync.hpp>
#include <cstring>
#include <game/game/d_demo_actor.hpp>
#include <game/game/d_net.hpp>
#include <game/game/d_player_mgr.hpp>
#include <game/game/d_post_office.hpp>

namespace dDemoSync {

static u32 s_demoIds[DEMO_SYNC_MEMBER_NUM];
static Pending_c s_pending[DEMO_SYNC_MEMBER_NUM];
static u8 s_state = DEMO_SYNC_IDLE;

#define HOST_MEMBER 0
#define SCENE_OWNER_MEMBER 5

// 8008205C
void init() {
    for (int i = 0; i < DEMO_SYNC_MEMBER_NUM; i++) {
        s_demoIds[i] = 0;
        s_pending[i].clear();
    }
    s_state = DEMO_SYNC_IDLE;
    dPostOffice::setSyncWait();
}

// 800820DC
int checkRequest(u32 demoId) {
    if (!dNet::isSessionActive()) {
        return DEMO_SYNC_CHECK_OK;
    }
    if (dNet::isHost()) {
        if (canStart(HOST_MEMBER, demoId)) {
            return DEMO_SYNC_CHECK_OK;
        }
        return DEMO_SYNC_CHECK_BUSY;
    }
    return DEMO_SYNC_CHECK_REQUEST;
}

// 80082148
void request(u32 demoId) {
    if (dNet::isSessionActive()) {
        if (dNet::isHost()) {
            s_demoIds[HOST_MEMBER] = demoId;
        } else {
            u8 packet[5];
            s_state = DEMO_SYNC_REQUEST;
            packet[0] = DEMO_SYNC_REQUEST;
            packet[1] = demoId;
            packet[2] = demoId >> 8;
            packet[3] = demoId >> 16;
            packet[4] = demoId >> 24;
            dNet::beginPacket();
            dNet::addPacket(packet, sizeof(packet));
            dNet::sendPacket(NET_PACKET_DEMO_SYNC, HOST_MEMBER);
        }
    }
}

// 800821D8
void release() {
    if (dNet::isSessionActive()) {
        if (dNet::isHost()) {
            releaseSlot(HOST_MEMBER);
        } else {
            sendCmd(DEMO_SYNC_RELEASE, HOST_MEMBER);
        }
    }
}

// 80082224
u8 getState() {
    return s_state;
}

// 8008222C
BOOL canStart(u8 member, u32 demoId) {
    if (s_demoIds[member] != 0) {
        return FALSE;
    }
    for (int i = 0; i < DEMO_SYNC_MEMBER_NUM; i++) {
        if (demoId == s_demoIds[i]) {
            return FALSE;
        }
    }
    return TRUE;
}

// 800822A8
void releaseSlot(u8 member) {
    s_demoIds[member] = 0;
}

// 800822C0
BOOL isRemote() {
    if (dNet::isSessionActive() == false) {
        return FALSE;
    }
    return dNet::isSceneOwner() == FALSE;
}

// 800822FC
void sendStart(u32 demoId, u8 check) {
    send(demoId, check, DEMO_SYNC_START);
}

// 80082304
void sendCommand(u32 demoId, u8 cmd) {
    send(demoId, cmd, DEMO_SYNC_COMMAND);
}

// 8008230C
void send(u32 demoId, u8 arg, u8 cmd) {
    if (dNet::isSessionActive()) {
        u8 packet[DEMO_SYNC_PACKET_SIZE];
        s_state = cmd;
        packet[0] = cmd;
        packet[1] = demoId;
        packet[2] = demoId >> 8;
        packet[3] = demoId >> 16;
        packet[4] = demoId >> 24;
        packet[5] = arg;
        dNet::beginPacket();
        dNet::addPacket(packet, sizeof(packet));
        dNet::sendPacket(NET_PACKET_DEMO_SYNC, SCENE_OWNER_MEMBER);
    }
}

// 8008239C
void receive(int member, const u8 *data, u32 size) {
    u8 cmd = data[0];
    switch (cmd) {
    case DEMO_SYNC_REQUEST: {
        u32 demoId = readDemoId(data);
        if (canStart(member, demoId)) {
            s_demoIds[member] = demoId;
            reply(DEMO_SYNC_ACCEPT, member);
        } else {
            reply(DEMO_SYNC_REJECT, member);
        }
        break;
    }
    case DEMO_SYNC_ACCEPT:
    case DEMO_SYNC_REJECT:
        s_state = cmd;
        break;
    case DEMO_SYNC_RELEASE:
        releaseSlot(member);
        break;
    case DEMO_SYNC_START:
    case DEMO_SYNC_COMMAND: {
        u32 demoId = readDemoId(data);
        u8 arg = data[5];
        dDemoActor_c *actor = dDemoActor_c::findByDemoID(demoId);
        dPlayerActor_c *player = dPlayerMgr::getPlayerActor(member);
        if (actor == NULL || player == NULL) {
            if (cmd == DEMO_SYNC_START) {
                if (member != dNet::getMyNo()) {
                    reply(DEMO_SYNC_REJECT, member);
                } else {
                    s_state = DEMO_SYNC_REJECT;
                }
            } else if (cmd == DEMO_SYNC_COMMAND) {
                storePending(member, data);
            }
        } else if (cmd == DEMO_SYNC_COMMAND) {
            actor->onDemoCmd(arg, member);
        } else {
            int ok = FALSE;
            switch (arg) {
            case DEMO_CMD_TALK:
                ok = actor->canInteract(player);
                break;
            case DEMO_CMD_SPEECH:
                ok = actor->canSpeech(player);
                break;
            case DEMO_CMD_SPEAK:
                ok = actor->canSpeak(player);
                break;
            }
            if (member != dNet::getMyNo()) {
                if (ok) {
                    actor->onDemoCmd(DEMO_CMD_START, member);
                    reply(DEMO_SYNC_ACCEPT, member);
                } else {
                    reply(DEMO_SYNC_REJECT, member);
                }
            } else {
                if (ok) {
                    actor->onDemoCmd(DEMO_CMD_START, DEMO_SYNC_MEMBER_SELF);
                    s_state = DEMO_SYNC_ACCEPT;
                } else {
                    s_state = DEMO_SYNC_REJECT;
                }
            }
        }
        break;
    }
    }
}

// 80082600
u32 readDemoId(const u8 *data) {
    return data[1] | (data[2] << 8) | (data[3] << 16) | (data[4] << 24);
}

// 80082624
void sendCmd(u8 cmd, int member) {
    u8 packet[1];
    packet[0] = cmd;
    dNet::beginPacket();
    dNet::addPacket(packet, sizeof(packet));
    dNet::sendPacket(NET_PACKET_DEMO_SYNC, member);
}

// 8008266C
void reply(u8 cmd, int member) {
    sendCmd(cmd, member);
}

// 80082670
void Pending_c::clear() {
    mValid = FALSE;
}

// 8008267C
void Pending_c::set(const u8 *data) {
    mValid = TRUE;
    memcpy(mData, data, DEMO_SYNC_PACKET_SIZE);
}

// 8008268C
BOOL Pending_c::get(u8 *out) const {
    if (!mValid) {
        return FALSE;
    }
    memcpy(out, mData, DEMO_SYNC_PACKET_SIZE);
    return TRUE;
}

// 800826D4
void storePending(int member, const u8 *data) {
    s_pending[member].set(data);
}

// 800826EC
void update() {
    for (int i = 0; i < DEMO_SYNC_MEMBER_NUM; i++) {
        u8 packet[DEMO_SYNC_PACKET_SIZE];
        if (s_pending[i].get(packet)) {
            s_pending[i].clear();
            if (dNet::isSceneOwner()) {
                receive(i, packet, sizeof(packet));
            } else {
                dNet::beginPacket();
                dNet::addPacket(packet, sizeof(packet));
                dNet::sendPacket(NET_PACKET_DEMO_SYNC, SCENE_OWNER_MEMBER);
            }
        }
    }
}

} // namespace dDemoSync
