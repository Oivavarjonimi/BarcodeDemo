cmake_minimum_required(VERSION 3.21)
project(BarcodeDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia)
qt_standard_project_setup()

find_package(PkgConfig REQUIRED)
pkg_check_modules(ZBAR REQUIRED zbar)

set(ZXING_READERS ON  CACHE BOOL "" FORCE)
set(ZXING_WRITERS OFF CACHE STRING "" FORCE)

set(ZXING_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ZXING_BLACKBOX_TESTS OFF CACHE BOOL "" FORCE)
set(ZXING_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(ZXING_INSTALL_TEST OFF CACHE BOOL "" FORCE)
set(ZXING_PYTHON_MODULE OFF CACHE BOOL "" FORCE)
set(ZXING_C_API OFF CACHE BOOL "" FORCE)

set(ZXING_DEPENDENCIES LOCAL CACHE STRING "" FORCE)

# ZXing-cpp as submodule under extern/zxing-cpp
add_subdirectory(extern/zxing-cpp)

qt_add_executable(BarcodeDemo
    src/main.cpp
    src/MainWindow.h src/MainWindow.cpp
    src/CameraController.h src/CameraController.cpp
    src/IDecoder.h
    src/ZXingDecoder.h src/ZXingDecoder.cpp
    src/ZBarDecoder.h  src/ZBarDecoder.cpp
)

target_include_directories(BarcodeDemo PRIVATE
    ${ZBAR_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/zxing-cpp/core/src
)

target_link_libraries(BarcodeDemo PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::Multimedia
    ZXing::ZXing
    ${ZBAR_LIBRARIES}
)

# Optional but often needed with pkg-config modules:
# target_compile_options(BarcodeDemo PRIVATE ${ZBAR_CFLAGS_OTHER})
# target_link_directories(BarcodeDemo PRIVATE ${ZBAR_LIBRARY_DIRS})
