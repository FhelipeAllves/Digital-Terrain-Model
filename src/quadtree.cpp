/**
 * @file quadtree.cpp
 * @brief Implementation of the QuadTree for spatial indexing.
 */

#include "quadtree.hpp"
#include <algorithm>
#include <iostream>

bool BoundingBox::contains(double x, double y) const {
  return x >= minX && x <= maxX && y >= minY && y <= maxY;
}

bool BoundingBox::intersects(const BoundingBox &other) const {
  return !(other.minX > maxX || other.maxX < minX || other.minY > maxY ||
           other.maxY < minY);
}

/**
 * @brief Computes the bounding box of a single triangle.
 * @param t The triangle.
 * @param points The list of vertex points.
 * @return BoundingBox The minimal bounding box containing the triangle.
 */
BoundingBox getTriangleBounds(const Triangle &t,
                              const std::vector<Point> &points) {
  const Point &p1 = points[t.p1];
  const Point &p2 = points[t.p2];
  const Point &p3 = points[t.p3];

  return {std::min({p1.x, p2.x, p3.x}), std::min({p1.y, p2.y, p3.y}),
          std::max({p1.x, p2.x, p3.x}), std::max({p1.y, p2.y, p3.y})};
}

/**
 * @brief Checks if a 2D point lies inside a 2D triangle using barycentric
 * coordinates.
 * @param px X coordinate of the point.
 * @param py Y coordinate of the point.
 * @param p1 First vertex of the triangle.
 * @param p2 Second vertex of the triangle.
 * @param p3 Third vertex of the triangle.
 * @return true if the point is inside or on the edge, false otherwise.
 */
bool isPointInTriangle(double px, double py, const Point &p1, const Point &p2,
                       const Point &p3) {
  double area = 0.5 * (-p2.y * p3.x + p1.y * (-p2.x + p3.x) +
                       p1.x * (p2.y - p3.y) + p2.x * p3.y);
  double s =
      1.0 / (2.0 * area) *
      (p1.y * p3.x - p1.x * p3.y + (p3.y - p1.y) * px + (p1.x - p3.x) * py);
  double t =
      1.0 / (2.0 * area) *
      (p1.x * p2.y - p1.y * p2.x + (p1.y - p2.y) * px + (p2.x - p1.x) * py);
  return s >= 0 && t >= 0 && (1 - s - t) >= 0;
}

QuadTree::QuadTree(const BoundingBox &bounds, int depth)
    : bounds(bounds), depth(depth) {}

bool QuadTree::isLeaf() const { return children[0] == nullptr; }

void QuadTree::subdivide() {
  double midX = (bounds.minX + bounds.maxX) / 2.0;
  double midY = (bounds.minY + bounds.maxY) / 2.0;

  // Create 4 children
  children[0] = std::make_unique<QuadTree>(
      BoundingBox{bounds.minX, midY, midX, bounds.maxY}, depth + 1); // NW
  children[1] = std::make_unique<QuadTree>(
      BoundingBox{midX, midY, bounds.maxX, bounds.maxY}, depth + 1); // NE
  children[2] = std::make_unique<QuadTree>(
      BoundingBox{bounds.minX, bounds.minY, midX, midY}, depth + 1); // SW
  children[3] = std::make_unique<QuadTree>(
      BoundingBox{midX, bounds.minY, bounds.maxX, midY}, depth + 1); // SE
}

void QuadTree::insert(const Triangle &triangle,
                      const std::vector<Point> &points) {
  // If triangle is not in this node bounds, ignore
  BoundingBox tBounds = getTriangleBounds(triangle, points);
  if (!bounds.intersects(tBounds)) {
    return;
  }

  // If leaf and has room, or max depth reached
  if (isLeaf() && (triangles.size() < MAX_TRIANGLES || depth >= MAX_DEPTH)) {
    triangles.push_back(triangle);
  } else {
    if (isLeaf()) {
      subdivide();
      // Move existing triangles to children
      for (const auto &t : triangles) {
        for (auto &child : children) {
          child->insert(t, points);
        }
      }
      triangles.clear();
    }
    // Insert new triangle to children
    for (auto &child : children) {
      child->insert(triangle, points);
    }
  }
}

std::optional<Triangle> QuadTree::find(double x, double y,
                                       const std::vector<Point> &points) const {
  if (!bounds.contains(x, y)) {
    return std::nullopt;
  }

  if (isLeaf()) {
    for (const auto &t : triangles) {
      if (isPointInTriangle(x, y, points[t.p1], points[t.p2], points[t.p3])) {
        return t;
      }
    }
    return std::nullopt;
  }

  // Check children
  // Optimization: check which child contains the point
  double midX = (bounds.minX + bounds.maxX) / 2.0;
  double midY = (bounds.minY + bounds.maxY) / 2.0;

  if (x <= midX) {
    if (y >= midY)
      return children[0]->find(x, y, points); // NW
    else
      return children[2]->find(x, y, points); // SW
  } else {
    if (y >= midY)
      return children[1]->find(x, y, points); // NE
    else
      return children[3]->find(x, y, points); // SE
  }
}
