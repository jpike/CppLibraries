#if 0

@ECHO OFF

REM MAKE SURE THE COMPILER IS CONFIGURED FOR x64.
REM This currently cannot be done via the C++ build system as just using std::system() to execute these batch files
REM does not result in appropriate environment variables being preserved.
IF NOT DEFINED VCINSTALLDIR (
    ECHO "Visual Studio tools not configured...Configuring for x64..."
    CALL "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
    IF %ERRORLEVEL% NEQ 0 CALL "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x64
    IF %ERRORLEVEL% NEQ 0 CALL "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
)

IF "%VSCMD_ARG_TGT_ARCH%"=="x86" (
    ECHO "Incorrect Visual Studio target architecture...Reconfiguring for x64..."
    CALL "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
    IF %ERRORLEVEL% NEQ 0 CALL "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x64
    IF %ERRORLEVEL% NEQ 0 CALL "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
)

ECHO "Double-checking for compiler..."
WHERE cl.exe
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

    // DEFINE A PROJECT FOR BUILDING ALL C++ LIBRARIES AS A SINGLE LIBRARY.
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
        }
    };
    build.Add(combined_cpp_libraries);

    // BUILD DEBUG VERSIONS OF THE PROJECT.
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
