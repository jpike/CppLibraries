mkdir build

g++ --version
clang++ --version

g++-11 --version
clang++-11 --version

g++-11 -std=c++23 -o ContainersTests.exe -x c++ ContainersTests.cpp -I ../.. -I ../../ThirdParty/Catch

ContainersTests.exe
