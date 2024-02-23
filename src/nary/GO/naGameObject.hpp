//
//  naGameObject.hpp
//  nary
//
//  Created by Ninter6 on 2023/8/17.
//

#pragma once

#include <iostream>
#include <unordered_map>

#include "naModel.hpp"
#include "GoComponent.hpp"

namespace nary {

class Scene;

class naGameObject {
public:
    using id_t = uint32_t;
    using Map = std::unordered_map<id_t, naGameObject>;
    
    static naGameObject createGameObject() {
        return naGameObject(currentId++);
    }
    
    static naGameObject createGameObject(const naGameObject& go) {
        auto r{go};
        r.id = currentId++;
        return r;
    }
    
    static naGameObject createPointLight(float radius = .1f, mathpls::vec3 color = mathpls::vec3{1.f});
    
    naGameObject(naGameObject&&);
    naGameObject& operator=(naGameObject&&);
    
    id_t getId() const {return id;}
    
    Scene* scene;
    
    std::vector<ComponentWapper> components;
    
    TransformComponent& transform() const;
    
    template <class T, class... Args>
    std::enable_if_t<std::is_base_of_v<Component, T>, T*>
    addComponent(Args...args) {
        auto p = new T{this, std::forward<Args>(args)...};
        components.emplace_back(p);
        return p;
    }
    
    template <class T>
    std::enable_if_t<std::is_base_of_v<Component, T>, T*>
    getComponent() const {
        auto r = std::find_if(components.begin(), components.end(), [](auto&& i){
            return dynamic_cast<T*>(i.get()) != nullptr;
        });
        if (r == components.end()) return nullptr;
        return dynamic_cast<T*>(r->get());
    }
    
private:
    naGameObject(id_t ID);
    naGameObject(const naGameObject&);
    naGameObject& operator=(const naGameObject&);
    
    id_t id;
    
    void applyComponent();
    
    static inline id_t currentId = 0;
};

}
