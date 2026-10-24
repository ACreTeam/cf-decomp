#ifndef NW4R_SND_MOVE_VALUE_H
#define NW4R_SND_MOVE_VALUE_H

namespace nw4r {
namespace snd {
namespace detail {
template <typename TValue, typename TTime> class MoveValue {
  public:
    MoveValue()
        : mOrigin(TValue()), mTarget(TValue()), mFrame(TTime()),
          mCounter(TTime()) {
    }

    TValue GetValue() const {
        if (IsFinished()) {
            return mTarget;
        }

        return mOrigin + mCounter * (mTarget - mOrigin) / mFrame;
    }

    void SetTarget(TValue targetValue, TTime frames) {
        mOrigin = GetValue();
        mTarget = targetValue;
        mFrame = frames;
        mCounter = TTime();
    }

    bool IsFinished() const {
        return mCounter >= mFrame;
    }

    TTime GetRemainingTime() const {
        return IsFinished() ? 0 : mFrame - mCounter;
    }

    void InitValue(TValue value) {
        mOrigin = value;
        mTarget = value;
        mFrame = TTime();
        mCounter = TTime();
    }

    void Update() {
        if (mCounter < mFrame) {
            mCounter++;
        }
    }

  private:
    TValue mOrigin;
    TValue mTarget;
    TTime mFrame;
    TTime mCounter;
};
} // namespace detail
} // namespace snd
} // namespace nw4r

#endif
