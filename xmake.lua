add_rules("mode.debug", "mode.release")

-- 添加 Catch2 依赖
-- add_requires("catch2 3.x")   

target("yatha")
    set_kind("binary")
    set_languages("c++17")

    add_files("src/*.cpp")

    add_includedirs("include")
    add_includedirs("third_party/cppjieba")

    set_targetdir("bin")
    set_rundir("data/")
    set_runargs("-s")
    
    -- 确保跨平台正确编译
    if is_plat("windows") then 
        add_cxflags("/utf-8", {tools = "cl"})
        add_cxflags("-finput-charset=UTF-8", "-fexec-charset=UTF-8", {tools = {"gcc", "clang"}})
    elseif is_plat("linux", "macosx") then 
        add_syslinks("pthread", "m")
    end

-- 测试目标 - WordRanker
target("test_word_ranker")
    set_kind("binary")
    set_languages("c++17")
    set_default(false)
    
    add_files("tests/test_word_ranker.cpp")
    add_files("src/word_ranker.cpp")
    
    add_includedirs("include")
    add_packages("catch2")
    
    set_targetdir("bin")
    
    if is_plat("linux", "macosx") then 
        add_syslinks("pthread", "m")
    end

-- 测试目标 - StopWordsManager
target("test_stop_words_manager")
    set_kind("binary")
    set_languages("c++17")
    set_default(false)
    
    add_files("tests/test_stop_words_manager.cpp")
    add_files("src/stop_words_manager.cpp")
    
    add_includedirs("include")
    add_packages("catch2")
    
    set_targetdir("bin")
    
    if is_plat("linux", "macosx") then 
        add_syslinks("pthread", "m")
    end

-- 测试目标 - TimeWindowManager
target("test_time_window_manager")
    set_kind("binary")
    set_languages("c++17")
    set_default(false)
    
    add_files("tests/test_time_window_manager.cpp")
    add_files("src/time_window_manager.cpp")
    add_files("src/word_ranker.cpp")
    
    add_includedirs("include")
    add_packages("catch2")
    
    set_targetdir("bin")
    
    if is_plat("linux", "macosx") then 
        add_syslinks("pthread", "m")
    end

target("test_ha_engine_sse")
    set_kind("binary")
    set_languages("c++17")
    set_default(false)

    add_files("tests/test_ha_engine_sse.cpp")
    add_files("src/ha_engine_sse.cpp")
    add_files("src/ha_engine.cpp")
    add_files("src/time_window_manager.cpp")
    add_files("src/word_ranker.cpp")
    add_files("src/stop_words_manager.cpp")
    
    add_includedirs("include")
    add_includedirs("third_party/cppjieba")
    add_packages("catch2")

    set_targetdir("bin")
    set_rundir(".")

    if is_plat("linux", "macosx") then 
        add_syslinks("pthread", "m")
    end
