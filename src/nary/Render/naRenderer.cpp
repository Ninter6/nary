//
//  naRenderer.cpp
//  nary
//
//  Created by Ninter6 on 2023/8/18.
//

#include "naRenderer.hpp"

namespace nary {

naRenderer::naRenderer(naWin& window, naDevice& device)
: window(window), device(device) {
    recreateSwapChain();
    createCommandBuffer();
    createFrameBuffer();
}

naRenderer::~naRenderer(){
    freeCommandBuffers();
}

void naRenderer::createCommandBuffer(){
    commandBuffers.resize(naSwapChain::MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = device.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
    if(vkAllocateCommandBuffers(device.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS){
        throw std::runtime_error("Failed to allocate command buffers!");
    }
}

void naRenderer::recreateSwapChain(){
    auto extent = window.getExtent();
    while (extent.width == 0 || extent.height == 0) {
        extent = window.getExtent();
        glfwWaitEvents();
    }
    
    vkDeviceWaitIdle(device.device());
    
    if (swapChain == nullptr)
        swapChain = std::make_unique<naSwapChain>(device, extent);
    else{
        std::shared_ptr<naSwapChain> oldSwapChain = std::move(swapChain);
        swapChain = std::make_unique<naSwapChain>(device, extent, oldSwapChain);
        if (!oldSwapChain->compareSwapFormats(*swapChain.get())) {
            throw std::runtime_error("Swap chain image(or depth) format has changed!");
        }
    }
}

void naRenderer::createFrameBuffer() {
    VkSubpassDependency dependency{};
    dependency.dstSubpass = 1;
    dependency.dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcSubpass = 0;
    dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    
    m_FrameBuffer = naFrameBuffer::Builder(device)
        .setImageCount(naSwapChain::MAX_FRAMES_IN_FLIGHT)
        .setImageExtent(swapChain->getSwapChainExtent())
        .addColorResources(1, true, swapChain->getSwapChainImageFormat())
        .addColorResources(1, true, VK_FORMAT_R8_UNORM)
        .addDepthResources(2, true, swapChain->findDepthFormat())
        .finishResourceAddition()
        .addSubpass({}, {1}, 1)
        .addSubpass({3}, {0}, 0)
        .addDependency(dependency)
        .build();
}

void naRenderer::freeCommandBuffers(){
    vkFreeCommandBuffers(device.device(), device.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
    commandBuffers.clear();
}

VkCommandBuffer naRenderer::beginFrame() {
    assert(!isFrameStarted && "Can't call beginFrame while already in progress");

    auto result = swapChain->acquireNextImage(&currentImageIndex);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return nullptr;
    }
    if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
        throw std::runtime_error("Failed to acquire swap chain image!");
    }
    
    isFrameStarted = true;
    
    auto commandBuffer = getCurrentCommandBuffer();
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if(vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS){
        throw std::runtime_error("Failed to begin recording command buffer!");
    }
    return commandBuffer;
}

void naRenderer::endFrame() {
    assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
    auto commandBuffer = getCurrentCommandBuffer();
    
    if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS){
        throw std::runtime_error("Failed to record command buffer!");
    }
    
    auto result = swapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasWindowResized()) {
        recreateSwapChain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to present swap chain image!");
    }
    
    isFrameStarted = false;
    currentFrameIndex = (currentFrameIndex + 1) % naSwapChain::MAX_FRAMES_IN_FLIGHT;
}

void naRenderer::beginRenderPass() {
    assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
    
    auto commandBuffer = getCurrentCommandBuffer();
    
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = getRenderPass();
    renderPassInfo.framebuffer = m_FrameBuffer->get(currentFrameIndex);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_FrameBuffer->getImageExtent();
    
    std::vector<VkClearValue> clearValue(m_FrameBuffer->getAttanchmentCount());
    clearValue[0].color = {
        pow(0.1f, 2.2f),
        pow(0.15f, 2.2f),
        pow(0.15f, 2.2f),
        1.0f};
    clearValue[1].color = {1.f, 0, 0, 1.0f};
    clearValue[4].depthStencil = {1.0f, 0};
    clearValue[5].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValue.size());
    renderPassInfo.pClearValues = clearValue.data();
    
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    VkViewport viewport{};
    viewport.x = 0.f;
    viewport.y = 0.f;
    viewport.width = static_cast<float>(m_FrameBuffer->getImageExtent().width);
    viewport.height = static_cast<float>(m_FrameBuffer->getImageExtent().height);
    viewport.maxDepth = 1.f;
    viewport.minDepth = 0.f;
    VkRect2D scissor{{0, 0}, m_FrameBuffer->getImageExtent()};
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void naRenderer::endRenderPass() {
    assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
    vkCmdEndRenderPass(getCurrentCommandBuffer());
}

void naRenderer::beginSwapChainRenderPass() {
    assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
    
    auto commandBuffer = getCurrentCommandBuffer();
    
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = swapChain->getRenderPass();
    renderPassInfo.framebuffer = swapChain->getFrameBuffer(currentImageIndex);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChain->getSwapChainExtent();
    
    std::array<VkClearValue, 2> clearValue{};
    clearValue[0].color = {
        static_cast<float>(pow(0.1f, 2.2)),
        static_cast<float>(pow(0.15f, 2.2)),
        static_cast<float>(pow(0.15f, 2.2)),
        1.0f};
    clearValue[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValue.size());
    renderPassInfo.pClearValues = clearValue.data();
    
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    VkViewport viewport{};
    viewport.x = 0.f;
    viewport.y = static_cast<float>(swapChain->getSwapChainExtent().height);
    viewport.width = static_cast<float>(swapChain->getSwapChainExtent().width);
    viewport.height =-static_cast<float>(swapChain->getSwapChainExtent().height);
    viewport.maxDepth = 1.f;
    viewport.minDepth = 0.f;
    VkRect2D scissor{{0, 0}, swapChain->getSwapChainExtent()};
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void naRenderer::endSwapChainRenderPass() {
    assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
    vkCmdEndRenderPass(getCurrentCommandBuffer());
}

void naRenderer::nextSubpass() const {
    assert(isFrameStarted && "Can't call nextSubpass if frame is not in progress");
    vkCmdNextSubpass(getCurrentCommandBuffer(), VK_SUBPASS_CONTENTS_INLINE);
}

}
