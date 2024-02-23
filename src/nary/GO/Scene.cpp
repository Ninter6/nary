#include "Scene.hpp"

#include <cassert>
#include <queue>

namespace nary {

SceneNode* SceneNode::addChild(naGameObject* object) {
    childern.emplace_back(new SceneNode);
    childern.back()->parent = this;
    childern.back()->gameObject = object;
    return childern.back().get();
}

void SceneNode::removeChild(SceneNode* child) {
    auto it = std::find_if(childern.begin(), childern.end(), [&](auto&& i) {
        return i.get() == this;
    });
    assert(it != childern.end());
    childern.erase(it);
}

void SceneNode::changeParent(SceneNode* newParent) {
#ifndef NDEBUG
    auto p = newParent;
    while (p) {
        assert(p != this);
        p = p->parent;
    }
#endif // check if new parent is its children

    auto it = std::find_if(parent->childern.begin(), parent->childern.end(), [&](auto&& i){
        return i.get() == this;
    });
    assert(it != parent->childern.end());
    
    newParent->childern.push_back(*it);
    parent->childern.erase(it);

    parent = newParent;
}

void Scene::Update() {
    std::queue<SceneNode*> que;
    que.push(&m_RootNode);

    while (!que.empty()) {
        auto& node = que.front();
        if (node->parent) { // root 没有 parent
            if (node->parent->parent)
                node->absoluteModelMat = node->parent->absoluteModelMat * node->gameObject->transform().mat4();
            else // root 的子节点直接赋值
                node->absoluteModelMat = node->gameObject->transform().mat4();
        }
        for (auto& i : node->childern)
            que.push(i.get());
        que.pop();
    }
}


naGameObject::id_t Scene::addGameObject(naGameObject&& object) {
    object.scene = this;
    auto id = object.getId();
    auto [it, success] = m_GameObjects.emplace(id, std::move(object));

    assert(success);
    m_GoNodeMap.emplace(id, m_RootNode.addChild(&it->second));
    return id;
}

naGameObject::id_t Scene::addGameObject(naGameObject&& object, naGameObject::id_t parent_id) {
    object.scene = this;
    auto id = object.getId();
    auto [it, success] = m_GameObjects.emplace(id, std::move(object));

    assert(success);
    m_GoNodeMap.emplace(id, m_GoNodeMap.at(parent_id)->addChild(&it->second));
    return id;
}

void Scene::destoryGameObject(naGameObject::id_t id) {
    auto it = m_GoNodeMap.find(id);
    assert(it != m_GoNodeMap.end());

    auto node = it->second;
    for (auto& i : node->childern)
        i->changeParent(&m_RootNode);
    node->parent->removeChild(node);

    m_GoNodeMap.erase(it);
    m_GameObjects.erase(id);
}

void Scene::destoryGameObjectAndChildren(naGameObject::id_t id) {
    auto it = m_GoNodeMap.find(id);
    assert(it != m_GoNodeMap.end());

    auto node = it->second;
    node->parent->removeChild(node);

    m_GoNodeMap.erase(it);
    m_GameObjects.erase(id);
}

naGameObject* Scene::getGameObject(naGameObject::id_t id) {
    return &m_GameObjects.at(id);
}

naGameObject::Map& Scene::getGameObjects() {
    return m_GameObjects;
}

const naGameObject::Map& Scene::getGameObjects() const {
    return m_GameObjects;
}

naGameObject::id_t Scene::SetActiveCamera(std::unique_ptr<naCamera>&& camera) {
    m_Camera = std::move(camera);
    m_Camera->scene = this;

    m_GoNodeMap.emplace(m_Camera->getId(), m_RootNode.addChild(m_Camera.get()));
    return m_Camera->getId();
}

naGameObject::id_t Scene::SetActiveCamera(std::unique_ptr<naCamera>&& camera, naGameObject::id_t parent_id) {
    m_Camera = std::move(camera);
    m_Camera->scene = this;

    m_GoNodeMap.emplace(m_Camera->getId(), m_GoNodeMap.at(parent_id)->addChild(m_Camera.get()));
    return m_Camera->getId();
}

naCamera* Scene::getActiveCamera() const {
    return m_Camera.get();
}

void Scene::changeParent(naGameObject::id_t id, naGameObject::id_t new_parent_id) {
    m_GoNodeMap.at(id)->changeParent(m_GoNodeMap.at(new_parent_id));
}

void Scene::changeParentWithRoot(naGameObject::id_t id) {
    m_GoNodeMap.at(id)->changeParent(&m_RootNode);
}

mathpls::mat4 Scene::absoluteModelMat(naGameObject::id_t id) const {
    return m_GoNodeMap.at(id)->absoluteModelMat;
}

}