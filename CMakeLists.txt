# CMakeList.txt : CMake project for Recreating-DOS2-SurfaceSystem, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.24)

project(RaylibCppStarter VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# raylib
#
# Use an already installed raylib when one is available (vcpkg, a system
# install, or -Draylib_DIR=...). Otherwise fetch the pinned release from
# GitHub and build it together with the game. That download only happens on
# the first CMake configure.
# ---------------------------------------------------------------------------
set(RAYLIB_VERSION 6.2)

# Accept any raylib 6.x that is already installed (vcpkg, system install, ...).
find_package(raylib 6.0 CONFIG QUIET)

if (NOT raylib_FOUND)
    include(FetchContent)

    message(STATUS "raylib not found locally - fetching raylib ${RAYLIB_VERSION} from GitHub")
    message(STATUS "This happens only on the first configure and can take a minute.")

    FetchContent_Declare(
        raylib
        URL      https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
        URL_HASH SHA256=DF7A13129B1BEC9E73A6962EFFF170BEBA310EE6EAB8F94CD000B7316B4DB5EE
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(raylib)
endif()

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("octree-visualiser")

# Add source to this project's executable.
add_executable (octree-visualiser "src/main.cpp"  "src/surface/grid.h" "src/surface/grid.cpp" "src/structs/vec3.h" "src/structs/gameObjects.h" "src/structs/gameObjects.cpp" "src/structs/resources.h" "src/constants/colors.h" "src/game/game.h" "src/game/game.cpp"  "src/player/player.h" "src/player/player.cpp"  "src/surface/groundCube.h"  "src/collision detection/AABB.h" "src/collision detection/CollisionWorld.cpp" "src/collision detection/CollisionWorld.h" "src/collision detection/Octree.cpp" "src/collision detection/Octree.h" "src/visualiser/OctreeVisualiser.h" "src/visualiser/OctreeVisualiser.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET octree-visualiser PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
target_include_directories(octree-visualiser PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(octree-visualiser PRIVATE raylib)

# Make `Recreating-DOS2-SurfaceSystem` the startup project if the generated .sln is opened directly.
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT octree-visualiser)