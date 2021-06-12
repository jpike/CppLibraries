mkdir build

g++ --version
clang++ --version

g++-11 --version
clang++-11 --version

g++-11 -std=c++23 -o build/CppLibraryTests.exe -x c++ CppLibraryTests.project -I ThirdParty/Catch

build/CppLibraryTests.exe
