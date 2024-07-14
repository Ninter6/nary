//
//  naPhysicsWorld.cpp
//  pxpls
//
//  Created by Ninter6 on 2023/11/4.
//

#include "naPhysicsWorld.hpp"
#include "naEventListener.hpp"

namespace nary {

naPhysicsWorld::naPhysicsWorld() : m_World(std::make_unique<pxpls::UniformGrid>(pxpls::Bounds{-100, 100}, mathpls::uivec3{10})) {
    m_World.Gravity = {0, -10, 0};
    m_World.AddSolver(&solver);

    floor.collider = &collider;

    m_World.AddCollisionBody(&floor);
}

void naPhysicsWorld::pickGameObjs(float dt, const naGameObject::Map& objs) {
    for (const auto& [id, obj] : objs) {
        auto rb = obj.getComponent<RigidBodyComponent>();
        if (!rb) continue;
        if (!m_Objs.contains(id)) {
            PhysicsObject phyobj;
            phyobj.body = std::make_shared<pxpls::Rigidbody>();
            phyobj.body->IsKinematic = true;
            
            phyobj.body->SetPosition(obj.transform().translation);
            phyobj.body->transform.Scale = obj.transform().scale;

            phyobj.body->lastPostion -= rb->velocity * dt;
            phyobj.body->mass = rb->mass;

            phyobj.collider = std::make_shared<pxpls::SphereCollider>(0, 1.f);
            phyobj.body->collider = phyobj.collider.get();
            
            auto [it, success] = m_Objs.emplace(id, std::move(phyobj));
            
            assert(success && "failed to create physical objest!");
            m_World.AddRigidbody(it->second.body.get());
        }
    }
}

void naPhysicsWorld::Update(float dt, naGameObject::Map& objs) {
    pickGameObjs(std::clamp(dt, 1e-8f, .1f), objs);
    
    m_World.Step(dt);
    
    for (auto& [id, phyobj] : m_Objs) {
        auto& go = objs.at(id);
        
        go.transform().translation = phyobj.body->Position();
    }
}

}
