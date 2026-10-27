cmake_minimum_required(VERSION 4.3)
project(NetComp)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

# Assimp options (set before MakeAvailable)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG        v6.0.2
        GIT_SHALLOW    TRUE
)

FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG        master    # better: pin to a commit hash
)

FetchContent_MakeAvailable(assimp stb)

# stb is header-only
add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE ${stb_SOURCE_DIR})

# shaderc from the Vulkan SDK
find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)

add_executable(NetComp main.cpp)

target_link_libraries(NetComp PRIVATE
        assimp::assimp
        stb_image
        Vulkan::shaderc_combined
)
