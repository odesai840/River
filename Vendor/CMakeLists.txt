# Vendor dependencies
cmake_minimum_required(VERSION 3.16)

# Check if SDL directory exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SDL/CMakeLists.txt")
    message(STATUS "Building SDL3...")
    
    # Disable SDL3 examples, tests, etc. to speed up build
    set(SDL_SHARED OFF CACHE BOOL "Build SDL3 as shared library" FORCE)
    set(SDL_STATIC ON CACHE BOOL "Build SDL3 as static library" FORCE)
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "Build SDL3 test library" FORCE)
    
    add_subdirectory(SDL)
    
    # Ensure SDL_image builds as static to match our SDL3 static build
    set(BUILD_SHARED_LIBS OFF)
    
    # Disable heavy dependencies that require additional tools/libraries
    set(SDLIMAGE_AVIF OFF CACHE BOOL "Enable AVIF image support" FORCE)
    set(SDLIMAGE_JXL OFF CACHE BOOL "Enable JPEG XL image support" FORCE) 
    set(SDLIMAGE_WEBP OFF CACHE BOOL "Enable WebP image support" FORCE)
    set(SDLIMAGE_TIF OFF CACHE BOOL "Enable TIFF image support" FORCE)
    
    # Keep basic formats enabled
    set(SDLIMAGE_BMP ON CACHE BOOL "Enable BMP image support" FORCE)
    set(SDLIMAGE_PNG ON CACHE BOOL "Enable PNG image support" FORCE)
    set(SDLIMAGE_JPG ON CACHE BOOL "Enable JPEG image support" FORCE)
    
    add_subdirectory(SDL_image)
else()
    message(FATAL_ERROR "SDL subdirectory not found. Please add SDL3 source to Vendor/SDL.")
endif()

# Check if libzmq directory exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libzmq/CMakeLists.txt")
    message(STATUS "Building libzmq...")
    
    # Configure libzmq build options
    set(BUILD_SHARED OFF CACHE BOOL "Build libzmq as shared library" FORCE)
    set(BUILD_STATIC ON CACHE BOOL "Build libzmq as static library" FORCE)
    set(BUILD_TESTS OFF CACHE BOOL "Build libzmq tests" FORCE)
    set(ENABLE_CPACK OFF CACHE BOOL "Enable libzmq packaging" FORCE)
    set(WITH_DOCS OFF CACHE BOOL "Build libzmq documentation" FORCE)
    
    add_subdirectory(libzmq)
else()
    message(FATAL_ERROR "libzmq subdirectory not found. Please add libzmq source to Vendor/libzmq.")
endif()