# connect

**iOS Build Instructions**

1. Downalod and install Xcode
2. Install Command Line Tools for Xcode
3. Download prebuilt iOS libs for connect sdk from `https://drive.google.com/drive/folders/0B2-ro67XodZTZXJpRFZCTUZYTlU`
4. Unzip archinve and define two env variables to each of the folder inside it - `BOOST_IOS_ROOT` and `CURL_IOS_ROOT`
5. Run commands from the root of connect folder
```
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../build-scripts/iOS.cmake -DIOS_PLATFORM=OS
make -j4
```
