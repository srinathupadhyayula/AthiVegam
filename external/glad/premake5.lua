project "glad"
	kind "StaticLib"
	language "C"
	staticruntime "on"

	targetdir(tdir)
	objdir(odir)

	files
	{
		"include/**.h",
		"src/**.c"
	}

	sysincludedirs
	{
		"include"
	}

	
	filter {"system:windows"}
		systemversion "latest"
		
		
	filter {"system:macosx"}
		xcodebuildsettings
		{
			["MACOSX_DEPLOYMENT_TARGET"] = "10.15",
			["UseModernBuildSystem"] = "NO"
		}
		

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
		buildoptions "/MTd"
		
	filter "configurations:Release"
		runtime "Release"
		symbols "off"
		optimize "on"
		buildoptions "/MT"