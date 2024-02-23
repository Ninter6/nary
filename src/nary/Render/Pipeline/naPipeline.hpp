//
//  naPipeline.hpp
//  nary
//
//  Created by Ninter6 on 2023/5/2.
//

#pragma once

#include "naDevice.hpp"

#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include <iostream>
#include <fstream>

namespace nary {
struct PipelineConfigInfo{
    PipelineConfigInfo() = default;
    PipelineConfigInfo(const PipelineConfigInfo&) = delete;
    PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;
    
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    VkPipelineViewportStateCreateInfo viewportInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    std::vector<VkDynamicState> dynamicStaticEnables;
    VkPipelineDynamicStateCreateInfo dynamicStaticInfo;
    VkPipelineLayout pipelineLayout = nullptr;
    VkRenderPass renderPass = nullptr;
    uint32_t subpass = 0;
};

class naPipeline {
public:
    naPipeline(naDevice& device, const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo);
    ~naPipeline();
    
    naPipeline(const naPipeline&) = delete;
    naPipeline operator=(const naPipeline&) = delete;
    
    void bind(VkCommandBuffer commandbuffer);
    
    static void defaultPiplineConfigInfo(PipelineConfigInfo& configInfo);
    static void enableAlphaBlending(PipelineConfigInfo& configInfo);
    
private:
    static std::vector<char> CompileShader(const std::string& path);
    
    void createPipline(const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo);
    
    void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
    
    naDevice& device;
    VkPipeline graphicsPipeline;
    VkShaderModule vertModule, fragModule;
    
};
}

