# Connect SDK

##General Build Instructions (linux, macOS)

1. From Connect SDK root, run the following

  ```
  ./build.sh
  ```

##iOS Build Instructions

1. Downalod and install Xcode
2. Install Command Line Tools for Xcode
3. From Connect SDK root, run the following

  ```
  ./build.sh --platform=ios
  ```
4. Please note that output universal lib contains ```arm64``` and ```x86_64``` archs, so old devices are not supported

##Hanwha-wn build instructions

1. Download and install OpenSDK v2.03 from Samsung
2. From Connect SDK root, run the following

 ```
 ./platforms/hanwha-wn/build.sh
 ```
 
##Axis-mips build instructions

1. Download and install AXIS Embedded Development SDK (aka ACAP SDK) v2.0.3
2. Set environment variable AXIS_SDK_ROOT to full path to folder containing init_env script e.g.

 ```
 export AXIS_SDK_ROOT=/home/<user>/axis/emb-app-sdk_2_0_3
 ```

3. Install MIPS toolchain from ${AXIS_SDK_ROOT}/tools/compiler/mips/ folder. It will install to /usr/local/mips... folder.
4. Set BOOST_ROOT to point prebuild Boost 1.62.0 library
5. From Connect SDK root, run the following

 ```
 ./build.sh --platform=axis-mips
 ```

