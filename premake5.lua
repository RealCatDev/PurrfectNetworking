workspace "PurrfectNetworking"
    configurations { "Debug", "Release", "Dist" }
    architecture "x86_64"
	startproject "Test"

    flags { "MultiProcessorCompile" }

group "Core"
    include "Core"
group ""

include "Examples"