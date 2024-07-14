#define MATHPLS_DEPTH_0_1
#include "RenderScene.hpp"
#include "RenderUtil.hpp"

#include "se_tools.h"

#include <algorithm>

namespace nary {

void RenderScene::clear() {
    m_DirectionalLight.reset();
    m_PointLights.clear();
    m_Entities.clear();
    m_VisableEntities.clear();
    m_DirectionalLightVisableEntities.clear();
    m_PointLightsVisableEntities.clear();
}

void RenderScene::Update(const Scene& scene, const RenderResource& resource) {
    clear();

    pickUpEntities(scene, resource);
    filterCameraVisable();
    filterPointLightVisable();
    processDirectionalLight();

    updateGlobalUbo(resource);
}

void RenderScene::pickUpEntities(const Scene& scene, const RenderResource& resource) {
    // pick up camera
    auto pCam = scene.getActiveCamera();
    auto camModelMat = scene.absoluteModelMat(pCam->getId());
    m_Camera.projMat = pCam->getProjection();
    m_Camera.viewMat = CameraViewFromAbsoluteModelMat(camModelMat);
    m_Camera.invViewMat = CameraInvViewFromAbsoluteModelMat(camModelMat);
    m_Camera.viewFrustum = CreateFrustumFromMatrix(m_Camera.projMat * m_Camera.viewMat);

    // entities
    for (auto& [id, obj] : scene.getGameObjects()) {
        if (!obj.getActive()) continue;
        const auto& modelMat = scene.absoluteModelMat(id);
        auto model = obj.getComponent<MeshComponent>();
        if (model) {
            auto& entity = m_Entities.emplace_back();
            entity.model = resource.getMesh(model->mesh_id);
            auto material = obj.getComponent<MaterialComponent>();
            if (material) entity.material = material->material_id;
            entity.modelMat = modelMat;
            entity.boundingSphere = BoundingSphereTransform(entity.model->getBoundingSphere(), modelMat);
        }
        auto pointLight = obj.getComponent<PointLightComponent>();
        if (pointLight) {
            auto& m_pointLight = m_PointLights.emplace_back();
            m_pointLight.flux = pointLight->flux;
            m_pointLight.radius = m_pointLight.calculateRadius();
            m_pointLight.position = modelMat[3];
        }
        auto directionalLight = obj.getComponent<DirectionalLightComponent>();
        if (directionalLight && !m_DirectionalLight.has_value()) {
            m_DirectionalLight.emplace();
            m_DirectionalLight->direction = directionalLight->direction;
            m_DirectionalLight->color = directionalLight->color;
        }
    }
}

void RenderScene::filterCameraVisable() {
    std::copy_if(m_Entities.begin(), m_Entities.end(), std::back_inserter(m_VisableEntities), [&](auto&&i) {
        return m_Camera.viewFrustum.isOverlapping(i.boundingSphere);
    });
    std::erase_if(m_PointLights, [&](auto&&i) {
        return !m_Camera.viewFrustum.isOverlapping(pxpls::Sphere{i.position, i.radius});
    });

    // sort to group by material
    std::sort(m_VisableEntities.begin(), m_VisableEntities.end(), [](auto&&a, auto&&b) {
        return a.material < b.material;
    });
}

void RenderScene::processDirectionalLight() {
    if (!m_DirectionalLight.has_value() || m_VisableEntities.empty())
        return;

    pxpls::Sphere fbs; // Frustum Bounding Sphere
    {
        mathpls::vec3 v[4] {{1, 1, 1}, {-1, -1, 1}, {1, -1, 0},{-1, 1, 0}};
        auto M = mathpls::inverse(m_Camera.projMat * m_Camera.viewMat);

        for (auto& i : v){
            auto u = M * mathpls::vec4{i, 1.f};
            i = u / u.w;
        }

        fbs = pxpls::BoundingSphereFromPoints(v);
    }

    auto& dir = m_DirectionalLight->direction.normalize() *= -1;

    std::copy_if(m_Entities.begin(), m_Entities.end(), std::back_inserter(m_DirectionalLightVisableEntities), [&](auto&&i) {
        if (pxpls::IntersectSphereSphere(i.boundingSphere, fbs))
            return true;
        auto D = fbs.center - i.boundingSphere.center;
        auto R = fbs.radius + i.boundingSphere.radius;
        auto P = mathpls::cross(D, dir).length_squared();
        return mathpls::dot(D, dir) < 0 && R * R > P;
    });

    m_DirectionalLight->projView = DirectionalLightProjView(m_DirectionalLightVisableEntities, dir);
}

void RenderScene::filterPointLightVisable() {
    if (m_PointLights.size() == 0) return;
    m_PointLightsVisableEntities.resize(m_PointLights.size(), m_VisableEntities);
    LOOP (m_PointLights.size()) {
        pxpls::Sphere lightSphere{m_PointLights[i].position, m_PointLights[i].radius};
        std::erase_if(m_PointLightsVisableEntities[i], [&](auto&&e){
            return !pxpls::IntersectSphereSphere(e.boundingSphere, lightSphere);
        });
    }
}

void RenderScene::updateGlobalUbo(const RenderResource& resource) {
    GlobalUbo ubo;
    ubo.view = m_Camera.viewMat;
    ubo.inverseView = m_Camera.invViewMat;
    ubo.projection = m_Camera.projMat;
    if (m_DirectionalLight.has_value()) {
        ubo.directionalLight.projView = m_DirectionalLight->projView;
        ubo.directionalLight.color = m_DirectionalLight->color;
        ubo.directionalLight.direction = m_DirectionalLight->direction;
    }
    ubo.numLights = m_PointLights.size();
    assert(ubo.numLights <= MAX_NUM_POINT_LIGHTS && "Point lights exceed maximum specified!");
    mathpls::vec3 cameraPos = m_Camera.invViewMat[3];
    std::sort(m_PointLights.begin(), m_PointLights.end(), [&](auto&& a, auto&& b){
        return mathpls::distance_quared(a.position, cameraPos) > mathpls::distance_quared(b.position, cameraPos);
    });
    std::copy(m_PointLights.begin(), m_PointLights.end(), std::begin(ubo.pointLights));
    resource.updateGlobalUbo(ubo);
}

}