//
//  naUIRenderer.cpp
//  nary
//
//  Created by Ninter6 on 2023/9/3.
//

#include "naUISystem.hpp"
#include "naFrameBuffer.hpp"

#include "cpix_font.h"

namespace nary {

naUISystem::naUISystem(naDevice& device, VkRenderPass renderPass, naWin& window) : device(device), window(window) {
    createDescriptorPool();
    initImGui(renderPass);
}

naUISystem::~naUISystem() {
    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplVulkan_Shutdown();
    ImGui::DestroyContext();
    
    vkDestroyDescriptorPool(device.device(), descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device.device(), descriptorSetLayout, nullptr);
}

void naUISystem::createDescriptorPool() {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional
    
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;
    
    if (vkCreateDescriptorSetLayout(device.device(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
    
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 2;
    
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 2;
    
    if (vkCreateDescriptorPool(device.device(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
    
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    
    if (vkAllocateDescriptorSets(device.device(), &allocInfo, &descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }
}

void naUISystem::initImGui(VkRenderPass renderPass) {
    ImGui::CreateContext();
    auto io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
//    io.ConfigFlags |= ImGuiViewportFlags_NoDecoration;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiCol_DockingEmptyBg;
    
    io.Fonts->AddFontFromMemoryCompressedBase85TTF(cpix_compressed_data_base85, 14, nullptr, io.Fonts->GetGlyphRangesChineseFull());
    
    ImGui_ImplGlfw_InitForVulkan(window.glfw(), true);
    
    ImGui_ImplVulkan_InitInfo info;
    info.DescriptorPool = descriptorPool;
    info.RenderPass = renderPass;
    info.Device = device.device();
    info.PhysicalDevice = device.physicalDevice();
    info.ImageCount = naSwapChain::MAX_FRAMES_IN_FLIGHT;
    info.MsaaSamples = VK_SAMPLE_COUNT_1_BIT; // it will directly render to the swapchain
    ImGui_ImplVulkan_Init(&info);
    
    auto commandBuffer = device.beginSingleTimeCommands();
    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    device.endSingleTimeCommands(commandBuffer);
    
    vkDeviceWaitIdle(device.device());
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void naUISystem::beginFrame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void naUISystem::endFrame(VkCommandBuffer commandBuffer) {
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer, 0, nullptr);
}

}
