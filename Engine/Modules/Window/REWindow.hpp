#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>  
#include <string>

namespace RenderingEngine
{
    class Window
    {
    public:
        Window(int width, int height, std::string name);
        ~Window();

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        bool shouldClose() { return glfwWindowShouldClose(mWindow); }
        VkExtent2D getExtent() { return {static_cast<uint32_t>(mWidth), static_cast<uint32_t>(mHeight)}; }
        bool wasWindowResized() { return framebufferResized; }
        void resetWindowResizedFlag() { framebufferResized = false; }
        GLFWwindow *getGLFWwindow() const { return mWindow; }

        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

    private:
        void initWindow();
        static void framebufferResizeCallback(GLFWwindow *window, int width, int height);

        int mWidth;
        int mHeight;
        bool framebufferResized = false;
        std::string mWindowName;
        GLFWwindow *mWindow;
    };

}