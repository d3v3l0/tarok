/* Copyright 2020 Semantic Weights. All rights reserved. */

#include "src/game.h"

#include <memory>

#include "open_spiel/spiel.h"
#include "src/state.h"

namespace tarok {

// game facts
const open_spiel::GameType kGameType{
    "tarok",            // short_name
    "Slovenian Tarok",  // long_name
    open_spiel::GameType::Dynamics::kSequential,
    open_spiel::GameType::ChanceMode::kSampledStochastic,
    open_spiel::GameType::Information::kImperfectInformation,
    open_spiel::GameType::Utility::kGeneralSum,
    open_spiel::GameType::RewardModel::kTerminal,
    3,      // max_num_players
    3,      // min_num_players
    true,   // provides_information_state_string
    false,  // provides_information_state_tensor
    false,  // provides_observation_string
    false,  // provides_observation_tensor
    // parameter_specification
    {{"num_players", open_spiel::GameParameter(kDefaultNumPLayers)},
     {"rng_seed", open_spiel::GameParameter(kDefaultRngSeed)}}};

// game definition
TarokGame::TarokGame(const open_spiel::GameParameters& params)
    : Game(kGameType, params),
      num_players_(this->ParameterValue<int>("num_players")),
      rng_seed_(this->ParameterValue<int>("rng_seed")) {}

int TarokGame::NumDistinctActions() const { return 0; }

std::unique_ptr<open_spiel::State> TarokGame::NewInitialState() const {
  return std::unique_ptr<open_spiel::State>(new TarokState(shared_from_this()));
}

int TarokGame::NumPlayers() const { return num_players_; }

double TarokGame::MinUtility() const { return 0.0; }

double TarokGame::MaxUtility() const { return 0.0; }

std::shared_ptr<const open_spiel::Game> TarokGame::Clone() const {
  return nullptr;
}

int TarokGame::MaxGameLength() const { return 0; }

std::shared_ptr<const open_spiel::Game> NewTarokGame(
    const open_spiel::GameParameters& params) {
  return std::shared_ptr<const open_spiel::Game>(new TarokGame(params));
}

}  // namespace tarok
