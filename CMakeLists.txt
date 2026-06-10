cmake_minimum_required(VERSION 3.21)

project(VirtualNDITray VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)   # moc for Q_OBJECT
set(CMAKE_AUTORCC ON)   # rcc for .qrc files
set(CMAKE_AUTOUIC ON)   # uic for .ui files (not used yet, but good to have)

# -----------------------------------------------------------------------
# Fetch Qt at configure-time and cache it under <root>/qt/
# This must happen BEFORE find_package(Qt6) so CMAKE_PREFIX_PATH is set.
# -----------------------------------------------------------------------
include(cmake/FetchQt.cmake)

# -----------------------------------------------------------------------
# Qt 6 — found automatically via the path set by FetchQt.cmake
# -----------------------------------------------------------------------
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# WIN32 suppresses the console window on Windows
add_executable(VirtualNDITray WIN32
    src/main.cpp
    src/TrayApp.h
    src/TrayApp.cpp
    src/DXGICapture.cpp
    src/DXGICapture.h
    src/FrameReader.cpp
    src/FrameReader.h
    src/global.h
    src/ndi-loader.cpp
    src/ndi-loader.h
    src/parsec-vdd.h
    src/DisplayManager.cpp
    src/DisplayManager.h
    src/List.cpp
    src/List.h
    src/NDISender.h
    src/NDISender.cpp
)

target_include_directories(VirtualNDITray PRIVATE src)

target_link_libraries(VirtualNDITray PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# -----------------------------------------------------------------------
# Post-build: copy required Qt DLLs next to the executable.
# Qt6::windeployqt is an imported executable target provided by find_package.
# -----------------------------------------------------------------------
add_custom_command(TARGET VirtualNDITray POST_BUILD
    COMMAND Qt6::windeployqt
    ARGS    --no-translations "$<TARGET_FILE:VirtualNDITray>"
    COMMENT "Running windeployqt…"
)
