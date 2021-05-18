mkdir build

g++ --version
clang++ --version

g++-11 --version
clang++-11 --version

g++-11 -c -o build/CppLibraries.o -x c++ CppLibraries.project -I Containers/code -I ErrorHandling -I ErrorHandling/code -I Filesystem -I Filesystem/code -I Graphics -I Graphics/code -I Math -I Math/code -I String -I String/code -I Windowing -I Windowing/code

#clang++-11 -c -o build/CppLibraries.o -x c++ CppLibraries.project -I Containers/code -I ErrorHandling -I ErrorHandling/code -I Filesystem -I Filesystem/code -I Graphics -I Graphics/code -I Math -I Math/code -I String -I String/code -I Windowing -I Windowing/code

ar rcs build/libCppLibraries.a build/CppLibraries.o
