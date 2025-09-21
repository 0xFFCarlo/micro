cmake_minimum_required(VERSION 3.10)

project(micro VERSION 1.0)

# Disallow in-source builds (helps with SDL/OpenAL subprojects)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "Use: cmake -S . -B build && cmake --build build")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)   # needed when linking static libs into shared

option(LIBRARY_BUILD "Build micro as a shared library" ON)

# Output dirs
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Warnings
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

# Sources
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.c")

# -------------------- Dependencies --------------------

# SDL2 (static)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_TEST   OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/dep/sdl EXCLUDE_FROM_ALL)

# OpenAL Soft (shared) — keep LGPL simple
set(BUILD_SHARED_LIBS ON  CACHE BOOL "Build shared libraries (OpenAL as shared)" FORCE)
set(ALSOFT_EXAMPLES OFF   CACHE BOOL "" FORCE)
set(ALSOFT_UTILS   OFF    CACHE BOOL "" FORCE)
set(ALSOFT_TESTS   OFF    CACHE BOOL "" FORCE)
set(ALSOFT_NO_CONFIG_UTIL ON CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/dep/openal-soft EXCLUDE_FROM_ALL)

# Resolve dependency target names
set(OPENAL_TARGET OpenAL)
if(NOT TARGET ${OPENAL_TARGET})
  set(OPENAL_TARGET openal)
endif()

# --- Chipmunk2D (force static) ---
set(BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(BUILD_STATIC ON CACHE BOOL "" FORCE)
set(BUILD_DEMOS OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/dep/Chipmunk2D EXCLUDE_FROM_ALL)

# Resolve Chipmunk target robustly
set(CHIPMUNK_TARGET "")
foreach(cand chipmunk chipmunk_static Chipmunk2D::Chipmunk Chipmunk::Chipmunk)
  if(TARGET ${cand})
    set(CHIPMUNK_TARGET ${cand})
    break()
  endif()
endforeach()
if(NOT CHIPMUNK_TARGET)
  message(FATAL_ERROR "Chipmunk2D target not found after add_subdirectory(dep/Chipmunk2D)")
endif()
message(STATUS "Using Chipmunk target: ${CHIPMUNK_TARGET}")

# Ensure PIC if it’s a static lib being linked into your shared lib
get_target_property(_chip_type ${CHIPMUNK_TARGET} TYPE)
if(_chip_type STREQUAL "STATIC_LIBRARY")
  set_property(TARGET ${CHIPMUNK_TARGET} PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

# -------------------- Your target --------------------

if(LIBRARY_BUILD)
  add_library(micro SHARED ${SOURCES})
  set(MICRO_TARGET micro)
else()
  add_executable(main ${SOURCES})
  set(MICRO_TARGET main)
endif()

find_package(OpenGL REQUIRED)

# Include dirs needed by *your* code (not globally)
target_include_directories(${MICRO_TARGET} PRIVATE
  ${CMAKE_SOURCE_DIR}/dep/Chipmunk2D/include   # for <chipmunk/chipmunk.h>
  /usr/local/include/
  ${CMAKE_SOURCE_DIR}/dep/SDL/include
  ${CMAKE_SOURCE_DIR}/src/systems
  ${CMAKE_SOURCE_DIR}/src/components
  ${CMAKE_SOURCE_DIR}/src/entities
)

# Linux-only GLEW (if you use it there)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(${MICRO_TARGET} PRIVATE GLEW)
endif()

# Link everything
target_link_libraries(${MICRO_TARGET}
  PRIVATE
    m
    ${OPENGL_LIBRARIES}
    SDL2-static
    ${OPENAL_TARGET}
    ${CHIPMUNK_TARGET}
)

# macOS: ensure dylibs are found at runtime after install
if(APPLE)
  set(CMAKE_MACOSX_RPATH ON)
  set_target_properties(${MICRO_TARGET} PROPERTIES
    INSTALL_RPATH "@loader_path/../lib;@rpath")
endif()

# -------------------- Install --------------------

# Install public headers
install(
  DIRECTORY ${CMAKE_SOURCE_DIR}/src/
  DESTINATION include
  FILES_MATCHING PATTERN "*.h"
)

# Install your binary/library
if(LIBRARY_BUILD)
  install(TARGETS micro DESTINATION lib)
else()
  install(TARGETS main DESTINATION bin)
endif()

# Build-type C flags
set(CMAKE_C_FLAGS_DEBUG   "-g -rdynamic -O0 -pg")
set(CMAKE_C_FLAGS_RELEASE "-O3")

