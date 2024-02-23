//
//  naWin.hpp
//  nary
//
//  Created by Ninter6 on 2023/5/2.
//

#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <iostream>

#include "math_helper.h"

namespace nary {

enum class KeyState {
    Release = 0,
    Press = 1
};

class naWin {
public:
    naWin(int w, int h, std::string name);
    ~naWin();
    
    naWin(const naWin&) = delete;
    naWin& operator=(const naWin&) = delete; 
    
    bool shouldClose() const;
    void setShouldClose();
    VkExtent2D getExtent() const {return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};}
    float extentAspectRatio() const {return static_cast<float>(width) / static_cast<float>(height);}
    void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
    bool wasWindowResized() const {return framebufferResized;}
    void nextFrame();
    
    KeyState getMouseButtom(int MouseID) const;
    KeyState getKey(int KeyID) const;
    mathpls::dvec2 getMousePos() const;
    
    GLFWwindow* glfw() const {return window;}
    
private:
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    
    void initWindow();
    void resetWindowResizedFlag() {framebufferResized = false;}
    
    int width, height;
    bool framebufferResized = false;
    
    std::string windowName;
    GLFWwindow* window;
};

}
