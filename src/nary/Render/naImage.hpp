//
//  naImage.hpp
//  nary
//
//  Created by Ninter6 on 2023/11/11.
//

#pragma once

#include "naDevice.hpp"
#include "naBuffer.hpp"

#include <span>

namespace nary {

struct ImageInfo {
    uint32_t width, height;
    uint32_t mipLevels = 1;
    VkFormat format;
    VkSampleCountFlagBits numSamples = VK_SAMPLE_COUNT_1_BIT;
};

class naImage {
public:
    naImage(naDevice& device, const ImageInfo& info, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageLayout finalLayout/*for images that will change layout automicly*/ = VK_IMAGE_LAYOUT_UNDEFINED);
    ~naImage();
    
    naImage(naImage&&);
    
    naImage(const naImage&) = delete;
    naImage& operator=(const naImage&) = delete;
    
    VkImage get() const {return m_Image;}
    VkImageView getView() const {return m_ImageView;}
    ImageInfo getInfo() const {return m_Info;}
    
    static naImage loadImageFromFile(naDevice& device, std::string_view filename);
    static naImage createWithImageData(naDevice& device, std::span<const uint8_t> data, const ImageInfo& info);
    
    /**
     * create a original Vulkan image.
     */
    static void createImage(naDevice& device, const ImageInfo& info, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    
private:
    
    static uint8_t* loadImageFile(std::string_view filename, ImageInfo& info);
    static std::unique_ptr<naBuffer> stageBuffer(naDevice& device, std::span<const uint8_t> data);
    
    void generateMipmaps();
    void transitionImageLayout(VkImageLayout newLayout);
    void copyBufferToImage(const naBuffer& buffer);
    
    void createImageView();
    
private:
    VkImage m_Image = VK_NULL_HANDLE;
    VkDeviceMemory m_ImageMemory = VK_NULL_HANDLE;
    VkImageView m_ImageView = VK_NULL_HANDLE;
    VkImageLayout m_Layout;
    
    ImageInfo m_Info;
    
    naDevice& device;
    
    friend class naSampler;
    
};

enum class SamplerType {
    Linear,
    Nearest,
    Mipmap
};

class naSampler {
public:
    /**
     * \param mip_level When the sampler type is Mipmap, this parameter needs to be filled in
     */
    naSampler(naDevice& device, SamplerType type, uint32_t mip_level = 0);
    
    VkDescriptorImageInfo descriptorInfo(naImage& image) const;
    
private:
    VkSampler m_Sampler = VK_NULL_HANDLE;
};

}
