cmake_minimum_required(VERSION 3.10)
project(DumpsterU)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )

add_subdirectory(libs/wfslib/wfslib)

find_package(PkgConfig)

pkg_check_modules(GTKMM gtkmm-3.0)
pkg_check_modules(LIBXML libxml++-3.0)

include(GResource)
gresource(${CMAKE_CURRENT_SOURCE_DIR}/resources/gtk
            dumpsteru.resources.xml
            ${CMAKE_CURRENT_SOURCE_DIR}/src/gtk
            RESOURCE_FILE)

link_directories(
    ${GTKMM_LIBRARY_DIRS}
    ${LIBXML_LIBRARY_DIRS}
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GTKMM_INCLUDE_DIRS}
    ${LIBXML_INCLUDE_DIRS}
    src
    libs
    libs/wfslib
)

file(GLOB SOURCES
    src/*.cpp
    src/menu/*.cpp
    src/utils/*.cpp
)

if (MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -static-libstdc++")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17")
endif()

if (MINGW)
    set(WINDOWS_RESOURCE "${CMAKE_CURRENT_SOURCE_DIR}/resources/windows/dumpsteru.rc")
endif()

add_executable(${CMAKE_PROJECT_NAME}
                ${SOURCES}
                ${RESOURCE_FILE}
                ${WINDOWS_RESOURCE})
if (MINGW)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        ${GTKMM_LIBRARIES}
        ${LIBXML_LIBRARIES}
        wfslib
        cryptopp
        boost_system
        boost_filesystem
        pthread
        Wbemuuid
    )
else()
    target_link_libraries(${CMAKE_PROJECT_NAME}
        ${GTKMM_LIBRARIES}
        wfslib
        libxml++-3.0.a
        libxml2.a
        z
        lzma
        icuuc
        dl
        libcrypto++.a
        libboost_system.a
        libboost_filesystem.a
        pthread
    )
endif()