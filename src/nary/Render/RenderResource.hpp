#pragma once

#include "naDevice.hpp"
#include "naImage.hpp"
#include "naCamera.hpp"
#include "naModel.hpp"
#include "naDescriptors.hpp"
#include "ResourceManager.hpp"

#include <vector>
#include <unordered_map>
#include <memory>

namespace nary {

/**
 * \brief material
 */
struct Material {
    static constexpr size_t ubo_offset = 11 * sizeof(float);

    mathpls::vec4 baseColorFactor{};
    float metallicFactor    = 0.0f;
    float roughnessFactor   = 0.0f;
    float normalScale       = 0.0f;
    float occlusionStrength = 0.0f;
    mathpls::vec3 emissiveFactor{};

    bool is_blend = 0;
    bool is_double_side = 0;

    UID base_color_texture = 0;
    UID metallic_roughness_texture = 0;
    UID normal_texture = 0;
    UID occlusion_texture_image = 0;
    UID emissive_texture = 0;
};

struct PointLight {
    mathpls::vec3 position;
    float radius;
    // radiant flux in W
    mathpls::vec3 flux;
    float _dont_worry_be_happy = 1.f;

    // calculate an appropriate radius for light culling
    // a windowing function in the shader will perform a smooth transition to zero
    // this is not physically based and usually artist controlled
    float calculateRadius() const {
        // radius = where attenuation would lead to an intensity of 1W/m^2
        constexpr float INTENSITY_CUTOFF    = 1.0f;
        constexpr float ATTENTUATION_CUTOFF = 0.05f;
        mathpls::vec3   intensity           = flux * 0.25f * mathpls::inv_pi();
        float           maxIntensity        = std::max(intensity.x, std::max(intensity.y, intensity.z));
        float           attenuation = std::max(INTENSITY_CUTOFF, ATTENTUATION_CUTOFF * maxIntensity) / maxIntensity;
        return 1.0f / std::sqrtf(attenuation);
    }
};

struct DirectionalLight {
    mathpls::mat4 projView;
    mathpls::vec3 direction;
    mathpls::vec3 color;
};

struct RenderCamera {
    mathpls::mat4 viewMat;
    mathpls::mat4 invViewMat;
    mathpls::mat4 projMat;
    Frustum viewFrustum;
};

#define MAX_NUM_POINT_LIGHTS 10

struct GlobalUbo {
    mathpls::mat4 projection{1.f};
    mathpls::mat4 view{1.f};
    mathpls::mat4 inverseView{1.f};
    mathpls::mat4 directionalLightSpace{1.f};
    mathpls::vec4 ambientLightColor{1.f, 1.f, 1.f, .02f};
    PointLight pointLights[MAX_NUM_POINT_LIGHTS];
    int numLights;
};

enum class RenderEntityType {
    model,
    placard,
    shader_placard
};

struct RenderEntity {
    mathpls::mat4 modelMat;

    RenderEntityType type;

    naModel* model; // model
    UID material; // model, placard

    pxpls::Sphere boundingSphere;
};

class RenderResource {
public:
    RenderResource(naDevice& device);

    RenderResource(const RenderResource&) = delete;
    RenderResource& operator=(const RenderResource&) = delete;

    naDevice* getDevice() const;

    UID addMaterial(const Material& m);
    UID addMesh(std::unique_ptr<naModel>&& m);
    UID addTexture(std::unique_ptr<naImage>&& t);

    Material* getMaterial(UID id) const;
    naModel* getMesh(UID id) const;
    naImage* getTexture(UID id) const;

    naDescriptorPool* getDescriptorPool() const;
    naDescriptorSetLayout* getOneImageSetLayout() const;
    naDescriptorSetLayout* getGlbalUboSetLayout() const;
    naDescriptorSetLayout* getMaterialSetLayout() const;

    void updateGlobalUbo(const GlobalUbo& ubo) const;
    VkDescriptorSet getGlobalUboDescriptorSet() const;

    void updateMaterialDescriptorSet(UID material_id);
    VkDescriptorSet getMaterialDescriptorSet(UID material_id) const;

    void setCurrentFrameIndex(uint32_t current_index);
    uint32_t getCurrentFrameIndex() const;

private:
    naDevice* p_Device;

    std::unique_ptr<naDescriptorPool> m_DescriptorPool;

    std::unique_ptr<naDescriptorSetLayout> m_OneImageSetLayout; // use for offscreen, shadowmap
    std::unique_ptr<naDescriptorSetLayout> m_GlobalUboSetLayout;
    std::unique_ptr<naDescriptorSetLayout> m_MaterialSetLayout;

    std::unique_ptr<naBuffer> m_GlobalUniformBuffer;
    VkDescriptorSet m_GlobalUboDescriptorSet;

    std::unordered_map<UID, std::unique_ptr<naBuffer>> m_MaterialUniformBuffers;
    std::unordered_map<UID, VkDescriptorSet> m_MaterialDescriptorSets;
    
    UidMap<Material> m_Materials;
    UidMap<std::unique_ptr<naModel>> m_Models;
    UidMap<std::unique_ptr<naImage>> m_Textures;

    uint32_t curr_frame_index;

    void createDescriptorPool();
    void createSetLayouts();
    void createGlobalUniformBuffer();
    void createDefaultMaterial();
    void createDefaulrTexture();

    void createMaterialUniformBuffers(UID id);

};

}