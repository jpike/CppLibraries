mkdir build

g++ --version
clang++ --version

g++-11 --version
clang++-11 --version

g++-11 -std=c++23 -o build/CppLibraryTests.exe -x c++ CppLibraryTests.project -I Containers/code -I ErrorHandling -I ErrorHandling/code -I Filesystem -I Filesystem/code -I Graphics -I Graphics/code -I Math -I Math/code -I String -I String/code -I Windowing -I Windowing/code -I ThirdParty/Catch -L Graphics/build -L Math/build -l Graphics -l Math

build/CppLibraryTests.exe
