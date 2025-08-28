cmake_minimum_required(VERSION 3.16)

project(
    RiverEngine 
    VERSION 1.0.0
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(Vendor/SDL_image EXCLUDE_FROM_ALL)

# Add subdirectories in order of dependencies
add_subdirectory(Vendor)
add_subdirectory(Engine)
add_subdirectory(Game)