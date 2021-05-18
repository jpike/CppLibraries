mkdir build

g++ --version
clang++ --version

g++-11 --version
clang++-11 --version

g++-11 -std=c++23 -c -o build/Filesystem.o -x c++ Filesystem.project -I code

ar rcs build/libFilesystem.a build/Filesystem.o
