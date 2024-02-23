//
//  resource_path.h
//  nary
//
//  Created by Ninter6 on 2023/11/12.
//

#pragma once

#include <filesystem>

#include "se_tools.h"

#if !defined(ROOT_FOLDER)
#define ROOT_FOLDER /Users/mac/Documents/CPPPRO/my-nary/
#endif

namespace nary::res {

constexpr auto modelPath = SE_XSTR(ROOT_FOLDER)"assets/models/";
constexpr auto imagePath = SE_XSTR(ROOT_FOLDER)"assets/images/";
constexpr auto shaderPath = SE_XSTR(ROOT_FOLDER)"assets/shaders/";
constexpr auto shaderCachePath = SE_XSTR(ROOT_FOLDER)"assets/shaders/cache/";

// all folders above must be existent

inline std::filesystem::path fullname(const char* resPath, std::string_view filename) {
    std::filesystem::path path = filename;
    if (path.is_relative()) path = resPath/path;
    return path;
}

}
