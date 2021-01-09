workspace "Long_March_Engine_Project"
	architecture "x86_64"
	startproject "application"

	configurations {
		"Debug",
		"Release"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["glfw"]     = "engine/external/glfw/include"
IncludeDir["ImGui"]      =   "engine/external/imgui"
IncludeDir["lua"]      =   "engine/external/lua540/src"
IncludeDir["glad"]     = "engine/external/glad/include"
IncludeDir["tileson"]      ="engine/external/tileson/include"
IncludeDir["SOIL2"]      = "engine/external/SOIL2"
IncludeDir["jsoncpp"]  = "engine/external/jsoncpp/include"

IncludeDir["assimp"]     = "engine/vendors/assimp/include"
IncludeDir["glm"]      = "engine/vendors/glm"
IncludeDir["fmod_core"]  = "engine/vendors/fmod/api/core/inc"
IncludeDir["fmod_bank"]  = "engine/vendors/fmod/api/fsbank/inc"
IncludeDir["fmod_studio"]  = "engine/vendors/fmod/api/studio/inc"
IncludeDir["spdlog"] 		= "engine/vendors/spdlog/include"
IncludeDir["phmap"]     = "engine/vendors/phmap/include"
IncludeDir["blaze"]     = "engine/vendors/blaze/include"
IncludeDir["qu3e"]     = "engine/vendors/qu3e/include"
IncludeDir["sol2"]     = "engine/vendors/sol2/include"

LibDir = {}
LibDir["assimp"] = "engine/vendors/assimp/lib/x64"
LibDir["qu3e"] = "engine/vendors/qu3e/lib/x64"
LibDir["fmod_core"] = "engine/vendors/fmod/api/core/lib/x64"
LibDir["fmod_bank"] = "engine/vendors/fmod/api/fsbank/lib/x64"
LibDir["fmod_studio"] = "engine/vendors/fmod/api/studio/lib/x64"

LibName = {}
LibName["fmod_core"] = "fmod_vc.lib"
LibName["fmod_bank"] = "fsbank_vc.lib"
LibName["fmod_studio"] = "fmodstudio_vc.lib"
LibName["assimp_debug"] = "/Release/assimp-vc142-mt.lib"
LibName["assimp"] = "/Release/assimp-vc142-mt.lib"
LibName["qu3e"] = "/Release/qu3e.lib"
LibName["qu3e_debug"] = "/Debug/qu3e.lib"

DllName = {}
DllName["assimp_debug"] = "/Release/assimp-vc142-mt.dll"
DllName["assimp"] = "/Release/assimp-vc142-mt.dll"
DllName["fmod_core"] = "fmod.dll"
DllName["fmod_bank"] = "fsbank.dll"
DllName["fmod_bank2"] = "libfsbvorbis64.dll"
DllName["fmod_studio"] = "fmodstudio.dll"

-- Build non-header-only lib
include "engine/external/glad"
include "engine/external/glfw"
include "engine/external/SOIL2"
include "engine/external/imgui/imgui"
include "engine/external/lua540/src"
include "engine/external/jsoncpp"
include "engine/external/tileson"

project "engine"
	location "engine"
	kind "StaticLib"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-intermediate/" .. outputdir .. "/%{prj.name}")
	
	pchheader "engine-precompiled-header.h"
	pchsource "engine/source/engine-precompiled-header.cpp"

	files
	{
		"%{prj.name}/source/**.h",
		"%{prj.name}/source/**.inl",
		"%{prj.name}/source/**.cpp",

		"%{prj.name}/vendors/glm/glm/**.hpp",
		"%{prj.name}/vendors/glm/glm/**.inl",
	}

	includedirs
	{
		"%{prj.name}/source",
		"%{IncludeDir.assimp}",
		"%{IncludeDir.qu3e}",
		"%{IncludeDir.blaze}",
		"%{IncludeDir.phmap}",
		"%{IncludeDir.glfw}",
		"%{IncludeDir.glad}",
		"%{IncludeDir.SOIL2}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.jsoncpp}",
		"%{IncludeDir.fmod_core}",
		"%{IncludeDir.fmod_bank}",
		"%{IncludeDir.fmod_studio}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.tileson}",
		"%{IncludeDir.lua}",
		"%{IncludeDir.sol2}",
	}

	libdirs
	{
		"%{LibDir.assimp}",
		"%{LibDir.qu3e}",
		"%{LibDir.fmod_core}",
		"%{LibDir.fmod_bank}",
		"%{LibDir.fmod_studio}",
	}

	links
	{
		"opengl32.lib",
		"glad",
		"glfw",
		"soil2",
		"ImGui",
		"Lua540",
		"jsoncpp",
		"tileson",
		"%{LibName.fmod_core}",
		"%{LibName.fmod_bank}",
		"%{LibName.fmod_studio}",
	}

	filter "configurations:Debug"
		links
		{
			"%{LibName.assimp_debug}",
			"%{LibName.qu3e_debug}",
		}

	filter "configurations:Release"
		links
		{
			"%{LibName.assimp}",
			"%{LibName.qu3e}",
		}

	
	filter { "files:**.c" }
		compileas "C++"

	filter "system:windows"
		cppdialect "C++latest"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			--"ENGINE_BUILD_DLL",
			"DEBUG_DRAW",
			"WINDOWS_APP",
			"GLFW_INCLUDE_NONE",
			"MULTITHREAD_UPDATE",
		}

		postbuildcommands
		{
			--("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/application/\""),
			("{COPY} $(SolutionDir)%{LibDir.fmod_core}/%{DllName.fmod_core} \"$(SolutionDir)bin/" .. outputdir .. "/application/\""),
			("{COPY} $(SolutionDir)%{LibDir.fmod_bank}/%{DllName.fmod_bank} \"$(SolutionDir)bin/" .. outputdir .. "/application/\""),
			("{COPY} $(SolutionDir)%{LibDir.fmod_bank}/%{DllName.fmod_bank2} \"$(SolutionDir)bin/" .. outputdir .. "/application/\""),
			("{COPY} $(SolutionDir)%{LibDir.fmod_studio}/%{DllName.fmod_studio} \"$(SolutionDir)bin/" .. outputdir .. "/application/\"")
		}

	filter "configurations:Debug"
		postbuildcommands
		{
			("{COPY} $(SolutionDir)%{LibDir.assimp}/%{DllName.assimp_debug} \"$(SolutionDir)bin/" .. outputdir .. "/application/\""),
		}

	filter "configurations:Release"
		postbuildcommands
		{
			("{COPY} $(SolutionDir)%{LibDir.assimp}/%{DllName.assimp} \"$(SolutionDir)bin/" .. outputdir .. "/application/\""),
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
		symbols "Off"
		optimize "Speed"
		flags "LinkTimeOptimization"

project "application"
	location "application"
	kind "WindowedApp"
	language "C++"
	icon "icon.ico"
	targetname ("Long March")
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-intermediate/" .. outputdir .. "/%{prj.name}")

	pchheader "application-precompiled-header.h"
	pchsource "application/source/application-precompiled-header.cpp"

	files
	{
		"%{prj.name}/source/**.h",
		"%{prj.name}/source/**.inl",
		"%{prj.name}/source/**.cpp",
	}

	includedirs
	{
		"application/source",
		"engine/source",
		"engine/vendors",
		"%{IncludeDir.assimp}",
		"%{IncludeDir.qu3e}",
		"%{IncludeDir.blaze}",
		"%{IncludeDir.phmap}",
		"%{IncludeDir.glfw}",
		"%{IncludeDir.glad}",
		"%{IncludeDir.SOIL2}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.jsoncpp}",
		"%{IncludeDir.fmod_core}",
		"%{IncludeDir.fmod_bank}",
		"%{IncludeDir.fmod_studio}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.tileson}",
		"%{IncludeDir.lua}",
		"%{IncludeDir.sol2}",
	}

	links
	{
		"engine"
	}
	
	filter "system:windows"
		cppdialect "C++latest"
		staticruntime "On"
		systemversion "latest"
		files { 'resources.rc', '**.ico' }
		vpaths { ['/*'] = { '*.rc', '**.ico' } }
		
		defines
		{
			"DEBUG_DRAW",
			"WINDOWS_APP",
			"MULTITHREAD_UPDATE",
		}


		postbuildcommands
		{
			("{COPY} $(SolutionDir)application/asset \"$(SolutionDir)bin/" .. outputdir .. "/application/asset\""),
			("{COPY} $(SolutionDir)application/engine-config.json \"$(SolutionDir)bin/" .. outputdir .. "/application/\""),
			("{COPY} $(SolutionDir)application/imgui.ini \"$(SolutionDir)bin/" .. outputdir .. "/application/\"") 
		}

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

--All premake configurations flags could be found here:
--https://github.com/premake/premake-core/blob/master/tests/tools/test_msc.lua