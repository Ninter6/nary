//
//  naPhysicsWorld.cpp
//  pxpls
//
//  Created by Ninter6 on 2023/11/4.
//

#include "naPhysicsWorld.hpp"

namespace nary {

//pxpls::PhysicalObj& naPhysicsWorld::GetPhyObject(naGameObject::id_t id) {
//    auto it = m_PhyObjs.find(id);
//    if (it != m_PhyObjs.end())
//        return it->second;
//
//    auto& obj = m_PhyObjs[id];
//
//    m_PhyWorld.AddObject(&obj); // push obj to physics world
//
//    return obj;
//}

void naPhysicsWorld::pickGameObjs(const naGameObject::Map& objs) {
    for (const auto& [id, obj] : objs) {
        auto rb = obj.getComponent<RigidBodyComponent>();
        if (!rb) continue;
        if (!m_PhyObjs.contains(id)) {
            pxpls::PhysicalObj phyobj;
            
            phyobj.transform.Position = {
                obj.transform().translation.x,
               -obj.transform().translation.y,
                obj.transform().translation.z
            };
            phyobj.transform.Scale = {
                obj.transform().scale.x,
               -obj.transform().scale.y,
                obj.transform().scale.z
            };
            
            auto& po_rb = phyobj.SetRigidbody();
            po_rb.velocity = {rb->velocity.x, rb->velocity.y, rb->velocity.z};
            po_rb.mass = rb->mass;
            
            auto [it, success] = m_PhyObjs.emplace(id, std::move(phyobj));
            
            assert(success && "failed to create physical objest!");
            m_PhyWorld.AddObject(&it->second);
        }
    }
}

void naPhysicsWorld::Update(float dt, naGameObject::Map& objs) {
    pickGameObjs(objs);
    
    m_PhyWorld.Update(dt);
    
    for (auto& [id, phyobj] : m_PhyObjs) {
//        std::cout
//            << phyobj.transform.Position.x << " "
//            << phyobj.transform.Position.y << " "
//            << phyobj.transform.Position.z << " "
//            << std::endl;
        auto& go = objs.at(id);
        
        go.transform().translation = {
            phyobj.transform.Position.x,
           -phyobj.transform.Position.y,
            phyobj.transform.Position.z
        };
        
        if (phyobj.transform.Position.y < .2f && phyobj.rigidbody().velocity.y < 0) phyobj.rigidbody().velocity.y *= -.707f;
    }
}

}
