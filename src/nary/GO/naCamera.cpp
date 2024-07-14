//
//  naCamera.cpp
//  nary
//
//  Created by Ninter6 on 2023/8/19.
//

#include "naCamera.hpp"

#include <algorithm>

namespace nary {

naCamera::naCamera() : naGameObject(createGameObject()) {
    EventList.insert({{0, 0, 0, 2, true}, std::bind(&naCamera::PMM, this)});
    EventList.insert({{0, 0, 'W', 2}, std::bind(&naCamera::Advance, this)});
    EventList.insert({{0, 0, 'S', 2}, std::bind(&naCamera::Retreat, this)});
    EventList.insert({{0, 0, 'A', 2}, std::bind(&naCamera::GoLeft, this)});
    EventList.insert({{0, 0, 'D', 2}, std::bind(&naCamera::GoRight, this)});
}

void naCamera::setOrthographicProjection(
    float left, float right, float top, float bottom, float near, float far) {
  projectionMatrix = mathpls::mat4{1.0f};
  projectionMatrix[0][0] = 2.f / (right - left);
  projectionMatrix[1][1] = 2.f / (bottom - top);
  projectionMatrix[2][2] = 1.f / (far - near);
  projectionMatrix[3][0] = -(right + left) / (right - left);
  projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
  projectionMatrix[3][2] = -near / (far - near);
}

void naCamera::setPerspectiveProjection(float fovy, float aspect, float near, float far) {
  assert(std::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
  const float tanHalfFovy = std::tan(fovy / 2.f);
  projectionMatrix = mathpls::mat4{0.0f};
  projectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
  projectionMatrix[1][1] = 1.f / (tanHalfFovy);
  projectionMatrix[2][2] = far / (far - near);
  projectionMatrix[2][3] = 1.f;
  projectionMatrix[3][2] = -(far * near) / (far - near);
}

mathpls::mat4 naCamera::getView() const {
    const auto& Forward = transform().Forward(), Up = transform().Up();
    return mathpls::lookAt(transform().translation, transform().translation + Forward, Up);
}

mathpls::mat4 naCamera::getInverseView() const {
    auto v = transform().Up(), w = transform().Forward();
    mathpls::mat4 inverseViewMatrix;
    inverseViewMatrix[0] = mathpls::cross(w, v);
    inverseViewMatrix[1] = v;
    inverseViewMatrix[2] = w;
    inverseViewMatrix[3] = transform().translation;
    return inverseViewMatrix;
}

void naCamera::PMM() {
    auto mpd = GetMousePosDelta() / 333.333;
    transform().rotation.y += mpd.x;
    transform().rotation.x -= abs(transform().rotation.x - mpd.y) > 1.5707 ? 0 : mpd.y;
}

}
