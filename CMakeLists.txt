cmake_minimum_required(VERSION 3.20)

project(nary)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

find_package(glfw3 3.3 REQUIRED)
find_package(vulkan REQUIRED)

include_directories(${Vulkan_INCLUDE_DIR})

# shaderc_shared library provided by vulkan
set(shaderc_shared /Users/mac/Documents/VulkanSDK/1.3.243.0/macOS/lib/libshaderc_shared.dylib)

add_subdirectory(dep)

# build nary
file(GLOB nary_src
    "src/nary/*/*.cpp"
    "src/nary/*/*/*.cpp")
add_library(nary STATIC ${nary_src})
target_link_libraries(nary PUBLIC se_tools pxpls imgui glfw vma ${Vulkan_LIBRARY} ${shaderc_shared})
# target_compile_options(nary PUBLIC "-Wno-changes-meaning")
target_compile_definitions(nary PRIVATE "ROOT_FOLDER=${CMAKE_SOURCE_DIR}/")
target_include_directories(nary PUBLIC
    src/nary/Core
    src/nary/Compon
    src/nary/Device
    src/nary/GO
    src/nary/Render
    src/nary/Render/Pipeline
    src/nary/Render/System
    src/nary/Render/Util
    src/nary/Resource
    src/nary/Tool
    src/nary/UI
)

option(NARY_BUILD_DEMO on)

# demo
# if (NARY_BUILD_DEMO)
    aux_source_directory(src/demo demo_src)
    add_executable(demo ${demo_src})
    target_link_libraries(demo PUBLIC nary)
# endif()