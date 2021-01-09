project "glfw"
	kind "StaticLib"
	language "C"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"include/glfw3.h",
		"include/glfw3native.h",
		"src/glfw_config.h",
		"src/context.c",
		"src/init.c",
		"src/input.c",
		"src/monitor.c",
		"src/vulkan.c",
		"src/window.c",
	}

	filter "system:windows"
		systemversion "latest"
		staticruntime "On"

		files
		{
			"src/win32_init.c",
			"src/win32_joystick.c",
			"src/win32_monitor.c",
			"src/win32_time.c",
			"src/win32_thread.c",
			"src/win32_window.c",
			"src/wgl_context.c",
			"src/egl_context.c",
			"src/osmesa_context.c"
		}

		defines
		{
			"_GLFW_WIN32",
			"_CRT_SECURE_NO_WARNINGS"
		}

	filter "configurations:Debug"
		buildoptions "/MDd /Zi /utf-8 /EHsc /Ob1"
		flags "MultiProcessorCompile"
		vectorextensions "AVX2"
		floatingpoint "Fast"
		symbols "On"
		optimize "Debug"
		--flags "LinkTimeOptimization"

	filter "configurations:Release"
		buildoptions "/MD /Zi /utf-8 /EHsc /Ob2"
		flags "MultiProcessorCompile"
		vectorextensions "AVX2"
		floatingpoint "Fast"
		--symbols "Off"
		optimize "Speed"
		flags "LinkTimeOptimization"
