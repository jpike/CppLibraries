mkdir build

g++ --version
clang++ --version

g++-11 --version
clang++-11 --version

g++-11 -std=c++23 -c -o Filesystem.o -x c++ Filesystem.project -I ..

ar rcs libFilesystem.a Filesystem.o
