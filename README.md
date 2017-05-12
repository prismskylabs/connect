# Connect SDK

##iOS Build Instructions

1. Downalod and install Xcode
2. Install Command Line Tools for Xcode
3. Download prebuilt iOS libs for connect sdk from `https://drive.google.com/open?id=0B2-ro67XodZTZXJpRFZCTUZYTlU`
4. Unzip archinve and define env variable that point to this folder - `CONNECT_PREBUILT_ROOT`
5. From Connect SDK root, run the following

  ```
  ./platforms/ios/build-ios.sh
  ```

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

