cmake_minimum_required(VERSION 3.10)

project(micro VERSION 1.0)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Release)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add include directories
include_directories(/usr/local/include)

# Source files
file(GLOB SOURCES "./src/*.c")

# Executable
add_executable(main ${SOURCES})

# Link libraries
find_package(OpenGL REQUIRED)

target_link_libraries(main
    ${OPENGL_LIBRARIES}
    "-framework SDL2"
    "-framework SDL2_ttf"
    "-framework SDL2_mixer"
    m # math library
)

# Add the frameworks search path
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -F/Library/Frameworks")

# Compiler flags
target_compile_options(main PRIVATE -Wall -O3)
