aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} imgui_src)
add_library(imgui ${imgui_src})
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(imgui PUBLIC glfw)