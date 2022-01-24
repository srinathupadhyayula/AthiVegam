workspace "AthiVegam"
	startproject "Parugu"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release"
	}

tdir = "Build/bin/%{cfg.buildcfg}/%{prj.name}"
odir = "Build/bin_obj/%{cfg.buildcfg}/%{prj.name}"

-- External Dependencies
externals = {}
externals["sdl2"] = "external/sdl2"
externals["spdlog"] = "external/spdlog"
externals["glad"] = "external/glad"
externals["imgui"] = "external/imgui"

-- Process Glad first --
include "external/glad"
include "external/imgui"

project "AthiVegam"
	location "AthiVegam"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir(tdir)
	objdir(odir)

	files
	{
		"%{prj.name}/include/**.h",
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/**.natvis"
	}

	sysincludedirs
	{
		"%{prj.name}/include",
		"%{externals.sdl2}/include",
		"%{externals.spdlog}/include",
		"%{externals.glad}/include",
		"%{externals.imgui}/"
	}

	libdirs
	{
		"%{externals.sdl2}/lib"
	}

	flags
	{
		"FatalWarnings"
	}

	disablewarnings 
	{
		"4005"
	}

	defines
	{
		"GLFW_INCLUDE_NONE" --Make sure Glad does now include GLFW as we are using SDL2--
	}
	
	filter {"system:windows", "configurations:*"}
		systemversion "latest"
		
		defines
		{
			"AV_PLATFORM_WINDOWS"
		}
		
		links
		{
			"SDL2",
			"glad",
			"imgui"
		}
		
	filter {"system:macosx", "configurations:*"}
		xcodebuildsettings
		{
			["MACOSX_DEPLOYMENT_TARGET"] = "10.15",
			["UseModernBuildSystem"] = "NO"
		}
		
		defines
		{
			"AV_PLATFORM_MAC"
		}
		
		links
		{
			"SDL2.framework",
			"glad",
			"imgui"
		}
		
	filter {"system:linux", "configurations:*"}
		defines
		{
			"AV_PLATFORM_LINUX" 
		}
		
		links
		{
			"SDL2",
			"glad",
			"dl",
			"imgui"
		}
	
	filter "configurations:Debug"
		defines
		{
			"AV_CONFIG_DEBUG"
		}
		runtime "Debug"
		symbols "on"
		buildoptions "/MTd"
		
	filter "configurations:Release"
		defines
		{
			"AV_CONFIG_RELEASE"
		}
		runtime "Release"
		symbols "off"
		optimize "on"
		buildoptions "/MT"

		
project "Parugu"
	location "Parugu"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"
	links "AthiVegam"

	targetdir(tdir)
	objdir(odir)

	files
	{
		"%{prj.name}/include/**.h",
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	sysincludedirs
	{
		"AthiVegam/include",
		"%{prj.name}/include",
		"%{externals.spdlog}/include",
	}

	flags
	{
		"FatalWarnings"
	}

	filter {"system:windows", "configurations:*"}
		systemversion "latest"
		
		defines
		{
			"AV_PLATFORM_WINDOWS"
		}

		--libdirs
		--{
		--	"%{externals.sdl2}/lib"
		--}

		--links
		--{
		--	"SDL2",
		--	"glad",
		--	"imgui"
		--}
		
	filter {"system:macosx", "configurations:*"}
		xcodebuildsettings
		{
			["MACOSX_DEPLOYMENT_TARGET"] = "10.15",
			["UseModernBuildSystem"] = "NO"
		}
		
		defines
		{
			"AV_PLATFORM_MAC"
		}

		--links
		--{
		--	"SDL2.framework",
		--	"glad",
		--	"imgui"
		--}
		
	filter {"system:linux", "configurations:*"}
		defines
		{
			"AV_PLATFORM_LINUX" 
		}

		--links
		--{
		--	"SDL2",
		--	"glad",
		--	"dl",
		--	"imgui"
		--}
	
	filter "configurations:Debug"
		defines
		{
			"AV_CONFIG_DEBUG"
		}
		runtime "Debug"
		symbols "on"
		buildoptions "/MTd"
		
	filter "configurations:Release"
		defines
		{
			"AV_CONFIG_RELEASE"
		}
		runtime "Release"
		symbols "off"
		optimize "on"
		buildoptions "/MT"