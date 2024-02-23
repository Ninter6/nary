//
//  naShadowSystem.hpp
//  nary
//
//  Created by Ninter6 on 2023/12/16.
//

#pragma once

#include "naPipeline.hpp"
#include "naRenderer.hpp"
#include "naGameObject.hpp"
#include "RenderResource.hpp"
#include "RenderScene.hpp"

namespace nary {

class naShadowSystem {
public:
    naShadowSystem(naDevice& device, VkRenderPass renderPass, const RenderResource& renderResource);
    ~naShadowSystem();
    
    naShadowSystem(const naShadowSystem&) = delete;
    naShadowSystem operator=(const naShadowSystem&) = delete;
    
    void renderGameObjects(const RenderScene& scene, VkCommandBuffer commandBuffer);
    
private:
    void createPipelineLayout();
    void createPipeline(VkRenderPass renderPass);
    
    naDevice& device;
    const RenderResource& renderResource;
    
    std::unique_ptr<naPipeline> pipeline;
    VkPipelineLayout pipelineLayout;
};

}
