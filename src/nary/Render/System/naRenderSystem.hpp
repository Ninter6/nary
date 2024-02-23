//
//  naRenderSystem.hpp
//  nary
//
//  Created by Ninter6 on 2023/8/19.
//

#pragma once

#include "naPipeline.hpp"
#include "naGameObject.hpp"
#include "RenderResource.hpp"
#include "RenderScene.hpp"

namespace nary {

class naRenderSystem {
public:
    naRenderSystem(naDevice& device, VkRenderPass renderPass, const RenderResource& renderResource);
    ~naRenderSystem();
    
    naRenderSystem(const naRenderSystem&) = delete;
    naRenderSystem operator=(const naRenderSystem&) = delete;
    
    void renderGameObjects(const RenderScene& renderScene, VkCommandBuffer commandBuffer, VkDescriptorSet shadowMapDescriptorSet);
    
private:
    void createPipelineLayout();
    void createPipeline(VkRenderPass renderPass);
    
    naDevice& device;
    const RenderResource& renderResource;
    
    std::unique_ptr<naPipeline> pipeline;
    VkPipelineLayout pipelineLayout;
};

}
