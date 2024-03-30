//
//  GOComponent.hpp
//  pxpls
//
//  Created by Ninter6 on 2023/11/4.
//

#pragma once

#include "ResourceManager.hpp"

#include <memory>
#include <functional>

#include "mathpls.h"

namespace nary {

class naGameObject;

class Component {
public:
    Component(naGameObject* obj) : m_Obj(obj) {}
    virtual ~Component() = default;
    
    naGameObject& gameObject() {return *m_Obj;}
    
private:
    naGameObject* m_Obj;
    
    friend naGameObject;
};

class ComponentWapper {
public:
    template <class T, class = std::enable_if_t<std::is_base_of_v<Component, T>>>
    ComponentWapper(T* compo) : m_Compo(compo) {
        copy_fn = [](Component* op) -> Component* {
            return new T{*dynamic_cast<T*>(op)};
        };
    }
    ComponentWapper(const ComponentWapper& o);
    ComponentWapper& operator=(const ComponentWapper& o);
    ComponentWapper(ComponentWapper&& o) = default;
    ComponentWapper& operator=(ComponentWapper&& o) = default;
    
    Component* get() const {return m_Compo.get();}
    Component* copy() const {return copy_fn(get());}
    
private:
    std::unique_ptr<Component> m_Compo;
    
    std::function<Component*(Component*)> copy_fn;
};

struct TransformComponent : public Component {
    TransformComponent(naGameObject* obj) : Component(obj) {}
    
    mathpls::vec3 translation{}; // position offset
    mathpls::vec3 scale{1.f};
    mathpls::vec3 rotation{};
    
    mathpls::vec3 Forward() const {
        mathpls::vec3 Forward{};
        auto Pitch = rotation.x, Yaw = rotation.y;
        Forward.x = std::cos(Pitch) * std::sin(Yaw);
        Forward.y =-std::sin(Pitch);
        Forward.z = std::cos(Pitch) * std::cos(Yaw);
        return Forward;
    }
    
    mathpls::vec3 Up() const {
        return {-sin(rotation.z), cos(rotation.z), 0};
    }
    
    mathpls::vec3 Right() const {
        return mathpls::normalize(mathpls::cross(Forward(), Up()));
    }
    
    mathpls::mat4 mat4() const;
};

struct MeshComponent : public Component {
    MeshComponent(naGameObject* obj) : Component(obj) {}
    MeshComponent(naGameObject* obj, UID mesh_id) : Component(obj), mesh_id(mesh_id) {}

    UID mesh_id;
};

struct MaterialComponent : public Component {
    MaterialComponent(naGameObject* obj) : Component(obj) {}
    MaterialComponent(naGameObject* obj, UID material_id) : Component(obj), material_id(material_id) {}

    UID material_id;
};

struct PointLightCompnent : public Component {
    PointLightCompnent(naGameObject* obj) : Component(obj) {}
    PointLightCompnent(naGameObject* obj, mathpls::vec3 flux)
    : Component(obj), flux(flux) {}
    
    mathpls::vec3 flux;
};

struct DirectionalLightCompnent : public Component {
    DirectionalLightCompnent(naGameObject* obj) : Component(obj) {}
    DirectionalLightCompnent(naGameObject* obj, mathpls::vec3 color, mathpls::vec3 direction)
    : Component(obj), color(color), direction(direction) {}
    
    mathpls::vec3 color;
    mathpls::vec3 direction;
};

struct RigidBodyComponent : public Component {
    RigidBodyComponent(naGameObject* obj);
    
    float mass = 1;
    mathpls::vec3 velocity;
};

}
