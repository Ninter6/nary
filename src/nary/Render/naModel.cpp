//
//  naModel.cpp
//  nary
//
//  Created by Ninter6 on 2023/5/14.
//

#include "naModel.hpp"
#include "se_tools.h"

#include "resource_path.h"

#include "math_helper.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <filesystem>
#include <unordered_map>

namespace std {
template<>
struct hash<nary::naModel::Vertex> {
    size_t operator()(nary::naModel::Vertex const& v) const {
        size_t seed = 0;
        hash_combine(seed, v.position, v.normal, v.uv, v.color);
        return seed;
    }
};
}

namespace nary {

naModel::naModel(naDevice& device, const Builder& builder) : device(device) {
    createBoundingSphere(builder.vertices);
    createVertexBuffers(builder.vertices);
    createIndexBuffers(builder.indices);
}

std::unique_ptr<naModel> naModel::createModelFromFile(naDevice& device, const std::string& filepath) {
    Builder builder;
    builder.loadModel(filepath);
    
    st::log::titled_log(filepath, "Vertex count: {}", builder.vertices.size());
    
    return std::make_unique<naModel>(device, builder);
}

void naModel::Builder::loadModel(std::string_view filepath) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
    
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, res::fullname(res::modelPath, filepath).c_str())) {
        throw std::runtime_error(warn + err);
    }
    
    vertices.clear();
    indices.clear();
    
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};
            
            if (index.vertex_index >= 0) {
                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };
                vertex.color = {
                    attrib.colors[3 * index.vertex_index + 0],
                    attrib.colors[3 * index.vertex_index + 1],
                    attrib.colors[3 * index.vertex_index + 2]
                };
            }
            
            if (index.normal_index >= 0) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            }
            
            if (index.texcoord_index >= 0) {
                vertex.uv = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1]
                };
            }
            
            if (!uniqueVertices.count(vertex)) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }
            indices.push_back(uniqueVertices[vertex]);
        }
    }
}

void naModel::createVertexBuffers(const std::vector<Vertex> &vertices){
    vertexCount = static_cast<uint32_t>(vertices.size());
    constexpr uint32_t vertexSize = sizeof(Vertex);
    
    naBuffer stagingBuffer{
        device,
        vertexSize,
        vertexCount,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };
    
    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void*)vertices.data());
    
    vertexBuffer = std::make_unique<naBuffer>(
        device,
        vertexSize,
        vertexCount,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    device.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), vertexSize * vertexCount);
}

void naModel::createIndexBuffers(const std::vector<uint32_t>& indices) {
    indexCount = static_cast<uint32_t>(indices.size());
    if (!(hasIndexBuffer = indexCount > 0)) return;
    
    constexpr VkDeviceSize indexSize = sizeof(uint32_t);
    
    naBuffer stagingBuffer{
        device,
        indexSize,
        indexCount,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };
    
    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void*)indices.data());
    
    indexBuffer = std::make_unique<naBuffer>(
        device,
        indexSize,
        indexCount,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    device.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), indexSize * indexCount);
}

void naModel::createBoundingSphere(const std::vector<Vertex>& vertices) {
    auto& sphere = meshBoundingSphere;
    pxpls::Point sp[4];
    int ns = 0;
    for (auto& i : vertices) {
        if (!pxpls::IsPointOutSphere(i.position, meshBoundingSphere)) continue;

        sp[ns++] = i.position;
        switch (ns) {
        case 1: {
            sphere.center = sp[0];
            break;
        }
        case 2: {
            sphere.center = (sp[0] + sp[1]) / 2;
		    sphere.radius = mathpls::distance(sp[0], sp[1]) / 2;
            break;
        }
        case 3: {
            auto e0 = sp[1] - sp[0], e1 = sp[2] - sp[0];
		    auto a = mathpls::dot(e0, e0), b = mathpls::dot(e0, e1), c = mathpls::dot(e1, e1);
		    auto d = a * c - b * b;
		    if (std::abs(d) > 1e-3f) {
		        auto s = (a - b)*c / (2 * d), t = (c - b)*a / (2 * d);
		        sphere.center = sp[0] + s * e0 + t * e1;
		        sphere.radius = (sp[0] - sphere.center).length();
            }
            break;
        }
        case 4: {
            auto v1 = sp[1] - sp[0], v2 = sp[2] - sp[0], v3 = sp[3] - sp[0];
		    auto V = mathpls::dot(v1, mathpls::cross(v2, v3));
		    // Check that the three points are not on the same plane.
		    if (std::abs(V) > 1e-3f) {
		        V *= 2.0;
		        auto L1 = v1.length_squared(), L2 = v2.length_squared(), L3 = v3.length_squared();
		        sphere.center.x = (sp[0].x + ((v2.y*v3.z - v3.y*v2.z)*L1 - (v1.y*v3.z - v3.y*v1.z)*L2 + (v1.y*v2.z - v2.y*v1.z)*L3) / V);
		        sphere.center.y = (sp[0].y + (-(v2.x*v3.z - v3.x*v2.z)*L1 + (v1.x*v3.z - v3.x*v1.z)*L2 - (v1.x*v2.z - v2.x*v1.z)*L3) / V);
		        sphere.center.z = (sp[0].z + ((v2.x*v3.y - v3.x*v2.y)*L1 - (v1.x*v3.y - v3.x*v1.y)*L2 + (v1.x*v2.y - v2.x*v1.y)*L3) / V);
		        sphere.radius = (sphere.center - sp[0]).length();
            }
            ns--; // pop last support point
        }
        default:
            break;
        }
    }

    DEBUG_LOG("Bounding Sphere: {[{}, {}, {}], {}}", sphere.center.x, sphere.center.y, sphere.center.z, sphere.radius);
}

void naModel::draw(VkCommandBuffer commandBufffer){
    if (hasIndexBuffer) {
        vkCmdDrawIndexed(commandBufffer, indexCount, 1, 0, 0, 0);
    } else {
        vkCmdDraw(commandBufffer, vertexCount, 1, 0, 0);
    }
}

void naModel::bind(VkCommandBuffer commandBufffer){
    VkBuffer buffers[] = {vertexBuffer->getBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBufffer, 0, 1, buffers, offsets);
    
    if (hasIndexBuffer) {
        vkCmdBindIndexBuffer(commandBufffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
    }
}

std::vector<VkVertexInputBindingDescription> naModel::Vertex::getBindingDescriptions(){
    return {{0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}};
}

std::vector<VkVertexInputAttributeDescription> naModel::Vertex::getAttributeDescriptions(){
    return {{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)},
            {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)},
            {2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)},
            {3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)}};
}

}
