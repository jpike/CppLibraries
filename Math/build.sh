mkdir build

g++ --version
clang++ --version

g++-11 --version
clang++-11 --version

g++-11 -std=c++23 -c -o build/Math.o -x c++ Math.project -I code

ar rcs build/libMath.a build/Math.o
