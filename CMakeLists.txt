cmake_minimum_required(VERSION 3.10)

project(micro VERSION 1.0)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add include directories
include_directories(/usr/local/include)
include_directories(${CMAKE_SOURCE_DIR}/src/ext) # external libraries
include_directories(${CMAKE_SOURCE_DIR}/src/systems)
include_directories(${CMAKE_SOURCE_DIR}/src/components)
include_directories(${CMAKE_SOURCE_DIR}/src/entities)

# Source files
file(GLOB_RECURSE SOURCES "./src/*.c")

# Executable
add_executable(main ${SOURCES})

# Link libraries
find_package(OpenGL REQUIRED)

target_link_libraries(main
    ${OPENGL_LIBRARIES}
    m # math library
)

if(APPLE)
    # macOS specific configuration
    target_link_libraries(main
        "-framework SDL2"
        "-framework SDL2_mixer"
    )
    # Add the frameworks search path
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -F/Library/Frameworks")
elseif(UNIX)
    # Linux specific configuration
    target_link_libraries(main
        SDL2
        SDL2_mixer
    )
endif()

# Compiler flags
target_compile_options(main PRIVATE -Wall -O3)
