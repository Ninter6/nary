#pragma once

#include "math_helper.h"

namespace nary {

class RenderScene;

mathpls::mat4 CameraViewFromAbsoluteModelMat(const mathpls::mat4& modelMat);
mathpls::mat4 CameraInvViewFromAbsoluteModelMat(const mathpls::mat4& modelMat);

float FindMaxScaleValueFromModelMat( const mathpls::mat4& modelMat);

pxpls::Sphere BoundingSphereTransform(const pxpls::Sphere& sph, const mathpls::mat4& mat);

pxpls::Bounds MergeBoundsPoint(const pxpls::Bounds& bnd, const pxpls::Point& pnt);
pxpls::Bounds BoundsTransform(const pxpls::Bounds& bnd, const mathpls::mat4& mat);

mathpls::mat4 DirectionalLightProjView(const RenderScene& scene);

Frustum CreateFrustumFromMatrix(const mathpls::mat4& mat, float x_left = -1.f, float x_right = 1.f, float y_top = -1.f, float y_bottom = 1.f, float z_near = 0.f, float z_far = 1.f);

}