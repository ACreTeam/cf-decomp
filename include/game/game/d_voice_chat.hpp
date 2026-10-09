#pragma once

#include <types.h>

// Wii Speak voice chat (namespace dVoiceChat). Source: src/dol/game/d_voice_chat.cpp
// (.text 801711EC..801732D4). There is no RTTI: every name here is inferred from behaviour and
// from the callers (d_net_dwc, d_net_mgr, both still asm). See notes/d_voice_chat.txt.
//
// It wires three libraries together:
//   - the DWC voice chat library (VCT, include/lib/dwc/vct.h): sessions with the other players
//     (up to 16 aids, one bit each in the client mask), audio encode / decode;
//   - the Wii Speak library (PMIC, include/lib/revolution/PMIC.h): the microphone, driven by a
//     small state machine (updateMic), with echo cancellation fed from the AI output;
//   - AX: the received audio is upsampled 4x (8 kHz -> 32 kHz) into a looping AX voice.
// A periodic alarm (frameAlarm) moves one audio frame each period: receive -> gain -> speaker
// buffer, microphone -> echo cancel -> send.
namespace dVoiceChat {
    BOOL init(u8 aid);                // 801717E8: VCT init with this aid (once)
    void cleanup();                   // 80171920
    BOOL start();                     // 8017195C: start the audio (needs init)
    void stop();                      // 801719EC
    s32 update();                     // 80171A2C: microphone state machine step
    void calc();                      // 80171A5C: VCT main, while started
    void connect(u8 aid);             // 80171A70
    void disconnect(u8 aid);          // 80171AE0
    BOOL isStarted();                 // 80171B08
    u16 getClientMask();              // 80171B10: one bit per connected aid
    u32 getFrameTime();               // 80171B18: the alarm period in ms
    void setParam(u8 value);          // 80171B2C: forwarded to fn_80368A60 (VCT)
    BOOL isMicConnected();            // 80171B50
    BOOL isMicActive();               // 80171B58: mic connected while started
    BOOL isMicShutDown();             // 80171B74: mic closed, or stopped after a failure
    void setVolume(int volume);       // 80171BB0: speaker gain = volume * 20 (0..200)
    void setMute(u8 mute);            // 80171BB8: mutes the echo-cancel reference
    void setMemberCount(u8 count);    // 80171BC0: picks the PMIC preset for 2..9 members
    BOOL fn_80171C08(int arg);        // returns TRUE
    BOOL fn_80171C10(u16 arg);        // returns FALSE
}
