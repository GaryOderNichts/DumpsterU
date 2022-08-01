cmake_minimum_required(VERSION 3.18)

project(wfslib)

option(BUILD_STATIC "Build static" OFF)

add_library(${PROJECT_NAME}
    src/area.cpp
    src/block.cpp
    src/data_block.cpp
    src/device_encryption.cpp
    src/directory.cpp
    src/directory_items_iterator.cpp
    src/file.cpp
    src/file_device.cpp
    src/key_file.cpp
    src/metadata_block.cpp
    src/structs.cpp
    src/sub_block_allocator.cpp
    src/wfs.cpp
    src/wfs_item.cpp
)

if(BUILD_STATIC AND MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

target_include_directories(${PROJECT_NAME}
    PUBLIC 
        $<INSTALL_INTERFACE:include>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/include/wfslib
)

find_package(Boost 1.64.0 REQUIRED)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Boost::boost
        $<$<AND:$<BOOL:BUILD_STATIC>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-static-libstdc++ -static-libgcc $<$<CONFIG:Release>:-s>>
    PRIVATE
        cryptopp
)
