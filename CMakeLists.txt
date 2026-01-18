cmake_minimum_required(VERSION 3.14)
project(TerrainRasterizer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Fetch Delaunay Triangulation library
FetchContent_Declare(
    delaunator
    GIT_REPOSITORY https://github.com/delfrrr/delaunator-cpp.git
    GIT_TAG        v0.4.0
)
FetchContent_MakeAvailable(delaunator)

# Find PROJ library
find_package(PROJ REQUIRED)

# Include directories
include_directories(include)
include_directories(${delaunator_SOURCE_DIR}/include)

# Create executable
add_executable(create_raster 
    src/main.cpp
    src/MNT.cpp
    src/triangulation.cpp
    src/quadtree.cpp
    src/rasterizer.cpp
)

# Link libraries
target_link_libraries(create_raster 
    PRIVATE 
    PROJ::proj
)