//
//  naPointLightSystem.cpp
//  nary
//
//  Created by Ninter6 on 2023/8/23.
//

#include "naPointLightSystem.hpp"

namespace nary {

naPointLightSystem::naPointLightSystem(naDevice& device, VkRenderPass renderPass, const RenderResource& renderResource)
: device(device), renderResource(renderResource) {
    createPipelineLayout();
    createPipeline(renderPass);
}

naPointLightSystem::~naPointLightSystem() {
    vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
}

void naPointLightSystem::createPipelineLayout() {
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
        renderResource.getGlbalUboSetLayout()->get()
    };
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
    if(vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS){
        throw std::runtime_error("Failded to create pipeline layout!");
    }
}

void naPointLightSystem::createPipeline(VkRenderPass renderPass){
    PipelineConfigInfo pipelineConfig{};
    naPipeline::defaultPiplineConfigInfo(pipelineConfig);
    naPipeline::enableAlphaBlending(pipelineConfig);
    pipelineConfig.bindingDescriptions.clear();
    pipelineConfig.attributeDescriptions.clear();
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = pipelineLayout;
    pipelineConfig.subpass = 1;
    pipeline = std::make_unique<naPipeline>(device, "point_light_vertex.vert", "point_light_fragment.frag", pipelineConfig);
}

//     static const mathpls::mat4 rotate = 
//         mathpls::translate(mathpls::vec3{0, 0, 2.5f}, 
//             mathpls::rotate(mathpls::vec3{0, 1.f, 0}, 0.01, 
//                 mathpls::translate(mathpls::vec3{0, 0, -2.5})));

void naPointLightSystem::render(const RenderScene& scene, VkCommandBuffer commandBuffer) {
    auto num_lights = static_cast<uint32_t>(scene.m_PointLights.size());
    if (num_lights == 0)
        return;

    pipeline->bind(commandBuffer);
    
    auto global_ubo = renderResource.getGlobalUboDescriptorSet();
    vkCmdBindDescriptorSets(commandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout,
                            0, 1,
                            &global_ubo,
                            0, nullptr);

    vkCmdDraw(commandBuffer, 6, num_lights, 0, 0);
}

}
