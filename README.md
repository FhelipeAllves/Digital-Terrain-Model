# Terrain Rasterizer

## Overview

**Terrain Rasterizer** is a C++ application designed to transform raw geographic terrain data (latitude, longitude, altitude) into a visual 2D raster image. The project simulates a top-down view of the terrain, applying color mapping based on altitude and 3D shading to create a realistic Digital Elevation Model (DEM) visualization.

## How It Works

The process is divided into a pipeline of distinct steps:

1.  **Data Loading & Projection**:
    The raw data usually comes in geographic coordinates (Latitude/Longitude). To visualize it on a flat 2D plane (like an image), we must project these spherical coordinates onto a flat surface. This project uses the **Lambert93** projection (standard for France) via the PROJ library.

2.  **Triangulation**:
    The projected points are scattered and do not form a solid surface. To create a continuous terrain, we connect these points using **Delaunay Triangulation**. This creates a "mesh" of non-overlapping triangles that covers the area.

3.  **Spatial Indexing (QuadTree)**:
    When generating the image, we check thousands or millions of pixels. For each pixel, we need to know "which triangle is underneath this?". Searching the entire list of triangles for every pixel is too slow. A **QuadTree** data structure is used to divide the space into smaller regions, making this search extremely fast.

4.  **Rasterization & Rendering**:
    The program iterates over every pixel in the output image:
    *   It finds the corresponding triangle.
    *   It calculates the exact altitude of that pixel using **Barycentric Interpolation** (a weighted average of the triangle's corners).
    *   It assigns a color based on altitude (using a Haxby color map).
    *   It calculates lighting/shading based on the slope of the triangle to verify depth perception.

## Project Structure

Here is a breakdown of the key files and their responsibilities:

*   **`src/main.cpp`**:
    The entry point of the application. It orchestrates the entire workflow: reading arguments, calling the loader, running triangulation, and triggering image generation.

*   **`src/MNT.cpp` (Modèle Numérique de Terrain)**:
    Handles data ingestion. It reads the input text file and uses the **PROJ** library to convert coordinates from WGS84 (Lat/Lon) to Lambert93 (X/Y meters).

*   **`src/triangulation.cpp`**:
    Contains the logic to build the mesh. It uses the `delaunator-cpp` library to perform the Delaunay triangulation on the cleaned data points.

*   **`src/quadtree.cpp`**:
    Implements the **QuadTree** data structure. This is an optimization engine. It recursively splits the 2D space into four quadrants (NW, NE, SW, SE) to store triangles, allowing for efficient spatial queries.

*   **`src/rasterizer.cpp`**:
    The rendering engine. It:
    *   Maps pixel coordinates to terrain coordinates.
    *   Performs **Barycentric Interpolation** to find the Z (altitude) value for any point inside a triangle.
    *   Applies a **Haxby color map** (Deep Blue -> Green -> Brown -> White) to represent elevation.
    *   Computes **Shading** (Simulated Light) based on the surface normal of each triangle to give a 3D relief effect.
    *   Saves the final result as a binary **PPM** image.

## Building the Project

This project uses **CMake**. Ensure you have CMake, a C++ compiler, and the **PROJ** library installed on your system.

### Prerequisites (Linux)
```bash
sudo apt-get install cmake g++ libproj-dev
```

### Compilation
Run the provided build script or use standard CMake commands:

```bash
# Using the helper script
./build.sh

# OR manually
mkdir build
cd build
cmake ..
make
```

## Usage

Run the executable `create_raster` with the path to your data file and the desired image width.

```bash
./build/create_raster <path_to_data_file> <image_width>
```

**Example:**
```bash
./build/create_raster data/terrain_data.txt 1000
```
This command will read `data/terrain_data.txt`, generate a 1000-pixel wide image, and save it as `output.ppm`.

## Output

The program produces a file named `output.ppm` in the working directory. A PPM (Portable Pixel Map) file can be opened by most image viewers (like GIMP, IrfanView, or standard Linux image viewers).
