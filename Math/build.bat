@ECHO OFF

REM PUT THE COMPILER IN THE PATH IF IT ISN'T ALREADY.
WHERE cl.exe
IF %ERRORLEVEL% NEQ 0 CALL "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
WHERE cl.exe

REM READ THE BUILD MODE COMMAND LINE ARGUMENT.
REM Either "debug" or "release" (no quotes).
REM If not specified, will default to debug.
IF NOT "%1" == "" (
    SET build_mode=%1
) ELSE (
    SET build_mode=debug
)

REM DEFINE COMPILER OPTIONS.
SET COMMON_COMPILER_OPTIONS=/EHsc /WX /W4 /TP /std:c++latest
SET DEBUG_COMPILER_OPTIONS=%COMMON_COMPILER_OPTIONS% /Z7 /Od /MTd
SET RELEASE_COMPILER_OPTIONS=%COMMON_COMPILER_OPTIONS% /O2 /MT

REM DEFINE FILES TO COMPILE/LINK.
SET COMPILATION_FILE="..\..\Math.project"
SET MAIN_CODE_DIR="..\..\code"

REM CREATE THE COMMAND LINE OPTIONS FOR THE FILES TO COMPILE/LINK.
SET INCLUDE_DIRS=/I %MAIN_CODE_DIR%
SET PROJECT_FILES_DIRS_AND_LIBS=%COMPILATION_FILE% %INCLUDE_DIRS%

REM MOVE INTO THE BUILD DIRECTORY.
SET build_directory=build\%build_mode%
IF NOT EXIST "%build_directory%" MKDIR "%build_directory%"
PUSHD "%build_directory%"

    REM BUILD THE PROGRAM BASED ON THE BUILD MODE.
    IF "%build_mode%"=="release" (
        cl.exe %RELEASE_COMPILER_OPTIONS% %PROJECT_FILES_DIRS_AND_LIBS%
    ) ELSE (
        cl.exe %DEBUG_COMPILER_OPTIONS% %PROJECT_FILES_DIRS_AND_LIBS%
    )
    
    lib.exe "Math.obj"

POPD

ECHO Done building Math.lib.

@ECHO ON
