#include "REWindow.hpp"

#include <stdexcept>
namespace RenderingEngine
{
    Window::Window(int w, int h, std::string name) : mWidth(w), mHeight(h), mWindowName(name)
    {
        initWindow();
    }

    Window::~Window()
    {
        glfwDestroyWindow(mWindow);
        glfwTerminate();
    }
    void Window::initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        mWindow = glfwCreateWindow(mWidth, mHeight, mWindowName.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(mWindow, this);
        glfwSetFramebufferSizeCallback(mWindow, framebufferResizeCallback);
    }

    void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
    {
        if (glfwCreateWindowSurface(instance, mWindow, nullptr, surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface");
        }
    }

    void Window::framebufferResizeCallback(GLFWwindow *window, int width, int height)
    {
        auto reWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
        reWindow->framebufferResized = true;
        reWindow->mWidth = width;
        reWindow->mHeight = height;
    }
}