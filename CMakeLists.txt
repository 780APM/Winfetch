cmake_minimum_required(VERSION 3.16)
project(Winfetch VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Find required packages
find_package(PkgConfig REQUIRED)

# Windows specific settings
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DNOMINMAX)
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/system_info.cpp
    src/display.cpp
    src/config.cpp
    src/ascii_art.cpp
)

# Header files
set(HEADERS
    include/system_info.h
    include/display.h
    include/config.h
    include/ascii_art.h
)

# Create executable
add_executable(winfetch ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(winfetch PRIVATE include)

# Link libraries
if(WIN32)
    target_link_libraries(winfetch 
        kernel32
        user32
        gdi32
        winspool
        shell32
        ole32
        oleaut32
        uuid
        comdlg32
        advapi32
        psapi
        powrprof
        wbemuuid
    )
endif()

# Compiler specific options
if(MSVC)
    target_compile_options(winfetch PRIVATE /W4)
else()
    target_compile_options(winfetch PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Install target
install(TARGETS winfetch DESTINATION bin)
