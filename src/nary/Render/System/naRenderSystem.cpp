//
//  naRenderSystem.cpp
//  nary
//
//  Created by Ninter6 on 2023/8/19.
//

#include "naRenderSystem.hpp"

namespace nary {

struct SimplePushConstantData {
    mathpls::mat4 modelMatrix;
};

naRenderSystem::naRenderSystem(naDevice& device, VkRenderPass renderPass, const RenderResource& renderResource)
: device(device), renderResource(renderResource) {
    createPipelineLayout();
    createPipeline(renderPass);
}

naRenderSystem::~naRenderSystem(){
    vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
}

void naRenderSystem::createPipelineLayout(){
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);
    
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
        renderResource.getGlbalUboSetLayout()->get(),
        renderResource.getOneImageSetLayout()->get(),
        renderResource.getMaterialSetLayout()->get()
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

void naRenderSystem::createPipeline(VkRenderPass renderPass){
    PipelineConfigInfo pipelineConfig{};
    naPipeline::defaultPiplineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = pipelineLayout;
    pipelineConfig.subpass = 1;
    pipeline = std::make_unique<naPipeline>(device, "vertex.vert", "fragment.frag", pipelineConfig);
}

void naRenderSystem::renderGameObjects(const RenderScene& renderScene, VkCommandBuffer commandBuffer, VkDescriptorSet shadowMapDescriptorSet) {
    if (renderScene.m_VisableEntities.size() == 0)
        return;

    pipeline->bind(commandBuffer);

    VkDescriptorSet sets[2]{
        renderResource.getGlobalUboDescriptorSet(),
        shadowMapDescriptorSet
    };
    vkCmdBindDescriptorSets(commandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout,
                            0, 2,
                            sets,
                            0, nullptr);

    auto& entities = renderScene.m_VisableEntities;
    auto mtl = entities.front().material;

    while (true) {
        // bind material descriptor sets
        auto material_descriptor_set = renderResource.getMaterialDescriptorSet(mtl);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipelineLayout,
                                2, 1,
                                &material_descriptor_set,
                                0, nullptr);

        auto bng = std::lower_bound(entities.begin(), entities.end(), mtl, [](const RenderEntity& a, UID b) {
            return a.material < b;
        });
        auto end = std::upper_bound(entities.begin(), entities.end(), mtl, [](UID a, const RenderEntity& b) {
            return a < b.material;
        });

        for (; bng != end; ++bng) {
            auto& entity = *bng;

            // push constants
            SimplePushConstantData push{};
            push.modelMatrix = entity.modelMat;
            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_ALL_GRAPHICS, 0, sizeof(SimplePushConstantData), &push);

            entity.model->bind(commandBuffer);
            entity.model->draw(commandBuffer);
        }

        if (end == entities.end())
            break;
        else
            mtl = end->material;
    }
}

}
