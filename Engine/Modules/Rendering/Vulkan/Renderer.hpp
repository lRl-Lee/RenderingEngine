#pragma once
#include "../../Window/REWindow.hpp"
#include "Device.hpp"
#include "SwapChain.hpp"

#include <memory>
#include <vector>
#include <cassert>

namespace RenderingEngine
{
    class LveRenderer
    {
    public:
        
        LveRenderer(Window& window, LveDevice& device);
        ~LveRenderer();
        
        LveRenderer(const LveRenderer&) = delete;
        LveRenderer& operator=(const LveRenderer&) = delete;
        
        VkRenderPass getSwapChainRenderPass() const { return mSwapChain->getRenderPass(); }
        float getAspectRatio() const { return mSwapChain->extentAspectRatio(); }
        bool isFrameInProgress() const { return isFrameStarted; }
        
        VkCommandBuffer getCurrentCommandBuffer() const { 
            assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
            return commandBuffers[currentFrameIndex]; 
        }
        
        VkCommandBuffer beginFrame();
        void endFrame();
        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
        int getFrameIndex() const { 
            assert(isFrameStarted && "Cannot get frame index when frame is not in progress");
            return currentFrameIndex;
        }
    private:
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapChain();
        
    
        Window& mWindow;
        LveDevice& mDevice;
        std::unique_ptr<LveSwapChain> mSwapChain; // {Device,mWindow.getExtent()};
        std::vector<VkCommandBuffer> commandBuffers;
        
        uint32_t currentImageIndex;
        int currentFrameIndex{0};
        bool isFrameStarted = false;
    };

}