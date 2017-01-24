cmake -DCMAKE_TOOLCHAIN_FILE=./ios.cmake -DIOS_PLATFORM=OS -H. -Bbuild -GXcode
cmake --build build/ --config Debug
cmake --build build/ --config Release
