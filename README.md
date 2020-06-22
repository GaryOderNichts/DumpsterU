To build for windows you need to install the required packages using msys2:  
For 64-bit:  
`pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-gtkmm3 pkg-config mingw-w64-x86_64-boost mingw-w64-x86_64-crypto++ mingw-w64-x86_64-libxml++`
For 32-Bit:
`pacman -S mingw-w64-i686-gcc mingw-w64-i686-gtkmm3 pkg-config mingw-w64-i686-boost mingw-w64-i686-crypto++ mingw-w64-i686-libxml++`