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
#include <random>

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
    std::vector<pxpls::Point> points(vertices.size());
    std::transform(vertices.begin(), vertices.end(), points.begin(), [](auto&& v) {
        return v.position;
    });
    std::mt19937 g{std::random_device{}()};
    std::shuffle(points.begin(), points.end(), g);

    if (points.size() > 10e4)
        meshBoundingSphere = pxpls::BoundingSphereFromPointsFast(points);
    else
        meshBoundingSphere = pxpls::BoundingSphereFromPoints(points); // this may cost much time

    DEBUG_LOG("Bounding Sphere: {[{}, {}, {}], {}}", meshBoundingSphere.center.x, meshBoundingSphere.center.y, meshBoundingSphere.center.z, meshBoundingSphere.radius);
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
