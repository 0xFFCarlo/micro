cmake_minimum_required(VERSION 3.10)

project(micro VERSION 1.0)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# Option to build as a library
option(LIBRARY_BUILD "Build micro as a shared library" ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if(APPLE)
  include_directories(/opt/homebrew/include /usr/local/include)
  link_directories(/opt/homebrew/lib /usr/local/lib)
elseif(UNIX)
  include_directories(/usr/local/include)
  link_directories(/usr/local/lib)
endif()

# Optimization flags
# set(CMAKE_C_FLAGS_RELEASE "-O3")

# Add warnings
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

# Add include directories
include_directories(${CMAKE_SOURCE_DIR}/src/systems)
include_directories(${CMAKE_SOURCE_DIR}/src/components)
include_directories(${CMAKE_SOURCE_DIR}/src/entities)

# Add libraries directories

# Source files
file(GLOB_RECURSE SOURCES "./src/*.c")

if(LIBRARY_BUILD)
  # Build as a shared library
  add_library(micro SHARED ${SOURCES})
else()
  # Build as an executable
  add_executable(main ${SOURCES})
endif()

# Link libraries
find_package(OpenGL REQUIRED)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "Configuring for Linux")
    if(LIBRARY_BUILD)
      target_link_libraries(micro GLEW)
    else()
      target_link_libraries(main GLEW)
    endif()
endif()

if(LIBRARY_BUILD)
  target_link_libraries(micro
    m         # math library
    ${OPENGL_LIBRARIES}
    SDL2
    openal
    chipmunk
    )
else()
  target_link_libraries(main
    m         # math library
    ${OPENGL_LIBRARIES}
    SDL2
    openal
    chipmunk
    )
endif()

# Compiler flags
# target_compile_options(main PRIVATE -Wall -O3)

# Compiler flags based on the build type
set(CMAKE_C_FLAGS_DEBUG "-g -rdynamic -O0 -pg")
set(CMAKE_C_FLAGS_RELEASE "-O3")

