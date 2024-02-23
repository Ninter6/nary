//
//  naDescriptors.cpp
//  nary
//
//  Created by Ninter6 on 2023/8/22.
//

#include "naDescriptors.hpp"

#include <cassert>
#include <stdexcept>

namespace nary {

// *************** Descriptor Set Layout Builder *********************

naDescriptorSetLayout::Builder& naDescriptorSetLayout::Builder::addBinding(
    uint32_t binding,
    VkDescriptorType descriptorType,
    VkShaderStageFlags stageFlags,
    uint32_t count) {
    assert(bindings.count(binding) == 0 && "Binding already in use");
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = descriptorType;
    layoutBinding.descriptorCount = count;
    layoutBinding.stageFlags = stageFlags;
    bindings[binding] = layoutBinding;
    return *this;
}

std::unique_ptr<naDescriptorSetLayout> naDescriptorSetLayout::Builder::build() const {
    return std::make_unique<naDescriptorSetLayout>(device, bindings);
}

// *************** Descriptor Set Layout *********************

naDescriptorSetLayout::naDescriptorSetLayout(
    naDevice& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
: device{device}, bindings{bindings} {
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
    for (auto kv : bindings) {
        setLayoutBindings.push_back(kv.second);
    }
    
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
    descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();
    
    if (vkCreateDescriptorSetLayout(device.device(),
                                    &descriptorSetLayoutInfo,
                                    nullptr,
                                    &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

naDescriptorSetLayout::~naDescriptorSetLayout() {
    vkDestroyDescriptorSetLayout(device.device(), descriptorSetLayout, nullptr);
}

// *************** Descriptor Pool Builder *********************

naDescriptorPool::Builder& naDescriptorPool::Builder::addPoolSize(
    VkDescriptorType descriptorType, uint32_t count) {
    poolSizes.push_back({descriptorType, count});
    return *this;
}

naDescriptorPool::Builder& naDescriptorPool::Builder::setPoolFlags(
    VkDescriptorPoolCreateFlags flags) {
    poolFlags = flags;
    return *this;
}
naDescriptorPool::Builder& naDescriptorPool::Builder::setMaxSets(uint32_t count) {
    maxSets = count;
    return *this;
}

std::unique_ptr<naDescriptorPool> naDescriptorPool::Builder::build() const {
    return std::make_unique<naDescriptorPool>(device, maxSets, poolFlags, poolSizes);
}

// *************** Descriptor Pool *********************

naDescriptorPool::naDescriptorPool(
    naDevice &device,
    uint32_t maxSets,
    VkDescriptorPoolCreateFlags poolFlags,
    const std::vector<VkDescriptorPoolSize> &poolSizes)
: device{device} {
    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    descriptorPoolInfo.pPoolSizes = poolSizes.data();
    descriptorPoolInfo.maxSets = maxSets;
    descriptorPoolInfo.flags = poolFlags;
    
    if (vkCreateDescriptorPool(device.device(), &descriptorPoolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

naDescriptorPool::~naDescriptorPool() {
    vkDestroyDescriptorPool(device.device(), descriptorPool, nullptr);
}

bool naDescriptorPool::allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    allocInfo.descriptorSetCount = 1;
    
    // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
    // a new pool whenever an old pool fills up. But this is beyond our current scope
    if (vkAllocateDescriptorSets(device.device(), &allocInfo, &descriptor) != VK_SUCCESS) {
        return false;
    }
    return true;
}

void naDescriptorPool::freeDescriptors(uint32_t count, VkDescriptorSet* sets) const {
    vkFreeDescriptorSets(device.device(),
                         descriptorPool,
                         count,
                         sets);
}

void naDescriptorPool::resetPool() {
    vkResetDescriptorPool(device.device(), descriptorPool, 0);
}

// *************** Descriptor Writer *********************

naDescriptorWriter::naDescriptorWriter(naDescriptorSetLayout& setLayout, naDescriptorPool& pool) : setLayout{setLayout}, pool{pool} {}

naDescriptorWriter &naDescriptorWriter::writeBuffer(
    uint32_t binding, VkDescriptorBufferInfo *bufferInfo) {
    assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");
    
    auto &bindingDescription = setLayout.bindings[binding];
    
    assert(
           bindingDescription.descriptorCount == 1 &&
           "Binding single descriptor info, but binding expects multiple");
    
    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pBufferInfo = bufferInfo;
    write.descriptorCount = 1;
    
    writes.push_back(write);
    return *this;
}

naDescriptorWriter& naDescriptorWriter::writeImage(
    uint32_t binding, VkDescriptorImageInfo* imageInfo) {
    assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");
    
    auto &bindingDescription = setLayout.bindings[binding];
    
    assert(
           bindingDescription.descriptorCount == 1 &&
           "Binding single descriptor info, but binding expects multiple");
    
    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pImageInfo = imageInfo;
    write.descriptorCount = 1;
    
    writes.push_back(write);
    return *this;
}

bool naDescriptorWriter::build(VkDescriptorSet& set) {
    bool success = pool.allocateDescriptor(setLayout.get(), set);
    if (!success) {
        return false;
    }
    overwrite(set);
    return true;
}

void naDescriptorWriter::overwrite(VkDescriptorSet& set) {
    for (auto &write : writes) {
        write.dstSet = set;
    }
    vkUpdateDescriptorSets(pool.device.device(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

}
