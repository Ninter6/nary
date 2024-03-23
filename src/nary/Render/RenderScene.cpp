#define MATHPLS_VULKAN
#include "RenderScene.hpp"
#include "RenderUtil.hpp"

#include "se_tools.h"

#include <algorithm>

namespace nary {

void RenderScene::clear() {
    m_DirectionalLight.reset();
    m_PointLights.clear();
    m_VisableEntities.clear();
    m_Entities.clear();
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
        const auto& modelMat = scene.absoluteModelMat(id);
        auto model = obj.getComponent<MeshCompnent>();
        if (model) {
            auto& entity = m_Entities.emplace_back();
            entity.model = resource.getMesh(model->mesh_id);
            auto material = obj.getComponent<MaterialCompnent>();
            if (material) entity.material = material->material_id;
            entity.modelMat = modelMat;
            entity.boundingSphere = BoundingSphereTransform(entity.model->getBoundingSphere(), modelMat);
        }
        auto pointLight = obj.getComponent<PointLightCompnent>();
        if (pointLight) {
            auto& m_pointLight = m_PointLights.emplace_back();
            m_pointLight.flux = pointLight->flux;
            m_pointLight.radius = m_pointLight.calculateRadius();
            m_pointLight.position = modelMat[3];
        }
        auto directionalLight = obj.getComponent<DirectionalLightCompnent>();
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
    if (!m_DirectionalLight.has_value()) return;

    m_DirectionalLight->projView = DirectionalLightProjView(*this);
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
    if (m_DirectionalLight.has_value())
        ubo.directionalLightSpace = m_DirectionalLight->projView;
    ubo.numLights = m_PointLights.size();
    assert(ubo.numLights <= MAX_NUM_POINT_LIGHTS && "Point lights exceed maximum specified!");
    std::copy(m_PointLights.begin(), m_PointLights.end(), std::begin(ubo.pointLights));
    mathpls::vec3 cameraPos = m_Camera.invViewMat[3];
    std::sort(ubo.pointLights, ubo.pointLights + ubo.numLights, [&](auto&& a, auto&& b){
        return mathpls::distance_quared(a.position, cameraPos) > mathpls::distance_quared(b.position, cameraPos);
    });
    resource.updateGlobalUbo(ubo);
}

}