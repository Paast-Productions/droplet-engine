project "Test"

    location(projectsPath)

    kind "ConsoleApp"
    targetdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")

    files 
    {
        "../Test/src/**.hpp",
        "../Test/src/**.cpp"
    }

    includedirs
    {
        "../include", 
        targetBuildPath .. "/External/include"
    }

    libdirs
    {
        targetBuildPath .. "/External/lib",
        targetBuildPath .. "/External/lib64"
    }

    dependson 
    {
        "GoogleTest",
        "Engine"
    }

    links
    {
        "Engine", 
        "gtest"
    }