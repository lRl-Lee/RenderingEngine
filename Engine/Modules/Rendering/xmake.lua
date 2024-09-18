includes("Vulkan")

target("BasicPipeline")
    set_kind("static")
    add_files("*.cpp")
    add_deps("VulkanRHI")
    add_includedirs("Vulkan")