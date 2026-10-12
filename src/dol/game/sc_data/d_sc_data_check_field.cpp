#include <game/game/d_sc_data.hpp>

namespace dScData {

struct SceneDataStorage_c {
    dSceneData_c mScene;
    void *mExtra;
};

static SceneDataStorage_c l_scene;

} // namespace dScData

dSceneData_c *sScDataCheckField = &dScData::l_scene.mScene;
