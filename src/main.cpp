/**
 * @file main.cpp
 * @brief Main entry point for the terrain rasterizer application.
 *
 * Orchestrates the pipeline: loading data, triangulation, spatial indexing,
 * and rasterization.
 */

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "MNT.hpp"
#include "rasterizer.hpp"
#include "triangulation.hpp"

int main(int argc, char *argv[]) {
  // Vérification des arguments
  if (argc < 3) {
    std::cerr << "Usage: ./create_raster <fichier_donnees> <largeur_image>"
              << std::endl;
    return EXIT_FAILURE;
  }

  std::string nomFichier = argv[1];
  int largeur = std::atoi(argv[2]);

  // Appel de la fonction de conversion
  std::cout << "Lecture et projection des données..." << std::endl;
  auto terrain = lireEtConvertir(nomFichier);

  std::cout << "Nombre de points chargés : " << terrain.size() << std::endl;

  if (!terrain.empty()) {
    std::cout << "Premier point (projeté) : x=" << terrain[0].x
              << ", y=" << terrain[0].y << ", z=" << terrain[0].z << std::endl;

    // Triangulation
    std::cout << "Lancement de la triangulation..." << std::endl;
    Mesh mesh = triangulate(terrain);
    std::cout << "Triangulation terminée." << std::endl;

    // Rasterization
    std::cout << "Génération de l'image..." << std::endl;
    generateImage("output.ppm", largeur, mesh);
  }

  return EXIT_SUCCESS;
}