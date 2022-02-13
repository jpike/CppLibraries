@ECHO OFF

REM TODO - Incorporate SDL2.dll copying into build system.
COPY /Y ThirdParty\SDL\SDL2.dll build\debug\SDL2.dll
COPY /Y ThirdParty\SDL\SDL2.dll build\release\SDL2.dll

REM RUN DEBUG VERSIONS OF TESTERS.
PUSHD "build/debug"
    @ECHO ON
    ContainersTests.exe
    ErrorHandlingTests.exe
    FilesystemTests.exe
    GraphicsTests.exe
    MathTests.exe
    StringTests.exe
    CppLibraryTests.exe
    @ECHO OFF
POPD

REM RUN RELEASE VERSIONS OF TESTERS.
PUSHD "build/release"
    @ECHO ON
    ContainersTests.exe
    ErrorHandlingTests.exe
    FilesystemTests.exe
    GraphicsTests.exe
    MathTests.exe
    StringTests.exe
    CppLibraryTests.exe
    @ECHO OFF
POPD

ECHO Done building/running tests.

@ECHO ON
