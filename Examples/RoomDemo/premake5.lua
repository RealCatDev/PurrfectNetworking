project "RoomDemo - Server"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++20"
        
        targetdir ("$(SolutionDir)/bin/%{cfg.system}-$(Configuration)/$(TargetDir)")
        objdir ("$(SolutionDir)/bin-int/%{cfg.system}-$(Configuration)/$(TargetDir)")

        files { 
            "src/server.cpp"
        }

        includedirs {
            "$(SolutionDir)Core/include/",
        }

        libdirs {}

        links {
        }

        filter "system:windows"
            defines { "PURR_WINDOWS" }
            systemversion "latest"

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

project "RoomDemo - Client"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    
    targetdir ("$(SolutionDir)/bin/%{cfg.system}-$(Configuration)/$(TargetDir)")
    objdir ("$(SolutionDir)/bin-int/%{cfg.system}-$(Configuration)/$(TargetDir)")

    files { 
        "src/client.cpp"
    }

    includedirs {
        "$(SolutionDir)Core/include/",
    }

    libdirs {}

    links {
    }

    filter "system:windows"
        defines { "PURR_WINDOWS" }
        systemversion "latest"

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