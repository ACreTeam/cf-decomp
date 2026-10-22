#include <nw4r/snd.h>
#include <nw4r/ut.h>

namespace nw4r {
namespace snd {

FxReverbHiDpl2::FxReverbHiDpl2() : mIsActive(false) {
    ReverbHiDpl2Param param;
    SetParam(param);
}

u32 FxReverbHiDpl2::GetRequiredMemSize() {
    return ut::Max(
        ut::RoundUp(AXFXReverbHiExpGetMemSizeDpl2(&mAxfxParamDpl) +
                        detail::AxfxImpl::HEAP_SIZE_MIN,
                    32),
        ut::RoundUp(AXFXReverbHiExpGetMemSize(&mAxfxParam) +
                        detail::AxfxImpl::HEAP_SIZE_MIN,
                    32));
}

bool FxReverbHiDpl2::AssignWorkBuffer(void* pBuffer, u32 size) {
    return mImpl.CreateHeap(pBuffer, size);
}

void FxReverbHiDpl2::ReleaseWorkBuffer() {
    mImpl.DestroyHeap();
}

bool FxReverbHiDpl2::StartUp() {
    u32 memSize = GetRequiredMemSize();
    if (memSize > mImpl.GetHeapTotalSize()) {
        return false;
    }

    AXFXAllocHook allocHook;
    AXFXFreeHook freeHook;
    BOOL success;

    if (detail::AxManager::GetInstance().GetOutputMode() == OUTPUT_MODE_DPL2) {
        mOutputMode = 1;

        mImpl.HookAlloc(&allocHook, &freeHook);
        success = AXFXReverbHiExpInitDpl2(&mAxfxParamDpl);
        mImpl.RestoreAlloc(allocHook, freeHook);

        (void)AXFXReverbHiExpGetMemSizeDpl2(&mAxfxParamDpl); // debug leftover
    } else {
        mOutputMode = 0;

        mImpl.HookAlloc(&allocHook, &freeHook);
        success = AXFXReverbHiExpInit(&mAxfxParam);
        mImpl.RestoreAlloc(allocHook, freeHook);

        (void)AXFXReverbHiExpGetMemSize(&mAxfxParam); // debug leftover
    }

    mIsActive = true;
    return success;
}

void FxReverbHiDpl2::Shutdown() {
    if (!mIsActive) {
        return;
    }

    AXFXAllocHook allocHook;
    AXFXFreeHook freeHook;

    mIsActive = false;
    mImpl.HookAlloc(&allocHook, &freeHook);

    if (mOutputMode == 1) {
        AXFXReverbHiExpShutdownDpl2(&mAxfxParamDpl);
    } else {
        AXFXReverbHiExpShutdown(&mAxfxParam);
    }

    mImpl.RestoreAlloc(allocHook, freeHook);
}

bool FxReverbHiDpl2::SetParam(const ReverbHiDpl2Param& rParam) {
    mParam = rParam;

    f32 preDelayTimeMax = ut::Max(rParam.preDelayTimeMax, 0.0f);
    bool changed = preDelayTimeMax != mAxfxParam.preDelayTimeMax;

    mAxfxParam.earlyMode = mAxfxParamDpl.earlyMode = rParam.earlyMode;
    mAxfxParam.preDelayTimeMax = mAxfxParamDpl.preDelayTimeMax =
        preDelayTimeMax;

    mAxfxParam.preDelayTime = mAxfxParamDpl.preDelayTime =
        ut::Clamp(rParam.preDelayTime, 0.0f, rParam.preDelayTimeMax);
    mAxfxParam.fusedMode = mAxfxParamDpl.fusedMode = rParam.fusedMode;
    mAxfxParam.fusedTime = mAxfxParamDpl.fusedTime =
        ut::Max(rParam.fusedTime, 0.0f);
    mAxfxParam.coloration = mAxfxParamDpl.coloration =
        ut::Clamp(rParam.coloration, 0.0f, 1.0f);
    mAxfxParam.damping = mAxfxParamDpl.damping =
        ut::Clamp(rParam.damping, 0.0f, 1.0f);
    mAxfxParam.crosstalk = mAxfxParamDpl.crosstalk =
        ut::Clamp(rParam.crossTalk, 0.0f, 1.0f);
    mAxfxParam.earlyGain = mAxfxParamDpl.earlyGain =
        ut::Clamp(rParam.earlyGain, 0.0f, 1.0f);
    mAxfxParam.fusedGain = mAxfxParamDpl.fusedGain =
        ut::Clamp(rParam.fusedGain, 0.0f, 1.0f);
    mAxfxParam.outGain = mAxfxParamDpl.outGain =
        ut::Clamp(rParam.outGain, 0.0f, 1.0f);

    mAxfxParam.busIn = NULL;
    mAxfxParam.busOut = NULL;
    mAxfxParam.sendGain = 0.0f;

    mAxfxParamDpl.busIn = NULL;
    mAxfxParamDpl.busOut = NULL;
    mAxfxParamDpl.sendGain = 0.0f;

    if (!mIsActive) {
        return true;
    }

    u32 memSize = GetRequiredMemSize();
    if (memSize > mImpl.GetHeapTotalSize()) {
        return false;
    }

    AXFXAllocHook allocHook;
    AXFXFreeHook freeHook;
    BOOL success;

    if (changed) {
        mImpl.HookAlloc(&allocHook, &freeHook);

        if (mOutputMode == 1) {
            success = AXFXReverbHiExpSettingsDpl2(&mAxfxParamDpl);
        } else {
            success = AXFXReverbHiExpSettings(&mAxfxParam);
        }

        mImpl.RestoreAlloc(allocHook, freeHook);
    } else {
        if (mOutputMode == 1) {
            success = AXFXReverbHiExpSettingsUpdateDpl2(&mAxfxParamDpl);
        } else {
            success = AXFXReverbHiExpSettingsUpdate(&mAxfxParam);
        }
    }

    return success;
}

void FxReverbHiDpl2::UpdateBuffer(int channels, void** ppBuffer, u32 size,
                                  SampleFormat format, f32 sampleRate,
                                  OutputMode mode) {
#pragma unused(channels)
#pragma unused(size)
#pragma unused(format)
#pragma unused(sampleRate)

    if (!mIsActive) {
        return;
    }

    if ((mode == OUTPUT_MODE_DPL2) != mOutputMode) {
        return;
    }

    if (mOutputMode == 1) {
        AXFX_BUS_DPL2 update;
        update.L = static_cast<s32 *>(ppBuffer[AX_DPL2_L]);
        update.R = static_cast<s32 *>(ppBuffer[AX_DPL2_R]);
        update.Ls = static_cast<s32 *>(ppBuffer[AX_DPL2_LS]);
        update.Rs = static_cast<s32 *>(ppBuffer[AX_DPL2_RS]);
        AXFXReverbHiExpCallbackDpl2(&update, &mAxfxParamDpl);
    } else {
        AXFX_BUS update;
        update.left = static_cast<s32*>(ppBuffer[0]);
        update.right = static_cast<s32*>(ppBuffer[1]);
        update.surround = static_cast<s32*>(ppBuffer[2]);
        AXFXReverbHiExpCallback(&update, &mAxfxParam);
    }
}

void FxReverbHiDpl2::OnChangeOutputMode() {
    if (mOutputMode !=
        (detail::AxManager::GetInstance().GetOutputMode() == OUTPUT_MODE_DPL2)) {
        Shutdown();
        StartUp();
    }
}

} // namespace snd
} // namespace nw4r
