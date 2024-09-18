#pragma once
#include "BasicRenderSystem.hpp"
#include <stdexcept>
namespace RenderingEngine
{
    struct SimplePushConstantData
    {
        glm::mat4 modelMatrix{1.0f};
        glm::mat4 normalMatrix{1.0f};
    };
    
    BasicRenderSystem::BasicRenderSystem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalDescriptorSetLayout)
        : mDevice(device)
    {
        createPipelineLayout(globalDescriptorSetLayout);
        createPipeline(renderPass);
    }
    
    BasicRenderSystem::~BasicRenderSystem()
    {
        vkDestroyPipelineLayout(mDevice.device(), pipelineLayout, nullptr);
    }
    
    void BasicRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalDescriptorSetLayout)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);
    
        renderSystemLayout = LveDescriptorSetLayout::Builder(mDevice)
                                  .addBinding(
                                      0,
                                      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
                                  .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)                                  
                                .build();

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
            globalDescriptorSetLayout,
            renderSystemLayout->getDescriptorSetLayout()};


        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        
        if (vkCreatePipelineLayout(mDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create pipeline layout!");
        }
    }
    
    void BasicRenderSystem::createPipeline(VkRenderPass renderPass)
    {
        PipelineConfigInfo pipelineConfig{};
        BasicPipeline::defaultPipelineConfigInfo(pipelineConfig);
        
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        // root folder
        Pipeline = std::make_unique<BasicPipeline>(mDevice, 
            "E:/Projects/VulkanEngine/build/ShaderBin/bling_phong.vert.spv", 
            "E:/Projects/VulkanEngine/build/ShaderBin/bling_phong.frag.spv", 
            pipelineConfig);
    }
    void BasicRenderSystem::renderGameObjects(FrameInfo& frameInfo)
    {
        Pipeline->bind(frameInfo.commandBuffer);
        
        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0, // firstSet
            1,
            &frameInfo.globalDescriptorSets,
            0,
            nullptr);

        for (auto& kv : frameInfo.gameObjects)
        {
            auto& obj = kv.second;
            if (obj.model == nullptr)
            {
                continue;
            }

            auto bufferInfo = obj.getBufferInfo(frameInfo.frameIndex);
            auto imageInfo = obj.diffuseMap->getImageInfo();
            VkDescriptorSet gameObjectDescriptorSet;

            LveDescriptorWriter(*renderSystemLayout, frameInfo.frameDescriptorPool)
                .writeBuffer(0, &bufferInfo)
                .writeImage(1, &imageInfo)
                .build(gameObjectDescriptorSet);

            vkCmdBindDescriptorSets(
                frameInfo.commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineLayout,
                1,  // starting set (0 is the globalDescriptorSet, 1 is the set specific to this system)
                1,  // set count
                &gameObjectDescriptorSet,
                0,
                nullptr);


            SimplePushConstantData push{};
            push.normalMatrix = obj.transform.normalMatrix();
            push.modelMatrix = obj.transform.mat4();
            vkCmdPushConstants(
                frameInfo.commandBuffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(SimplePushConstantData),
                &push);
            obj.model->bind(frameInfo.commandBuffer);
            obj.model->draw(frameInfo.commandBuffer);
        }
    }
    

}