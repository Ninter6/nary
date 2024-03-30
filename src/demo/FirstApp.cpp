//
//  FirstApp.cpp
//  nary
//
//  Created by Ninter6 on 2023/5/2.
//

#include "se_tools.h"
#include "setimer.h"

#include "FirstApp.hpp"
#include "naImage.hpp"
#include "naFrameBuffer.hpp"

#include "scene_loader.hpp"

namespace nary {

// define MSAA sample count
VkSampleCountFlagBits naFrameBuffer::MsaaSamples = VK_SAMPLE_COUNT_2_BIT;

// mathpls::vec3 lightPos = {-0.785f, -4.f, 3.448f};
// mathpls::vec3 lightDir = {-0.522f, -0.42f, -0.492f};

naGameObject::id_t lightPrt;
naGameObject::id_t Dlight;

void FirstApp::run(){
#ifndef NDEBUG
    st::Countdown timer(1);
    timer.Start();
    uint16_t frameCount = 0;
#endif
    
    while (!window.shouldClose()) {
        window.nextFrame();
        naEventListener::Update(window);
        scene.getActiveCamera()->UpdateEvents(window);
        eventListener.UpdateEvents(window);

        scene.getGameObject(lightPrt)->transform().rotation.y += M_PI_4 * naEventListener::DeltaTime();

        scene.Update();
        
//        lightPos = camera.transform().translation;
//        lightDir = camera.transform().Forward();
        renderManager.tick(scene);
        DrawUI();
        
        if (window.wasWindowResized()) {
            auto aspect = window.extentAspectRatio();
            scene.getActiveCamera()->setPerspectiveProjection(mathpls::radians(60.f), aspect, .1f, 10.f);
        } // process window resizing
        
        UpdatePhysics();
        
#ifndef NDEBUG
        ++frameCount;
        if (timer.IsTimeOut()) {
            INFO_LOG("FPS: {}", frameCount);
            frameCount = 0;
            timer.Start();
        }
#endif
    }
}

void FirstApp::DrawUI() {
    ImGui::Begin("Info"); {
        
        auto lightDir = scene.getGameObject(Dlight)->getComponent<DirectionalLightCompnent>()->direction.value_ptr();
        ImGui::DragFloat3("Light Direction", lightDir, .05f);

        if (ImGui::Button("Point Light"))
            scene.getGameObject(lightPrt)->setActive(!scene.getGameObject(lightPrt)->getActive());

        for (auto& i : renderManager.getRenderResource()->getMaterials()) {
            if (i.first == 0) continue;
            ImGui::PushID(i.first);
            auto& mat = i.second;
            ImGui::Text("Material %lu:", i.first);
            ImGui::ColorEdit4("Base Color", mat.baseColorFactor.value_ptr());
            ImGui::DragFloat("Metallic", &mat.metallicFactor, 0.01f, 0.f, 1.f);
            ImGui::DragFloat("Roughness", &mat.roughnessFactor, 0.01f, 0.f, 1.f);
            if (ImGui::Button("Update"))
                renderManager.getRenderResource()->updateMaterialDescriptorSet(i.first);
            ImGui::PopID();
        }

    } ImGui::End();
}

void FirstApp::UpdatePhysics() {
    phyworld.Update(naEventListener::DeltaTime(), scene.getGameObjects());
}

FirstApp::FirstApp(){
    loadGameObjects();
    initEvents();
}

FirstApp::~FirstApp() {
#ifndef NDEBUG
    ShaderCompiler::DeleteAllCaches();
#endif // on debuging, delete shader caches
}

void FirstApp::loadGameObjects(){
//    std::shared_ptr<naModel> model0 = naModel::createModelFromFile(device, "smooth_vase.obj");
//    std::shared_ptr<naModel> model1 = naModel::createModelFromFile(device, "flat_vase.obj");
//    std::shared_ptr<naModel> model2 = naModel::createModelFromFile(device, "quad.obj");
    auto quad_id = assetManager.loadModel("quad.obj");

//    auto smooth_vase = naGameObject::createGameObject();
//    smooth_vase.model = model0;
//    smooth_vase.transform.translation = {.5, 0, 2.5f};
//    smooth_vase.transform.scale = {3.5f, 1.5f, 3.5f};
//    smooth_vase.color = {.4f, .84f, .53f};
//
//    auto flat_vase = naGameObject::createGameObject();
//    flat_vase.model = model1;
//    flat_vase.transform.translation = {-.5, 0, 2.5f};
//    flat_vase.transform.scale = {3.5f, 1.5f, 3.5f};
//    flat_vase.color = {.45f, .3f, .1f};

    auto floor_tex_id = assetManager.loadImage("floor.jpg");
    Material floor_material{};
    floor_material.baseColorFactor.a = 0.f;
    floor_material.base_color_texture = floor_tex_id;

    auto floor_material_id = renderManager.getRenderResource()->addMaterial(floor_material);

    auto floor = naGameObject::createGameObject();
    floor.addComponent<MeshComponent>(quad_id);
    floor.addComponent<MaterialComponent>(floor_material_id);
    floor.transform().translation = {0, 0, 2.5f};
    floor.transform().scale = {3.f, 1.f, 3.f};
    floor.transform().rotation.y = mathpls::pi<float>();

//    gameObjects.emplace(smooth_vase.getId(), std::move(smooth_vase));
//    gameObjects.emplace(flat_vase.getId(), std::move(flat_vase));
    scene.addGameObject(std::move(floor));
    
    auto ball_mdl_id = assetManager.loadModel("sphere.obj");
    auto smartface_id = assetManager.loadImage("fair-smartface.jpg");
    auto basket_ball_tex_id = assetManager.loadImage("basket ball.jpg");

    Material material1{};
    material1.baseColorFactor = {.4f, .84f, .53f, 0.5f};
    material1.base_color_texture = smartface_id;
    material1.metallicFactor = 0.8f;
    material1.roughnessFactor = 0.2f;

    Material material2{};
    material2.baseColorFactor = {.45f, .3f, .1f, 0.f};
    material2.base_color_texture = basket_ball_tex_id;
    material2.metallicFactor = 0.3f;
    material2.roughnessFactor = 0.5f;

    auto material1_id = renderManager.getRenderResource()->addMaterial(material1);
    auto material2_id = renderManager.getRenderResource()->addMaterial(material2);
    
    auto ball = naGameObject::createGameObject();
    ball.addComponent<MeshComponent>(ball_mdl_id);
    ball.transform().translation = {0, -1.f, 2.5f};
    ball.transform().scale = {.3f, .3f, .3f};
    ball.transform().rotation = {0, 3.14f, 0};
    ball.addComponent<MaterialComponent>()->material_id = material1_id;
    
    auto ball1 = naGameObject::createGameObject(ball);
    ball1.getComponent<MaterialComponent>()->material_id = material2_id;
    ball1.transform().translation.x += 2;
    ball1.addComponent<RigidBodyComponent>()->velocity = {-.5f, 20, 0};
    
    auto ball_id = scene.addGameObject(std::move(ball));
    scene.addGameObject(std::move(ball1));
    
    // point lights
    mathpls::vec3 lightColors[] = {
        {.06f, .93f, .8f},
        {1.f, .68f, .2f},
        {.37f, .83f, .09f},
        {.87f, .93f, .2f},
        {1.f, .67, .47f},
        {1.f, .73f, .8f}
    };

   lightPrt = scene.addGameObject(naGameObject::createGameObject(), ball_id);
   scene.getGameObject(lightPrt)->transform().scale /= scene.getGameObject(ball_id)->transform().scale;
    
    LOOP(6) {
        auto color = lightColors[i] * (mathpls::random::rand01() * 10 + 10);
        auto light = naGameObject::createPointLight(.1f, color);
        light.transform().translation = {
            cos(i * 1.0471f),
            0,
            sin(i * 1.0471f)
        };
        
        scene.addGameObject(std::move(light), lightPrt);
    }

    auto Dlight = naGameObject::createGameObject();
    auto dl = Dlight.addComponent<DirectionalLightCompnent>();
    dl->direction = {-0.522f, -0.42f, -0.492f};
    dl->color = {1.f, 1.f, 1.f};
    nary::Dlight = scene.addGameObject(std::move(Dlight));

    auto camera = std::make_unique<naCamera>();
    auto aspect = window.extentAspectRatio();
    camera->setPerspectiveProjection(mathpls::radians(60.f), aspect, .1f, 100.f);
    camera->transform().translation = {0, .3f, -2.5f};
    camera->transform().rotation.y = mathpls::pi<float>();
    camera->transform().rotation.x = mathpls::pi<float>() / -5;
    
    scene.SetActiveCamera(std::move(camera));

    SceneLoader::load(scene, assetManager, *renderManager.getRenderResource(), "/Users/mac/Documents/CPPPRO/my-nary/assets/scene0.json");
}

void FirstApp::initEvents() {
    eventListener.EventList.insert({{0, 0, 256}, [&]{window.setShouldClose();}});
}

}
