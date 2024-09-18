#include "Model.hpp"

#include "../../../External/utility.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../../../External/tiny_obj_loader.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>



#include <cassert>
#include <cstring>
#include <iostream>
#include <unordered_map>

namespace std
{
    template <>
    struct hash<RenderingEngine::LveModel::Vertex>
    {
        size_t operator()(RenderingEngine::LveModel::Vertex const& vertex) const
        {
            size_t seed = 0;
            RenderingEngine::hashCombine(seed, vertex.position, vertex.normal, vertex.tangent, vertex.bitangent, vertex.uv);
            return seed;
        }
    };
}

namespace RenderingEngine{

    LveModel::LveModel(LveDevice& device, const Builder& builder): mDevice{device} {
        createVertexBuffer(builder.vertices);
        createIndexBuffer(builder.indices);
    }
    LveModel::~LveModel(){}
    
    void LveModel::createVertexBuffer(const std::vector<Vertex>& vertices){
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
        uint32_t vertexSize = sizeof(vertices[0]);
        
        // create host visible buffer as temporary buffer
        // the final data saves in gpu local memory
        LveBuffer stagingBuffer{
            mDevice,
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };
        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)vertices.data());

        vertexBuffer = std::make_unique<LveBuffer>(
            mDevice,
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );
         // staging buffer to vertex buffer
         mDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
    }
    
    void LveModel::createIndexBuffer(const std::vector<uint32_t>& indices){
        indexCount = static_cast<uint32_t>(indices.size());
        hasIndexBuffer = indexCount > 0;
        if(!hasIndexBuffer){
            return;
        }
        VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
        uint32_t indexSize = sizeof(indices[0]);
        LveBuffer stagingBuffer{
            mDevice,
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };
        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)indices.data());

        indexBuffer = std::make_unique<LveBuffer>(
            mDevice,
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );
        mDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
    }
    
    std::unique_ptr<LveModel> LveModel::createModelFromFile(LveDevice& device, const std::string& filePath){
        Builder builder{};
        //builder.loadObjModel(filePath);
        builder.loadFbxModel(filePath);
        std::cout << "Vertex count: " << builder.vertices.size() << std::endl;
        return std::make_unique<LveModel>(device, builder);
    }

    void LveModel::draw(VkCommandBuffer commandBuffer){
        if(hasIndexBuffer){
            vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
        }else{
        vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
        }
    }
    void LveModel::bind(VkCommandBuffer commandBuffer){
        VkBuffer vertexBuffers[] = {vertexBuffer->getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        if(hasIndexBuffer){
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        }
        
        //draw(commandBuffer);
    }
    
    std::vector<VkVertexInputBindingDescription> LveModel::Vertex::getBindingDescriptions(){
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }
    std::vector<VkVertexInputAttributeDescription> LveModel::Vertex::getAttributeDescriptions(){
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
        attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
        attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
        attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, tangent)});
        attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, bitangent)});
        attributeDescriptions.push_back({4, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

        return attributeDescriptions;
    }

    void LveModel::Builder::loadObjModel(const std::string& modelPath){
        // load model from file
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;
            
        if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str())){
            throw std::runtime_error(warn + err);
        }


        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        vertices.clear();
        indices.clear();
        for(const auto& shape : shapes){
            for(const auto& index : shape.mesh.indices){
                Vertex vertex{};
                if(index.vertex_index >= 0){
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                    };

                    // delete color attribute
                    //vertex.color = {
                    //    attrib.colors[3 * index.vertex_index + 0],
                    //    attrib.colors[3 * index.vertex_index + 1],
                    //    attrib.colors[3 * index.vertex_index + 2]
                    //};
                }

                if(index.normal_index >= 0){
                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                    };
                }

                if(index.texcoord_index >= 0){
                    vertex.uv = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                    };
                }
                if(uniqueVertices.count(vertex) == 0){
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(uniqueVertices[vertex]);
            }
        }
        
    } 

    void LveModel::Builder::loadFbxModel(const std::string& modelPath){
        Assimp::Importer importer;
        const unsigned int ImportFlags = 
            aiProcess_CalcTangentSpace |
            aiProcess_Triangulate |
            aiProcess_SortByPType |
            aiProcess_PreTransformVertices |
            aiProcess_GenNormals |
            aiProcess_GenUVCoords |
            aiProcess_OptimizeMeshes |
            aiProcess_Debone |
            aiProcess_ValidateDataStructure;
        
        const aiScene* scene = importer.ReadFile(modelPath, ImportFlags);
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
            throw std::runtime_error("Failed to load model: " + std::string(importer.GetErrorString()));
        }
        auto mesh = scene->mMeshes[0];

        assert(mesh->HasPositions());
        assert(mesh->HasNormals());

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};
        vertices.reserve(mesh->mNumVertices);
        for(size_t i=0; i<vertices.capacity(); ++i) {
            Vertex vertex;
            vertex.position = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
            vertex.normal = {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};

            if(mesh->HasTangentsAndBitangents()) {
                vertex.tangent = {mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z};
                vertex.bitangent = {mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z};
                   
            }

            if(mesh->HasTextureCoords(0)) {
                vertex.uv = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
            }

            if(uniqueVertices.count(vertex) == 0){
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }



    }

}
