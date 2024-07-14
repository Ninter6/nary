//
//  naShadowSystem.cpp
//  nary
//
//  Created by Ninter6 on 2023/12/16.
//

#include "naShadowSystem.hpp"

namespace nary {

struct ShadowPushConstantData {
    mathpls::mat4 modelMatrix;
};

naShadowSystem::naShadowSystem(naDevice& device, VkRenderPass renderPass, const RenderResource& renderResource)
: device(device), renderResource(renderResource) {
    createPipelineLayout();
    createPipeline(renderPass);
}

naShadowSystem::~naShadowSystem() {
    vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
}

void naShadowSystem::createPipelineLayout() {
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(ShadowPushConstantData);
    
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
        renderResource.getGlbalUboSetLayout()->get()
    };
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    if(vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS){
        throw std::runtime_error("Failded to create pipeline layout!");
    }
}

void naShadowSystem::createPipeline(VkRenderPass renderPass) {
    PipelineConfigInfo pipelineConfig{};
    naPipeline::defaultPiplineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = pipelineLayout;
//    pipelineConfig.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipelineConfig.subpass = 0;
    pipeline = std::make_unique<naPipeline>(device, "shadow.vert", "shadow.frag", pipelineConfig);
}

void naShadowSystem::renderGameObjects(const RenderScene& scene, VkCommandBuffer commandBuffer) {
    DEBUG_LOG("now {} objects cast shadow", scene.m_DirectionalLightVisableEntities.size());

     if (!scene.m_DirectionalLight.has_value() || scene.m_DirectionalLightVisableEntities.empty())
         return;

    pipeline->bind(commandBuffer);
    
    auto global_ubo = renderResource.getGlobalUboDescriptorSet();
    vkCmdBindDescriptorSets(commandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout,
                            0, 1,
                            &global_ubo,
                            0, nullptr);
    
    for (auto& entity : scene.m_DirectionalLightVisableEntities) {
        ShadowPushConstantData push{};
        push.modelMatrix = entity.modelMat;
        
        vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_ALL_GRAPHICS, 0, sizeof(ShadowPushConstantData), &push);
        
        entity.model->bind(commandBuffer);
        entity.model->draw(commandBuffer);
    }
}

}
