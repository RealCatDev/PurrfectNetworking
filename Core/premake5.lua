project "Core"
        kind "None"
        language "C++"
        cppdialect "C++20"
        
        targetdir ("$(SolutionDir)/bin/%{cfg.system}-$(Configuration)/$(TargetDir)")
        objdir ("$(SolutionDir)/bin-int/%{cfg.system}-$(Configuration)/$(TargetDir)")

        files { 
            "include/**.hpp", 
        }

        includedirs {
            "include/",
        }

        libdirs {}

        links {}

        filter "system:windows"
            systemversion "latest"
            defines { "PURR_WINDOWS" }
            links { "ws2_32.lib" }

        filter "configurations:Debug"
            defines { "PURR_DEBUG" }
            symbols "On"
            runtime "Debug"

        filter "configurations:Release"
            defines { "PURR_RELEASE" }
            runtime "Release"
            optimize "On"
            symbols "On"

        filter "configurations:Dist"
            defines { "PURR_DIST" }
            runtime "Release"
            optimize "On"
            symbols "Off"