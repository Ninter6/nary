//
//  naCamera.hpp
//  nary
//
//  Created by Ninter6 on 2023/8/19.
//

#pragma once

#include "naGameObject.hpp"
#include "naEventListener.hpp"

#include "math_helper.h"

namespace nary {

class naCamera : public naGameObject, public naEventListener {
public:
    naCamera();
    
    void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
    void setPerspectiveProjection(float fovy, float aspect, float near, float far);
    
    mathpls::mat4 getProjection() const {return projectionMatrix;}
    mathpls::mat4 getView() const;
    mathpls::mat4 getInverseView() const;

    void PMM(); // process mouse movement
    void Advance() {transform().translation -= transform().Forward() * naEventListener::DeltaTime();}
    void Retreat() {transform().translation += transform().Forward() * naEventListener::DeltaTime();}
    void GoLeft() {transform().translation -= transform().Right() * naEventListener::DeltaTime();}
    void GoRight() {transform().translation += transform().Right() * naEventListener::DeltaTime();}
    
private:
    mathpls::mat4 projectionMatrix{1.f};

};

}
