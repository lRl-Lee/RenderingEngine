#include "Renderer.hpp"

#include <array>
#include <stdexcept>


namespace RenderingEngine
{    
    LveRenderer::LveRenderer(Window& window, LveDevice& device):
        mWindow(window), mDevice(device)
    {
        recreateSwapChain();
        createCommandBuffers();
    }
    LveRenderer::~LveRenderer()
    {
        freeCommandBuffers();
    }
    
    void LveRenderer::createCommandBuffers()
    {   
        // primary command buffer: render pass, pipeline, textures, vertices, etc
        // double or trible frame buffers
        commandBuffers.resize(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = mDevice.getCommandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
        
        if (vkAllocateCommandBuffers(mDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate command buffers!");
        }
       
    }
    
    void LveRenderer::recreateSwapChain()
    {
        auto extent = mWindow.getExtent();
        while (extent.width == 0 || extent.height == 0)
        {
            extent = mWindow.getExtent();
            glfwWaitEvents();
        }
        
        vkDeviceWaitIdle(mDevice.device());
        if (mSwapChain == nullptr)
        {
            mSwapChain = std::make_unique<LveSwapChain>(mDevice, extent);
        }
        else
        {
            std::shared_ptr<LveSwapChain> oldSwapChain = std::move(mSwapChain);
            mSwapChain = std::make_unique<LveSwapChain>(mDevice, extent, oldSwapChain);
            
            if (!oldSwapChain->compareSwapFormats(*mSwapChain.get()))
            {
                throw std::runtime_error("Swap chain image format has changed!");
            }

        }
    }
    
    void LveRenderer::freeCommandBuffers()
    {
        vkFreeCommandBuffers(
            mDevice.device(), 
            mDevice.getCommandPool(),
            static_cast<uint32_t>(commandBuffers.size()),
            commandBuffers.data());
        commandBuffers.clear();
    }
    
    VkCommandBuffer LveRenderer::beginFrame()
    {
        assert(!isFrameStarted && "Can't call beginFrame while alread in progress");
        
        auto result = mSwapChain->acquireNextImage(&currentImageIndex);

        if(result == VK_ERROR_OUT_OF_DATE_KHR) // window resized
        {
            recreateSwapChain();
            return nullptr;
        }

        if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image");
        }

        isFrameStarted = true;

        auto commandBuffer = getCurrentCommandBuffer();
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if(vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin recording command buffer");
        }
        return commandBuffer;
    }
    
    void LveRenderer::endFrame()
    {
        assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
        auto commandBuffer = getCurrentCommandBuffer();

        if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to record command buffer");
        }

        auto result = mSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);

        if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mWindow.wasWindowResized())
        {
            mWindow.resetWindowResizedFlag();
            recreateSwapChain();
        }
        else if(result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to present swap chain image");
        }
        
        isFrameStarted = false;
        currentFrameIndex = (currentFrameIndex + 1) % LveSwapChain::MAX_FRAMES_IN_FLIGHT;
    }
    
    void LveRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
    {
        assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");
        
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = mSwapChain->getRenderPass();
        renderPassInfo.framebuffer = mSwapChain->getFrameBuffer(currentImageIndex);
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = mSwapChain->getSwapChainExtent();
        
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();
        
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(mSwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(mSwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, mSwapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }
    
    void LveRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
    {
        assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame");
        
        vkCmdEndRenderPass(commandBuffer);
    }

}