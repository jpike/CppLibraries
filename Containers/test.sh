mkdir build

g++ --version
clang++ --version

g++-11 --version
clang++-11 --version

g++-11 -std=c++23 -o build/ContainersTests.exe -x c++ testing/main.cpp -I code -I ../ThirdParty/Catch

build/ContainersTests.exe
