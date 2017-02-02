# connect

**iOS Build Instructions**

1. Downalod and install Xcode
2. Install Command Line Tools for Xcode
3. Download prebuilt iOS libs for connect sdk from `https://drive.google.com/open?id=0B2-ro67XodZTZXJpRFZCTUZYTlU`
4. Unzip archinve and define env variable that point to this folder - `CONNECT_PREBUILT_ROOT`
5. From Connect SDK root, run the following  
  	```
  	mkdir build  
  	cd build  
  	cmake -DCMAKE_TOOLCHAIN_FILE=../platforms/ios/toolchain.cmake -DIOS_PLATFORM=OS -GXcode ..  
  	cmake --build ./ --config Debug  
  	cmake --build ./ --config Release
	```
