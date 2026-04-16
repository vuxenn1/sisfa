cmake_minimum_required(VERSION 3.20)
project(SISFA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

find_package(OpenSSL REQUIRED)

add_executable(SISFA
    src/main.cpp
    src/stb_impl.cpp
    src/steganography.cpp
    src/crypto.cpp
    src/vault.cpp
)

target_link_libraries(SISFA OpenSSL::SSL OpenSSL::Crypto)