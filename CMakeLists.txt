cmake_minimum_required(VERSION 3.20)
project(SISFA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_PREFIX_PATH "C:/msys64/ucrt64")

include_directories(include)

find_package(OpenSSL REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Qt needs this to process signals/slots
qt_standard_project_setup()

# CLI version
add_executable(SISFA_CLI
    src/main.cpp
    src/stb_impl.cpp
    src/steganography.cpp
    src/crypto.cpp
    src/vault.cpp
)
target_link_libraries(SISFA_CLI OpenSSL::Crypto)

# GUI version
add_executable(SISFA_GUI
    src/main_gui.cpp
    src/mainwindow.cpp
    src/embeddialog.cpp
    src/extractdialog.cpp
    src/stb_impl.cpp
    src/steganography.cpp
    src/crypto.cpp
    src/vault.cpp
)

target_include_directories(SISFA_GUI PRIVATE include src)

target_link_libraries(SISFA_GUI
    OpenSSL::Crypto
    Qt6::Core
    Qt6::Widgets
)