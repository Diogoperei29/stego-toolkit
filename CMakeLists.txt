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
  src/algorithms/StegoHandler.cpp
  src/core/CLI.cpp
  src/utils/CryptoModule.cpp
  src/utils/ErrorHandler.cpp
  src/utils/ImageIO.cpp
  src/algorithms/lsb/LSBStegoHandler.cpp
  src/algorithms/lsb/ordered/LSBStegoHandlerOrdered.cpp
  src/algorithms/lsb/shuffle/LSBStegoHandlerShuffle.cpp
)

set(LIB_HEADERS
  src/algorithms/StegoHandler.h
  src/core/CLI.h
  src/utils/CryptoModule.h
  src/utils/ErrorHandler.h
  src/utils/ImageIO.h
  src/algorithms/lsb/LSBStegoHandler.h
  src/algorithms/lsb/ordered/LSBStegoHandlerOrdered.h
  src/algorithms/lsb/shuffle/LSBStegoHandlerShuffle.h
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

# Test helpers library (shared by all tests)
add_library(test_helpers STATIC tests/test_helpers.cpp tests/test_helpers.h)
target_include_directories(test_helpers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tests)
target_link_libraries(test_helpers PUBLIC stegtool_lib)
target_compile_options(test_helpers PRIVATE ${TEST_WARNING_FLAGS})
target_compile_definitions(test_helpers PUBLIC # definitions to find paths faster
    TEST_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures"
    TEST_OUTPUT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/output"
)

# Unit tests
add_executable(test_unit
    tests/unit/test_lsb_handler.cpp
    tests/unit/test_crypto.cpp
    tests/unit/test_image_io.cpp
    tests/unit/test_error_handler.cpp
)
target_link_libraries(test_unit PRIVATE stegtool_lib test_helpers GTest::gtest_main)
target_compile_options(test_unit PRIVATE ${TEST_WARNING_FLAGS})

# Integration tests
add_executable(test_integration
    tests/integration/test_embed_extract.cpp
)
target_link_libraries(test_integration PRIVATE stegtool_lib test_helpers GTest::gtest_main)
target_compile_options(test_integration PRIVATE ${TEST_WARNING_FLAGS})

# E2E/CLI tests
add_executable(test_e2e
    tests/e2e/test_cli.cpp
)
target_link_libraries(test_e2e PRIVATE stegtool_lib test_helpers GTest::gtest_main)
target_compile_options(test_e2e PRIVATE ${TEST_WARNING_FLAGS})

# All tests  - order of tests is reverse of the order here
add_executable(test_all
    tests/e2e/test_cli.cpp
    tests/integration/test_embed_extract.cpp
    tests/unit/test_lsb_handler.cpp
    tests/unit/test_crypto.cpp
    tests/unit/test_image_io.cpp
    tests/unit/test_error_handler.cpp
)
target_link_libraries(test_all PRIVATE stegtool_lib test_helpers GTest::gtest_main)
target_compile_options(test_all PRIVATE ${TEST_WARNING_FLAGS})

# Add all tests to CTest
add_test(NAME UnitTests COMMAND test_unit)
add_test(NAME IntegrationTests COMMAND test_integration)
add_test(NAME E2ETests COMMAND test_e2e)
add_test(NAME AllTests COMMAND test_all)


# Print build information
message(STATUS "")
message(STATUS "StegTool Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "")
