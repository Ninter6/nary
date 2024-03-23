#define MATHPLS_VULKAN
#include "RenderUtil.hpp"
#include "RenderScene.hpp"

#include <cmath>
#include <cfloat>

namespace nary {

mathpls::mat4 CameraViewFromAbsoluteModelMat(const mathpls::mat4& modelMat) {
    mathpls::vec3 Up = {0, 1, 0}; // found some problems here
    mathpls::vec3 Forward = modelMat[2];
    mathpls::vec3 eye = modelMat[3];
    return mathpls::lookAt(eye, eye + Forward, Up);
}

mathpls::mat4 CameraInvViewFromAbsoluteModelMat(const mathpls::mat4& modelMat) {
    mathpls::mat4 inverseViewMatrix = modelMat;
    inverseViewMatrix[0] *=-1 / std::sqrtf(inverseViewMatrix[0].length_squared());
    inverseViewMatrix[1] *= 1 / std::sqrtf(inverseViewMatrix[1].length_squared());
    inverseViewMatrix[2] *=-1 / std::sqrtf(inverseViewMatrix[2].length_squared());
    return inverseViewMatrix;
}

float FindMaxScaleValueFromModelMat( const mathpls::mat4& modelMat) {
    float x = std::sqrtf(modelMat[0].length_squared());
    float y = std::sqrtf(modelMat[1].length_squared());
    float z = std::sqrtf(modelMat[2].length_squared());
    return std::max(x, std::max(y, z));
}

pxpls::Sphere BoundingSphereTransform(const pxpls::Sphere& sph, const mathpls::mat4& mat) {
    pxpls::Sphere res;
    res.center = mat * mathpls::vec4{sph.center, 1.f};
    res.radius = sph.radius * FindMaxScaleValueFromModelMat(mat);
    return res;
}

pxpls::Bounds MergeBoundsPoint(const pxpls::Bounds& bnd, const pxpls::Point& pnt) {
    auto res = bnd;

    res.min.x = std::min(res.min.x, pnt.x);
    res.min.y = std::min(res.min.y, pnt.y);
    res.min.z = std::min(res.min.z, pnt.z);

    res.max.x = std::max(res.max.x, pnt.x);
    res.max.y = std::max(res.max.y, pnt.y);
    res.max.z = std::max(res.max.z, pnt.z);

    return res;
}

pxpls::Bounds BoundsTransform(const pxpls::Bounds& bnd, const mathpls::mat4& mat) {
    pxpls::Bounds res{{FLT_MAX}, {-FLT_MAX}};
    for (const auto& i : bnd.allVertices())
        res = MergeBoundsPoint(res, mat * mathpls::vec4{i, 1.f});
    return res;
}

mathpls::mat4 DirectionalLightProjView(const RenderScene& scene) {
    pxpls::Bounds scene_bounding_box;
    {
        scene_bounding_box.min = mathpls::vec3(FLT_MAX);
        scene_bounding_box.max = mathpls::vec3(-FLT_MAX);

        for (const RenderEntity& entity : scene.m_Entities)
        {
            const auto& [cnt, r] = entity.boundingSphere;
            scene_bounding_box = MergeBoundsPoint(scene_bounding_box, {cnt.x + r, cnt.y, cnt.z});
            scene_bounding_box = MergeBoundsPoint(scene_bounding_box, {cnt.x - r, cnt.y, cnt.z});
            scene_bounding_box = MergeBoundsPoint(scene_bounding_box, {cnt.x, cnt.y + r, cnt.z});
            scene_bounding_box = MergeBoundsPoint(scene_bounding_box, {cnt.x, cnt.y - r, cnt.z});
            scene_bounding_box = MergeBoundsPoint(scene_bounding_box, {cnt.x, cnt.y, cnt.z + r});
            scene_bounding_box = MergeBoundsPoint(scene_bounding_box, {cnt.x, cnt.y, cnt.z - r});
        }
    }

    mathpls::mat4 light_view;
    mathpls::mat4 light_proj;
    {
        mathpls::vec3 box_center((scene_bounding_box.max.x + scene_bounding_box.min.x) * 0.5,
                             (scene_bounding_box.max.y + scene_bounding_box.min.y) * 0.5,
                             (scene_bounding_box.max.z + scene_bounding_box.min.z));
        mathpls::vec3 box_extents((scene_bounding_box.max.x - scene_bounding_box.min.x) * 0.5,
                              (scene_bounding_box.max.y - scene_bounding_box.min.y) * 0.5,
                              (scene_bounding_box.max.z - scene_bounding_box.min.z) * 0.5);

        mathpls::vec3 eye =
            box_center + scene.m_DirectionalLight->direction * box_extents.length();
        mathpls::vec3 center = box_center;
        light_view       = mathpls::lookAt(eye, center, mathpls::vec3(0.0, 0.0, 1.0));

        pxpls::Bounds scene_bounding_box_light_view   = BoundsTransform(scene_bounding_box, light_view);
        light_proj = mathpls::ortho(
            scene_bounding_box_light_view.min.x,
            scene_bounding_box_light_view.max.x,
            scene_bounding_box_light_view.min.y,
            scene_bounding_box_light_view.max.y,
            scene_bounding_box_light_view.max
                 .z, // the objects which are nearer than the frustum bounding box may caster shadow as well
            scene_bounding_box_light_view.min.z);
    }

    mathpls::mat4 light_proj_view = (light_proj * light_view);
    return light_proj_view;
}

Frustum CreateFrustumFromMatrix(const mathpls::mat4& mat, float x_left, float x_right, float y_top, float y_bottom, float z_near, float z_far) {
    Frustum f;
    auto& plane_right   = f.planes[0];
    auto& plane_left    = f.planes[1];
    auto& plane_top     = f.planes[2];
    auto& plane_bottom  = f.planes[3];
    auto& plane_near    = f.planes[4];
    auto& plane_far     = f.planes[5];

    // calculate the tiled frustum
    // [Fast Extraction of Viewing Frustum Planes from the WorldView - Projection
    // Matrix](http://gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf)
    
#define NORMALIZE(x) do { \
    float rlen = 1.f / std::sqrtf(x.normal.length_squared()); \
    x.normal *= rlen; \
    x.D *= rlen; \
} while(0)

    auto mat_column = mat.transposed();
    
    plane_right = mat_column[3] * x_right - mat_column[0];
    NORMALIZE(plane_right);

    plane_left = mat_column[0] - mat_column[3] * x_left;
    NORMALIZE(plane_left);

    plane_top = mat_column[3] * y_top - mat_column[1];
    NORMALIZE(plane_top);

    plane_bottom = mat_column[1] - mat_column[3] * y_bottom;
    NORMALIZE(plane_bottom);

    plane_near = mat_column[2] - mat_column[3] * z_near;
    NORMALIZE(plane_near);

    plane_far = mat_column[3] * z_far - mat_column[2];
    NORMALIZE(plane_far);

    return f;
    
#undef NORMALIZE
}

}