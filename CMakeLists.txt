cmake_minimum_required(VERSION 3.16)

project(StegTool VERSION 1.0 LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

### EXTERNAL LIBRARIES ###
include(FetchContent)
# Fetch Google Test
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.17.0
)
FetchContent_MakeAvailable(googletest)

# For clean cli parsing: cxxopts
FetchContent_Declare(
  cxxopts
  GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
  GIT_TAG        v3.3.1
)
FetchContent_MakeAvailable(cxxopts)

# For stb_images (image loading/writing)
FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
)
FetchContent_MakeAvailable(stb)

# Create an interface library for stb headers
add_library(stb_headers INTERFACE)
target_include_directories(stb_headers SYSTEM INTERFACE ${stb_SOURCE_DIR}) # SYSTEM to suppress warnings

# OpenSSL (system or package-managed)
find_package(OpenSSL REQUIRED)


# Set warning flags based on compiler  (after external libraries)
if(MSVC)
    set(WARNING_FLAGS /W4 /WX)
    set(TEST_WARNING_FLAGS /W4 /WX)
else()
    set(WARNING_FLAGS -Wall -Wextra -pedantic -Werror)
    set(TEST_WARNING_FLAGS -Wall -Wextra -pedantic -Werror)
endif()

# Create a library for the core functionality (used by both main and tests)
set(LIB_SOURCES
  src/core/CLI.cpp
  src/utils/CryptoModule.cpp
  src/utils/ErrorHandler.cpp
  src/utils/ImageIO.cpp
  src/algorithms/lsb/LSBStegoHandler.cpp
)

set(LIB_HEADERS
  src/algorithms/StegoHandler.h
  src/core/CLI.h
  src/utils/CryptoModule.h
  src/utils/ErrorHandler.h
  src/utils/ImageIO.h
  src/algorithms/lsb/LSBStegoHandler.h
)

# StegTool library
add_library(stegtool_lib STATIC ${LIB_SOURCES} ${LIB_HEADERS})
target_include_directories(stegtool_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(stegtool_lib PUBLIC OpenSSL::Crypto stb_headers cxxopts::cxxopts)
target_compile_options(stegtool_lib PRIVATE ${WARNING_FLAGS})

# Main Executable
add_executable(stegtool src/main.cpp)
target_link_libraries(stegtool PRIVATE stegtool_lib)
target_compile_options(stegtool PRIVATE ${WARNING_FLAGS})



enable_testing()

# General Unit Tests
add_executable(test_all tests/test_all.cpp)
target_link_libraries(test_all PRIVATE stegtool_lib GTest::gtest_main)
target_compile_options(test_all PRIVATE ${TEST_WARNING_FLAGS})

# Add tests to CTest
add_test(NAME TestAll COMMAND test_all)



# Print build information
message(STATUS "")
message(STATUS "StegTool Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "")