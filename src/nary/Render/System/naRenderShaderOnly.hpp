//
//  naRenderShaderOnly.hpp
//  demo
//
//  Created by Ninter6 on 2023/11/25.
//

#pragma once

#include "naPipeline.hpp"
#include "RenderResource.hpp"

namespace nary {

class naRenderShaderOnly {
public:
    naRenderShaderOnly(naDevice& device, VkRenderPass renderPass, const RenderResource& renderResource);
    ~naRenderShaderOnly();
    
    naRenderShaderOnly(const naRenderShaderOnly&) = delete;
    naRenderShaderOnly operator=(const naRenderShaderOnly&) = delete;
    
    void render(VkDescriptorSet inputImageDescriptorSet,
                VkCommandBuffer commandBuffer,
                uint32_t vertexCount,
                uint32_t instanceCount,
                uint32_t firstVertex = 0,
                uint32_t firstInstance = 0);
    
    naRenderShaderOnly& setShaders(const std::string& vertFile, const std::string& fragFile, bool enableMSAA = false);
    
private:
    void createPipelineLayout();
    void createPipeline(const std::string& vertFile, const std::string& fragFile, bool enableMSAA);
    
    naDevice& device;
    const RenderResource& renderResource;
    
    std::unique_ptr<naPipeline> pipeline = nullptr;
    VkPipelineLayout pipelineLayout;
    VkRenderPass renderPass;
    
};

}
