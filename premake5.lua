workspace "AthiVegam"
	startproject "ParuguEditor"
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
	}

	sysincludedirs
	{
		"%{prj.name}/include/AthiVegam",
		"%{externals.sdl2}/include"
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
		
	filter {"system:linux", "configurations:*"}
		defines
		{
			"AV_PLATFORM_LINUX" 
		}
	
	filter "configurations:Debug"
		defines
		{
			"AV_CONFIG_DEBUG"
		}
		runtime "Debug"
		symbols "on"
		
	filter "configurations:Release"
		defines
		{
			"AV_CONFIG_RELEASE"
		}
		runtime "Release"
		symbols "off"
		optimize "on"

		
project "ParuguEditor"
	location "ParuguEditor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"
	links "AthiVegam"

	targetdir(tdir)
	objdir(odir)

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	sysincludedirs
	{
		"AthiVegam/include"
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

		libdirs
		{
			"%{externals.sdl2}/lib"
		}

		links
		{
			"SDL2"
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
			"SDL2.framework"
		}
		
	filter {"system:linux", "configurations:*"}
		defines
		{
			"AV_PLATFORM_LINUX" 
		}

		links
		{
			"SDL2"
		}
	
	filter "configurations:Debug"
		defines
		{
			"AV_CONFIG_DEBUG"
		}
		runtime "Debug"
		symbols "on"
		
	filter "configurations:Release"
		defines
		{
			"AV_CONFIG_RELEASE"
		}
		runtime "Release"
		symbols "off"
		optimize "on"