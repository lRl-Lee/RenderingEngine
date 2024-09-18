#pragma once

#include "Vulkan/Device.hpp"
#include "Vulkan/Model.hpp"
#include <string>
#include <vector>


namespace RenderingEngine
{
    struct ComputePipelineConfigInfo{
        ComputePipelineConfigInfo() = default;
        ComputePipelineConfigInfo(const ComputePipelineConfigInfo&) = delete;
        ComputePipelineConfigInfo& operator=(const ComputePipelineConfigInfo&) = delete;

        VkPipelineLayout pipelineLayout = nullptr;  
        uint32_t pushConstantSize = 0; 
    };

    class ComputePipeline
    {
    public:
        ComputePipeline(
                    LveDevice& device,
                    const std::string& computeShaderPath, 
                    const ComputePipelineConfigInfo& configInfo);
                    
        ~ComputePipeline();
        
        ComputePipeline(const ComputePipeline&) = delete;
        ComputePipeline& operator=(const ComputePipeline&) = delete;
        
        void bind(VkCommandBuffer commandBuffer);

        static void defaultPipelineConfigInfo(ComputePipelineConfigInfo& configInfo);
        void dispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ); 

    private:
        static std::vector<char> readFile(const std::string &filename);

        void createComputePipeline(const std::string& computeShaderPath, const ComputePipelineConfigInfo& configInfo);
        
        void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

        LveDevice& mDevice;
        VkPipeline computePipeline;
        VkShaderModule computeShaderModule;
    };
}