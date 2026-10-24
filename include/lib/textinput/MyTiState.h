#ifndef TEXTINPUT_MYTISTATE_H
#define TEXTINPUT_MYTISTATE_H

#include <lib/textinput/MyTiManager.h>

namespace textinput {
namespace extend {
namespace memo {

class DispMemoState : public State {
  public:
    virtual Manager::StateType getStateType() {
        return Manager::ST_Hidden;
    }
    virtual void create();
    virtual void init();
    virtual void draw();
    virtual void memoDraw();
    virtual void calc();
    virtual bool updateInput(int chan, f32 x, f32 y, u32 trig, u32 hold,
                             u32 release);
    virtual bool updateInput(input::HKBManager &hkbManager);
    virtual void start();
    virtual void end() {
    }
};

class AppearMemoState : public State {
  public:
    AppearMemoState() : mfCounter(0.0f) {
    }

    virtual Manager::StateType getStateType() {
        return Manager::ST_Appearing;
    }
    virtual void create();
    virtual void init();
    virtual void draw();
    virtual void memoDraw();
    virtual void calc();
    virtual bool updateInput(int chan, f32 x, f32 y, u32 trig, u32 hold,
                             u32 release);
    virtual bool updateInput(input::HKBManager &hkbManager);
    virtual void start();
    virtual void end() {
    }
    virtual ~AppearMemoState() {
    }

  protected:
    /* 0x08 */ f32 mfCounter;
}; // size 0xC

class EditMemoState : public State {
  public:
    virtual Manager::StateType getStateType() {
        return Manager::ST_Visible;
    }
    virtual void create();
    virtual void init();
    virtual void draw();
    virtual void memoDraw();
    virtual void calc();
    virtual bool updateInput(int chan, f32 x, f32 y, u32 trig, u32 hold,
                             u32 release);
    virtual bool updateInput(input::HKBManager &hkbManager);
    virtual void start();
    virtual void end() {
    }
};

class DisappearMemoState : public AppearMemoState {
  public:
    virtual Manager::StateType getStateType() {
        return Manager::ST_Disappearing;
    }
    virtual void calc();
    virtual void start();
};

} // namespace memo
} // namespace extend
} // namespace textinput

#endif
