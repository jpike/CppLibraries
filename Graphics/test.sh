mkdir build

g++ --version
clang++ --version

g++-11 --version
clang++-11 --version

g++-11 -std=c++23 -o build/GraphicsTests.exe -x c++ testing/main.cpp -I code -I ../Containers/code -I ../Math/code -I ../ThirdParty/Catch -L build -L ../Math/build -l Graphics -l Math

build/GraphicsTests.exe
