//
//  naPhysicsWorld.hpp
//  pxpls
//
//  Created by Ninter6 on 2023/11/4.
//

#pragma once

#include "PhysicsWorld.hpp"
#include "naGameObject.hpp"

#include <memory>

namespace nary {

struct PhysicsObject {
    std::shared_ptr<pxpls::Rigidbody> body;
    std::shared_ptr<pxpls::Collider> collider;
};

class naPhysicsWorld {
public:
    naPhysicsWorld();
    
    void Update(float dt, naGameObject::Map& objs);
    
private:
    pxpls::DynamicsWorld m_World;

    pxpls::VerletSolver solver;
    pxpls::CollisionBody floor;
    pxpls::PlaneCollider collider{{0, -1, 0, 0}};
    
    std::unordered_map<naGameObject::id_t, PhysicsObject> m_Objs;
    
    void pickGameObjs(float dt, const naGameObject::Map& objs);
    PhysicsObject& GetObject(naGameObject::id_t id);

};

}
