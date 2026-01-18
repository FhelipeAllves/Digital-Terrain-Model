#ifndef RASTERIZER_HPP
#define RASTERIZER_HPP

#include "triangulation.hpp"
#include <string>

/**
 * @brief Generates a colorized raster image (PPM) from the triangulated mesh.
 *
 * Rasterizes the mesh into a PPM image of the specified width. The height is
 * calculated automatically to maintain aspect ratio. Altitude is visualized
 * using a color map.
 *
 * @param filename The output filename (e.g., "output.ppm").
 * @param width The desired width of the output image in pixels.
 * @param mesh The triangulated mesh to rasterize.
 */
void generateImage(const std::string &filename, int width, const Mesh &mesh);

#endif // RASTERIZER_HPP
