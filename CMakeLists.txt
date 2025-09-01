cmake_minimum_required(VERSION 3.15)
project(StegTool VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Warnings
if (MSVC)
  add_compile_options(/W4 /permissive-)
else()
  add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

set(SOURCES
  src/main.cpp
  src/CLI.cpp
  src/ImageStegoHandler.cpp
  src/CryptoModule.cpp
  src/Utils.cpp
)

set(HEADERS
  src/CLI.h
  src/ImageStegoHandler.h
  src/CryptoModule.h
  src/Utils.h
)

add_executable(stegtool ${SOURCES} ${HEADERS})
target_include_directories(stegtool PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# OpenSSL (system or package-managed)
find_package(OpenSSL REQUIRED)
target_link_libraries(stegtool PRIVATE OpenSSL::Crypto)

# ---------------------------
# FetchContent dependencies
# ---------------------------
include(FetchContent)

# For clean cli parsing: cxxopts
FetchContent_Declare(
  cxxopts
  GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
)
FetchContent_MakeAvailable(cxxopts)
target_link_libraries(stegtool PRIVATE cxxopts::cxxopts)

# For stb_images (image loading/writing)
FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
)
# This performs the download/configure step (and sets *_SOURCE_DIR/BUILD_DIR)
FetchContent_MakeAvailable(stb)

# Create a header-only target that exposes the fetched include path
add_library(stb_headers INTERFACE)
# Mark the include directory as SYSTEM to silence warnings from stb headers
target_include_directories(stb_headers
  SYSTEM INTERFACE
    ${stb_SOURCE_DIR}  # provided by FetchContent
)

# Link the stb interface target so code can #include "stb_image.h"
target_link_libraries(stegtool PRIVATE stb_headers)

# # Tests (optional)
# option(BUILD_TESTS "Build unit tests" ON)
# if (BUILD_TESTS)
#   enable_testing()
#   add_subdirectory(tests)
# endif()