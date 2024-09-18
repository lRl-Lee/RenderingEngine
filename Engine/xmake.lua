includes("Modules")

target("RenderingEngine")
    set_default(true) -- set this target as default build
    set_kind("binary")
    add_deps("Modules")
    add_includedirs("Modules")
    add_files("*.cpp")
    set_rundir("$(projectdir)/Engine")