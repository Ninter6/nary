add_subdirectory(imgui)
add_subdirectory(pxpls)

add_library(se_tools INTERFACE)
target_include_directories(se_tools INTERFACE se_tools)
target_compile_features(se_tools INTERFACE cxx_std_20)

add_library(vma STATIC vma/vk_mem_alloc.cc)
target_include_directories(vma PUBLIC vma)
target_compile_options(vma PRIVATE "-w")