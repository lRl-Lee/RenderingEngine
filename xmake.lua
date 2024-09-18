set_project("RenderingEngine")

set_arch("x64")
set_warnings("all")
set_languages("c++20")
--set_exceptions("cxx") -- prevent error message: exceptions disabled

add_rules("mode.debug", "mode.release")

set_defaultmode("debug") -- set default compile mode

--if is_mode("debug") then  
--    add_defines("ENABLE_VALIDATION_LAYERS")  
--end  

add_requires("vulkansdk", "glm", "vcpkg::glfw3", "assimp")
add_packages("vulkansdk", "glm", "vcpkg::glfw3", "assimp")
add_syslinks("user32", "gdi32", "shell32")
includes("Engine","Shaders")