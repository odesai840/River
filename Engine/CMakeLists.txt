# Engine static library
cmake_minimum_required(VERSION 3.16)

# Collect all source files
file(GLOB_RECURSE ENGINE_SOURCES 
    "Source/*.cpp"
    "Source/*.c"
)

file(GLOB_RECURSE ENGINE_HEADERS 
    "Source/*.h"
    "Source/*.hpp"
)

# Create the engine static library
add_library(Engine STATIC 
    ${ENGINE_SOURCES}
    ${ENGINE_HEADERS}
)

# Set target properties
target_compile_features(Engine PUBLIC cxx_std_17)

# Find Vulkan
find_package(Vulkan REQUIRED)

# Include directories
target_include_directories(Engine 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/Core
        ${Vulkan_INCLUDE_DIRS}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link with SDL3 and Vulkan
target_link_libraries(Engine 
    PUBLIC 
        SDL3_Vendor
        Vulkan::Vulkan
)

# Set compile options
if(MSVC)
    target_compile_options(Engine PRIVATE /W4)
else()
    target_compile_options(Engine PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Create an alias for easier linking
add_library(Engine::Engine ALIAS Engine)
