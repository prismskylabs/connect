cmake -DCMAKE_TOOLCHAIN_FILE=./platforms/ios/toolchain.cmake -DIOS_PLATFORM=OS -H. -Bbuild -GXcode
cmake --build ./build/ --config Debug
cmake --build ./build/ --config Release
