/* Copyright 2020 Semantic Weights. All rights reserved. */

#include "open_spiel/spiel.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "src/game.h"
#include "src/state.h"

namespace tarok {

namespace py = pybind11;

PYBIND11_MODULE(pytarok, m) {
  py::module::import("pyspiel");

  // game object
  py::class_<TarokGame, open_spiel::Game, std::shared_ptr<TarokGame>>
      tarok_game(m, "TarokGame");

  tarok_game.def(py::init([](const open_spiel::GameParameters& params) {
    // instantiate the game instance via a shared_ptr
    // (see Game declaration comments in open_spiel/spiel.h)
    return std::shared_ptr<TarokGame>(new TarokGame(params));
  }));

  tarok_game.def("new_initial_tarok_state", &TarokGame::NewInitialTarokState);

  // state object
  py::class_<TarokState, open_spiel::State> tarok_state(m, "TarokState");
}

}  // namespace tarok
