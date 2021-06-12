mkdir build

g++ --version
clang++ --version

g++-11 --version
clang++-11 --version

g++-11 -std=c++23 -c -o build/Graphics.o -x c++ Graphics.project -I code -I ../Containers/code -I ../Math/code

ar rcs build/libGraphics.a build/Graphics.o
