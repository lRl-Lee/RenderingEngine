#pragma once

#include "../Rendering/BasicPipeline.hpp"
#include "../Rendering/ComputePipeline.hpp"
#include "../Rendering/Vulkan/Device.hpp"
#include "../Rendering/Vulkan/Descriptors.hpp"
#include "../GameFramework/GameObject.hpp"
#include "../GameFramework/Camera.hpp"
#include "../GameFramework/FrameInfo.hpp"

#include <memory>
#include <vector>

namespace RenderingEngine
{

    class PBRRenderSystem
    {
    public:
        
        PBRRenderSystem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalDescriptorSetLayout);
        ~PBRRenderSystem();
        
        PBRRenderSystem(const PBRRenderSystem&) = delete;
        PBRRenderSystem& operator=(const PBRRenderSystem&) = delete;
        void renderGameObjects(FrameInfo& frameInfo);
   
    private:
        void createPipelineLayout(VkDescriptorSetLayout globalDescriptorSetLayout);
        void createPipeline(VkRenderPass renderPass);

        void createComputePipeline(); 

        void performComputePass(FrameInfo& frameInfo);  
        void performRenderPass(FrameInfo& frameInfo);  

        LveDevice& mDevice;

        std::unique_ptr<BasicPipeline> graphicsPipeline;  
        std::unique_ptr<ComputePipeline> computePipeline;  
        VkPipelineLayout graphicsPipelineLayout;  
        VkPipelineLayout computePipelineLayout;  

        std::unique_ptr<LveDescriptorSetLayout> renderSystemLayout;  
        std::unique_ptr<LveDescriptorSetLayout> computeSystemLayout;
    };

}