//
//  naImage.cpp
//  nary
//
//  Created by Ninter6 on 2023/11/11.
//

#include "naImage.hpp"
#include "VulkanUtil.hpp"

#include "resource_path.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "se_tools.h"

namespace nary {

naImage::naImage(naDevice& device, const ImageInfo& info, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageLayout finalLayout)
: device(device), m_Info(info), m_Layout(finalLayout) {
    createImage(device, info, tiling, usage, properties, m_Image, m_ImageMemory);
    createImageView();
}

naImage::~naImage() {
    if (m_ImageView != VK_NULL_HANDLE)
        vkDestroyImageView(device.device(), m_ImageView, nullptr);
    if (m_Image != VK_NULL_HANDLE)
        vkDestroyImage(device.device(), m_Image, nullptr);
    if (m_ImageMemory != VK_NULL_HANDLE)
        vkFreeMemory(device.device(), m_ImageMemory, nullptr);
}

naImage::naImage(naImage&& o)
: device(o.device), m_Info(o.m_Info), m_Layout(o.m_Layout) {
    std::swap(m_Image, o.m_Image); // after swaping, o.m_Image becomes null
    std::swap(m_ImageView, o.m_ImageView);
    std::swap(m_ImageMemory, o.m_ImageMemory);
}

naImage naImage::loadImageFromFile(naDevice& device, std::string_view filename) {
    ImageInfo info;
    auto data = loadImageFile(filename, info);
    
    auto stagingBuffer = stageBuffer(device, {data, info.width * info.height * 4});
    
    auto image = naImage{
        device,
        info,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    };
    
    image.transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    image.copyBufferToImage(*stagingBuffer);
    image.generateMipmaps();
    
    return image;
}

naImage naImage::createWithImageData(naDevice& device, std::span<const uint8_t> data, const ImageInfo& info) {
    
    auto stagingBuffer = stageBuffer(device, data);
    
    auto image = naImage{
        device,
        info,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    };
    
    image.transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    image.copyBufferToImage(*stagingBuffer);
    image.transitionImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
    return image;
}

uint8_t* naImage::loadImageFile(std::string_view filename, ImageInfo& info) {
    int texWidth, texHeight, texChannels;
    
    stbi_uc* pixels = stbi_load(res::fullname(res::imagePath, filename).c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    
    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }
    
    info.width = texWidth;
    info.height = texHeight;
    info.format = VK_FORMAT_R8G8B8A8_SRGB;
    info.mipLevels = std::floor(std::log2(std::max(texWidth, texHeight))) + 1;
    
    return pixels;
}

std::unique_ptr<naBuffer> naImage::stageBuffer(naDevice& device, std::span<const uint8_t> data) {
    VkDeviceSize imageSize = data.size();
    
    return naBuffer::createStagingBuffer(device, (void*)data.data(), imageSize);
}

void naImage::createImage(naDevice& device, const ImageInfo& info, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
    
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = info.width;
    imageInfo.extent.height = info.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = info.mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = info.format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = info.numSamples;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    device.createImageWithInfo(imageInfo, properties, image, imageMemory);
}

void naImage::createImageView() {
    m_ImageView = device.createImageView(m_Image, m_Info.format, m_Info.mipLevels);
}

void naImage::generateMipmaps() {
    // Check if image format supports linear blitting
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(device.physicalDevice(), m_Info.format, &formatProperties);
    
    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }
    
    VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();
    
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = m_Image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;
    
    int32_t mipWidth = m_Info.width;
    int32_t mipHeight = m_Info.height;
    
    for (uint32_t i = 1; i < m_Info.mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        
        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);
        
        VkImageBlit blit{};
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;
        
        vkCmdBlitImage(commandBuffer,
                       m_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       m_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &blit,
                       VK_FILTER_LINEAR);
        
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        
        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);
        
        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }
    
    barrier.subresourceRange.baseMipLevel = m_Info.mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    
    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);
    
    device.endSingleTimeCommands(commandBuffer);
    
    m_Layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // record the new layout
}

void naImage::transitionImageLayout(VkImageLayout newLayout) {
    auto commandBuffer = device.beginSingleTimeCommands();
    
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = m_Layout;
    barrier.newLayout = newLayout;
    
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    
    barrier.image = m_Image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = m_Info.mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    
    barrier.srcAccessMask = 0; // TODO
    barrier.dstAccessMask = 0; // TODO
    
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (m_Layout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (m_Layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }
    
    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
    
    device.endSingleTimeCommands(commandBuffer);
    
    m_Layout = newLayout; // record the new layout
}

void naImage::copyBufferToImage(const naBuffer& buffer) {
    device.copyBufferToImage(buffer.getBuffer(), m_Image, m_Info.width, m_Info.height, 1);
}

naSampler::naSampler(naDevice& device, SamplerType type, uint32_t mip_level) {
    switch (type) {
    case SamplerType::Linear:
        m_Sampler = VulkanUtil::getOrCreateLinearSampler(device.physicalDevice(), device.device());
        break;
    case SamplerType::Nearest:
        m_Sampler = VulkanUtil::getOrCreateNearestSampler(device.physicalDevice(), device.device());
        break;
    case SamplerType::Mipmap:
        m_Sampler = VulkanUtil::getOrCreateMipmapSampler(device.physicalDevice(), device.device(), mip_level);
        break;
    
    default:
        WARNING_LOG("Unknow samplaer type!");
        break;
    }
}

VkDescriptorImageInfo naSampler::descriptorInfo(naImage& image) const {
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = image.m_Layout;
    imageInfo.imageView = image.m_ImageView;
    imageInfo.sampler = m_Sampler;
    
    return imageInfo;
}

}
