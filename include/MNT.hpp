#ifndef MNT_HPP
#define MNT_HPP

#include <string>
#include <vector>

/**
 * @struct Point
 * @brief Represents a point in 3D space.
 */
struct Point {
  double x; /**< X coordinate (e.g., longitude or projected X). */
  double y; /**< Y coordinate (e.g., latitude or projected Y). */
  double z; /**< Z coordinate (e.g., altitude). */
};

/**
 * @brief Reads terrain data from a file and converts coordinates.
 *
 * This function reads a text file containing geographic coordinates
 * (latitude, longitude, altitude) and converts them to a projected
 * coordinate system (Lambert93) using the PROJ library.
 *
 * @param nomFichier The path to the input data file.
 * @return std::vector<Point> A vector of projected 3D points.
 */
std::vector<Point> lireEtConvertir(const std::string &nomFichier);

#endif // MNT_HPP
