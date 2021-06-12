mkdir build

g++ --version
clang++ --version

g++-11 --version
clang++-11 --version

g++-11 -std=c++23 -o GraphicsTests.exe -x c++ GraphicsTests.cpp ../Math/Math.project ../Graphics/Graphics.project -I .. -I ../../ThirdParty/Catch

GraphicsTests.exe
