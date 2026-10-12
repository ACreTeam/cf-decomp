// SoundLoader: loads sound archive groups. .text 801E2C3C..801E2E58, .data 80503D58..80503DA0, .sdata
// 8074C738..8074C748. See include/game/snd/snd_manager.hpp.
#include <game/snd/snd_manager.hpp>
#include <lib/nw4r/snd/snd_TaskManager.h>

// 801E2C3C
SoundLoader::SoundLoader() : nw4r::snd::detail::Task() {
    mCancelled = false;
    mPlayer = NULL;
    mAllocatable = NULL;
    mGroupId = 0;
    mState = STATE_IDLE;
}

// 801E2C70
SoundLoader::~SoundLoader() {}

// 801E2CC8
void SoundLoader::loadGroup(u32 groupId, nw4r::snd::SoundArchivePlayer *player,
                            nw4r::snd::SoundMemoryAllocatable *allocatable) {
    while (mState != STATE_IDLE && mState != 3) {
    }
    mState = STATE_LOADING;
    player->LoadGroup(groupId, allocatable, 0);
    mState = STATE_IDLE;
}

// 801E2D28
bool SoundLoader::requestGroup(u32 groupId, nw4r::snd::SoundArchivePlayer *player,
                               nw4r::snd::SoundMemoryAllocatable *allocatable) {
    bool requested = false;
    if (mState == STATE_IDLE) {
        mPlayer = player;
        mAllocatable = allocatable;
        mGroupId = groupId;
        mState = STATE_QUEUED;
        nw4r::snd::detail::TaskManager::GetInstance().AppendTask(this, nw4r::snd::detail::TaskManager::PRIORITY_MIDDLE);
        requested = true;
    }
    return requested;
}

// 801E2D8C
void SoundLoader::restart() {
    if (mCancelled && mState == STATE_QUEUED) {
        nw4r::snd::detail::TaskManager::GetInstance().AppendTask(this, nw4r::snd::detail::TaskManager::PRIORITY_MIDDLE);
        mCancelled = false;
    }
}

// 801E2DE4
void SoundLoader::Execute() {
    mState = STATE_LOADING;
    mPlayer->LoadGroup(mGroupId, mAllocatable, 0);
    mPlayer = NULL;
    mAllocatable = NULL;
    mGroupId = 0;
    mState = STATE_IDLE;
}

// 801E2E3C
void SoundLoader::Cancel() {
    if (mState == STATE_QUEUED) {
        mCancelled = true;
    }
}

// 801E2E54
void SoundLoader::OnCancel() {}
