/**
 * @file triangulation.cpp
 * @brief Implementation of Delaunay triangulation using delaunator-cpp.
 */

#include "triangulation.hpp"
#include <cmath>
#include <delaunator.hpp>
#include <iostream>

/**
 * @brief Calculates the squared Euclidean distance between two points.
 *
 * Used to avoid square root calculations when comparing distances.
 *
 * @param a First point.
 * @param b Second point.
 * @return double Squared distance between a and b.
 */
double distSq(const Point &a, const Point &b) {
  return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}

Mesh triangulate(const std::vector<Point> &points) {
  Mesh mesh;
  mesh.points = points;

  // Préparation pour Delaunator
  std::vector<double> coords;
  coords.reserve(points.size() * 2);
  for (const auto &p : points) {
    coords.push_back(p.x);
    coords.push_back(p.y);
  }

  // Exécution de Delaunay
  delaunator::Delaunator d(coords);

  // Filtrage des triangles trop grands
  // Seuil : Si un côté du triangle fait plus de X mètres, on le jette.
  const double MAX_EDGE_LENGTH = 70.0;
  const double MAX_DIST_SQ = MAX_EDGE_LENGTH * MAX_EDGE_LENGTH;

  int trianglesRejetes = 0;

  for (std::size_t i = 0; i < d.triangles.size(); i += 3) {
    std::size_t idx0 = d.triangles[i];
    std::size_t idx1 = d.triangles[i + 1];
    std::size_t idx2 = d.triangles[i + 2];

    const Point &p0 = points[idx0];
    const Point &p1 = points[idx1];
    const Point &p2 = points[idx2];

    // Vérifier la longueur des 3 côtés
    if (distSq(p0, p1) > MAX_DIST_SQ || distSq(p1, p2) > MAX_DIST_SQ ||
        distSq(p2, p0) > MAX_DIST_SQ) {

      trianglesRejetes++;
      continue; // Ce triangle est trop grand, il ne l'ajoute pas !
    }

    // Si le triangle est valide, il l'ajoute
    mesh.triangles.push_back({idx0, idx1, idx2});
  }

  std::cout << "Triangulation terminée." << std::endl;
  std::cout << "  Triangles gardés  : " << mesh.triangles.size() << std::endl;
  std::cout << "  Triangles rejetés : " << trianglesRejetes << " (trop longs)"
            << std::endl;

  return mesh;
}