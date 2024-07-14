#include "RenderManager.hpp"

namespace nary {

UID ui_texture;

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
    m_UI->beginFrame();
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
//    static char text[21*14]{};
//    static mathpls::vec4 a{3.f, 2.f, 2.95f, 2.03f};
//    constexpr float G = 514;
//
//    {
//        ImGui::Begin("Hello world window");
//
//        mathpls::vec2 buf = {a[0], a[1]};
//        auto dt = 1/60.f;
//        auto d = mathpls::vec2{10, 7} - buf;
//        auto k = G/d.length_squared()*dt*dt;
//        d.normalize();
//        a[0] += a[0] - a[2] + d.x * k;
//        a[1] += a[1] - a[3] + d.y * k;
//        a[2] = buf[0];
//        a[3] = buf[1];
//
//        memset(text, '-', sizeof(text));
//        mathpls::uivec2 pa(std::round(a[0]), std::round(a[1]));
//        if (pa.x >= 0 && pa.x < 20 && pa.y >= 0 && pa.y < 14)
//            text[pa.x + pa.y * 21] = '*';
//        LOOP(14) text[20 + i*21] = '\n';
//
//        ImGui::Text("%s", text);
//
//        ImGui::End();
//    }

    ImGui::Begin("Buffer");

    ImGui::Image(&m_ShadowMapSets[m_Renderer->getFrameIndex()], {320, 200});

    ImGui::End();
    
    m_UI->endFrame(cmdbuf);
    m_UI->beginFrame(); // so that it can be used externally
}

naWin* RenderManager::getWindow() const {
    return &m_Window;
}

RenderResource* RenderManager::getRenderResource() const {
    return m_RenderResource.get();
}

}