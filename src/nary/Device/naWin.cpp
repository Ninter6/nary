//
//  naWin.cpp
//  nary
//
//  Created by Ninter6 on 2023/5/2.
//

#include "naWin.hpp"

namespace nary {
naWin::naWin(int w, int h, std::string name) : width(w), height(h), windowName(name){
    initWindow();
}

naWin::~naWin(){
    glfwDestroyWindow(window);
    glfwTerminate();
}

void naWin::initWindow(){
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
    window = glfwCreateWindow(
#ifdef __APPLE__
                              width / 2, height / 2,
#else
                              width, height,
#endif
                              windowName.c_str(), NULL, NULL);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
//    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

bool naWin::shouldClose() const {
    return glfwWindowShouldClose(window);
}

void naWin::setShouldClose() {
    glfwSetWindowShouldClose(window, true);
}

void naWin::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface){
    if(glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS){
        throw std::runtime_error("failed to create window surface");
    }
}

void naWin::framebufferResizeCallback(GLFWwindow* window, int width, int height){
    auto win = reinterpret_cast<naWin*>(glfwGetWindowUserPointer(window));
    win->framebufferResized = true;
    win->width = width;
    win->height = height;
}

void naWin::nextFrame() {
    resetWindowResizedFlag();
    glfwPollEvents();
}

KeyState naWin::getMouseButtom(int MouseID) const {
    if (glfwGetMouseButton(glfw(), MouseID) == GLFW_RELEASE) {
        return KeyState::Release;
    } else {
        return KeyState::Press;
    }
}

KeyState naWin::getKey(int KeyID) const {
    if (glfwGetKey(glfw(), KeyID) == GLFW_RELEASE) {
        return KeyState::Release;
    } else {
        return KeyState::Press;
    }
}

mathpls::dvec2 naWin::getMousePos() const {
    mathpls::dvec2 pos;
    glfwGetCursorPos(glfw(), &pos.x, &pos.y);
    return pos;
}

}
