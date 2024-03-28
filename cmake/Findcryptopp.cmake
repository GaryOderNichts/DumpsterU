#
# Since cryptopp no longer ships with CMake support, this files provides basic find_package support for it.
#
# Based on <https://izzys.casa/2020/12/how-to-find-packages-with-cmake-the-basics/>
#

include (FindPackageHandleStandardArgs)

find_library (cryptopp_LIBRARY NAMES cryptopp)
find_path (cryptopp_INCLUDE_DIR NAMES cryptopp/aes.h)

find_package_handle_standard_args (cryptopp REQUIRED_VARS cryptopp_LIBRARY cryptopp_INCLUDE_DIR)

if (cryptopp_FOUND)
  mark_as_advanced (cryptopp_LIBRARY)
  mark_as_advanced (cryptopp_INCLUDE_DIR)
endif()

if (cryptopp_FOUND AND NOT TARGET cryptopp::cryptopp)
  add_library(cryptopp::cryptopp IMPORTED UNKNOWN)
  set_property(TARGET cryptopp::cryptopp PROPERTY IMPORTED_LOCATION ${cryptopp_LIBRARY})
  target_include_directories(cryptopp::cryptopp INTERFACE ${cryptopp_INCLUDE_DIR})
endif()
