To build for windows you need to install the required packages using msys2:  
For 64-bit:  
```pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-gtkmm3 pkg-config mingw-w64-x86_64-libxml++ mingw-w64-x86_64-cmake mingw-w64-x86_64-make```  
For 32-Bit:  
```pacman -S mingw-w64-i686-gcc mingw-w64-i686-gtkmm3 pkg-config mingw-w64-i686-libxml++ mingw-w64-i686-cmake mingw-w64-i686-make```  
You need to build boost and cryptopp from source to avoid issues with the msys2 packages  
Compile with:  
`cmake -G "MinGW Makefiles" ..`  
`mingw32-make`