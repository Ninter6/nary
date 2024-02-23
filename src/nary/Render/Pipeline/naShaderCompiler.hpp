//
//  naShaderCompiler.hpp
//  nary
//
//  Created by Ninter6 on 2023/11/12.
//

#pragma once

#include <vector>
#include <string>

namespace nary {

enum class ShaderType {
    Vertex = 0,
    TessControl,
    TessEvaluation,
    Geometry,
    Fragment,
    Compute
};

using ShaderSource_t = std::vector<uint32_t>;

class ShaderCompiler {
public:
    ShaderCompiler() = delete; // Instance creation not allowed
    
    static ShaderSource_t CompileShaderCode(const std::string& code, ShaderType type);
    static ShaderSource_t CompileShaderFromFile(const std::string& filename);
    static ShaderSource_t RecompileShaderFromFile(const std::string& filename);
    static ShaderSource_t CompileShaderFromFileWithoutCache(const std::string& filename);
    
    static ShaderType find_type(const std::string& filename);
    
    static void DeleteCache(const std::string& filename);
    static void DeleteAllCaches();
    
private:
    static std::string cached_filename(const std::string& filename);
    
    static bool CacheExist(const std::string& filename);
    static void CacheShader(const std::string& filename, const ShaderSource_t& source);
    static ShaderSource_t ReadCache(const std::string& filename);
};

}
