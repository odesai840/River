# Vendor dependencies
cmake_minimum_required(VERSION 3.16)

# Check if SDL directory exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SDL/CMakeLists.txt")
    message(STATUS "Building SDL3...")
    
    # Disable SDL3 examples, tests, etc. to speed up build
    set(SDL_SHARED OFF CACHE BOOL "Build SDL3 as shared library" FORCE)
    set(SDL_STATIC ON CACHE BOOL "Build SDL3 as static library" FORCE)
    set(SDL_TEST OFF CACHE BOOL "Build SDL3 tests" FORCE)
    
    add_subdirectory(SDL)
    
    # Create an interface library to wrap built SDL3
    add_library(SDL3_Vendor INTERFACE)
    target_link_libraries(SDL3_Vendor INTERFACE SDL3-static)
else()
    message(FATAL_ERROR "SDL subdirectory not found. Please add SDL3 source to Vendor/SDL.")
endif()
