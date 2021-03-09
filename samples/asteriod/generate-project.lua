--All premake configurations flags could be found here:
--https://github.com/premake/premake-core/blob/master/tests/tools/

function preferred_path(path)
    return path:gsub("\\", "/")
end

function str_split (inputstr, sep)
        if sep == nil then
                sep = "%s"
        end
        local t={}
        for str in string.gmatch(inputstr, "([^"..sep.."]+)") do
                table.insert(t, str)
        end
        return t
end

--http://lua-users.org/wiki/SimpleStack
-- Stack Table
-- Uses a table as stack, use <table>:push(value) and <table>:pop()
-- Lua 5.1 compatible
-- GLOBAL
Stack = {}
-- Create a Table with stack functions
function Stack:Create()

  -- stack table
  local t = {}
  -- entry table
  t._et = {}

  -- push a value on to the stack
  function t:push(...)
    if ... then
      local targs = {...}
      -- add values
      for _,v in ipairs(targs) do
        table.insert(self._et, v)
      end
    end
  end

  -- pop a value from the stack
  function t:pop(num)

    -- get num values from stack
    local num = num or 1

    -- return table
    local entries = {}

    -- get values into entries
    for i = 1, num do
      -- get last entry
      if #self._et ~= 0 then
        table.insert(entries, self._et[#self._et])
        -- remove last value
        table.remove(self._et)
      else
        break
      end
    end
    -- return unpacked entries
    return (entries)
  end

  -- get size
  function t:size()
    return #self._et
  end

  -- get entries
  function t:get()
    return self._et
  end

  -- print values
  function t:print()
    for i,v in pairs(self._et) do
      print(i, v)
    end
  end
  return t
end

function compact_path(path)
	local ret = ""
	local tokens = str_split(path, "/")
	local stack = Stack:Create()

	for i = 1, #tokens do
		--print(tokens[i])
    	if (tokens[i] ~= "..") then
    		stack:push(tokens[i])
    	else
    		stack:pop()
    	end
    end

	local stack_table = stack:get()
    for i = 1, #stack_table do
    	if (i == 1) then
    		ret = stack_table[i]
    	else
    		ret = ret .. "/" .. stack_table[i]
    	end
    end
    --print(ret)
    return ret
end
workspace "Asteriod"
	architecture "x86_64"
	startproject "application"

	configurations {
		"Debug",
		"Release"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
cwd = preferred_path(os.getenv("PWD") or io.popen("cd"):read()); --https://www.programming-idioms.org/idiom/106/get-program-working-directory/3804/lua

enginedir = compact_path(cwd .. "/../../engine")
EngineDir = {}
EngineDir["root"]     			= (enginedir)
EngineDir["external"]     		= (enginedir .. "/external")
EngineDir["vendors"]     		= (enginedir .. "/vendors")
EngineDir["source"]     		= (enginedir .. "/source")

applicationdir = compact_path(cwd .. "/application")
ApplicationDir = {}
ApplicationDir["root"]     		= (applicationdir)
ApplicationDir["source"]     	= (applicationdir .. "/source")

IncludeDir = {}
IncludeDir["glfw"]     		= "%{EngineDir.external}/glfw/include"
IncludeDir["ImGui"]      	= "%{EngineDir.external}/imgui"
IncludeDir["lua"]      		= "%{EngineDir.external}/lua540/src"
IncludeDir["glad"]     		= "%{EngineDir.external}/glad/include"
IncludeDir["tileson"]      	= "%{EngineDir.external}/tileson/include"
IncludeDir["SOIL2"]      	= "%{EngineDir.external}/SOIL2"
IncludeDir["jsoncpp"]  		= "%{EngineDir.external}/jsoncpp/include"

IncludeDir["assimp"]     	= "%{EngineDir.vendors}/assimp/include"
IncludeDir["glm"]      		= "%{EngineDir.vendors}/glm"
IncludeDir["fmod_core"]  	= "%{EngineDir.vendors}/fmod/api/core/inc"
IncludeDir["fmod_bank"]  	= "%{EngineDir.vendors}/fmod/api/fsbank/inc"
IncludeDir["fmod_studio"]  	= "%{EngineDir.vendors}/fmod/api/studio/inc"
IncludeDir["spdlog"] 		= "%{EngineDir.vendors}/spdlog/include"
IncludeDir["phmap"]     	= "%{EngineDir.vendors}/phmap/include"
IncludeDir["blaze"]     	= "%{EngineDir.vendors}/blaze/include"
IncludeDir["sol2"]     		= "%{EngineDir.vendors}/sol2/include"
IncludeDir["FastBVH"]     	= "%{EngineDir.vendors}/Fast-BVH/include"
IncludeDir["miniz_cpp"]     = "%{EngineDir.vendors}/miniz-cpp"

LibDir = {}
LibDir["assimp"] 			= "%{EngineDir.vendors}/assimp/lib/x64"
LibDir["fmod_core"] 		= "%{EngineDir.vendors}/fmod/api/core/lib/x64"
LibDir["fmod_bank"] 		= "%{EngineDir.vendors}/fmod/api/fsbank/lib/x64"
LibDir["fmod_studio"] 		= "%{EngineDir.vendors}/fmod/api/studio/lib/x64"

LibName = {}
LibName["fmod_core"] 		= "fmod_vc.lib"
LibName["fmod_bank"] 		= "fsbank_vc.lib"
LibName["fmod_studio"] 		= "fmodstudio_vc.lib"
LibName["assimp_debug"] 	= "/Release/assimp-vc142-mt.lib"
LibName["assimp"] 			= "/Release/assimp-vc142-mt.lib"

DllName = {}
DllName["assimp_debug"] 	= "/Release/assimp-vc142-mt.dll"
DllName["assimp"] 			= "/Release/assimp-vc142-mt.dll"
DllName["fmod_core"] 		= "fmod.dll"
DllName["fmod_bank"] 		= "fsbank.dll"
DllName["fmod_bank2"] 		= "libfsbvorbis64.dll"
DllName["fmod_studio"] 		= "fmodstudio.dll"

-- Build non-header-only lib
include (enginedir .. "/external/glad")
include (enginedir .. "/external/glfw")
include (enginedir .. "/external/SOIL2")
include (enginedir .. "/external/imgui/imgui")
include (enginedir .. "/external/lua540/src")
include (enginedir .. "/external/jsoncpp")
include (enginedir .. "/external/tileson")

project "engine"
	location (enginedir)
	kind "StaticLib"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-intermediate/" .. outputdir .. "/%{prj.name}")
	
	pchheader "engine-precompiled-header.h"
	pchsource "%{EngineDir.source}/engine-precompiled-header.cpp"

	files
	{
		"%{EngineDir.source}/**.h",
		"%{EngineDir.source}/**.inl",
		"%{EngineDir.source}/**.cpp",

		"%{EngineDir.vendors}/glm/glm/**.hpp",
		"%{EngineDir.vendors}/glm/glm/**.inl",
	}

	includedirs
	{
		"%{EngineDir.source}",
		"%{IncludeDir.assimp}",
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
		"%{IncludeDir.FastBVH}",
		"%{IncludeDir.miniz_cpp}",
	}

	libdirs
	{
		"%{LibDir.assimp}",
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
		}

	filter "configurations:Release"
		links
		{
			"%{LibName.assimp}",
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
			("{COPY} %{EngineDir.root}/asset \"$(SolutionDir)bin/" .. outputdir .. "/application/asset\""),
			("{COPY} %{LibDir.fmod_core}/%{DllName.fmod_core} \"$(SolutionDir)bin/" .. outputdir .. "/application/\""),
			("{COPY} %{LibDir.fmod_bank}/%{DllName.fmod_bank} \"$(SolutionDir)bin/" .. outputdir .. "/application/\""),
			("{COPY} %{LibDir.fmod_bank}/%{DllName.fmod_bank2} \"$(SolutionDir)bin/" .. outputdir .. "/application/\""),
			("{COPY} %{LibDir.fmod_studio}/%{DllName.fmod_studio} \"$(SolutionDir)bin/" .. outputdir .. "/application/\"")
		}

	filter "configurations:Debug"
		postbuildcommands
		{
			("{COPY} %{LibDir.assimp}/%{DllName.assimp_debug} \"$(SolutionDir)bin/" .. outputdir .. "/application/\""),
		}

	filter "configurations:Release"
		postbuildcommands
		{
			("{COPY} %{LibDir.assimp}/%{DllName.assimp} \"$(SolutionDir)bin/" .. outputdir .. "/application/\""),
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
	location (applicationdir)
	kind "WindowedApp"
	language "C++"
	icon "icon.ico"
	targetname ("Asteriod")
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-intermediate/" .. outputdir .. "/%{prj.name}")
	debugdir ("bin/" .. outputdir .. "/%{prj.name}")

	pchheader "application-precompiled-header.h"
	pchsource "%{ApplicationDir.source}/application-precompiled-header.cpp"

	files
	{
		"%{ApplicationDir.source}/**.h",
		"%{ApplicationDir.source}/**.inl",
		"%{ApplicationDir.source}/**.cpp",
	}

	includedirs
	{
		"%{ApplicationDir.source}",
		"%{EngineDir.source}",
		"%{EngineDir.vendors}",
		"%{IncludeDir.assimp}",
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
		"%{IncludeDir.FastBVH}",
		"%{IncludeDir.miniz_cpp}",
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
			("{COPY} %{ApplicationDir.root}/asset \"$(SolutionDir)bin/" .. outputdir .. "/application/asset\""),
			("{COPY} %{ApplicationDir.root}/engine-config.json \"$(SolutionDir)bin/" .. outputdir .. "/application/\""),
			("{COPY} %{ApplicationDir.root}/imgui.ini \"$(SolutionDir)bin/" .. outputdir .. "/application/\"") 
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