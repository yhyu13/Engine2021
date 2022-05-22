project "imgui"
    kind "StaticLib"
    language "C"
    
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "imconfig.h",
        "imgui.h",
        "imgui.cpp",
        "imgui_demo.cpp",
        "imgui_draw.cpp",
        "imgui_internal.h",
        "imgui_tables.cpp",
        "imgui_widgets.cpp",
        "imstb_rectpack.h",
        "imstb_textedit.h",
        "imstb_truetype.h",
    }
    
    filter "system:windows"
        cppdialect "C++20"
        systemversion "latest"
        staticruntime "On"
        
    filter "configurations:Debug"
        buildoptions "/MDd /Zi /utf-8 /EHsc /Ob1"
        flags "MultiProcessorCompile"
        vectorextensions "AVX2"
        floatingpoint "Fast"
        symbols "On"
        optimize "Debug"

    filter "configurations:Release"
        buildoptions "/MD /Zi /utf-8 /EHsc /Ob2"
        flags "MultiProcessorCompile"
        vectorextensions "AVX2"
        floatingpoint "Fast"
        optimize "Speed"
        flags "LinkTimeOptimization"
