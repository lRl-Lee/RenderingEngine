#pragma once

#include "../Rendering/BasicPipeline.hpp"
#include "../Rendering/Vulkan/Device.hpp"
#include "../Rendering/Vulkan/Descriptors.hpp"
#include "../GameFramework/GameObject.hpp"
#include "../GameFramework/Camera.hpp"
#include "../GameFramework/FrameInfo.hpp"

#include <memory>
#include <vector>

namespace RenderingEngine
{
    // bling-phong redering model
    class BasicRenderSystem
    {
    public:
        
        BasicRenderSystem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalDescriptorSetLayout);
        ~BasicRenderSystem();
        
        BasicRenderSystem(const BasicRenderSystem&) = delete;
        BasicRenderSystem& operator=(const BasicRenderSystem&) = delete;
        void renderGameObjects(FrameInfo& frameInfo);
   
    private:
        void createPipelineLayout(VkDescriptorSetLayout globalDescriptorSetLayout);
        void createPipeline(VkRenderPass renderPass);

        
        LveDevice& mDevice;

        std::unique_ptr<BasicPipeline> Pipeline;
        VkPipelineLayout pipelineLayout;

        std::unique_ptr<LveDescriptorSetLayout> renderSystemLayout;
    };

}