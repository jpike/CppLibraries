mkdir build

g++ --version
clang++ --version

g++-11 --version
clang++-11 --version

g++-11 -std=c++23 -c -o Graphics.o -x c++ Graphics.project -I ..

ar rcs libGraphics.a Graphics.o
