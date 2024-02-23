#pragma once

#include "naRenderer.hpp"
#include "RenderResource.hpp"
#include "RenderScene.hpp"

#include "naRenderSystem.hpp"
#include "naPointLightSystem.hpp"
#include "naShadowSystem.hpp"
#include "naRenderShaderOnly.hpp"

#include "naUISystem.hpp"

namespace nary {

class RenderManager {
public:
    RenderManager(naWin& window);
    ~RenderManager();

    RenderManager(const RenderManager&) = delete;
    RenderManager& operator=(const RenderManager&) = delete;

    void tick(const Scene& scene);

    RenderResource* getRenderResource() const;
    
private:
    naWin& m_Window;

    std::unique_ptr<naDevice> m_Device;
    std::unique_ptr<naRenderer> m_Renderer;

    std::unique_ptr<RenderResource> m_RenderResource;
    std::unique_ptr<RenderScene> m_RenderScene;

    std::unique_ptr<naRenderSystem> m_RenderSystem;
    std::unique_ptr<naPointLightSystem> m_PointLightSystem;
    std::unique_ptr<naShadowSystem> m_ShadowSystem;
    std::unique_ptr<naRenderShaderOnly> m_PostProcessing;

    std::unique_ptr<naUISystem> m_UI;

    std::vector<VkDescriptorSet> m_OffScreenSets{naSwapChain::MAX_FRAMES_IN_FLIGHT};
    std::vector<VkDescriptorSet> m_ShadowMapSets{naSwapChain::MAX_FRAMES_IN_FLIGHT};

    void initialize();
    void createDescriptorSets();

    void DrawUI(VkCommandBuffer cmdbuf);

    friend ResourceManager;
};

}