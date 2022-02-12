@ECHO OFF

REM TODO - Incorporate SDL2.dll copying into build system.
COPY /Y ThirdParty\SDL\SDL2.dll build\debug\SDL2.dll
COPY /Y ThirdParty\SDL\SDL2.dll build\release\SDL2.dll

REM RUN DEBUG VERSIONS OF TESTERS.
PUSHD "build/debug"
    ContainersTests.exe
    ErrorHandlingTests.exe
    GraphicsTests.exe
    CppLibraryTests.exe
POPD

REM RUN RELEASE VERSIONS OF TESTERS.
PUSHD "build/release"
    ContainersTests.exe
    ErrorHandlingTests.exe
    GraphicsTests.exe
    CppLibraryTests.exe
POPD

ECHO Done building/running tests.

@ECHO ON
