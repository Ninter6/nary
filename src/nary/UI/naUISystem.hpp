//
//  naUISystem.hpp
//  nary
//
//  Created by Ninter6 on 2023/9/3.
//

#pragma once

#include "naDevice.hpp"
#include "naWin.hpp"
#include "naSwapChain.hpp"
#include "naDescriptors.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan_but_better.h"

namespace nary {

class naUISystem {
public:
    naUISystem(naDevice& device, VkRenderPass renderPass, naWin& window);
    ~naUISystem();
    naUISystem(const naUISystem&) = delete;
    naUISystem& operator=(const naUISystem&) = delete;
    
    void beginFrame();
    void endFrame(VkCommandBuffer commandBuffer);
    
private:
    naDevice& device;
    naWin& window;
    
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;
    
    void createDescriptorPool();
    void initImGui(VkRenderPass renderPass);
};

}
