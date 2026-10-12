// Furniture sounds. .text 801E1C78..801E2C3C, .rodata 8047F100..8047F980, .data 80503CD0..80503D58, .bss
// 80625DC8..80625DD8, .sdata 8074C708..8074C738, .sdata2 80751D00..80751D18. See include/game/snd/snd_obj.hpp.
#include <game/snd/snd_obj.hpp>
#include <game/snd/snd_manager.hpp>
#include <game/snd/snd_id.hpp>

// Per furniture sound item.
static const SoundObjFurniture::Data sData[152] = {
    {0, 0, 0x0000, 0}, // 0
    {SE_SOFA_SIT, SE_SOFA_STANDUP, 0x0000, 0}, // 1
    {SE_HikidashiOpen, SE_HikidashiClose, 0x0000, 0}, // 2
    {SE_closet_open, SE_closet_close, 0x0000, 0}, // 3
    {SE_BU_BU_SIT, SE_BU_BU_STANDUP, 0x0000, 0}, // 4
    {SE_FtrSwitchOn, SE_FtrSwitchOff, 0x0000, 0}, // 5
    {SE_KeikoutouOn, SE_KeikoutouOff, 0x0000, 0}, // 6
    {SE_candle_on, SE_candle_off, 0x0000, 0}, // 7
    {SE_CHAIR_SIT, SE_CHAIR_STANDUP, 0x0000, 0}, // 8
    {SE_BED_IN, SE_BED_OUT, 0x0000, 0}, // 9
    {SE_Inugoya, 0, 0x0002, 0}, // 10
    {SE_MasterSword, SE_MasterSword, 0x0000, 0}, // 11
    {SE_StarFox, SE_StarFox, 0x0000, 0}, // 12
    {SE_TriForce, 0, 0x0002, 0}, // 13
    {SE_DrumCan_Chairo, SE_DrumCan_Chairo, 0x0000, 0}, // 14
    {SE_DrumCan_Midori, SE_DrumCan_Midori, 0x0000, 0}, // 15
    {SE_DrumCan_Aka, SE_DrumCan_Aka, 0x0000, 0}, // 16
    {SE_DrumCan_Kiken, SE_DrumCan_Kiken, 0x0000, 0}, // 17
    {SE_DrumCan_Kiiro, SE_DrumCan_Kiiro, 0x0000, 0}, // 18
    {SE_DanshiyouToilet, 0, 0x0002, 0}, // 19
    {SE_Pikumin, 0, 0x0002, 0}, // 20
    {SE_okiagari, 0, 0x0002, 0}, // 21
    {SE_chokinbako, SE_chokinbako, 0x0002, 0}, // 22
    {SE_gong, 0, 0x0002, 0}, // 23
    {SE_mario_coin, SE_mario_coin, 0x0002, 0}, // 24
    {SE_mario_dokan, SE_mario_dokan, 0x0000, 0}, // 25
    {SE_mario_fire_flower, SE_mario_fire_flower, 0x0000, 0}, // 26
    {SE_mario_flag, SE_mario_flag, 0x0000, 0}, // 27
    {SE_mario_hatena_block, SE_mario_hatena_block, 0x0000, 0}, // 28
    {SE_mario_kinoko, SE_mario_kinoko, 0x0000, 0}, // 29
    {SE_mario_koura, SE_mario_koura, 0x0000, 0}, // 30
    {SE_mario_taihou, SE_mario_taihou, 0x0000, 0}, // 31
    {SE_Pinball, SE_Pinball, 0x0000, 0}, // 32
    {SE_sandbag, 0, 0x0002, 0}, // 33
    {SE_bowling, 0, 0x0002, 0}, // 34
    {SE_Totempole_ara, SE_Totempole_ara, 0x0000, 0}, // 35
    {SE_Totempole_oya, SE_Totempole_oya, 0x0000, 0}, // 36
    {SE_Totempole_sate, SE_Totempole_sate, 0x0000, 0}, // 37
    {SE_Totempole_maa, SE_Totempole_maa, 0x0000, 0}, // 38
    {SE_Chikyuugi, 0, 0x0000, 0}, // 39
    {SE_Mario_1up_kinoko, 0, 0x0000, 0}, // 40
    {SE_SpeedBag, 0, 0x0002, 0}, // 41
    {SE_Kyuuyuki, 0, 0x1007, SE_KyuuyukiLev}, // 42
    {SE_Teoshiguruma, 0, 0x0000, 0}, // 43
    {SE_TOILET_SIT, SE_TOILET_STANDUP, 0x0000, 0}, // 44
    {SE_Anmaisu_sit, SE_Anmaisu_standup, 0x0000, SE_Anmaisu}, // 45
    {SE_Jikkenisu_Sit, SE_Jikkenisu_Standup, 0x0000, SE_Jikkenisu}, // 46
    {SE_Jikkendai_In, SE_Jikkendai_Out, 0x0000, SE_Jikkendai}, // 47
    {0, 0, 0x0000, SE_Soratobuenban}, // 48
    {0, 0, 0x0000, SE_bbq}, // 49
    {0, 0, 0x0000, SE_hamster}, // 50
    {0, 0, 0x0000, SE_torch}, // 51
    {0, 0, 0x0000, SE_tori_room}, // 52
    {0, 0, 0x0000, SE_Takibi}, // 53
    {0, 0, 0x0000, SE_CampFire}, // 54
    {0, 0, 0x0008, SE_Irori}, // 55
    {0, 0, 0x0000, SE_DarumaStove}, // 56
    {0, 0, 0x0000, SE_Stove}, // 57
    {0, 0, 0x0000, SE_Danro}, // 58
    {0, 0, 0x0000, SE_pool}, // 59
    {0, 0, 0x0001, SE_Mario_KoopaStageBgm}, // 60
    {0, 0, 0x0001, SE_furui_mishin}, // 61
    {0, 0, 0x0001, SE_mario_star}, // 62
    {0, 0, 0x0000, SE_MerryGoRound}, // 63
    {0, 0, 0x0001, SE_Metroid}, // 64
    {0, 0, 0x0001, SE_UprightGameKi}, // 65
    {SE_Shibakariki, 0, 0x0002, 0}, // 66
    {SE_MaaraionJyaguchi_open, SE_MaaraionJyaguchi_close, 0x0000, SE_maaraion}, // 67
    {SE_FtrSwitchOn, SE_FtrSwitchOff, 0x0000, SE_MOKEI}, // 68
    {SE_FtrSwitchOn, SE_FtrSwitchOff, 0x0001, SE_SENPUUKI}, // 69
    {SE_SprinklerJyaguchi_open, SE_SprinklerJyaguchi_close, 0x0000, SE_sprinkler}, // 70
    {SE_SentakuNisoushikiFutaOpen, SE_SentakuNisoushikiFutaClose, 0x0000, SE_SentakuNisoushiki1}, // 71
    {SE_SentakuZenjidouFutaOpen, SE_SentakuZenjidouFutaClose, 0x0000, SE_SentakuZenjidou}, // 72
    {SE_FtrSwitchOn, SE_FtrSwitchOff, 0x0001, SE_Mixer}, // 73
    {0, 0, 0x0000, SE_Rotenburo}, // 74
    {SE_Sugosounakikai_On, 0, 0x1207, SE_Sugosounakikai}, // 75
    {SE_ShoubenkozouJyaguchi_open, SE_ShoubenkozouJyaguchi_close, 0x0000, SE_Shoubenkozou}, // 76
    {SE_CookingHeaterOn, SE_CookingHeaterOff, 0x0000, SE_CookingHeater}, // 77
    {SE_KitchenSink_Jyaguchi_open, SE_KitchenSink_Jyaguchi_close, 0x0000, SE_KitchenSink}, // 78
    {0, SE_SentakuDrumOff, 0x0001, SE_SentakuDrum}, // 79
    {SE_Range_Start, SE_Range_AutoStop, 0x0003, SE_Range_Lev}, // 80
    {0, 0, 0x0000, SE_room_mitsubachi}, // 81
    {0, 0, 0x0000, SE_room_suzumebachi}, // 82
    {0, 0, 0x0000, SE_room_aburazemi}, // 83
    {0, 0, 0x0000, SE_room_minminzemi}, // 84
    {0, 0, 0x0000, SE_room_higurashi}, // 85
    {0, 0, 0x0000, SE_room_mosqueto}, // 86
    {0, 0, 0x0000, SE_room_hae}, // 87
    {SE_ReizoukoOpen, SE_ReizoukoClose, 0x0000, 0}, // 88
    {SE_takarabako_open, SE_takarabako_close, 0x0002, 0}, // 89
    {SE_hitsugi_open, SE_hitsugi_close, 0x0000, 0}, // 90
    {SE_KinkoOpen, SE_KinkoClose, 0x0000, 0}, // 91
    {SE_Daruma_write, SE_Daruma_erase, 0x0000, 0}, // 92
    {SE_ZipperOpen, SE_ZipperClose, 0x0000, 0}, // 93
    {SE_Daruma_write2, SE_Daruma_erase2, 0x0000, 0}, // 94
    {SE_Daruma_write3, SE_Daruma_erase3, 0x0000, 0}, // 95
    {SE_Ninjyatou2, SE_Ninjyatou1, 0x0002, 0}, // 96
    {SE_PersonalComputer_On, SE_PersonalComputer_Off, 0x0002, 0}, // 97
    {SE_WoodChair_Sit, SE_WoodChair_Standup, 0x0000, 0}, // 98
    {SE_tokei, 0, 0x0000, 0}, // 99
    {SE_tokei, 0, 0x0000, 0}, // 100
    {SE_hatodokei, SE_tokei, 0x0000, 0}, // 101
    {SE_bonbondokei, 0, 0x1502, 0}, // 102
    {SE_FtrSwitchOn, SE_FtrSwitchOff, 0x0000, 0}, // 103
    {SE_TerevideoOn, SE_TerevideoOff, 0x0000, 0}, // 104
    {SE_FzeroON, 0, 0x1705, 0}, // 105
    {0, 0, 0x0F02, 0}, // 106
    {0, 0, 0x0001, SE_record}, // 107
    {SE_orgel_open, 0, 0x1102, SE_orgel}, // 108
    {0, 0, 0x0102, 0}, // 109
    {0, 0, 0x0202, 0}, // 110
    {0, 0, 0x0302, 0}, // 111
    {0, 0, 0x0402, 0}, // 112
    {0, 0, 0x0502, 0}, // 113
    {0, 0, 0x0602, 0}, // 114
    {0, 0, 0x0702, 0}, // 115
    {0, 0, 0x0802, 0}, // 116
    {0, 0, 0x0902, 0}, // 117
    {0, 0, 0x0A02, 0}, // 118
    {0, 0, 0x0B02, 0}, // 119
    {0, 0, 0x0C02, 0}, // 120
    {0, 0, 0x0D03, 0}, // 121
    {0, 0, 0x0E02, 0}, // 122
    {0, 0, 0x0011, SE_record}, // 123
    {0, 0, 0x0020, 0}, // 124
    {0, 0, 0x0031, SE_record}, // 125
    {0, 0, 0x0040, 0}, // 126
    {0, 0, 0x0050, 0}, // 127
    {0, 0, 0x0060, 0}, // 128
    {0, 0, 0x0070, 0}, // 129
    {0, 0, 0x0080, 0}, // 130
    {SE_Ubaguruma, 0, 0x0000, 0}, // 131
    {SE_MetalHikidashiOpen, SE_MetalHikidashiClose, 0x0000, 0}, // 132
    {0, 0, 0x1400, 0}, // 133
    {SE_Royal_BED_IN, SE_Royal_BED_OUT, 0x0000, 0}, // 134
    {0, 0, 0x0000, SE_room_shouryoubatta}, // 135
    {0, 0, 0x1300, 0}, // 136
    {0, 0, 0x0000, SE_room_tsukutsukuhoushi}, // 137
    {SE_ChairMove, 0, 0x0000, 0}, // 138
    {0, 0, 0x0000, SE_Yubune}, // 139
    {SE_Rocket, SE_Rocket, 0x0000, 0}, // 140
    {SE_Shinjitsunokuchi, 0, 0x0002, 0}, // 141
    {0, 0, 0x0000, SE_Kakigooriki}, // 142
    {SE_YossiEgg, 0, 0x0002, 0}, // 143
    {SE_BirthdayCake_On, SE_BirthdayCake_Off, 0x0000, 0}, // 144
    {SE_UranaiTelephone_On, SE_UranaiTelephone_Off, 0x0000, 0}, // 145
    {SE_SnowDome, 0, 0x0002, 0}, // 146
    {SE_ShiroiNihontou1, 0, 0x1602, 0}, // 147
    {SE_Harikonotora, 0, 0x0002, 0}, // 148
    {0, 0, 0x0000, 0}, // 149
    {0, 0, 0x0000, 0}, // 150
    {SE_CoolerBoxOpen, SE_CoolerBoxClose, 0x0000, 0}, // 151
};

// Rows of frame sounds (Data::mFlags & FLAG_FRAMES).
static const SoundObjFurniture::FrameSound sFrameSounds[24][10] = {
    {}, // 0
    {{0x0147, SE_SHISHIODOSHI}}, // 1
    {{0x0012, SE_MizunomidoriDown}, {0x004E, SE_MizunomidoriUp}}, // 2
    {{0x4012, SE_AmericanCracker}, {0x4039, SE_AmericanCracker}}, // 3
    {{0x0004, SE_gomibako1}, {0x001E, SE_gomibako2}, {0x003A, SE_gomibako3}, {0x0058, SE_gomibako4}, {0x0076, SE_gomibako5}, {0x0094, SE_gomibako6}, {0x00B2, SE_gomibako7}}, // 4
    {{0x400B, SE_roshianingyou}, {0x401C, SE_roshianingyou}, {0x8011, SE_roshianingyou}, {0x8022, SE_roshianingyou}}, // 5
    {{0x0002, SE_ToasterDown}, {0x0075, SE_ToasterUp}}, // 6
    {{0x0001, SE_CandyMachine1}, {0x000F, SE_CandyMachine2}}, // 7
    {{0x0009, SE_FurnitureRegister2}, {0x0033, SE_FurnitureRegister1}, {0x005A, SE_FurnitureRegister3}, {0x0073, SE_FurnitureRegister2}}, // 8
    {{0x4001, SE_Yokuwakaranaikikai_On}, {0x803B, SE_Yokuwakaranaikikai_Off}}, // 9
    {{0x0002, SE_Shiryokusokuteiki1}, {0x0052, SE_Shiryokusokuteiki2}, {0x00A2, SE_Shiryokusokuteiki3}}, // 10
    {{0x0002, SE_Yurikago1}, {0x0010, SE_Yurikago2}, {0x002A, SE_Yurikago3}, {0x0052, SE_Yurikago4}, {0x0070, SE_Yurikago5}, {0x0084, SE_Yurikago6}}, // 11
    {{0x0001, SE_BowlingReturn1}, {0x00A3, SE_BowlingReturn2}}, // 12
    {{0x4015, SE_METRANOME_LOW}, {0x403D, SE_METRANOME_LOW}}, // 13
    {{0x0000, SE_Mugibatake}, {0x0014, SE_Mugibatake}}, // 14
    {{0x0002, SE_bikkuribako}, {0x008A, SE_bikkuribako_close}}, // 15
    {{0x00E8, SE_KyuuyukiEnd}}, // 16
    {{0x8005, SE_orgel_close}}, // 17
    {{0x8064, SE_Sugosounakikai_Off}}, // 18
    {{0x0005, SE_room_okera}}, // 19
    {{0x4004, SE_LockerOpen}, {0x8017, SE_LockerClose}}, // 20
    {{0x201A, SE_tokei}, {0x2056, SE_tokei}}, // 21
    {{0x0050, SE_ShiroiNihontou2}}, // 22
    {{0x00A0, SE_FzeroOFF}}, // 23
};

// The labels of a furniture's own sequence data.
static const char *sLabels[] = {"se0", "se1", "se2", "se3", "se4", "se5", "se6", "se7", "se8", "se9"};

// 801E1C78
SoundObjFurniture::SoundObjFurniture()
    : mOn(false), mLoopOnce(false), mCount(-1), mCycles(-1), mDir(DIR_NONE), mFrame(-1.0f), mSeqId(0), mLabel(0),
      mData(NULL), mFrames(NULL), mSeq(NULL) {}

// 801E1D30
SoundObjFurniture::~SoundObjFurniture() {
    stopAll(0);
}

// 801E1DC4
void SoundObjFurniture::startSound(u32 id, const nw4r::snd::SoundStartable::StartInfo *startInfo) {
    int stage = SoundManager::getInstance()->_778;
    if (stage < 0x27 || stage > 0x2B) {
        SoundObj<1>::startSound(id, startInfo);
    }
}

// 801E1F48
void SoundObjFurniture::holdSound(u32 id) {
    int stage = SoundManager::getInstance()->_778;
    if (stage < 0x27 || stage > 0x2B) {
        SoundObj<1>::holdSound(id);
    }
}

// 801E20D4
void SoundObjFurniture::setup(u8 item, bool on) {
    mData = &sData[item];
    mOn = on;
    mCount = 0;
    mCycles = -1;
    mFrame = -1.0f;
    if (mData != NULL) {
        mLoopOnce = false;
        if (mData->mFlags & FLAG_LOOP_ONCE) {
            mLoopOnce = true;
        }
        int row = (mData->mFlags & FLAG_FRAMES) >> 8;
        if (row > 0) {
            mFrames = sFrameSounds[row];
        }
    }
}

// 801E2148
void SoundObjFurniture::setupSeq(const SeqData *data, const void *seq, bool on) {
    mSeq = seq;
    mData = &data->mData;
    mOn = on;
    mCount = 0;
    mCycles = -1;
    mFrame = -1.0f;
    if (mData != NULL) {
        mLoopOnce = false;
        if (mData->mFlags & FLAG_LOOP_ONCE) {
            mLoopOnce = true;
        }
        if (data->mFrames[0].mSound != 0) {
            mFrames = data->mFrames;
        }
        mSeqId = data->mSeqId;
    }
}

// 801E21AC
void SoundObjFurniture::update(const nw4r::math::VEC3 *pos, f32 frame) {
    SoundObj<1>::update(pos);
    if (mData == NULL) {
        return;
    }

    mDir = calcDir(mFrame, frame);
    if (mDir == DIR_WRAP_FORWARD || mDir == DIR_WRAP_BACKWARD) {
        mCycles++;
    }

    u16 loop = mData->mLoopSound;
    if (loop == SE_orgel) {
        holdSound(loop);
        mHandles[0].Pause(!mOn, 0);
    } else if (loop != 0) {
        if (mOn) {
            holdSound(loop);
        } else if (mDir != DIR_NONE && !mLoopOnce) {
            holdSound(loop);
        } else if (mData->mFlags & FLAG_LOOP_ALWAYS) {
            holdSound(loop);
        }
    }

    const FrameSound *entry = mFrames;
    if (entry != NULL && mDir != DIR_NONE) {
        u16 sound = 0;
        for (int i = 0; i < 10; i++, entry++) {
            if (entry->mSound == 0) {
                break;
            }
            u32 at = entry->mFrame & 0x1FFF;
            u32 when = entry->mFrame & 0xC000;
            if (mDir == DIR_FORWARD && when != 0x8000) {
                if (mFrame < at && at <= frame) {
                    sound = entry->mSound;
                    break;
                }
            } else if (mDir == DIR_BACKWARD && when != 0x4000) {
                if (frame <= at && at < mFrame) {
                    sound = entry->mSound;
                    break;
                }
            } else if (mDir == DIR_WRAP_FORWARD && when != 0x8000) {
                if (at <= frame || at > mFrame) {
                    sound = entry->mSound;
                    if (sound == SE_METRANOME_LOW) {
                        sound = 0;
                    }
                    break;
                }
            } else if (mDir == DIR_WRAP_BACKWARD && when != 0x4000) {
                if (at < mFrame || at >= frame) {
                    sound = entry->mSound;
                    break;
                }
            }
        }

        if (sound != 0 && !(mHandles[0].IsAttachedSound() && (entry->mFrame & 0x2000))) {
            mCount++;
            if (sound == SE_METRANOME_LOW && mCount % 4 == 0) {
                mCount = 0;
                sound = SE_METRANOME_HI;
            }
            if (sound & 0x8000) {
                playLabel(sound & 0x7FFF);
            } else if (sound != mHandles[0].GetId()) {
                if (sound != SE_orgel_close) {
                    startSound(sound, NULL);
                } else {
                    nw4r::snd::SoundHandle handle;
                    SoundManager::getInstance()->startSound(&handle, SE_orgel_close);
                    calcParam(&mPos, 0);
                    handle.SetVolume(mPosVolume * mVolume, 0);
                    handle.SetPan(mPan);
                    handle.SetSurroundPan(mSurroundPan);
                }
            } else if (mData->mFlags & FLAG_RESTART) {
                mHandles[0].Stop(0);
                startSound(sound, NULL);
            }
            if (mData->mFlags & FLAG_OFF_AFTER) {
                turnOff();
            }
        }
    }
    mFrame = frame;
}

// 801E2744
void SoundObjFurniture::turnOn() {
    u32 sound = mData->mOnSound;
    if (sound != 0) {
        if (sound != SE_orgel_open) {
            if (sound != mHandles[0].GetId() && sound != SE_Range_Start) {
                startSound(sound, NULL);
            } else if (mData->mFlags & FLAG_RESTART) {
                mHandles[0].Stop(0);
                startSound(sound, NULL);
            }
        } else {
            nw4r::snd::SoundHandle handle;
            SoundManager::getInstance()->startSound(&handle, SE_orgel_open);
            calcParam(&mPos, 0);
            handle.SetVolume(mPosVolume * mVolume, 0);
            handle.SetPan(mPan);
            handle.SetSurroundPan(mSurroundPan);
        }
    }
    mOn = true;
    if ((mData->mFlags & FLAG_FRAMES) != 0xD00) {
        mCount = 0;
    }
}

// 801E29E8
void SoundObjFurniture::turnOff() {
    u16 sound = mData->mOffSound;
    if (sound != 0) {
        if (sound != mHandles[0].GetId()) {
            startSound(sound, NULL);
        } else if (mData->mFlags & FLAG_RESTART) {
            mHandles[0].Stop(0);
            startSound(sound, NULL);
        }
    }
    mOn = false;
}

// 801E2AA8
int SoundObjFurniture::getType() {
    int type = 0;
    if (mData != NULL) {
        type = (mData->mFlags & FLAG_TYPE) >> 4;
    }
    return type;
}

// 801E2AC4
int SoundObjFurniture::calcDir(f32 prevFrame, f32 frame) {
    int dir = DIR_NONE;
    if (prevFrame > frame) {
        if (prevFrame - frame > 2.0f) {
            if (frame <= 2.0f) {
                dir = DIR_WRAP_FORWARD;
            } else {
                dir = DIR_JUMP;
            }
        } else {
            dir = DIR_BACKWARD;
        }
    } else if (prevFrame < frame) {
        if (frame - prevFrame > 2.0f) {
            if (prevFrame <= 2.0f) {
                dir = DIR_WRAP_BACKWARD;
            } else {
                dir = DIR_JUMP;
            }
        } else {
            dir = DIR_FORWARD;
        }
    }
    if (dir == DIR_NONE && mDir != DIR_JUMP && mDir != DIR_NONE) {
        dir = DIR_JUMP;
    }
    return dir;
}

// 801E2B60
void SoundObjFurniture::playLabel(int label) {
    const void *seq = mSeq;
    const char *name = sLabels[label];
    nw4r::snd::SoundStartable::StartInfo info;
    info.seqSoundInfo.startLocationLabel = NULL;
    info.enableFlag = nw4r::snd::SoundStartable::StartInfo::ENABLE_SEQ_SOUND_INFO;
    info.seqSoundInfo.seqDataAddress = seq;
    info.seqSoundInfo.startLocationLabel = name;
    if (!mHandles[0].IsAttachedSound() || label != mLabel) {
        startSound(mSeqId, &info);
        mLabel = label;
    } else if (mData->mFlags & FLAG_RESTART) {
        mHandles[0].Stop(0);
        startSound(mSeqId, &info);
        mLabel = label;
    }
}
