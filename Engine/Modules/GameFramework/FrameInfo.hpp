#pragma once

#include "Camera.hpp"
#include "../Rendering/Vulkan/Descriptors.hpp"
#include "GameObject.hpp"

// lib
#include <vulkan/vulkan.h>

namespace RenderingEngine
{

#define MAX_LIGHTS 10

    struct PointLight
    {
        glm::vec4 position{}; // ignore w
        alignas(16) glm::vec4 color{}; // w is intensity
    };

    struct GlobalUbo
    {
        glm::mat4 projection{1.0f};
        glm::mat4 view{1.0f};
        glm::mat4 inverseView{1.0f};
        glm::vec4 ambientLightColor{1.0f, 1.0f, 1.0f, 0.02f}; // w is light intensity
        PointLight pointLights[MAX_LIGHTS];
        int numLights;
    };

    struct FrameInfo
    {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        Camera& camera;
        VkDescriptorSet globalDescriptorSets;
        LveDescriptorPool &frameDescriptorPool; // pool of descriptors that is cleared each frame
        GameObject::Map &gameObjects;
    };
}