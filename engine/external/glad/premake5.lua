project "glad"
	kind "StaticLib"
	language "C"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"include/**.h",
		"src/**.c",
	}

	includedirs
	{
		"include",
	}

	filter "system:windows"
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
