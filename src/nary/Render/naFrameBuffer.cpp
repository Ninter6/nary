//
//  naFrameBuffer.cpp
//  nary
//
//  Created by Ninter6 on 2023/12/8.
//

#include "naFrameBuffer.hpp"
#include "naSwapChain.hpp"

namespace nary {

naFrameBuffer::Builder::Builder(naDevice& device) : device(device) {}

naFrameBuffer::Builder& naFrameBuffer::Builder::setImageCount(uint32_t imageCount) {
    this->imageCount = imageCount;
    return *this;
}

naFrameBuffer::Builder& naFrameBuffer::Builder::setImageExtent(uint32_t width,
                                                               uint32_t height) {
    imageExtent = {width, height};
    return *this;
}

naFrameBuffer::Builder& naFrameBuffer::Builder::setImageExtent(VkExtent2D extent) {
    imageExtent = extent;
    return *this;
}

void naFrameBuffer::Builder::createColorAttachment(bool enableMSAA, VkFormat format) {
    ColorAttachmentPair colorAttachmentPair;
    auto& [colorAttachment, colorAttachmentRef] = colorAttachmentPair.first;
    
    colorAttachment.format = format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    if (enableMSAA) {
        VkAttachmentDescription resolveAttachment{};
        VkAttachmentReference resolveAttachmentRef{};
        
        colorAttachment.samples = MsaaSamples;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        
        resolveAttachment.format = format;
        resolveAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        resolveAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        resolveAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        resolveAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        resolveAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        resolveAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        resolveAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        
        resolveAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        
        colorAttachmentPair.second = {resolveAttachment, resolveAttachmentRef};
    }
    
    colorAttachments.push_back(colorAttachmentPair);
}

void naFrameBuffer::Builder::createDepthAttachment(bool enableMSAA, VkFormat format) {
    AttachmentPair depthAttachmentPair;
    auto& [depthAttachment, depthAttachmentRef] = depthAttachmentPair;
    
    depthAttachment.format = format;
    depthAttachment.samples = enableMSAA ? MsaaSamples : VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    depthAttachments.push_back(depthAttachmentPair);
}

void naFrameBuffer::Builder::createSuppass(std::span<const uint32_t> inputResourceIndices,
                                           std::span<const uint32_t> colorResourceIndices,
                                           int32_t depthResourceIndex) {
    VkAttachmentReference *pColorAttachments = nullptr, *pResolveAttachments = nullptr;
    if (colorResourceIndices.size() != 1) {
        std::vector<VkAttachmentReference> colorRef, resolveRef;
        for (auto i : colorResourceIndices) {
            colorRef.push_back(colorAttachments[i].first.second);
            if (colorAttachments[i].second.has_value())
                resolveRef.push_back(colorAttachments[i].second->second);
        }
        attachmentRefs.push_back(std::move(colorRef));
        pColorAttachments = attachmentRefs.back().data();
        attachmentRefs.push_back(std::move(resolveRef));
        pResolveAttachments = attachmentRefs.back().data();
    } else {
        pColorAttachments = &colorAttachments[colorResourceIndices.front()].first.second;
        if (colorAttachments[colorResourceIndices.front()].second.has_value())
            pResolveAttachments = &colorAttachments[colorResourceIndices.front()].second->second;
    }
    
    VkAttachmentReference *pInputAttanchments = nullptr;
    if (inputResourceIndices.size() > 0) {
        std::vector<VkAttachmentReference> inputRef;
        for (auto i : inputResourceIndices) {
            VkAttachmentReference ref{};
            ref.attachment = i;
            ref.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            inputRef.push_back(ref);
        }
        attachmentRefs.push_back(std::move(inputRef));
        pInputAttanchments = attachmentRefs.back().data();
    }
    
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(colorResourceIndices.size());
    subpass.pColorAttachments = pColorAttachments;
    subpass.pResolveAttachments = pResolveAttachments;
    subpass.inputAttachmentCount = static_cast<uint32_t>(inputResourceIndices.size());
    subpass.pInputAttachments = pInputAttanchments;
    if (depthResourceIndex >= 0)
        subpass.pDepthStencilAttachment = &depthAttachments[depthResourceIndex].second;
    
    subpasses.push_back(subpass);
}

void naFrameBuffer::Builder::createDefaultDependency() {
    uint32_t currentIndex = static_cast<uint32_t>(dependencies.size());
    
    VkSubpassDependency dependency{};
    dependency.dstSubpass = currentIndex;
    dependency.dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcSubpass = currentIndex;
    dependency.srcAccessMask = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    
    dependencies.push_back(dependency);
}

void naFrameBuffer::Builder::mergeAttachments() {
    size_t attachmentSize = colorAttachments.size() * 2 + dependencies.size();
    assert(attachmentSize != 0 && "Called 'finishResourceAddition()' but added no resouce");
    
    // Allocate enough memory
    attachments.reserve(attachmentSize);
    
    for (auto& i : colorAttachments) {
        auto& [attachment, ref] = i.first;
        ref.attachment = static_cast<uint32_t>(attachments.size());
        attachments.push_back(attachment);
    } // add the color attachments
    for (auto& i : colorAttachments) {
        if (!i.second.has_value()) continue;
        auto& [attachment, ref] = i.second.value();
        ref.attachment = static_cast<uint32_t>(attachments.size());
        attachments.push_back(attachment);
    } // add the resolve attachments
    for (auto& i : depthAttachments) {
        auto& [attachment, ref] = i;
        ref.attachment = static_cast<uint32_t>(attachments.size());
        attachments.push_back(attachment);
    } // add the depth attachments
}

naFrameBuffer::Builder& naFrameBuffer::Builder::addColorResources(uint32_t count, bool enableMSAA, VkFormat format) {
    assert(attachments.size() == 0 && "You cant add resources after calling 'finishResourceAddition()'!");
    
    for (int i = 0; i < count; i++) createColorAttachment(enableMSAA, format);
    return *this;
}

naFrameBuffer::Builder& naFrameBuffer::Builder::addDepthResources(uint32_t count, bool enableMSAA, VkFormat format) {
    assert(attachments.size() == 0 && "You cant add resources after calling 'finishResourceAddition()'!");
    
    for (int i = 0; i < count; i++) createDepthAttachment(enableMSAA, format);
    return *this;
}

naFrameBuffer::Builder& naFrameBuffer::Builder::finishResourceAddition() {
    assert(attachments.size() == 0 && "This function cannot be called repeatedly!");
    
    mergeAttachments();
    return *this;
}

naFrameBuffer::Builder& naFrameBuffer::Builder::
addSubpass(const std::vector<uint32_t>& inputResourceIndices,
           const std::vector<uint32_t>& colorResourceIndices,
           int32_t depthResourceIndex) {
    assert(attachments.size() != 0 && "Forget to call 'finishResourceAddition()'!");
    
    createSuppass(inputResourceIndices, colorResourceIndices, depthResourceIndex);
    
    return *this;
}

naFrameBuffer::Builder& naFrameBuffer::Builder::addDependency(const VkSubpassDependency& dependency) {
    dependencies.push_back(dependency);
    
    return *this;
}

std::unique_ptr<naFrameBuffer> naFrameBuffer::Builder::build() {
    // check
    assert(imageExtent.width > 0 && imageExtent.height > 0
           && "illegal image extent or forget to call 'setImageExtent'");
    assert(subpasses.size() > 0 && "forget to add subpasses");
    
    return std::make_unique<naFrameBuffer>(device, subpasses, dependencies, attachments, imageCount, imageExtent);
}

naFrameBuffer::naFrameBuffer(naDevice& device,
                             std::span<const VkSubpassDescription> subpasses,
                             std::span<const VkSubpassDependency> dependencies,
                             std::span<const VkAttachmentDescription> attachments,
                             uint32_t imageCount,
                             VkExtent2D imageExtent)
: device(device), m_Groups(imageCount), m_ImageExtent(imageExtent) {
    createRenderPass(subpasses, dependencies, attachments);
    createFrameBufferGroups(attachments);
}

void naFrameBuffer::createRenderPass(std::span<const VkSubpassDescription> subpasses,
                                     std::span<const VkSubpassDependency> dependencies,
                                     std::span<const VkAttachmentDescription> attachments) {
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
    renderPassInfo.pSubpasses = subpasses.data();
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();
    
    if (vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void naFrameBuffer::createFrameBufferGroups(std::span<const VkAttachmentDescription> attachments) {
    for (auto& group : m_Groups) {
        group.images.reserve(attachments.size());
        std::vector<VkImageView> attachmentImages;
        
        for (auto& i : attachments) {
            ImageInfo info;
            info.width = m_ImageExtent.width;
            info.height = m_ImageExtent.height;
            info.format = i.format;
            info.numSamples = i.samples;
            
            VkImageUsageFlags usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
            switch (i.finalLayout) {
                case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                    usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                    break;
                    
                case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                    usage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                    break;
                    
                case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                    // usage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
                    usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
                    break;
                    
                default:
                    break;
            }
            
            group.images.emplace_back(device, info, VK_IMAGE_TILING_OPTIMAL, usage,  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, i.finalLayout);
            attachmentImages.push_back(group.images.back().getView());
        }
        
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_RenderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentImages.size());
        framebufferInfo.pAttachments = attachmentImages.data();
        framebufferInfo.width = m_ImageExtent.width;
        framebufferInfo.height = m_ImageExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(
                device.device(),
                &framebufferInfo,
                nullptr,
                &group.frameBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

naFrameBuffer::~naFrameBuffer() {
    vkDestroyRenderPass(device.device(), m_RenderPass, nullptr);
    for (auto& i : m_Groups) {
        vkDestroyFramebuffer(device.device(), i.frameBuffer, nullptr);
    }
}

}
