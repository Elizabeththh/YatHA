add_rules("mode.debug", "mode.release")

target("yatha")
    set_kind("binary")
    set_languages("c++17")

    add_files("src/*.cpp")

    add_includedirs("include")
    add_includedirs("third_party/cppjieba")

    set_targetdir("bin")
    set_rundir("$(projectdir)")
    set_runargs("input1.txt", "output.txt")
    
    -- 确保跨平台正确编译
    if is_plat("windows") then 
        add_cxflags("/utf-8", {tools = "cl"})
        add_cxflags("-finput-charset=UTF-8", "-fexec-charset=UTF-8", {tools = {"gcc", "clang"}})
    elseif is_plat("linux", "macosx") then 
        add_syslinks("pthread", "m")
    end

