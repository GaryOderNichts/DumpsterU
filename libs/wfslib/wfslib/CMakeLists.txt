cmake_minimum_required(VERSION 3.10)
project(wfslib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++14")

add_library(
    wfslib
    STATIC
    Area.cpp
    Block.cpp
    DataBlock.cpp
    DeviceEncryption.cpp
    Directory.cpp
    File.cpp
    FileDevice.cpp
    KeyFile.cpp
    MetadataBlock.cpp
    Structs.cpp
    SubBlockAllocator.cpp
    Wfs.cpp
    WfsItem.cpp
)