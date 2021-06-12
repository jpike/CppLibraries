mkdir build

g++ --version
clang++ --version

g++-11 --version
clang++-11 --version

g++-11 -std=c++23 -c -o Math.o -x c++ Math.project -I ..

ar rcs libMath.a Math.o
