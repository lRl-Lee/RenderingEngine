
rule("ShaderCompile")
    set_extensions(".frag", ".vert", ".comp")
    on_build_file(function (target, sourcefile, opt)
        -- find path of vulkan_sdk, and get path of glslangValidator.exe
        local vulkan_sdk = find_package("vulkansdk") 
        local glslang_validator_dir = path.join(vulkan_sdk["bindir"], "glslangValidator.exe") 

        -- make sure build directory exists
        local build_dir = vformat("$(buildir)")
        local shader_target_dir = path.join(build_dir, "ShaderBin");  -- build dir means .\build
        os.mkdir(shader_target_dir) 

        -- concat target file name
        local targetfile = path.join(shader_target_dir, path.filename(sourcefile)..".spv")

        -- call glslangValidator to build shader
        if is_mode("debug") then
            os.execv(glslang_validator_dir, {"--target-env", "vulkan1.0", sourcefile, "-g", "-o", targetfile})
        end
        if is_mode("release") then
            os.execv(glslang_validator_dir, {"--target-env", "vulkan1.0", sourcefile, "-o", targetfile})
        end
    end)

target("Shaders")
    set_kind("object")
    add_rules("ShaderCompile")
    add_files("*.vert", "*.frag", "*.comp")