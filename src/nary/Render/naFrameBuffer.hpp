//
//  naFrameBuffer.hpp
//  nary
//
//  Created by Ninter6 on 2023/12/8.
//

#pragma once

#include "naDevice.hpp"
#include "naImage.hpp"

namespace nary {

struct FrameBufferGroup {
    VkFramebuffer frameBuffer;
    std::vector<naImage> images;
};

class naFrameBuffer {
public:
    
    static VkSampleCountFlagBits MsaaSamples;
    
    struct Builder {
        Builder(naDevice& device);
        Builder(const Builder&) = delete;
        Builder& operator=(const Builder&) = delete;
        
        /**
         * set the image count for each resource
         */
        Builder& setImageCount(uint32_t imageCount);
        Builder& setImageExtent(uint32_t width, uint32_t height);
        Builder& setImageExtent(VkExtent2D extent);
        Builder& addColorResources(uint32_t count, bool enableMSAA, VkFormat format);
        Builder& addDepthResources(uint32_t count, bool enableMSAA, VkFormat format);
        /**
         * Call this function before adding supasses.You cant add resources after calling it.
         */
        Builder& finishResourceAddition();
        /**
         * Add a subpass. Resources' indices depend on the order of addition.
         * @param inputResourceIndices use the global indices of all attachments. [reselove, color, depth]
         * @param colorResourceIndices declare the color resourses for this subpass, you can give an empty array if need
         * @param depthResourceIndex declare the depth resourse for this subpass, set to '-1' (or any negative num) to disable
         */
        Builder& addSubpass(std::vector<uint32_t>const& inputResourceIndices,
                            std::vector<uint32_t>const& colorResourceIndices,
                            int32_t depthResourceIndex = -1);
        Builder& addDependency(const VkSubpassDependency& dependency);
        std::unique_ptr<naFrameBuffer> build();
        
    private:
        // <Attachment, Reference>
        using AttachmentPair = std::pair<VkAttachmentDescription, VkAttachmentReference>;
        // <Color attchment pair, Resolve attchment pair - if enabled MSAA>
        using ColorAttachmentPair = std::pair<AttachmentPair, std::optional<AttachmentPair>>;
        
        void createColorAttachment(bool enableMSAA, VkFormat format);
        void createDepthAttachment(bool enableMSAA, VkFormat format);
        void createSuppass(std::span<const uint32_t> inputResourceIndices,
                           std::span<const uint32_t> colorResourceIndices,
                           int32_t depthResourceIndex);
        void createDefaultDependency();
        
        void mergeAttachments();
        
    private:
        naDevice& device;
        
        std::vector<VkSubpassDescription> subpasses;
        std::vector<VkSubpassDependency> dependencies;
        std::vector<ColorAttachmentPair> colorAttachments;
        std::vector<AttachmentPair> depthAttachments;
        std::vector<VkAttachmentDescription> attachments;
        // store attachment reference arrays created for subpass if necessary
        std::vector<std::vector<VkAttachmentReference>> attachmentRefs;
        
        uint32_t imageCount = 1;
        VkExtent2D imageExtent{};
    };
    
    naFrameBuffer(naDevice& device,
                  std::span<const VkSubpassDescription> subpasses,
                  std::span<const VkSubpassDependency> dependencies,
                  std::span<const VkAttachmentDescription> attachments,
                  uint32_t imageCount,
                  VkExtent2D imageExtent);
    ~naFrameBuffer();
    naFrameBuffer(const naFrameBuffer&) = delete;
    naFrameBuffer& operator=(const naFrameBuffer&) = delete;
    
    uint32_t getImageCount() const {return static_cast<uint32_t>(m_Groups.size());}
    uint32_t getAttanchmentCount() const {return static_cast<uint32_t>(m_Groups[0].images.size());}
    VkExtent2D getImageExtent() const {return m_ImageExtent;}
    VkRenderPass getRenderPass() const {return m_RenderPass;}
    VkFramebuffer get(uint32_t index) const {return m_Groups[index].frameBuffer;}
    FrameBufferGroup& getGroup(uint32_t index) {return m_Groups[index];}
    const FrameBufferGroup& getGroup(uint32_t index) const {return m_Groups[index];}
    
private:
    
    naDevice& device;
    
    std::vector<FrameBufferGroup> m_Groups;
    
    VkExtent2D m_ImageExtent;
    VkRenderPass m_RenderPass;
    
private:
    
    void createRenderPass(std::span<const VkSubpassDescription> subpasses,
                          std::span<const VkSubpassDependency> dependencies,
                          std::span<const VkAttachmentDescription> attachments);
    void createFrameBufferGroups(std::span<const VkAttachmentDescription> attachments);
};

}
