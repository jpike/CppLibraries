mkdir build

g++ --version
clang++ --version

g++-11 --version
clang++-11 --version

g++-11 -std=c++23 -c -o build/String.o -x c++ String.project -I code

ar rcs build/libString.a build/String.o
