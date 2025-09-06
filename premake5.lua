dependencies = {
    basePath = "./deps"
}

function dependencies.load()
    dir = path.join(dependencies.basePath, "premake/*.lua")
    deps = os.matchfiles(dir)

    for i, dep in pairs(deps) do
        dep = dep:gsub(".lua", "")
        require(dep)
    end
end

function dependencies.imports()
    for i, proj in pairs(dependencies) do
        if type(i) == 'number' then
            proj.import()
        end
    end
end

function dependencies.projects()
    for i, proj in pairs(dependencies) do
        if type(i) == 'number' then
            proj.project()
        end
    end
end

include "deps/minhook.lua"
include "deps/rapidjson.lua"
include "build/conandeps.premake5.lua"

workspace "ImasSCSP-localify"
	conan_setup()

    location "./build"
    objdir "%{wks.location}/obj"
    targetdir "%{wks.location}/bin/%{cfg.platform}/%{cfg.buildcfg}"

    architecture "x64"
    platforms "x64"

    configurations {
        "Debug",
        "Release",
    }

    buildoptions {
        "/std:c++latest",
        "/utf-8",
    }
    systemversion "latest"
    symbols "On"
    staticruntime "On"
    editandcontinue "Off"
    warnings "Off"
    characterset "ASCII"

    flags {
        "NoIncrementalLink",
        "NoMinimalRebuild",
        "MultiProcessorCompile",
    }

    staticruntime "Off"

    filter "configurations:Release"
        optimize "Full"
        buildoptions "/Os"

    filter "configurations:Debug"
        optimize "Debug"

    dependencies.projects()

project "ImasSCSP-localify"
    targetname "version"

    language "C++"
    kind "SharedLib"

    files {
        "./src/**.hpp",
        "./src/**.cpp",
        "./src/**.asm",
        "./src/**.def",
    }

    includedirs {
        "./src",
        "%{prj.location}/src",
    }

    dependencies.imports()

    linkoptions { conan_sharedlinkflags }

    filter "configurations:Release"
        linkoptions "/SAFESEH:NO"
        syslibdirs {
            "./libs/Release",
        }

    filter "configurations:Debug"
        linkoptions "/SAFESEH:NO"
        syslibdirs {
            "./libs/Debug",
        }
