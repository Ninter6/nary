//
//  naModel.hpp
//  nary
//
//  Created by Ninter6 on 2023/5/14.
//

#pragma once

#include "naDevice.hpp"
#include "naBuffer.hpp"

#include "mathpls.h"
#include "Geometry.hpp"

#include <vector>
#include <memory>

namespace nary {

class naModel{
public:
    struct Vertex{
        mathpls::vec3 position;
        mathpls::vec3 normal;
        mathpls::vec2 uv;
        mathpls::vec3 color{1.f};
        
        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
        
        bool operator==(const Vertex& o) const {
            return position == o.position && normal == o.normal && uv == o.uv && color == o.color;
        }
    };
    
    struct Builder {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        
        void loadModel(std::string_view filepath);
    };
    
    naModel(naDevice& device, const Builder& builder);
    ~naModel() = default;
    
    static std::unique_ptr<naModel> createModelFromFile(naDevice& device, const std::string& filepath);
    
    naModel(const naModel&) = delete;
    naModel operator=(const naModel&) = delete;
    
    void bind(VkCommandBuffer commandBufffer);
    void draw(VkCommandBuffer commandBufffer);

    pxpls::Sphere getBoundingSphere() const {return meshBoundingSphere;}
    
private:
    void createVertexBuffers(const std::vector<Vertex>& vertices);
    void createIndexBuffers(const std::vector<uint32_t>& indices);

    void createBoundingSphere(const std::vector<Vertex>& vertices);
    
    naDevice& device;
    
    std::unique_ptr<naBuffer> vertexBuffer;
    uint32_t vertexCount;

    pxpls::Sphere meshBoundingSphere;
    
    bool hasIndexBuffer;
    std::unique_ptr<naBuffer> indexBuffer;
    uint32_t indexCount;
};

}
