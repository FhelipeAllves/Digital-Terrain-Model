/**
 * @file MNT.cpp
 * @brief Implementation of data loading and coordinate conversion.
 */

#include "MNT.hpp"
#include <cstdio>
#include <iostream>
#include <proj.h>

// Fonction qui va lire le fichier et convertir les données
std::vector<Point> lireEtConvertir(const std::string &nomFichier) {
  std::vector<Point> points;

  // Initialisation de PROJ
  // Source : EPSG:4326 (GPS classique en degrés : Lat, Lon)
  const char *src_desc = "EPSG:4326";
  // Cible : Lambert93 (EPSG:2154)
  const char *tgt_desc =
      "+proj=lcc +lat_1=49 +lat_2=44 +lat_0=46.5 +lon_0=3 +x_0=700000 "
      "+y_0=6600000 +ellps=GRS80 +towgs84=0,0,0,0,0,0,0 +units=m +no_defs";

  PJ_CONTEXT *C = proj_context_create();
  PJ *P = proj_create_crs_to_crs(C, src_desc, tgt_desc, NULL);

  if (P == 0) {
    std::cerr << "Erreur de création de la projection." << std::endl;
    return points;
  }

  // Normalisation pour s'assurer de l'ordre (Longitude, Latitude)
  PJ *P_norm = proj_normalize_for_visualization(C, P);
  if (P_norm) {
    proj_destroy(P);
    P = P_norm;
  }

  // Ouverture du fichier de données
  FILE *f = fopen(nomFichier.c_str(), "r");
  if (!f) {
    std::cerr << "Impossible d'ouvrir le fichier " << nomFichier << std::endl;
    proj_destroy(P);
    proj_context_destroy(C);
    return points;
  }

  // Boucle de lecture et transformation
  double lat, lon, alt;
  while (fscanf(f, "%lf %lf %lf", &lat, &lon, &alt) != EOF) {

    PJ_COORD c_in, c_out;

    // PROJ normalisé veut (Longitude, Latitude)
    c_in.lpzt.lam = lon;
    c_in.lpzt.phi = lat;
    c_in.lpzt.z = alt;
    c_in.lpzt.t = 0.0;

    // Transformation
    c_out = proj_trans(P, PJ_FWD, c_in);

    // Stockage du résultat transformé (x, y en mètres)
    points.push_back({c_out.xy.x, c_out.xy.y, alt});
  }

  // Nettoyage
  fclose(f);
  proj_destroy(P);
  proj_context_destroy(C);

  return points;
}