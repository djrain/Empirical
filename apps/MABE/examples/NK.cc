/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  NK.cc
 *  @brief A simple NK landscape with no brains in the genomes.
 */

#include "../core/World.h"
#include "../core/OrganismType.h"

#include "genomes/BitVectorGenome.h"
#include "environments/NKLandscape.h"
#include "schemas/TournamentSelect.h"

int main(int argc, char * argv[]) {
  // Build the world using these types.
  mabe::World<mabe::NKLandscape> world("NKWorld");

  using org_type_t = mabe::OrganismType<mabe::BitVectorGenome>;
  using org_t = org_type_t::Organism;

  world.AddOrgType<org_type_t>("Organisms");
  world.AddSchema<mabe::TournamentSelect>("TournamentSelect");

  // Configure the world using the "Aagos.cfg" file and command-line overrides.
  world.Config(argc, argv, "NK.cfg");
  
  // Get details on how the population was configured.
  world.PrintStatus();

  // Running world will go for full configured duration; can also manually Update() it.
  return world.Run();
}

