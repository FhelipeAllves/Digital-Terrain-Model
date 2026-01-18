#ifndef TRIANGULATION_HPP
#define TRIANGULATION_HPP

#include "MNT.hpp"
#include <cstddef>
#include <vector>

/**
 * @struct Triangle
 * @brief Represents a triangle defined by three indices into a point list.
 */
struct Triangle {
  std::size_t p1, p2, p3; /**< Indices of the three vertices. */
};

/**
 * @struct Mesh
 * @brief Represents a 3D mesh consisting of points and triangles.
 */
struct Mesh {
  std::vector<Point> points; /**< List of vertices in the mesh. */
  std::vector<Triangle>
      triangles; /**< List of triangles connecting the vertices. */
};

/**
 * @brief Performs Delaunay triangulation on a set of 2D points.
 *
 * Uses the delaunator-cpp library to generate a triangulation from the
 * projected X and Y coordinates of the input points. The Z coordinate is
 * preserved in the resulting mesh.
 *
 * @param points The vector of input points to triangulate.
 * @return Mesh The resulting triangular mesh containing points and triangles.
 */
Mesh triangulate(const std::vector<Point> &points);

#endif // TRIANGULATION_HPP
