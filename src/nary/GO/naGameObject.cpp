//
//  naGameObject.cpp
//  nary
//
//  Created by Ninter6 on 2023/8/17.
//

#include "naGameObject.hpp"

namespace nary {

naGameObject::naGameObject(id_t id) : id(id) {
    addComponent<TransformComponent>();
}

naGameObject::naGameObject(const naGameObject& o)
: id(o.id), components(o.components) {
    applyComponent();
}

naGameObject& naGameObject::operator=(const naGameObject& o) {
    id = o.id;
    components = o.components;
    
    applyComponent();
    
    return *this;
}

naGameObject::naGameObject(naGameObject&& o)
: id(o.id), components(std::move(o.components)) {
    applyComponent();
}

naGameObject& naGameObject::operator=(naGameObject&& o) {
    id = o.id;
    components = std::move(o.components);
    
    applyComponent();
    
    return *this;
}

void naGameObject::applyComponent() {
    for (auto& i : components)
        i.get()->m_Obj = this;
}

TransformComponent& naGameObject::transform() const {
    return *getComponent<TransformComponent>();
}

mathpls::mat4 TransformComponent::mat4() const {
    const float c3 = std::cos(rotation.z);
    const float s3 = std::sin(rotation.z);
    const float c2 = std::cos(rotation.x);
    const float s2 = std::sin(rotation.x);
    const float c1 = std::cos(rotation.y);
    const float s1 = std::sin(rotation.y);
    return mathpls::mat4{
        mathpls::vec4{
            scale.x * (c1 * c3 + s1 * s2 * s3),
            scale.x * (c2 * s3),
            scale.x * (c1 * s2 * s3 - c3 * s1),
            0.0f,
        },
        mathpls::vec4{
            scale.y * (c3 * s1 * s2 - c1 * s3),
            scale.y * (c2 * c3),
            scale.y * (c1 * c3 * s2 + s1 * s3),
            0.0f,
        },
        mathpls::vec4{
            scale.z * (c2 * s1),
            scale.z * (-s2),
            scale.z * (c1 * c2),
            0.0f,
        },
        mathpls::vec4{translation.x, translation.y, translation.z, 1.0f}};
}

naGameObject naGameObject::createPointLight(float radius, mathpls::vec3 color) {
    naGameObject obj = createGameObject();
    obj.transform().scale.x = radius;
    obj.addComponent<PointLightCompnent>(color);
    return obj;
}

}
