#include "RenderManager.hpp"

namespace nary {

mathpls::vec3 lightPos = {-0.785f, -4.f, 3.448f};
mathpls::vec3 lightDir = {-0.522f, -0.42f, -0.492f};

RenderManager::RenderManager(naWin& window) : m_Window(window) {
    initialize();
    createDescriptorSets();
}

RenderManager::~RenderManager() {
    vkDeviceWaitIdle(m_Device->device());
}

void RenderManager::initialize() {
    m_Device = std::make_unique<naDevice>(m_Window);
    m_Renderer = std::make_unique<naRenderer>(m_Window, *m_Device);

    m_RenderResource = std::make_unique<RenderResource>(*m_Device);
    m_RenderScene = std::make_unique<RenderScene>();

    m_RenderSystem = std::make_unique<naRenderSystem>(*m_Device, m_Renderer->getRenderPass(), *m_RenderResource);
    m_PointLightSystem = std::make_unique<naPointLightSystem>(*m_Device, m_Renderer->getRenderPass(), *m_RenderResource);
    m_ShadowSystem = std::make_unique<naShadowSystem>(*m_Device, m_Renderer->getRenderPass(), *m_RenderResource);

    m_PostProcessing = std::make_unique<naRenderShaderOnly>(*m_Device, m_Renderer->getSwapChainRenderPass(), *m_RenderResource);
    m_PostProcessing->setShaders("rectangle.vert", "FXAA.frag");

    m_UI = std::make_unique<naUISystem>(*m_Device, m_Renderer->getSwapChainRenderPass(), m_Window);
}

void RenderManager::createDescriptorSets() {
    LOOP (naSwapChain::MAX_FRAMES_IN_FLIGHT) {
        naSampler sampler{*m_Device, SamplerType::Linear};
        auto offscreenInfo = sampler.descriptorInfo(m_Renderer->getFrameBuffer().getGroup(i).images[2]);
        auto shadowMapInfo = sampler.descriptorInfo(m_Renderer->getFrameBuffer().getGroup(i).images[3]);
        naDescriptorWriter{*m_RenderResource->getOneImageSetLayout(), *m_RenderResource->getDescriptorPool()}
            .writeImage(0, &offscreenInfo)
            .build(m_OffScreenSets[i]);
        naDescriptorWriter{*m_RenderResource->getOneImageSetLayout(), *m_RenderResource->getDescriptorPool()}
            .writeImage(0, &shadowMapInfo)
            .build(m_ShadowMapSets[i]);
    }
}


void RenderManager::tick(const Scene& scene) {
    if (auto commandBuffer = m_Renderer->beginFrame()) {
        auto frame_index =  m_Renderer->getFrameIndex();
        m_RenderResource->setCurrentFrameIndex(frame_index);

        m_RenderScene->Update(scene, *m_RenderResource);

        m_Renderer->beginRenderPass();
        m_ShadowSystem->renderGameObjects(*m_RenderScene, commandBuffer);
        m_Renderer->nextSubpass();
        m_RenderSystem->renderGameObjects(*m_RenderScene, commandBuffer, m_ShadowMapSets[frame_index]);
        m_PointLightSystem->render(*m_RenderScene, commandBuffer);
        m_Renderer->endRenderPass();


        m_Renderer->beginSwapChainRenderPass();
        m_PostProcessing->render(m_OffScreenSets[frame_index], commandBuffer, 6, 1);
        
        DrawUI(commandBuffer);
        
        m_Renderer->endSwapChainRenderPass();
        m_Renderer->endFrame();
    }
}

void RenderManager::DrawUI(VkCommandBuffer cmdbuf) {
    m_UI->beginFrame(); {
        ImGui::Begin("Hello world window");

//        ImGui::Text("人生得意须尽欢，\n今后都得拉清单。");
//        ImGui::Image(&UItexDescriptorSet, ImVec2{300, 300});
        
        ImGui::DragFloat3("Light Position", &lightPos[0], .1f);
        ImGui::DragFloat3("Light Direction", &lightDir[0], .1f);

        ImGui::End();
    } m_UI->endFrame(cmdbuf);
}

RenderResource* RenderManager::getRenderResource() const {
    return m_RenderResource.get();
}

}