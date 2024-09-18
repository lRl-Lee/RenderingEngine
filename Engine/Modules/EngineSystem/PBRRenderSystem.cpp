#pragma once
#include "PBRRenderSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
namespace RenderingEngine
{
    struct SimplePushConstantData
    {
        glm::mat4 modelMatrix{1.0f};
        glm::mat4 normalMatrix{1.0f};
    };
    
    PBRRenderSystem::PBRRenderSystem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalDescriptorSetLayout)
        : mDevice(device)
    {
        createPipelineLayout(globalDescriptorSetLayout);
        createPipeline(renderPass);
        createComputePipeline();  
    }
    
    PBRRenderSystem::~PBRRenderSystem()
    {
        vkDestroyPipelineLayout(mDevice.device(), graphicsPipelineLayout, nullptr);  
        vkDestroyPipelineLayout(mDevice.device(), computePipelineLayout, nullptr);  
    }
    
    void PBRRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalDescriptorSetLayout)
    {
        // graphics pipeline layout
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);
        
    
        renderSystemLayout = LveDescriptorSetLayout::Builder(mDevice)
                                  .addBinding(
                                      0,
                                      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
                                  .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)   // albedo
                                  .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)   // normal
                                  .addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)   // roughness
                                  .addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)   // metallic
                                  .addBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)   // specular
                                  .addBinding(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)   // irradiance
                                  .addBinding(7, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)   // specularBRDF_LUT                       
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
        
        if (vkCreatePipelineLayout(mDevice.device(), &pipelineLayoutInfo, nullptr, &graphicsPipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create graphics pipeline layout!");
        }

        // compute pipeline layout
        computeSystemLayout = LveDescriptorSetLayout::Builder(mDevice)
                                  .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT) // input 
                                  .addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT) // output   
                                  .addBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)  // output mipmap 
                                .build();
        std::vector<VkDescriptorSetLayout> computeDescriptorSetLayouts{  
            computeSystemLayout->getDescriptorSetLayout()  
        }; 
        VkPipelineLayoutCreateInfo computePipelineLayoutInfo{};  
        computePipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;  
        computePipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(computeDescriptorSetLayouts.size());  
        computePipelineLayoutInfo.pSetLayouts = computeDescriptorSetLayouts.data();  
        computePipelineLayoutInfo.pushConstantRangeCount = 0;  
        computePipelineLayoutInfo.pPushConstantRanges = nullptr; 

        if (vkCreatePipelineLayout(mDevice.device(), &computePipelineLayoutInfo, nullptr, &computePipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create compute pipeline layout!");
        }
    }
    
    void PBRRenderSystem::createPipeline(VkRenderPass renderPass)
    {
        assert(graphicsPipelineLayout != nullptr && "Cannot create graphics pipeline before pipeline layout");
        
        PipelineConfigInfo pipelineConfig{};
        BasicPipeline::defaultPipelineConfigInfo(pipelineConfig);
        
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = graphicsPipelineLayout;
        
        // root folder
        graphicsPipeline = std::make_unique<BasicPipeline>(mDevice, 
            "E:/Projects/VulkanEngine/build/ShaderBin/pbr.vert.spv", 
            "E:/Projects/VulkanEngine/build/ShaderBin/pbr.frag.spv", 
            pipelineConfig);
    }

    void PBRRenderSystem::createComputePipeline()  
    {  
        assert(computePipelineLayout != nullptr && "Cannot create compute pipeline before pipeline layout");

        ComputePipelineConfigInfo computeConfig{};  
        ComputePipeline::defaultPipelineConfigInfo(computeConfig);  
        computeConfig.pipelineLayout = computePipelineLayout;  
        computePipeline = std::make_unique<ComputePipeline>(  
            mDevice,  
            "E:/Projects/VulkanEngine/build/ShaderBin/irmap.comp.spv",  
            computeConfig  
        );  
    } 

    void PBRRenderSystem::performRenderPass(FrameInfo& frameInfo)  
    {
        graphicsPipeline->bind(frameInfo.commandBuffer);
        
        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            graphicsPipelineLayout,
            0, // firstSet
            1, // descriptorSetCount
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
            auto albedoInfo = obj.diffuseMap->getImageInfo();
            auto normalInfo = obj.normalMap->getImageInfo();
            auto roughnessInfo = obj.roughnessMap->getImageInfo();
            auto metallicInfo = obj.metallicMap->getImageInfo();
                 
            VkDescriptorSet gameObjectDescriptorSet;

            LveDescriptorWriter(*renderSystemLayout, frameInfo.frameDescriptorPool)
                .writeBuffer(0, &bufferInfo)
                .writeImage(1, &albedoInfo)
                .writeImage(2, &normalInfo)
                .writeImage(3, &roughnessInfo)
                .writeImage(4, &metallicInfo)
                .writeImage(5, &metallicInfo)
                .writeImage(6, &metallicInfo)
                .writeImage(7, &metallicInfo)
                .build(gameObjectDescriptorSet);

            vkCmdBindDescriptorSets(
                frameInfo.commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                graphicsPipelineLayout,
                1,  // starting set (0 is the globalDescriptorSet, 1 is the set specific to this system)
                1,  // set count
                &gameObjectDescriptorSet,
                0,
                nullptr);


            SimplePushConstantData push{};
            push.modelMatrix = obj.transform.mat4();
            push.normalMatrix = obj.transform.normalMatrix();
            

            vkCmdPushConstants(
                frameInfo.commandBuffer,
                graphicsPipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(SimplePushConstantData),
                &push);

            obj.model->bind(frameInfo.commandBuffer);
            obj.model->draw(frameInfo.commandBuffer);
        }
    }

    void PBRRenderSystem::performComputePass(FrameInfo& frameInfo)  
    {  
        computePipeline->bind(frameInfo.commandBuffer);  
        
        for (auto& kv : frameInfo.gameObjects)
        {
            auto& obj = kv.second;
            if (obj.envMap == nullptr)
            {
                continue;
            }

            auto envMapInfo = obj.envMap->getImageInfo();
            auto albedoInfo = obj.diffuseMap->getImageInfo();

            VkDescriptorSet envMapDescriptorSet;

            LveDescriptorWriter(*computeSystemLayout, frameInfo.frameDescriptorPool)
                .writeImage(0, &albedoInfo)
                .writeImage(1, &envMapInfo)
                .writeImage(2, &envMapInfo)
                .build(envMapDescriptorSet);

            vkCmdBindDescriptorSets(
                frameInfo.commandBuffer,
                VK_PIPELINE_BIND_POINT_COMPUTE,
                computePipelineLayout,
                0, 
                1,  
                &envMapDescriptorSet,
                0,
                nullptr);
            }
            // computePipeline->dispatch(frameInfo.commandBuffer, 32, 32, 1);

    }  

    void PBRRenderSystem::renderGameObjects(FrameInfo& frameInfo)
    {
        performComputePass(frameInfo);
        performRenderPass(frameInfo);
    }
    

}