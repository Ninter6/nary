//
//  naPointLightSystem.hpp
//  nary
//
//  Created by Ninter6 on 2023/8/23.
//

#pragma once

#include "naPipeline.hpp"
#include "naRenderer.hpp"
#include "naGameObject.hpp"
#include "RenderResource.hpp"
#include "RenderScene.hpp"

namespace nary {

class naPointLightSystem {
public:
    naPointLightSystem(naDevice& device, VkRenderPass renderPass, const RenderResource& renderResource);
    ~naPointLightSystem();
    
    naPointLightSystem(const naPointLightSystem&) = delete;
    naPointLightSystem operator=(const naPointLightSystem&) = delete;
    
    void render(const RenderScene& scene, VkCommandBuffer commandBuffer);
    
private:
    void createPipelineLayout();
    void createPipeline(VkRenderPass renderPass);
    
    naDevice& device;
    const RenderResource& renderResource;
    
    std::unique_ptr<naPipeline> pipeline;
    VkPipelineLayout pipelineLayout;
};

}
