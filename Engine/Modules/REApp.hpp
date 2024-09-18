#pragma once

#include "Window/REWindow.hpp"
#include "Rendering/Vulkan/Descriptors.hpp"
#include "Rendering/Vulkan/Device.hpp"
#include "Rendering/Vulkan/Renderer.hpp"
#include "GameFramework/GameObject.hpp"



#include <memory>
#include <vector>

namespace RenderingEngine
{
    class REApp
    {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;
        
        REApp();
        ~REApp();
        
        REApp(const REApp&) = delete;
        REApp& operator=(const REApp&) = delete;
        
        void run();
        
    private:
        void loadGameObjects();
        
    
        Window mWindow{WIDTH,HEIGHT,"RE APP"};
        LveDevice Device{mWindow};
        LveRenderer Renderer{mWindow,Device};

        // note: order of declarations matters
        std::unique_ptr<LveDescriptorPool> globalPool{};
        std::vector<std::unique_ptr<LveDescriptorPool>> framePools;
        GameObjectManager gameObjectManager{Device};
    };

}