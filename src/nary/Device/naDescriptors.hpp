//
//  naDescriptors.hpp
//  nary
//
//  Created by Ninter6 on 2023/8/22.
//

#pragma once

#include "naDevice.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

namespace nary {

class naDescriptorSetLayout {
public:
    class Builder {
    public:
        Builder(naDevice &device) : device{device} {}
        
        Builder& addBinding(uint32_t binding,
                            VkDescriptorType descriptorType,
                            VkShaderStageFlags stageFlags,
                            uint32_t count = 1);
        std::unique_ptr<naDescriptorSetLayout> build() const;
        
    private:
        naDevice& device;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
    };
    
    naDescriptorSetLayout(naDevice& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
    ~naDescriptorSetLayout();
    naDescriptorSetLayout(const naDescriptorSetLayout&) = delete;
    naDescriptorSetLayout& operator=(const naDescriptorSetLayout &) = delete;
    
    VkDescriptorSetLayout get() const { return descriptorSetLayout; }
    
private:
    naDevice& device;
    VkDescriptorSetLayout descriptorSetLayout;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;
    
    friend class naDescriptorWriter;
};

class naDescriptorPool {
public:
    class Builder {
    public:
        Builder(naDevice &device) : device{device} {}
        
        Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
        Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
        Builder& setMaxSets(uint32_t count);
        std::unique_ptr<naDescriptorPool> build() const;
        
    private:
        naDevice &device;
        std::vector<VkDescriptorPoolSize> poolSizes{};
        uint32_t maxSets = 1000;
        VkDescriptorPoolCreateFlags poolFlags = 0;
    };
    
    naDescriptorPool(naDevice &device,
                     uint32_t maxSets,
                     VkDescriptorPoolCreateFlags poolFlags,
                     const std::vector<VkDescriptorPoolSize> &poolSizes);
    ~naDescriptorPool();
    naDescriptorPool(const naDescriptorPool&) = delete;
    naDescriptorPool& operator=(const naDescriptorPool&) = delete;
    
    bool allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;
    
    void freeDescriptors(uint32_t count, VkDescriptorSet* sets) const;
    
    void resetPool();
    
    VkDescriptorPool release() {
        auto r = descriptorPool;
        descriptorPool = nullptr;
        return r;
    }
    
private:
    naDevice& device;
    VkDescriptorPool descriptorPool;
    
    friend class naDescriptorWriter;
};

class naDescriptorWriter {
public:
    naDescriptorWriter(naDescriptorSetLayout& setLayout, naDescriptorPool& pool);
    
    naDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
    naDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);
    
    bool build(VkDescriptorSet& set);
    void overwrite(VkDescriptorSet& set);
    
private:
    naDescriptorSetLayout& setLayout;
    naDescriptorPool& pool;
    std::vector<VkWriteDescriptorSet> writes;
};

}
