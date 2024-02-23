//
//  naShaderCompiler.cpp
//  nary
//
//  Created by Ninter6 on 2023/11/12.
//

#include "naShaderCompiler.hpp"

#include "resource_path.h"

#include "se_tools.h"
#include "sebase64.h"

#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include <fstream>

namespace nary {

TBuiltInResource defaultResources() {
    TBuiltInResource Resources{};
    Resources.maxLights                                 = 32;
    Resources.maxClipPlanes                             = 6;
    Resources.maxTextureUnits                           = 32;
    Resources.maxTextureCoords                          = 32;
    Resources.maxVertexAttribs                          = 64;
    Resources.maxVertexUniformComponents                = 4096;
    Resources.maxVaryingFloats                          = 64;
    Resources.maxVertexTextureImageUnits                = 32;
    Resources.maxCombinedTextureImageUnits              = 80;
    Resources.maxTextureImageUnits                      = 32;
    Resources.maxFragmentUniformComponents              = 4096;
    Resources.maxDrawBuffers                            = 32;
    Resources.maxVertexUniformVectors                   = 128;
    Resources.maxVaryingVectors                         = 8;
    Resources.maxFragmentUniformVectors                 = 16;
    Resources.maxVertexOutputVectors                    = 16;
    Resources.maxFragmentInputVectors                   = 15;
    Resources.minProgramTexelOffset                     = -8;
    Resources.maxProgramTexelOffset                     = 7;
    Resources.maxClipDistances                          = 8;
    Resources.maxComputeWorkGroupCountX                 = 65535;
    Resources.maxComputeWorkGroupCountY                 = 65535;
    Resources.maxComputeWorkGroupCountZ                 = 65535;
    Resources.maxComputeWorkGroupSizeX                  = 1024;
    Resources.maxComputeWorkGroupSizeY                  = 1024;
    Resources.maxComputeWorkGroupSizeZ                  = 64;
    Resources.maxComputeUniformComponents               = 1024;
    Resources.maxComputeTextureImageUnits               = 16;
    Resources.maxComputeImageUniforms                   = 8;
    Resources.maxComputeAtomicCounters                  = 8;
    Resources.maxComputeAtomicCounterBuffers            = 1;
    Resources.maxVaryingComponents                      = 60;
    Resources.maxVertexOutputComponents                 = 64;
    Resources.maxGeometryInputComponents                = 64;
    Resources.maxGeometryOutputComponents               = 128;
    Resources.maxFragmentInputComponents                = 128;
    Resources.maxImageUnits                             = 8;
    Resources.maxCombinedImageUnitsAndFragmentOutputs   = 8;
    Resources.maxCombinedShaderOutputResources          = 8;
    Resources.maxImageSamples                           = 0;
    Resources.maxVertexImageUniforms                    = 0;
    Resources.maxTessControlImageUniforms               = 0;
    Resources.maxTessEvaluationImageUniforms            = 0;
    Resources.maxGeometryImageUniforms                  = 0;
    Resources.maxFragmentImageUniforms                  = 8;
    Resources.maxCombinedImageUniforms                  = 8;
    Resources.maxGeometryTextureImageUnits              = 16;
    Resources.maxGeometryOutputVertices                 = 256;
    Resources.maxGeometryTotalOutputComponents          = 1024;
    Resources.maxGeometryUniformComponents              = 1024;
    Resources.maxGeometryVaryingComponents              = 64;
    Resources.maxTessControlInputComponents             = 128;
    Resources.maxTessControlOutputComponents            = 128;
    Resources.maxTessControlTextureImageUnits           = 16;
    Resources.maxTessControlUniformComponents           = 1024;
    Resources.maxTessControlTotalOutputComponents       = 4096;
    Resources.maxTessEvaluationInputComponents          = 128;
    Resources.maxTessEvaluationOutputComponents         = 128;
    Resources.maxTessEvaluationTextureImageUnits        = 16;
    Resources.maxTessEvaluationUniformComponents        = 1024;
    Resources.maxTessPatchComponents                    = 120;
    Resources.maxPatchVertices                          = 32;
    Resources.maxTessGenLevel                           = 64;
    Resources.maxViewports                              = 16;
    Resources.maxVertexAtomicCounters                   = 0;
    Resources.maxTessControlAtomicCounters              = 0;
    Resources.maxTessEvaluationAtomicCounters           = 0;
    Resources.maxGeometryAtomicCounters                 = 0;
    Resources.maxFragmentAtomicCounters                 = 8;
    Resources.maxCombinedAtomicCounters                 = 8;
    Resources.maxAtomicCounterBindings                  = 1;
    Resources.maxVertexAtomicCounterBuffers             = 0;
    Resources.maxTessControlAtomicCounterBuffers        = 0;
    Resources.maxTessEvaluationAtomicCounterBuffers     = 0;
    Resources.maxGeometryAtomicCounterBuffers           = 0;
    Resources.maxFragmentAtomicCounterBuffers           = 1;
    Resources.maxCombinedAtomicCounterBuffers           = 1;
    Resources.maxAtomicCounterBufferSize                = 16384;
    Resources.maxTransformFeedbackBuffers               = 4;
    Resources.maxTransformFeedbackInterleavedComponents = 64;
    Resources.maxCullDistances                          = 8;
    Resources.maxCombinedClipAndCullDistances           = 8;
    Resources.maxSamples                                = 4;
    Resources.maxMeshOutputVerticesNV                   = 256;
    Resources.maxMeshOutputPrimitivesNV                 = 512;
    Resources.maxMeshWorkGroupSizeX_NV                  = 32;
    Resources.maxMeshWorkGroupSizeY_NV                  = 1;
    Resources.maxMeshWorkGroupSizeZ_NV                  = 1;
    Resources.maxTaskWorkGroupSizeX_NV                  = 32;
    Resources.maxTaskWorkGroupSizeY_NV                  = 1;
    Resources.maxTaskWorkGroupSizeZ_NV                  = 1;
    Resources.maxMeshViewCountNV                        = 4;

    Resources.limits.nonInductiveForLoops                 = 1;
    Resources.limits.whileLoops                           = 1;
    Resources.limits.doWhileLoops                         = 1;
    Resources.limits.generalUniformIndexing               = 1;
    Resources.limits.generalAttributeMatrixVectorIndexing = 1;
    Resources.limits.generalVaryingIndexing               = 1;
    Resources.limits.generalSamplerIndexing               = 1;
    Resources.limits.generalVariableIndexing              = 1;
    Resources.limits.generalConstantMatrixVectorIndexing  = 1;
    
    return Resources;
} // init resources

template <class...Args>
bool ends_with(const std::string& filename, Args...args) {
    return (filename.ends_with(args) || ...);
} // tool func

ShaderType ShaderCompiler::find_type(const std::string& filename) {
    if (ends_with(filename, ".vert", ".glslv", ".vsh")) {
        return ShaderType::Vertex;
    } else if (ends_with(filename, ".frag", ".glslf", ".fsh")) {
        return ShaderType::Fragment;
    } else if (ends_with(filename, ".comp", ".glslc", ".csh")) {
        return ShaderType::Compute;
    } else if (ends_with(filename, ".tesc", ".glslt", ".tsh")) {
        return ShaderType::TessControl;
    } else if (ends_with(filename, ".tese", ".glslte", ".tesh")) {
        return ShaderType::TessEvaluation;
    } else if (ends_with(filename, ".geom", ".glslg", ".gsh")) {
        return ShaderType::Geometry;
    } else {
        WARNING_LOG("UNKNOWN Shader Type: {}\nDefaultly compile as fragment", filename);
        return ShaderType::Fragment;
    }
}

ShaderSource_t ShaderCompiler::CompileShaderCode(const std::string& code, ShaderType type) {
    glslang::InitializeProcess();
    
    const char* code_cstr = code.c_str();
    
    // Create and compile the shader
    glslang::TShader shader((EShLanguage)type);
    shader.setStrings(&code_cstr, 1);
    
    auto Resources = defaultResources();
    
    if (!shader.parse(&Resources, 450, true, EShMessages(EShMsgSpvRules|EShMsgVulkanRules))) {
        ERROR_LOG("[Shader Compiler]: Compilation failed:\n{}", shader.getInfoLog());
    }
    
    // 将glslang中间表示转换为SPIR-V
    ShaderSource_t spirv;
    glslang::GlslangToSpv(*shader.getIntermediate(), spirv);
    
    // 释放内存并清理glslang库
    glslang::FinalizeProcess();
    
    return spirv;
}

ShaderSource_t ShaderCompiler::CompileShaderFromFile(const std::string& filename) {
    if (CacheExist(filename)) {
        return ReadCache(filename);
    }
    
    return RecompileShaderFromFile(filename);
}

ShaderSource_t ShaderCompiler::RecompileShaderFromFile(const std::string& filename) {
    const auto& source = CompileShaderFromFileWithoutCache(filename);
    CacheShader(filename, source);
    return source;
}

ShaderSource_t ShaderCompiler::CompileShaderFromFileWithoutCache(const std::string& filename) {
    std::ifstream ifs(res::fullname(res::shaderPath, filename));
    if (!ifs.good()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    
    std::string source_code((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    
    try {
        return CompileShaderCode(source_code, find_type(filename));
    } catch (const std::exception& e) {
        ERROR_LOG("Failed to compile shader: {}", filename);
    }
}

std::string ShaderCompiler::cached_filename(const std::string& filename) {
    return res::shaderCachePath + st::base58::encode(filename);
}

bool ShaderCompiler::CacheExist(const std::string& filename) {
    return std::filesystem::exists(cached_filename(filename));
}

void ShaderCompiler::CacheShader(const std::string& filename, const ShaderSource_t& source) {
    std::ofstream ofs(cached_filename(filename), std::ios::binary);
    ofs.write((char*)source.data(), source.size() * sizeof(source[0]));
}

ShaderSource_t ShaderCompiler::ReadCache(const std::string& filename) {
    ShaderSource_t source;
    std::ifstream ifs(cached_filename(filename), std::ios::ate | std::ios::binary);
    if(ifs.good()) {
        size_t fileSize = static_cast<size_t>(ifs.tellg());
        std::vector<char> buffer(fileSize);
        ifs.seekg(0);
        ifs.read(buffer.data(), fileSize);
        
        source.resize(fileSize / sizeof(source[0]));
        memcpy(source.data(), buffer.data(), fileSize);
    } else {
        ERROR_LOG("Failed to open cache file: {}", filename);
    }
    return source;
}

void ShaderCompiler::DeleteCache(const std::string& filename) {
    std::remove(cached_filename(filename).c_str());
}

void ShaderCompiler::DeleteAllCaches() {
    for (const auto& entry : std::filesystem::directory_iterator(res::shaderCachePath)) {
        try {
            std::filesystem::remove_all(entry.path());
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error deleting file " << entry.path() << ": " << e
.what() << std::endl;
        }
    }
}

}
