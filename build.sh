mkdir build

g++ --version
clang++ --version

g++-11 --version
clang++-11 --version

g++-10 -std=c++20 -c -o build/CppLibraries.o -x c++ CppLibraries.project -I .

#clang++-11 -std=c++20 -c -o build/CppLibraries.o -x c++ CppLibraries.project -I Containers/code -I ErrorHandling -I ErrorHandling/code -I Filesystem -I Filesystem/code -I Graphics -I Graphics/code -I Math -I Math/code -I String -I String/code -I Windowing -I Windowing/code

ar rcs build/libCppLibraries.a build/CppLibraries.o
