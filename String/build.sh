mkdir build

g++ --version
clang++ --version

g++-11 --version
clang++-11 --version

g++-11 -std=c++23 -c -o String.o -x c++ String.project -I ..

ar rcs libString.a String.o
