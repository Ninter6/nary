//
//  naFrameInfo.hpp
//  nary
//
//  Created by Ninter6 on 2023/8/22.
//

#pragma once

#include "naCamera.hpp"

namespace nary {

#define MAX_LIGHTS 10

struct PointLight {
    mathpls::vec3 position{};
    float radius;
    mathpls::vec4 color{};
};

struct GlobalUbo {
    mathpls::mat4 projection{1.f};
    mathpls::mat4 view{1.f};
    mathpls::mat4 inverseView{1.f};
    mathpls::mat4 lightSpace{1.f};
    mathpls::vec4 ambientLightColor{1.f, 1.f, 1.f, .02f};
    PointLight pointLights[MAX_LIGHTS];
    int numLights;
};

struct naFrameInfo {
    int frameIndex;
    VkCommandBuffer commandBuffer;
    naCamera& camera;
    VkDescriptorSet globalDescriptor;
    naGameObject::Map& gameObjects;
};

}
