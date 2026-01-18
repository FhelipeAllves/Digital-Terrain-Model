/**
 * @file rasterizer.cpp
 * @brief Implementation of the rasterization process (coloring, shading, and
 * image generation).
 */

#include "rasterizer.hpp"
#include "quadtree.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>

struct Color {
  unsigned char r, g, b;
};

/**
 * @brief Maps an altitude to a color using a Haxby-like colormap.
 * @param z Current altitude.
 * @param minZ Minimum altitude in the dataset.
 * @param maxZ Maximum altitude in the dataset.
 * @return Color The corresponding RGB color.
 */
Color getColor(double z, double minZ, double maxZ) {
  // Normalize z to [0, 1]
  double t = (z - minZ) / (maxZ - minZ);
  if (t < 0)
    t = 0;
  if (t > 1)
    t = 1;

  // Haxby-like colors (approximate)
  // Deep Blue -> Blue -> Cyan -> Green -> Yellow -> Orange -> Red -> White
  struct Stop {
    double t;
    Color c;
  };
  Stop stops[] = {
      {0.00, {0, 0, 128}},    // Deep Blue
      {0.10, {0, 0, 255}},    // Blue
      {0.25, {0, 255, 255}},  // Cyan
      {0.40, {0, 255, 0}},    // Green
      {0.60, {255, 255, 0}},  // Yellow
      {0.80, {255, 128, 0}},  // Orange
      {0.95, {255, 0, 0}},    // Red
      {1.00, {255, 255, 255}} // White
  };

  for (size_t i = 0; i < 7; ++i) {
    if (t >= stops[i].t && t <= stops[i + 1].t) {
      double localT = (t - stops[i].t) / (stops[i + 1].t - stops[i].t);
      return {(unsigned char)(stops[i].c.r +
                              localT * (stops[i + 1].c.r - stops[i].c.r)),
              (unsigned char)(stops[i].c.g +
                              localT * (stops[i + 1].c.g - stops[i].c.g)),
              (unsigned char)(stops[i].c.b +
                              localT * (stops[i + 1].c.b - stops[i].c.b))};
    }
  }
  return stops[7].c;
}

/**
 * @brief Computes the Z coordinate at point (px, py) within a triangle using
 * barycentric interpolation.
 * @param px X coordinate of the target point.
 * @param py Y coordinate of the target point.
 * @param p1 First vertex of the triangle.
 * @param p2 Second vertex of the triangle.
 * @param p3 Third vertex of the triangle.
 * @return double The interpolated altitude (Z).
 */
double interpolateZ(double px, double py, const Point &p1, const Point &p2,
                    const Point &p3) {
  double det = (p2.y - p3.y) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.y - p3.y);
  double lambda1 =
      ((p2.y - p3.y) * (px - p3.x) + (p3.x - p2.x) * (py - p3.y)) / det;
  double lambda2 =
      ((p3.y - p1.y) * (px - p3.x) + (p1.x - p3.x) * (py - p3.y)) / det;
  double lambda3 = 1.0 - lambda1 - lambda2;
  return lambda1 * p1.z + lambda2 * p2.z + lambda3 * p3.z;
}

/**
 * @brief Calculates a shading factor based on the triangle's normal and a fixed
 * light source.
 *
 * Computes the normal vector of the triangle (cross product of edges) and takes
 * the dot product with a fixed light direction (from NW).
 *
 * @param p1 First vertex.
 * @param p2 Second vertex.
 * @param p3 Third vertex.
 * @return double Shading factor (0.4 to 1.0).
 */
double calculateShade(const Point &p1, const Point &p2, const Point &p3) {
  // Vectors U = p2 - p1, V = p3 - p1
  double ux = p2.x - p1.x;
  double uy = p2.y - p1.y;
  double uz = p2.z - p1.z;

  double vx = p3.x - p1.x;
  double vy = p3.y - p1.y;
  double vz = p3.z - p1.z;

  // Normal N = U x V
  double nx = uy * vz - uz * vy;
  double ny = uz * vx - ux * vz;
  double nz = ux * vy - uy * vx;

  // Normalize Normal
  double lenN = std::sqrt(nx * nx + ny * ny + nz * nz);
  if (lenN > 0) {
    nx /= lenN;
    ny /= lenN;
    nz /= lenN;
  }

  // Light direction (Azimuth 315 deg (NW), Elevation 45 deg)
  // Converted to vector
  // x = cos(45) * sin(315) = 0.707 * -0.707 = -0.5
  // y = cos(45) * cos(315) = 0.707 * 0.707 = 0.5
  // z = sin(45) = 0.707
  // Let's approximate: Light coming from top-left-up
  double lx = -0.5;
  double ly = 0.5;
  double lz = 0.7;

  // Normalize Light
  double lenL = std::sqrt(lx * lx + ly * ly + lz * lz);
  lx /= lenL;
  ly /= lenL;
  lz /= lenL;

  // Dot product
  double intensity = nx * lx + ny * ly + nz * lz;

  // Clamp and scale
  if (intensity < 0)
    intensity = 0;

  // Ambient light
  return 0.4 + 0.6 * intensity;
}

void generateImage(const std::string &filename, int width, const Mesh &mesh) {
  if (mesh.points.empty())
    return;

  // Calculate Bounding Box of the whole mesh
  double minX = std::numeric_limits<double>::max();
  double maxX = std::numeric_limits<double>::lowest();
  double minY = std::numeric_limits<double>::max();
  double maxY = std::numeric_limits<double>::lowest();
  double minZ = std::numeric_limits<double>::max();
  double maxZ = std::numeric_limits<double>::lowest();

  for (const auto &p : mesh.points) {
    if (p.x < minX)
      minX = p.x;
    if (p.x > maxX)
      maxX = p.x;
    if (p.y < minY)
      minY = p.y;
    if (p.y > maxY)
      maxY = p.y;
    if (p.z < minZ)
      minZ = p.z;
    if (p.z > maxZ)
      maxZ = p.z;
  }

  // Build QuadTree
  std::cout << "Construction de QuadTree..." << std::endl;
  BoundingBox rootBounds{minX, minY, maxX, maxY};
  QuadTree quadTree(rootBounds);
  for (const auto &t : mesh.triangles) {
    quadTree.insert(t, mesh.points);
  }
  std::cout << "QuadTree construit." << std::endl;

  // Determine Image Dimensions
  double rangeX = maxX - minX;
  double rangeY = maxY - minY;

  if (rangeX <= 0 || rangeY <= 0) {
    std::cerr << "Dimensions du maillage invalides." << std::endl;
    return;
  }

  int height = static_cast<int>(width * (rangeY / rangeX));
  std::cout << "Générer une image " << width << "x" << height << std::endl;

  // Rasterization Loop
  std::vector<unsigned char> pixels;
  pixels.reserve(width * height * 3);

  double pixelSizeX = rangeX / width;
  double pixelSizeY = rangeY / height;

  for (int row = 0; row < height; ++row) {
    double y = maxY - (row + 0.5) * pixelSizeY;

    if (row % 100 == 0)
      std::cout << "Ligne de traitement " << row << "/" << height << "\r"
                << std::flush;

    for (int col = 0; col < width; ++col) {
      double x = minX + (col + 0.5) * pixelSizeX;

      auto triangleOpt = quadTree.find(x, y, mesh.points);

      Color c = {0, 0, 0};

      if (triangleOpt) {
        const Triangle &t = *triangleOpt;
        double z = interpolateZ(x, y, mesh.points[t.p1], mesh.points[t.p2],
                                mesh.points[t.p3]);
        c = getColor(z, minZ, maxZ);

        // Apply shading
        double shade = calculateShade(mesh.points[t.p1], mesh.points[t.p2],
                                      mesh.points[t.p3]);
        c.r = static_cast<unsigned char>(std::min(255.0, c.r * shade));
        c.g = static_cast<unsigned char>(std::min(255.0, c.g * shade));
        c.b = static_cast<unsigned char>(std::min(255.0, c.b * shade));
      }

      pixels.push_back(c.r);
      pixels.push_back(c.g);
      pixels.push_back(c.b);
    }
  }
  std::cout << std::endl;

  // Write PPM
  std::ofstream ofs(filename, std::ios::binary);
  ofs << "P6\n" << width << " " << height << "\n255\n";
  ofs.write(reinterpret_cast<const char *>(pixels.data()), pixels.size());
  ofs.close();
  std::cout << "Image enregistrée dans " << filename << std::endl;
}
