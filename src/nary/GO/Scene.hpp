#pragma once

#include "naGameObject.hpp"
#include "naCamera.hpp"

#include <vector>
#include <memory>
#include <unordered_map>

namespace nary {

struct SceneNode {
    mathpls::mat4 absoluteModelMat{};

    naGameObject* gameObject{nullptr};
    SceneNode* parent{nullptr};
    std::vector<std::shared_ptr<SceneNode>> childern;

    SceneNode* addChild(naGameObject* object);
    void removeChild(SceneNode* child);
    void changeParent(SceneNode* newParent);
};

class Scene {
public:
    Scene() = default;

    SceneNode* getRoot() {return &m_RootNode;}
    const SceneNode* getRoot() const {return &m_RootNode;}

    void Update();

    naGameObject::id_t addGameObject(naGameObject&& object);
    naGameObject::id_t addGameObject(naGameObject&& object, naGameObject::id_t parent_id);
    void destoryGameObject(naGameObject::id_t id);
    void destoryGameObjectAndChildren(naGameObject::id_t id);
    naGameObject* getGameObject(naGameObject::id_t id);

    naGameObject::Map& getGameObjects();
    const naGameObject::Map& getGameObjects() const;
    
    naGameObject::id_t SetActiveCamera(std::unique_ptr<naCamera>&& camera);
    naGameObject::id_t SetActiveCamera(std::unique_ptr<naCamera>&& camera, naGameObject::id_t parent_id);
    naCamera* getActiveCamera() const;

    void changeParent(naGameObject::id_t id, naGameObject::id_t new_parent_id);
    void changeParentWithRoot(naGameObject::id_t id);

    mathpls::mat4 absoluteModelMat(naGameObject::id_t id) const;
private:
    SceneNode m_RootNode;
    std::unordered_map<naGameObject::id_t, SceneNode*> m_GoNodeMap; // 这会包含camera
    naGameObject::Map m_GameObjects;

    std::unique_ptr<naCamera> m_Camera; // 设计失误，camera没法放到GO里面，等上了ECS之后一并改吧
};

}