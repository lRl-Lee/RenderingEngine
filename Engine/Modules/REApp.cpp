#include "REApp.hpp"
#include "EngineSystem/PBRRenderSystem.hpp"
//#include "EngineSystem/BasicRenderSystem.hpp"
#include "EngineSystem/PointLightSystem.hpp"
#include "GameFramework/Camera.hpp"
#include "GameFramework/KeyboardMovementController.hpp"
#include "Rendering/Vulkan/Buffer.hpp"


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>



// std
#include <array>
#include <chrono>
#include <stdexcept>
#include <iostream>

namespace RenderingEngine
{    
    REApp::REApp()
    {
        // global uniform buffer
        globalPool =
            LveDescriptorPool::Builder(Device)
                .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT) // 3
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
                .build();

        framePools.resize(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        auto framePoolBuilder = LveDescriptorPool::Builder(Device)
                                    .setMaxSets(1000)
                                    .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
                                    .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
                                    .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100)
                                    .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
        std::cout << "Frame pool size: " << framePools.size() << "\n";
        for (int i = 0; i < framePools.size(); i++) {
            framePools[i] = framePoolBuilder.build();
        }

        loadGameObjects();

    }
    
    REApp::~REApp(){}

    void REApp::loadGameObjects()
    {

        // load obj models
        std::shared_ptr<LveModel> mModel = LveModel::createModelFromFile(Device, "E:/Projects/VulkanEngine/Assets/Models/cerberus.fbx");
        GameObject& gameObj = gameObjectManager.createGameObject();
        gameObj.model = mModel;
        gameObj.color = {1.0f, 1.0f, 1.0f};
        gameObj.transform.translation = {0.0f, 0.0f, 0.0f};
        gameObj.transform.scale = {0.01f, 0.01f, 0.01f};
        gameObj.transform.rotation = {glm::pi<float>(), 0.0f, 0.0f};// .25 * glm::two_pi<float>();
        
        // load model textures 
        std::shared_ptr<LveTexture> mAlbedo = LveTexture::createTextureFromFile(Device, "E:/Projects/VulkanEngine/Assets/Textures/cerberus_A.png");
        gameObj.diffuseMap = mAlbedo;
        std::shared_ptr<LveTexture> mNormal = LveTexture::createTextureFromFile(Device, "E:/Projects/VulkanEngine/Assets/Textures/cerberus_N.png");
        gameObj.normalMap = mNormal;
        std::shared_ptr<LveTexture> mRoughness = LveTexture::createTextureFromFile(Device, "E:/Projects/VulkanEngine/Assets/Textures/cerberus_R.png");
        gameObj.roughnessMap = mRoughness;
        std::shared_ptr<LveTexture> mMetallic = LveTexture::createTextureFromFile(Device, "E:/Projects/VulkanEngine/Assets/Textures/cerberus_M.png");
        gameObj.metallicMap = mMetallic;

        // load env map
        //std::shared_ptr<LveTexture> mEnvMap = LveTexture::createTextureFromFile(Device, 
                                                                 //               "E:/Projects/VulkanEngine/Assets/Textures/environment.hdr",
                                                                //                 VK_FORMAT_R16G16B16A16_SFLOAT,
                                                               //                  VK_IMAGE_VIEW_TYPE_CUBE,
                                                                  //               VK_IMAGE_LAYOUT_GENERAL);
        //gameObj.envMap = mEnvMap;    
        
        //mModel = LveModel::createModelFromFile(Device, "E:/Projects/VulkanEngine/Assets/Models/skybox.obj");
        //GameObject& skyboxObj = gameObjectManager.createGameObject();
        //skyboxObj.model = mModel;
        //skyboxObj.color = {1.0f, 1.0f, 1.0f};
        //skyboxObj.transform.translation = {0.0f, 0.0f, 0.0f};
        //skyboxObj.transform.scale = {10.0f, 10.0f, 10.0f};
        //skyboxObj.transform.rotation = {0.0f, 0.0f, 0.0f};// .25 * glm::two_pi<float>();

        // point light
        GameObject& pointLightObj = gameObjectManager.makePointLight(5.0f,1.0f);
        pointLightObj.transform.translation = {0.0f, -1.0f, -2.0f};           
    }

    void REApp::run()
    {
        std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);

        for(int i=0; i<uboBuffers.size(); i++)
        {
            uboBuffers[i] = std::make_unique<LveBuffer>(
                Device,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            );
            uboBuffers[i]->map();
        }

        auto globalSetLayout = LveDescriptorSetLayout::Builder(Device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .build();
        
        std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        
        for (size_t i = 0; i < globalDescriptorSets.size(); i++)
        {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            LveDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }
        
        std::cout << "Alignment: " << Device.properties.limits.minUniformBufferOffsetAlignment << "\n";
        std::cout << "atom size: " << Device.properties.limits.nonCoherentAtomSize << "\n";

        //BasicRenderSystem basicRenderSystem{Device, Renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        PBRRenderSystem pbrRenderSystem{Device, Renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        
        PointLightSystem pointLightSystem{Device, Renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};

        Camera camera{};
        
        GameObject& viewObject = gameObjectManager.createGameObject();
        KeyboardMovementController cameraController{};
        
        auto currentTime = std::chrono::high_resolution_clock::now();
        
        
        while (!mWindow.shouldClose())
        {
            glfwPollEvents();
            
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;
            frameTime = glm::min(frameTime, 0.1f); // Prevent from large delta time after a breakpoint
            
            cameraController.moveInPlaneXZ(mWindow.getGLFWwindow(), frameTime, viewObject);
            camera.setViewYXZ(viewObject.transform.translation, viewObject.transform.rotation);
            
            float aspect = Renderer.getAspectRatio();
            
            //camera.setOrthographicProjection(-aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f);
            camera.setPerspectiveProjection(glm::radians(45.0f), aspect, 0.1f, 100.0f);
            
            if(auto commandBuffer = Renderer.beginFrame())
            {
                int frameIndex = Renderer.getFrameIndex();
                FrameInfo frameInfo
                {
                    frameIndex,
                    frameTime,
                    commandBuffer,
                    camera,
                    globalDescriptorSets[frameIndex],
                    *framePools[frameIndex],
                    gameObjectManager.gameObjects
                };

                // update
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
                ubo.ambientLightColor = {1.0f, 1.0f, 1.0f, 0.05f};

                pointLightSystem.update(frameInfo, ubo);

                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // final step of update is updating the game objects buffer data
                // The render functions MUST not change a game objects transform data
                gameObjectManager.updateBuffer(frameIndex);

                // render
                Renderer.beginSwapChainRenderPass(commandBuffer);
                // basicRenderSystem.renderGameObjects(frameInfo);
                pbrRenderSystem.renderGameObjects(frameInfo);
                pointLightSystem.render(frameInfo);

                Renderer.endSwapChainRenderPass(commandBuffer);
                Renderer.endFrame();
            }
        }
        
        vkDeviceWaitIdle(Device.device());
    }

}