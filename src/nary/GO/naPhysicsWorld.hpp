//
//  naPhysicsWorld.hpp
//  pxpls
//
//  Created by Ninter6 on 2023/11/4.
//

#pragma once

#include "PhysicsWorld.hpp"
#include "naGameObject.hpp"

namespace nary {

class naPhysicsWorld {
public:
    naPhysicsWorld() = default;
    
    void Update(float dt, naGameObject::Map& objs);
    
private:
    pxpls::PhysicsWorld m_PhyWorld;
    
    std::unordered_map<naGameObject::id_t, pxpls::PhysicalObj> m_PhyObjs;
    
    void pickGameObjs(const naGameObject::Map& objs);
    pxpls::PhysicalObj& GetPhyObject(naGameObject::id_t id);
    
};

}
