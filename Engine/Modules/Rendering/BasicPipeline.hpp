#pragma once

#include "Vulkan/Device.hpp"
#include "Vulkan/Model.hpp"
#include <string>
#include <vector>


namespace RenderingEngine
{
    // create and manage basic graphics pipeline
    struct PipelineConfigInfo{
        PipelineConfigInfo() = default;
        PipelineConfigInfo(const PipelineConfigInfo&) = delete;
        PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        
        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };

    class BasicPipeline
    {
    public:
        BasicPipeline(
                    LveDevice& device,
                    const std::string vertPath, 
                    const std::string fragPath, 
                    const PipelineConfigInfo& configInfo);
                    
        ~BasicPipeline();
        
        BasicPipeline(const BasicPipeline&) = delete;
        BasicPipeline& operator=(const BasicPipeline&) = delete;
        
        void bind(VkCommandBuffer commandBuffer);
        static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
        static void enableAlphaBlending(PipelineConfigInfo& configInfo);

    private:
        static std::vector<char> readFile(const std::string &filename);

        void createGraphicsPipeline(const std::string vertPath, const std::string fragPath,const PipelineConfigInfo& configInfo);
        
        void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

        LveDevice& mDevice;
        VkPipeline graphicsPipeline;
        VkShaderModule vertShaderModule;
        VkShaderModule fragShaderModule;
    };
}