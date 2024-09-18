#pragma once
#include "Device.hpp"

#include <vulkan/vulkan.h>

#include <memory>
#include <string>

namespace RenderingEngine{
    class LveTexture{
    public:
        LveTexture(LveDevice &device, const std::string &textureFilepath, VkFormat format, VkImageViewType viewType, VkImageLayout layout);
        LveTexture(
            LveDevice &device,
            VkFormat format,
            VkExtent3D extent,
            VkImageUsageFlags usage,
            VkSampleCountFlagBits sampleCount);        

        LveTexture(const LveTexture&) = delete;
        LveTexture& operator=(const LveTexture&) = delete;
        ~LveTexture();

        VkImageView imageView() const { return mTextureImageView; }
        VkSampler sampler() const { return mTextureSampler; }
        VkImage getImage() const { return mTextureImage; }
        VkImageView getImageView() const { return mTextureImageView; }
        VkDescriptorImageInfo getImageInfo() const { return mDescriptor; }
        VkImageLayout getImageLayout() const { return mTextureLayout; }
        VkExtent3D getExtent() const { return mExtent; }
        VkFormat getFormat() const { return mFormat; }

        void updateDescriptor();
        void transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout);
    
        static std::unique_ptr<LveTexture> createTextureFromFile(
          LveDevice &device, const std::string &filepath, 
            VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D,
            VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


    private:
        void createTextureImage(const std::string &filepath, VkFormat format, VkImageViewType viewType, VkImageLayout layout);
        void createTextureImageView(VkImageViewType viewType);
        void createTextureSampler();

        VkDescriptorImageInfo mDescriptor{};

        LveDevice &mDevice;
        VkImage mTextureImage = nullptr; // meta data of the image
        VkDeviceMemory mTextureImageMemory = nullptr; // vulkan device memory obj
        VkImageView mTextureImageView = nullptr; // define data format and range 
        VkSampler mTextureSampler = nullptr; // define sampler's filter mode and so on
        VkFormat mFormat;
        VkImageLayout mTextureLayout;
        VkImageViewType mViewType;
        uint32_t mMipLevels{1};
        uint32_t mLayerCount{1};
        VkExtent3D mExtent{};  
    };
}