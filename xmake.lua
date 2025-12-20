add_rules("mode.debug", "mode.release")


target("yatha")
    set_kind("binary")
    add_files("src/*.cpp")

    add_includedirs("include")
    add_includedirs("third_party/cppjieba")

    set_targetdir("bin")
    set_rundir("$(projectdir)")
    set_runargs("input1.txt", "output.txt")
    
    if is_plat("linux", "macosx") then 
        add_syslinks("pthread", "m")
    end

