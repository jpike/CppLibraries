# CppLibraries
![CodeQL](https://github.com/jpike/CppLibraries/actions/workflows/codeql-analysis.yml/badge.svg)

Collection of C++ libraries I hope will be re-usable.

These basically come from other projects in my GitHub repo but have generally been generic and good enough
to be worth factoring out to separate libraries for re-use.  This repository is in a very early state though,
so the code is not as high-quality yet as I would like.

So no guarantees about the state of the current code.  The "Graphics" library in particular is in relatively poor shape.
You are certainly welcome to try using it if you want, but it is currently largely intended for my own use.

# Building/Using
In general, I aim to minimize overall dependencies for these libraries, but I will introduce dependencies that
I deem "good enough" and "long-lasting" if they provide good enough value that does not seem worth me rewriting.
Current dependencies are basically just the following:
- C++ Standard Library
- Windows API
- GSL
- Catch (unit testing)
- SDL
- imgui
- bgfx (fork from https://github.com/jpike/bgfx along with https://github.com/pr0g/sdl-bgfx-imgui-starter)

A `build.bat` file exists in the root of this repo for building the `CppLibraries.project` file into a single `.lib`
file for all of the libraries in this repo (though note that a decent amount of code is header-only with C++ templates).

# Organization
Each individual library in this repo is organized in its own subdirectory with the following structure:
```
LibraryName/
    The main source code for each library is located here for easy inclusion from the root directory.
    build.bat (batch file for building the individual library; can check this for dependencies on other libraries; will output to this directory for ease-of-use)
    LibraryName.project (file for #including the files in the library for a unity/single-translation-unit build)
    testing/
        (A subdirectory to more clearly isolate test code from the main library code)
```

# License
The custom written code in this repository is in the public domain (via the UNLICENSE).
Basically, use the code however you want, but I make no guarantees about the quality/state of the code.

Third-party dependencies may be checked in under a `ThirdParty` directory.  That code is not covered by
the root LICENSE (UNLICENSE) file in this repo, though I try to keep any third-party dependencies with
relatively permissive licenses.  Check any subdirectories for third-party dependencies for specific licenses.
