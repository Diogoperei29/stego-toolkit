cmake_minimum_required(VERSION 3.15)

project(StegTool VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#if (MSVC)
#   add_compile_options(/W4 /permissive-)
#else()
#   add_compile_options(-Wall -Wextra -pedantic -Werror)
#endif()

set(SOURCES
   src/main.cpp
   src/StegoHandler.cpp
   src/ImageStegoHandler.cpp
   src/CryptoModule.cpp
   src/Utils.cpp
)

set(HEADERS
   src/StegoHandler.h
   src/ImageStegoHandler.h
   src/CryptoModule.h
   src/Utils.h
)

add_executable(stegtool ${SOURCES} ${HEADERS})

target_include_directories(stegtool PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

find_package(OpenSSL REQUIRED)
target_link_libraries(stegtool PRIVATE OpenSSL::Crypto)