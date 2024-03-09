#pragma once

#include "RenderResource.hpp"
#include "Scene.hpp"

#include <vector>
#include <optional>

namespace nary {

/**
 * 通过Scene转换到RenderScene
 * RenderScene只包含可见的GO
 * GO会被打包为RenderEntity
*/

class RenderScene {
public:
    RenderScene() = default;

    void Update(const Scene& scene, const RenderResource& renderResource);

    RenderCamera m_Camera;

    std::optional<DirectionalLight> m_DirectionalLight; // 目前只接受一盏平行光，渲染阴影
    std::vector<PointLight> m_PointLights; // 可见的点光源

    std::vector<RenderEntity> m_VisableEntities; // 会按照material排序, 使用时std::lower_bound/std::upper_bound分组
    std::vector<RenderEntity> m_DirectionalLightVisableEntities;
    std::vector<std::vector<RenderEntity>> m_PointLightsVisableEntities; // 点光源可见的可见实体(套娃)，一一对应(如果是前向渲染对应就没啥用)

private:
    void clear();

    void pickUpEntities(const Scene& scene, const RenderResource& renderResource);
    void filterCameraVisable();
    void filterDirectionalLightVisable();
    void filterPointLightVisable();

    void updateGlobalUbo(const RenderResource& resource);

};

}