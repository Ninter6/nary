//
//  naRenderShaderOnly.cpp
//  demo
//
//  Created by Ninter6 on 2023/11/25.
//

#include "naRenderShaderOnly.hpp"

namespace nary {

naRenderShaderOnly::naRenderShaderOnly(naDevice& device, VkRenderPass renderPass, const RenderResource& renderResource)
: device(device), renderPass(renderPass), renderResource(renderResource) {
    createPipelineLayout();
}

naRenderShaderOnly::~naRenderShaderOnly() {
    vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
}

void naRenderShaderOnly::createPipelineLayout() {
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
        renderResource.getGlbalUboSetLayout()->get(),
        renderResource.getOneImageSetLayout()->get()
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

void naRenderShaderOnly::createPipeline(const std::string& vertFile, const std::string& fragFile, bool enableMSAA){
    PipelineConfigInfo pipelineConfig{};
    naPipeline::defaultPiplineConfigInfo(pipelineConfig);
    naPipeline::enableAlphaBlending(pipelineConfig);
    if (!enableMSAA) pipelineConfig.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipelineConfig.bindingDescriptions.clear();
    pipelineConfig.attributeDescriptions.clear();
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = pipelineLayout;
    pipeline.reset(new naPipeline(device, vertFile, fragFile, pipelineConfig));
}

naRenderShaderOnly& naRenderShaderOnly::setShaders(const std::string& vertFile, const std::string& fragFile, bool enableMSAA) {
    createPipeline(vertFile, fragFile, enableMSAA);
    return *this;
}

void naRenderShaderOnly::render(VkDescriptorSet inputImageDescriptorSet,
                                VkCommandBuffer commandBuffer,
                                uint32_t vertexCount,
                                uint32_t instanceCount,
                                uint32_t firstVertex,
                                uint32_t firstInstance) {
    assert(pipeline && "forget to call 'setShaders'");
    
    pipeline->bind(commandBuffer);

    VkDescriptorSet sets[2]{
        renderResource.getGlobalUboDescriptorSet(),
        inputImageDescriptorSet
    };
    vkCmdBindDescriptorSets(commandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout,
                            0, 2,
                            sets,
                            0, nullptr);

    vkCmdDraw(commandBuffer,
              vertexCount, instanceCount, firstVertex, firstInstance);
}

}
