//
//  naRenderer.hpp
//  nary
//
//  Created by Ninter6 on 2023/8/18.
//

#pragma once

#include "naWin.hpp"
#include "naSwapChain.hpp"
#include "naGameObject.hpp"
#include "naFrameBuffer.hpp"

namespace nary {
class naRenderer {
public:
    naRenderer(naWin& window, naDevice& device);
    ~naRenderer();
    
    naRenderer(const naRenderer&) = delete;
    naRenderer operator=(const naRenderer&) = delete;
    
    VkRenderPass getSwapChainRenderPass() {return swapChain->getRenderPass();}
    VkImageView getSwapChainImageView(int index) {return swapChain->getImageView(index);}
    VkFormat getSwapChainImageFormat() {return swapChain->getSwapChainImageFormat();}
    VkExtent2D getSwapChainExtent() {return swapChain->getSwapChainExtent();}
    float getAspectRatio() const {return swapChain->extentAspectRatio();}
    bool isFrameInProgress() const {return isFrameStarted;}
    VkRenderPass getRenderPass() const {return m_FrameBuffer->getRenderPass();}
    naFrameBuffer& getFrameBuffer() {return *m_FrameBuffer;}
    
    VkCommandBuffer getCurrentCommandBuffer() const {
        assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
        return commandBuffers[currentFrameIndex];
    }
    
    int getFrameIndex() const {
        assert(isFrameStarted && "Cannot get frame index when frame not in progress");
        return currentFrameIndex;
    }
    
    VkCommandBuffer beginFrame();
    void endFrame();
    void beginRenderPass();
    void endRenderPass();
    void beginSwapChainRenderPass();
    void endSwapChainRenderPass();
    void nextSubpass() const;
    
private:
    void createCommandBuffer();
    void freeCommandBuffers();
    void recreateSwapChain();
    void createFrameBuffer();
    
    naWin& window;
    naDevice& device;
    std::unique_ptr<naSwapChain> swapChain;
    std::vector<VkCommandBuffer> commandBuffers;
    
    std::unique_ptr<naFrameBuffer> m_FrameBuffer;
    
    uint32_t currentImageIndex;
    int currentFrameIndex = 0;
    bool isFrameStarted = false;
};

}
