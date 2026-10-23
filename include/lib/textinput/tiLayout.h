#pragma once

#include <nw4r/lyt.h>

namespace textinput {

class AnimTransformPane : public nw4r::lyt::AnimTransformBasic {
  public:
    virtual ~AnimTransformPane() {
    }
    virtual void SetResource(const nw4r::lyt::res::AnimationBlock *pRes,
                             nw4r::lyt::ResourceAccessor *pResAccessor);
    virtual void Bind(nw4r::lyt::Pane *pane, bool bRecursive, bool bDisable);
    virtual void Bind(nw4r::lyt::Material *pMaterial);

    void ForceBind(nw4r::lyt::Pane *pane, const char *name);

  private:
    u32 CountAnimForPane_(const char *name);
    u32 CountAnimForPane_(nw4r::lyt::Pane *pane, bool bRecursive);
};

class TiLayout : public nw4r::lyt::Layout {
  public:
    virtual ~TiLayout() {
    }
    virtual nw4r::lyt::AnimTransform *
    CreateAnimTransform(const void *anmResBuf,
                        nw4r::lyt::ResourceAccessor *pResAcsr);
};

} // namespace textinput
