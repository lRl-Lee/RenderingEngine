#include "ComputePipeline.hpp"  
#include <fstream>  
#include <stdexcept>  
#include <iostream>  

namespace RenderingEngine  
{  
    ComputePipeline::ComputePipeline(  
        LveDevice& device,  
        const std::string& computeShaderPath,  
        const ComputePipelineConfigInfo& configInfo)  
        : mDevice{device}  
    {  
        createComputePipeline(computeShaderPath, configInfo);  
    }  

    ComputePipeline::~ComputePipeline()  
    {  
        vkDestroyShaderModule(mDevice.device(), computeShaderModule, nullptr);  
        vkDestroyPipeline(mDevice.device(), computePipeline, nullptr);  
    }  

    void ComputePipeline::bind(VkCommandBuffer commandBuffer)  
    {  
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);  
    }  

    void ComputePipeline::dispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)  
    {  
        vkCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);  
    }  

    void ComputePipeline::defaultPipelineConfigInfo(ComputePipelineConfigInfo& configInfo)  
    {  
        configInfo.pushConstantSize = 0;  
    }  

    std::vector<char> ComputePipeline::readFile(const std::string& filename)  
    {  
        std::ifstream file(filename, std::ios::ate | std::ios::binary);  

        if (!file.is_open()) {  
            throw std::runtime_error("failed to open file: " + filename);  
        }  

        size_t fileSize = (size_t)file.tellg();  
        std::vector<char> buffer(fileSize);  

        file.seekg(0);  
        file.read(buffer.data(), fileSize);  

        file.close();  

        return buffer;  
    }  

    void ComputePipeline::createComputePipeline(const std::string& computeShaderPath, const ComputePipelineConfigInfo& configInfo)  
    {   
        std::cout << "Compute Shader  Path: " << computeShaderPath << std::endl;
        
        auto computeShaderCode = readFile(computeShaderPath);  

        std::cout << "Compute Shader size: " << computeShaderCode.size() << " bytes" << std::endl;  

        createShaderModule(computeShaderCode, &computeShaderModule);  

        VkPipelineShaderStageCreateInfo shaderStage{};  
        shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;  
        shaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;  
        shaderStage.module = computeShaderModule;  
        shaderStage.pName = "main";  

        VkComputePipelineCreateInfo pipelineInfo{};  
        pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;  
        pipelineInfo.layout = configInfo.pipelineLayout;  
        pipelineInfo.stage = shaderStage;  

        if (vkCreateComputePipelines(mDevice.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline) != VK_SUCCESS) {  
            throw std::runtime_error("failed to create compute pipeline");  
        }  
    }  

    void ComputePipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule)  
    {  
        VkShaderModuleCreateInfo createInfo{};  
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;  
        createInfo.codeSize = code.size();  
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());  

        if (vkCreateShaderModule(mDevice.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {  
            throw std::runtime_error("failed to create shader module");  
        }  
    }  
}