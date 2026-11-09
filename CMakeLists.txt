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

# nlohmann_json options
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")

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

FetchContent_Declare(
        nlohmann_json
        URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)

FetchContent_Declare(
        utfcpp
        URL https://github.com/nemtrif/utfcpp/archive/refs/tags/v4.0.6.tar.gz
)

FetchContent_MakeAvailable(assimp stb nlohmann_json utfcpp)

# stb is header-only
add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE ${stb_SOURCE_DIR})

# shaderc from the Vulkan SDK
find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)

add_executable(NetComp
        main.cpp
        util/diagnostics.cpp
        raw_assets/tsv.cpp
        gb_packers/lang.cpp
        raw_assets/glsl.cpp
        raw_assets/png.cpp
        util/vfs.cpp
        gb_packers/settings.cpp
        gb_packers/font.cpp
        raw_assets/json.cpp
)

target_include_directories(NetComp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/)

target_link_libraries(NetComp PRIVATE
        assimp::assimp
        stb_image
        nlohmann_json::nlohmann_json
        utf8cpp
        Vulkan::shaderc_combined
)