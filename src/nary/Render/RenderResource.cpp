#include "RenderResource.hpp"

#include <cassert>

namespace nary {

RenderResource::RenderResource(naDevice& device) : p_Device(&device) {
    createDescriptorPool();
    createSetLayouts();
    createGlobalUniformBuffer();
    createDefaulrTexture();
    createDefaultMaterial();
}

void RenderResource::createDescriptorPool() {
    m_DescriptorPool = naDescriptorPool::Builder(*p_Device)
        .setMaxSets(1000)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100)
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100)
        .addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100)
        .build();
}

void RenderResource::createSetLayouts() {
    m_OneImageSetLayout = naDescriptorSetLayout::Builder(*p_Device)
        .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
        .build();
    m_GlobalUboSetLayout = naDescriptorSetLayout::Builder(*p_Device)
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
        .build();
    m_MaterialSetLayout = naDescriptorSetLayout::Builder(*p_Device)
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
        .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
        .build();
}

void RenderResource::createGlobalUniformBuffer() {
    m_GlobalUniformBuffer =
        std::make_unique<naBuffer>(*p_Device, sizeof(GlobalUbo), 1,VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    m_GlobalUniformBuffer->map();

    auto bufferInfo = m_GlobalUniformBuffer->descriptorInfo();
    naDescriptorWriter{*m_GlobalUboSetLayout, *m_DescriptorPool}
        .writeBuffer(0, &bufferInfo)
        .build(m_GlobalUboDescriptorSet);
}

void RenderResource::createDefaultMaterial() {
    Material default_material{};
    default_material.baseColorFactor = {1.f};
    m_Materials[0] = default_material;
    createMaterialUniformBuffers(0);
    updateMaterialDescriptorSet(0);
}

void RenderResource::createDefaulrTexture() {
    ImageInfo info{};
    info.format = VK_FORMAT_R8G8B8A8_UNORM;
    info.width = 1;
    info.height = 1;
    uint8_t data[4]{0, 0, 0, 0}; // default to a transparent all black image
    m_Textures[0] = std::make_unique<naImage>(naImage::createWithImageData(*p_Device, data, info));
}

void RenderResource::createMaterialUniformBuffers(UID id) {
    auto& buffer = m_MaterialUniformBuffers[id];
    buffer = std::make_unique<naBuffer>(*p_Device, 6*sizeof(float), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
}

naDevice* RenderResource::getDevice() const {
    return p_Device;
}

UID RenderResource::addMaterial(const Material& m) {
    auto id = m_Materials.insert(m);
    createMaterialUniformBuffers(id);
    updateMaterialDescriptorSet(id);
    return id;
}

UID RenderResource::addMesh(std::unique_ptr<naModel>&& m) {
    return m_Models.insert(std::move(m));
}

UID RenderResource::addTexture(std::unique_ptr<naImage>&& t) {
    return m_Textures.insert(std::move(t));
}

UidMap<Material>& RenderResource::getMaterials() {
    return m_Materials;
}

Material* RenderResource::getMaterial(UID id) const {
    return const_cast<Material*>(&m_Materials[id]);
}

naModel* RenderResource::getMesh(UID id) const {
    return m_Models[id].get();
}

naImage* RenderResource::getTexture(UID id) const {
    return m_Textures[id].get();
}

naDescriptorPool* RenderResource::getDescriptorPool() const {
    return m_DescriptorPool.get();
}

naDescriptorSetLayout* RenderResource::getOneImageSetLayout() const {
    return m_OneImageSetLayout.get();
}

naDescriptorSetLayout* RenderResource::getGlbalUboSetLayout() const {
    return m_GlobalUboSetLayout.get();
}

naDescriptorSetLayout* RenderResource::getMaterialSetLayout() const {
    return m_MaterialSetLayout.get();
}

void RenderResource::updateGlobalUbo(const GlobalUbo& ubo) const {
    m_GlobalUniformBuffer->writeToBuffer((void*)&ubo);
    m_GlobalUniformBuffer->flush();
}

VkDescriptorSet RenderResource::getGlobalUboDescriptorSet() const {
    return m_GlobalUboDescriptorSet;
}

void RenderResource::updateMaterialDescriptorSet(UID material_id) {
    assert(m_Materials.contains(material_id));

    auto& mat = m_Materials[material_id];
    auto& set = m_MaterialDescriptorSets[material_id];

    auto& uboBuffer = *m_MaterialUniformBuffers[material_id];
    uboBuffer.map();
    uboBuffer.writeToBuffer(&mat.baseColorFactor);
    uboBuffer.unmap();

    auto bufferInfo = uboBuffer.descriptorInfo();

    naDescriptorWriter writer{*m_MaterialSetLayout, *m_DescriptorPool};
    writer.writeBuffer(0, &bufferInfo);

    auto& base_color_texture = *m_Textures[mat.base_color_texture];
    naSampler sampler{*p_Device, SamplerType::Mipmap, base_color_texture.getInfo().mipLevels};
    auto imageInfo = sampler.descriptorInfo(base_color_texture);
    writer.writeImage(1, &imageInfo);

    if (set == VK_NULL_HANDLE)
        writer.build(set);
    else
        writer.overwrite(set);
}

VkDescriptorSet RenderResource::getMaterialDescriptorSet(UID material_id) const {
    return m_MaterialDescriptorSets.at(material_id);
}

void RenderResource::setCurrentFrameIndex(uint32_t current_index) {
    curr_frame_index = current_index;
}

uint32_t RenderResource::getCurrentFrameIndex() const {
    return curr_frame_index;
}

}