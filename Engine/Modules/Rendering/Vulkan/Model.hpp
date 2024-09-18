#pragma once

#include "Device.hpp"
#include "Buffer.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>



// std
#include <memory>
#include <vector>

namespace RenderingEngine
{
    class LveModel
    {
    public:
    
        struct Vertex{
            // pbr material 
            glm::vec3 position{};
            glm::vec3 normal{};
            glm::vec3 tangent{};
            glm::vec3 bitangent{};
            glm::vec2 uv{};
            // glm::vec3 color{};

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
        
            bool operator==(const Vertex& other) const
            {
                return position == other.position && normal == other.normal && tangent == other.tangent && bitangent == other.bitangent && uv == other.uv;
            }
        };
        
        struct Builder
        {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};
            void loadObjModel(const std::string& modelPath);
            void loadFbxModel(const std::string& modelPath);
        };

        
        LveModel(LveDevice& device, const Builder& builder);
        ~LveModel();
        
        LveModel(const LveModel&) = delete;
        LveModel& operator=(const LveModel&) = delete;
        
        static std::unique_ptr<LveModel> createModelFromFile(LveDevice& device, const std::string& filePath);

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);
    private:
        void createVertexBuffer(const std::vector<Vertex>& vertices);
        void createIndexBuffer(const std::vector<uint32_t>& indices);
        
        void createIndexBuffer();
        void createUniformBuffers();
        void createDescriptorPool();
        void createDescriptorSets();
        void createCommandBuffers();
        void createSyncObjects();
        
        LveDevice& mDevice;
        std::unique_ptr<LveBuffer> vertexBuffer;
        uint32_t vertexCount;
        
        bool hasIndexBuffer = false;
        std::unique_ptr<LveBuffer> indexBuffer;
        uint32_t indexCount;
    };
}