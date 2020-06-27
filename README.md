
![banner](https://i.imgur.com/x5hcGzV.png)
![C/C++ CI](https://github.com/GaryOderNichts/DumpsterU/workflows/C/C++%20CI/badge.svg)
# DumpsterU - Wii U Harddrive Dumper  
DumpsterU makes it really easy to dump your installed contents with very high speeds.  
You can dump your saves, games, updates and DLCs straight from your Wii U-formatted harddrive.  
It will dump decrypted contents that you can use with Cemu or other applications.  
You need a OTP and SEEPROM backup from the console the harddrive was formatted with. You can get the backup with [Wii U NAND Dumper](https://github.com/koolkdev/wiiu-nanddumper).  

## Usage  
Select your hardrive from the list and open your otp.bin and seeprom.bin.  
A list with all the games installed to that harddrive will open.  
Tip: You can press Ctrl + F to search or click on one of the headers to sort the list.  
Click on the title you want to dump, select an output folder and press on one of the buttons to dump the contents.

## Download
You can download DumpsterU from the [releases page](https://github.com/GaryOderNichts/DumpsterU/releases).  
Currently Windows and Ubuntu builds are available.

## FAQ
#### Where are my Wii VC saves?  
They are stored on the NAND and cannot be dumped by this tool.  
#### Why do I need my OTP and SEEPROM?
The contents of the harddrive are encrypted and the OTP and SEEPROM contain keys used for decrypting.
#### How fast is this?
The 14GB from Breath of the Wild were dumped in 1:40 minutes. This depends on harddrive speed though.

## Screenshots
<b>Windows</b>  
![screenshot_windows](https://i.imgur.com/Dbz085J.png "Screenshot on Windows")
<b>Ubuntu</b>
![screenshot_ubuntu](https://i.imgur.com/HLENggc.png "Screenshot on Ubuntu")

## Installation
<b>Windows</b>  
Run the installer

<b>Ubuntu</b>
Run from commandline:
```
sudo ./DumpsterU
```

## Credits
<b>GaryOderNichts</b>: DumpsterU  
<b>koolkdev</b>: [wfslib](https://github.com/koolkdev/wfslib)  
<b>Flump and NoahAbc12345</b>: Icons and beta testing

## Build
<b>Ubuntu</b>  
Install the dependencies  
```
sudo apt-get install cmake build-essential libboost-all-dev libcrypto++-dev libgtkmm-3.0-dev g++-9
```
Build and install libxml++-3.0 from [here](http://ftp.gnome.org/pub/GNOME/sources/libxml++/3.0)  
Compile with:  
`mkdir build && cd build`  
`cmake ..`  
`make`

<b>Windows</b>  
To build for windows you need to install the required packages using msys2:  
```
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-gtkmm3 pkg-config mingw-w64-x86_64-libxml++ mingw-w64-x86_64-cmake mingw-w64-x86_64-make
```  
You need to build boost and cryptopp from source to avoid issues with the msys2 packages  
Compile with:  
`mkdir build && cd build`  
`cmake -G "MinGW Makefiles" ..`  
`mingw32-make`

## Disclaimer
I am not responsible for any damage or data loss