#if 0

@ECHO OFF

REM MAKE SURE THE COMPILER IS CONFIGURED FOR x64.
REM This currently cannot be done via the C++ build system as just using std::system() to execute these batch files
REM does not result in appropriate environment variables being preserved.
IF NOT DEFINED VCINSTALLDIR (
    ECHO "Visual Studio tools not configured...Configuring for x64..."
    CALL "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
    REM TODO: Figure out why Visual Studio 2022 is returning 9009 instead of 0 here.
    IF %ERRORLEVEL% NEQ 9009 CALL "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
    IF %ERRORLEVEL% NEQ 9009 CALL "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x64
    IF %ERRORLEVEL% NEQ 9009 CALL "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
)

IF "%VSCMD_ARG_TGT_ARCH%"=="x86" (
    ECHO "Incorrect Visual Studio target architecture...Reconfiguring for x64..."
    CALL "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
    REM TODO: Figure out why Visual Studio 2022 is returning 9009 instead of 0 here.
    IF %ERRORLEVEL% NEQ 9009 CALL "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
    IF %ERRORLEVEL% NEQ 9009 CALL "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x64
    IF %ERRORLEVEL% NEQ 9009 CALL "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
)

ECHO "Double-checking for compiler..."
WHERE cl.exe
IF %ERRORLEVEL% NEQ 0 CALL "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
IF %ERRORLEVEL% NEQ 0 CALL "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
IF %ERRORLEVEL% NEQ 0 CALL "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x64
IF %ERRORLEVEL% NEQ 0 CALL "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
WHERE cl.exe

REM BUILD THE C++ BUILD PROGRAM.
REM For exception handling flag - https://docs.microsoft.com/en-us/cpp/build/reference/eh-exception-handling-model?view=msvc-160
REM /TP is needed to have this batch file treated as a .cpp file - https://docs.microsoft.com/en-us/cpp/build/reference/tc-tp-tc-tp-specify-source-file-type?view=msvc-160
cl.exe /std:c++latest /EHsc /TP build.bat

REM BUILD THE PROJECT
build.exe

@ECHO ON
EXIT /B

#endif

#include <cstdlib>
#include <filesystem>
#include "BuildSystem/CppBuild.cpp"

int main()
{
    // DEFINE THE PATH TO THE WORKSPACE.
    // The path is converted to an absolute path to ensure it remains correct in all commands.
    std::filesystem::path workspace_folder_path = std::filesystem::absolute(".");

    // DEFINE THE BUILD TO ADD PROJECTS TO.
    Build build;

    // DEFINE THIRD-PARTY PROJECTS FOR THE BUILD.
    Project windows_api = 
    {
        .Type = ProjectType::LIBRARY,
        .Name = "WindowsApi",
        .LinkerLibraryNames = { "user32.lib", "gdi32.lib" }
    };
    build.Add(&windows_api);

    Project direct_x = 
    {
        .Type = ProjectType::LIBRARY,
        .Name = "DirectX",
        .LinkerLibraryNames = { "d3d11.lib", "dxgi.lib", "d3dcompiler.lib" }
    };
    build.Add(&direct_x);

    Project open_gl = 
    {
        .Type = ProjectType::LIBRARY,
        .Name = "OpenGL",
        .LinkerLibraryNames = { "opengl32.lib" }
    };
    build.Add(&open_gl);

    Project gl3w_library = 
    {
        .Type = ProjectType::LIBRARY,
        .Name = "gl3w",
        .CodeFolderPath = workspace_folder_path / "ThirdParty/gl3w",
        .UnityBuildFilepath = workspace_folder_path / "ThirdParty/gl3w/gl3w.project",
        .LinkerLibraryNames = { "gl3w.lib" },
    };
    build.Add(&gl3w_library);

    Project sdl_library = 
    {
        .Type = ProjectType::LIBRARY,
        .Name = "SDL",
        .CodeFolderPath = workspace_folder_path / "ThirdParty/SDL",
        .AdditionalIncludeFolderPaths = 
        {
            workspace_folder_path / "ThirdParty",
        },
        .LinkerLibraryNames = { "SDL2.lib", "SDL2main.lib" },
    };
    build.Add(&sdl_library);

    Project stb_library = 
    {
        .Type = ProjectType::LIBRARY,
        .Name = "stb",
        .CodeFolderPath = workspace_folder_path / "ThirdParty/stb",
        .UnityBuildFilepath = workspace_folder_path / "ThirdParty/stb/stb.project",
        .LinkerLibraryNames = { "stb.lib" },
    };
    build.Add(&stb_library);

    Project stemmer_library = 
    {
        .Type = ProjectType::LIBRARY,
        .Name = "stmr",
        .CodeFolderPath = workspace_folder_path / "ThirdParty/stmr",
        .UnityBuildFilepath = workspace_folder_path / "ThirdParty/stmr/stmr.project",
        .LinkerLibraryNames = { "stmr.lib" },
    };
    build.Add(&stemmer_library);

    Project catch_library = 
    {
        .Type = ProjectType::LIBRARY,
        .Name = "Catch",
        .CodeFolderPath = workspace_folder_path / "ThirdParty/Catch",
    };
    build.Add(&catch_library);

    Project gsl_library = 
    {
        .Type = ProjectType::LIBRARY,
        .Name = "GSL",
        .CodeFolderPath = workspace_folder_path / "ThirdParty/GSL/include",
    };
    build.Add(&gsl_library);

    Project pugixml_library = 
    {
        .Type = ProjectType::LIBRARY,
        .Name = "pugixml",
        .CodeFolderPath = workspace_folder_path / "ThirdParty/pugixml",
        .UnityBuildFilepath = workspace_folder_path / "ThirdParty/pugixml/pugixml.cpp",
    };
    build.Add(&pugixml_library);

    Project imgui_library = 
    {
        .Type = ProjectType::LIBRARY,
        .Name = "imgui",
        .CodeFolderPath = workspace_folder_path / "ThirdParty/imgui",
        .UnityBuildFilepath = workspace_folder_path / "ThirdParty/imgui/ImGui.project",
    };
    build.Add(&imgui_library);

    // DEFINE CUSTOM PROJECTS FOR THE BUILD.
    Project containers_library = 
    {
        .Type = ProjectType::LIBRARY,
        .Name = "Containers",
        .CodeFolderPath = workspace_folder_path / "Containers",
    };
    build.Add(&containers_library);

    Project containers_tests = 
    {
        .Type = ProjectType::PROGRAM,
        .Name = "ContainersTests",
        .CodeFolderPath = workspace_folder_path / "Containers/testing",
        .UnityBuildFilepath = workspace_folder_path / "Containers/testing/ContainersTests.cpp",
        .Libraries = 
        { 
            &catch_library,
            &containers_library 
        },
    };
    build.Add(&containers_tests);

    Project debugging_library = 
    {
        .Type = ProjectType::LIBRARY,
        .Name = "Debugging",
        .CodeFolderPath = workspace_folder_path / "Debugging",
    };
    build.Add(&debugging_library);

    Project error_handling_library = 
    {
        .Type = ProjectType::LIBRARY,
        .Name = "ErrorHandling",
        .CodeFolderPath = workspace_folder_path / "ErrorHandling",
        .UnityBuildFilepath = workspace_folder_path / "ErrorHandling/ErrorHandling.project",
        .LinkerLibraryNames = { "ErrorHandling.lib" },
    };
    build.Add(&error_handling_library);

    Project error_handling_tests = 
    {
        .Type = ProjectType::PROGRAM,
        .Name = "ErrorHandlingTests",
        .CodeFolderPath = workspace_folder_path / "ErrorHandling/testing",
        .UnityBuildFilepath = workspace_folder_path / "ErrorHandling/testing/ErrorHandlingTests.cpp",
        .Libraries = 
        { 
            &catch_library,
            &error_handling_library 
        },
    };
    build.Add(&error_handling_tests);

    Project filesystem_library = 
    {
        .Type = ProjectType::LIBRARY,
        .Name = "Filesystem",
        .CodeFolderPath = workspace_folder_path / "Filesystem",
        .UnityBuildFilepath = workspace_folder_path / "Filesystem/Filesystem.project",
        .LinkerLibraryNames = { "Filesystem.lib" },
    };
    build.Add(&filesystem_library);

    Project filesystem_tests = 
    {
        .Type = ProjectType::PROGRAM,
        .Name = "FilesystemTests",
        .CodeFolderPath = workspace_folder_path / "Filesystem/testing",
        .UnityBuildFilepath = workspace_folder_path / "Filesystem/testing/FilesystemTests.cpp",
        .Libraries = 
        { 
            &catch_library,
            &filesystem_library 
        },
    };
    build.Add(&filesystem_tests);

    Project input_control_library = 
    {
        .Type = ProjectType::LIBRARY,
        .Name = "InputControl",
        .CodeFolderPath = workspace_folder_path / "InputControl",
    };
    build.Add(&input_control_library);

    Project math_library = 
    {
        .Type = ProjectType::LIBRARY,
        .Name = "Math",
        .CodeFolderPath = workspace_folder_path / "Math",
        .UnityBuildFilepath = workspace_folder_path / "Math/Math.project",
        .LinkerLibraryNames = { "Math.lib" },
    };
    build.Add(&math_library);

    Project math_tests = 
    {
        .Type = ProjectType::PROGRAM,
        .Name = "MathTests",
        .CodeFolderPath = workspace_folder_path / "Math/testing",
        .UnityBuildFilepath = workspace_folder_path / "Math/testing/MathTests.cpp",
        .Libraries = 
        { 
            &catch_library,
            &math_library 
        },
    };
    build.Add(&math_tests);

    Project memory_library = 
    {
        .Type = ProjectType::LIBRARY,
        .Name = "Memory",
        .CodeFolderPath = workspace_folder_path / "Memory",
        .Libraries = 
        {
            &gsl_library,
        },
    };
    build.Add(&memory_library);

    Project string_library = 
    {
        .Type = ProjectType::LIBRARY,
        .Name = "String",
        .CodeFolderPath = workspace_folder_path / "String",
        .UnityBuildFilepath = workspace_folder_path / "String/String.project",
        .LinkerLibraryNames = { "String.lib" },
    };
    build.Add(&string_library);

    Project string_tests = 
    {
        .Type = ProjectType::PROGRAM,
        .Name = "StringTests",
        .CodeFolderPath = workspace_folder_path / "String/testing",
        .UnityBuildFilepath = workspace_folder_path / "String/testing/StringTests.cpp",
        .Libraries = 
        { 
            &catch_library,
            &string_library 
        },
    };
    build.Add(&string_tests);

    Project windowing_library = 
    {
        .Type = ProjectType::LIBRARY,
        .Name = "Windowing",
        .CodeFolderPath = workspace_folder_path / "Windowing",
        .UnityBuildFilepath = workspace_folder_path / "Windowing/Windowing.project",
        .Libraries =
        {
            &sdl_library
        },
        .LinkerLibraryNames = { "Windowing.lib" },
    };
    build.Add(&windowing_library);

    Project graphics_library = 
    {
        .Type = ProjectType::LIBRARY,
        .Name = "Graphics",
        .CodeFolderPath = workspace_folder_path / "Graphics",
        .UnityBuildFilepath = workspace_folder_path / "Graphics/Graphics.project",
        .Libraries = 
        { 
            &windows_api,
            &direct_x,
            &open_gl,
            &gl3w_library, 
            &sdl_library, 
            &stb_library,
            &math_library,
            &memory_library,
            &filesystem_library,
            &string_library,
            &windowing_library,
        },
        .LinkerLibraryNames = { "Graphics.lib" },
    };
    build.Add(&graphics_library);

    Project graphics_tests = 
    {
        .Type = ProjectType::PROGRAM,
        .Name = "GraphicsTests",
        .CodeFolderPath = workspace_folder_path / "Graphics/testing",
        .UnityBuildFilepath = workspace_folder_path / "Graphics/testing/GraphicsTests.cpp",
        .Libraries = 
        { 
            &catch_library, 
            &graphics_library 
        }
    };
    build.Add(&graphics_tests);

    Project combined_cpp_libraries = 
    {
        .Type = ProjectType::LIBRARY,
        .Name = "CppLibraries",
        .CodeFolderPath = workspace_folder_path,
        .UnityBuildFilepath = workspace_folder_path / "CppLibraries.project",
        .AdditionalIncludeFolderPaths = 
        {
            workspace_folder_path / "ThirdParty",
            workspace_folder_path / "ThirdParty/gl3w",
            workspace_folder_path / "ThirdParty/GSL/include",
        },
        .LinkerLibraryNames = { "CppLibraries.lib" },
    };
    build.Add(&combined_cpp_libraries);

    Project combined_cpp_library_tests = 
    {
        .Type = ProjectType::PROGRAM,
        .Name = "CppLibraryTests",
        .CodeFolderPath = workspace_folder_path,
        .UnityBuildFilepath = workspace_folder_path / "CppLibraryTests.project",
        .Libraries = 
        { 
            &windows_api,
            &direct_x,
            &open_gl,
            &gl3w_library,
            &sdl_library,
            &stb_library,
            &catch_library,
            &combined_cpp_libraries 
        }
    };
    build.Add(&combined_cpp_library_tests);

    try
    {
        // BUILD DEBUG VERSIONS OF THE PROJECTS.
        int debug_build_exit_code = build.Run(workspace_folder_path, "debug");
        bool debug_build_succeeded = (EXIT_SUCCESS == debug_build_exit_code);
        if (!debug_build_succeeded)
        {
            return debug_build_exit_code;
        }

        // BUILD RELEASE VERSIONS OF THE PROJECT.
        int release_build_exit_code = build.Run(workspace_folder_path, "release");
        return release_build_exit_code;
    }
    catch (const std::exception& exception)
    {
        std::cout << "Standard exception: " << exception.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cout << "Unknown exception." << std::endl;
        return EXIT_FAILURE;
    }
}
