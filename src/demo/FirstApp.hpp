//
//  FirstApp.hpp
//  nary
//
//  Created by Ninter6 on 2023/5/2.
//

#pragma once

#include "naWin.hpp"
#include "naRenderer.hpp"
#include "naGameObject.hpp"
#include "naCamera.hpp"
#include "naBuffer.hpp"
#include "naDescriptors.hpp"
#include "naPhysicsWorld.hpp"
#include "naShaderCompiler.hpp"
#include "Scene.hpp"
#include "RenderManager.hpp"
#include "ResourceManager.hpp"

#include <chrono>
#include <numeric>

namespace nary {

class FirstApp {
public:
    FirstApp();
    ~FirstApp();
    
    FirstApp(const FirstApp&) = delete;
    FirstApp operator=(const FirstApp&) = delete;
    
    static constexpr int WIDTH = 1920;
    static constexpr int HEIGHT = 1200;
    
    void run();
    
private:
    void loadGameObjects();
    void initEvents();
    
    void UpdatePhysics();
    void DrawUI(VkCommandBuffer cmdbuf, uint32_t frameIndex);
    
    naWin window{WIDTH, HEIGHT, "Hello Vulkan!"};
    RenderManager renderManager{window};
    ResourceManager resourceManager{renderManager};
    // naUISystem UI{device, renderer.getSwapChainRenderPass(), window};
    naEventListener eventListener;
    
    naPhysicsWorld phyworld;
    
    // note: order of declarations matters
    Scene scene;
};

}
