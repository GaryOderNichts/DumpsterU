cmake_minimum_required(VERSION 3.21)

# Need C++23 for wfslib
set(CMAKE_CXX_STANDARD 23)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Build wfslib
add_subdirectory(libs/wfslib)

project(DumpsterU)

find_package(Boost COMPONENTS system filesystem REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)
pkg_check_modules(LIBXML REQUIRED libxml++-3.0)

# Compile GTK resources
include(GResource)
gresource(
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/gtk
    dumpsteru.resources.xml
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gtk
    RESOURCE_FILE
)

# Find sources
file(GLOB SOURCES
    src/*.cpp
    src/menu/*.cpp
    src/utils/*.cpp
)

# Add windows resources files
if (MINGW)
    set (WINDOWS_RESOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/windows/dumpsteru.rc"
    )
endif()

# Create executable
add_executable(${CMAKE_PROJECT_NAME}
    ${SOURCES}
    ${RESOURCE_FILE}
    ${WINDOWS_RESOURCES}
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    ${GTKMM_LIBRARIES}
    ${LIBXML_LIBRARIES}
    wfslib
    Boost::system
    Boost::filesystem
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GTKMM_INCLUDE_DIRS}
    ${LIBXML_INCLUDE_DIRS}
    src
)
