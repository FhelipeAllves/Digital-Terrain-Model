#ifndef QUADTREE_HPP
#define QUADTREE_HPP

#include "triangulation.hpp"
#include <memory>
#include <optional>
#include <vector>

/**
 * @struct BoundingBox
 * @brief Represents a 2D axis-aligned bounding box.
 */
struct BoundingBox {
  double minX, minY, maxX, maxY;

  /**
   * @brief Checks if a point is within the bounding box.
   * @param x X coordinate of the point.
   * @param y Y coordinate of the point.
   * @return true if the point is inside, false otherwise.
   */
  bool contains(double x, double y) const;

  /**
   * @brief Checks if this bounding box intersects with another.
   * @param other The other bounding box.
   * @return true if they intersect, false otherwise.
   */
  bool intersects(const BoundingBox &other) const;
};

/**
 * @class QuadTree
 * @brief A recursive QuadTree structure for spatial indexing of triangles.
 *
 * Used to efficiently query which triangle covers a given (x, y) point on the
 * map.
 */
class QuadTree {
public:
  /**
   * @brief Constructs a QuadTree node.
   * @param bounds The spatial bounds of this node.
   * @param depth The current depth in the tree (default is 0).
   */
  QuadTree(const BoundingBox &bounds, int depth = 0);

  /**
   * @brief Inserts a triangle into the QuadTree.
   * @param triangle The triangle to insert.
   * @param points The complete list of points (needed to get vertex
   * coordinates).
   */
  void insert(const Triangle &triangle, const std::vector<Point> &points);

  /**
   * @brief Finds the triangle containing a specific point.
   * @param x X coordinate.
   * @param y Y coordinate.
   * @param points The complete list of points.
   * @return std::optional<Triangle> The triangle containing the point, or
   * std::nullopt.
   */
  std::optional<Triangle> find(double x, double y,
                               const std::vector<Point> &points) const;

private:
  BoundingBox bounds;
  int depth;
  static const int MAX_DEPTH = 10;
  static const int MAX_TRIANGLES = 1500; // Leaf capacity

  std::vector<Triangle> triangles;
  std::unique_ptr<QuadTree> children[4]; // NW, NE, SW, SE

  bool isLeaf() const;
  void subdivide();
};

#endif // QUADTREE_HPP
