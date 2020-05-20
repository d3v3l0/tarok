/* Copyright 2020 Semantic Weights. All rights reserved. */

#include "src/state.h"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "open_spiel/spiel.h"
#include "src/cards.h"
#include "src/contracts.h"
#include "src/game.h"

namespace tarok {

// state definition
TarokState::TarokState(std::shared_ptr<const open_spiel::Game> game)
    : open_spiel::State(game),
      tarok_parent_game_(std::static_pointer_cast<const TarokGame>(game)),
      current_game_phase_(GamePhase::kCardDealing) {}

open_spiel::Player TarokState::CurrentPlayer() const {
  switch (current_game_phase_) {
    case GamePhase::kCardDealing:
      return open_spiel::kChancePlayerId;
    case GamePhase::kFinished:
      return open_spiel::kTerminalPlayerId;
    default:
      return current_player_;
  }
}

std::vector<open_spiel::Action> TarokState::LegalActions() const {
  switch (current_game_phase_) {
    case GamePhase::kCardDealing:
      // return a dummy action due to implicit stochasticity
      return {0};
    case GamePhase::kBidding:
      return LegalActionsInBidding();
    case GamePhase::kKingCalling:
      // todo: implement
      return {};
    case GamePhase::kTalonExchange:
      // todo: implement
      return {};
    case GamePhase::kTricksPlaying:
      // todo: implement
      return {};
    case GamePhase::kFinished:
      return {};
  }
}

std::vector<open_spiel::Action> TarokState::LegalActionsInBidding() const {
  // actions 1 - 12 correspond to contracts as returned by InitializeContracts()
  // respectively, action 0 means pass
  auto it = std::max_element(players_bids_.begin(), players_bids_.end());
  int max_bid = *it;
  int max_bid_player = it - players_bids_.begin();

  std::vector<open_spiel::Action> actions;
  if (num_players_ == 3)
    AddLegalActionsInBidding3(max_bid, max_bid_player, &actions);
  else
    AddLegalActionsInBidding4(max_bid, max_bid_player, &actions);
  return actions;
}

void TarokState::AddLegalActionsInBidding3(
    int max_bid, int max_bid_player,
    std::vector<open_spiel::Action>* result_actions) const {
  if (!AllButCurrentPlayerPassedBidding() ||
      (current_player_ == 2 && players_bids_.at(current_player_) == -1))
    // the last player can pass if no bidding has happened before
    // which results in a compulsory klop
    result_actions->push_back(0);

  for (const int& action : kBiddableContracts3) {
    if (action < max_bid) continue;
    if ((action > max_bid) ||
        (action == max_bid && current_player_ <= max_bid_player)) {
      result_actions->push_back(action);
    }
  }
}

void TarokState::AddLegalActionsInBidding4(
    int max_bid, int max_bid_player,
    std::vector<open_spiel::Action>* result_actions) const {
  if (current_player_ == 0 && players_bids_.at(current_player_) == -1 &&
      AllButCurrentPlayerPassedBidding())
    // it is possible to play klop or three and not possible to pass for
    // forehand in case all others have passed
    result_actions->insert(result_actions->end(), {1, 2});
  else if (!AllButCurrentPlayerPassedBidding())
    result_actions->push_back(0);

  for (const int& action : kBiddableContracts4) {
    if (action < max_bid) continue;
    if ((action > max_bid) ||
        (action == max_bid && current_player_ <= max_bid_player)) {
      result_actions->push_back(action);
    }
  }
}

std::string TarokState::ActionToString(open_spiel::Player player,
                                       open_spiel::Action action_id) const {
  switch (current_game_phase_) {
    case GamePhase::kCardDealing:
      // return a dummy action due to implicit stochasticity
      return "Deal cards";
    case GamePhase::kBidding: {
      if (action_id == 0)
        return "Pass";
      else
        return tarok_parent_game_->contracts_.at(action_id - 1).name;
    }
    case GamePhase::kKingCalling:
    case GamePhase::kTalonExchange:
    case GamePhase::kTricksPlaying:
      return tarok_parent_game_->card_deck_.at(action_id).ToString();
    case GamePhase::kFinished:
      return "";
  }
}

open_spiel::Action TarokState::StringToAction(
    open_spiel::Player player, const std::string& action_str) const {
  // todo: implement
  return 0;
}

std::string TarokState::ToString() const {
  // todo: implement
  return "";
}

bool TarokState::IsTerminal() const {
  return current_game_phase_ == GamePhase::kFinished;
}

std::vector<double> TarokState::Returns() const {
  // todo: implement
  return {};
}

std::string TarokState::InformationStateString(
    open_spiel::Player player) const {
  // todo: implement
  return "";
}

std::unique_ptr<open_spiel::State> TarokState::Clone() const {
  // todo: implement
  return nullptr;
}

open_spiel::ActionsAndProbs TarokState::ChanceOutcomes() const {
  if (current_game_phase_ == GamePhase::kCardDealing)
    // return a dummy action with probability 1 due to implicit stochasticity
    return {{0, 1.0}};
  return {};
}

GamePhase TarokState::CurrentGamePhase() const { return current_game_phase_; }

std::vector<std::string> TarokState::Talon() const {
  std::vector<std::string> talon;
  talon.reserve(talon_.size());

  for (const int& card_index : talon_) {
    talon.push_back(tarok_parent_game_->card_deck_.at(card_index).ToString());
  }
  return talon;
}

std::vector<std::string> TarokState::PlayerCards(
    open_spiel::Player player) const {
  if (current_game_phase_ == GamePhase::kCardDealing) return {};

  std::vector<std::string> player_cards;
  player_cards.reserve(players_cards_.at(player).size());

  for (const int& card_index : players_cards_.at(player)) {
    player_cards.push_back(
        tarok_parent_game_->card_deck_.at(card_index).ToString());
  }
  return player_cards;
}

void TarokState::DoApplyAction(open_spiel::Action action_id) {
  // todo: we should probably check that action is legal for the current player
  switch (current_game_phase_) {
    case GamePhase::kCardDealing:
      DoApplyActionInCardDealing();
      break;
    case GamePhase::kBidding:
      DoApplyActionInBidding(action_id);
      break;
    case GamePhase::kKingCalling:
      // todo: implement
      break;
    case GamePhase::kTalonExchange:
      // todo: implement
      break;
    case GamePhase::kTricksPlaying:
      // todo: implement
      break;
    case GamePhase::kFinished:
      open_spiel::SpielFatalError("Calling DoApplyAction on a terminal state.");
  }
}

void TarokState::DoApplyActionInCardDealing() {
  // do the actual sampling here due to implicit stochasticity
  // todo: deal again if any player without taroks and play compulsory klop
  std::tie(talon_, players_cards_) =
      DealCards(num_players_, tarok_parent_game_->RNG());

  current_game_phase_ = GamePhase::kBidding;
  players_bids_.reserve(num_players_);
  players_bids_.insert(players_bids_.end(), num_players_, -1);

  // lower player indices correspond to higher bidding priority,
  // i.e. 0 is the forehand, num_players - 1 is the dealer
  if (num_players_ == 3)
    current_player_ = 0;
  else
    // forehand has to wait
    current_player_ = 1;
}

void TarokState::DoApplyActionInBidding(open_spiel::Action action_id) {
  players_bids_.at(current_player_) = action_id;

  if (AllButCurrentPlayerPassedBidding()) {
    current_game_phase_ = GamePhase::kTalonExchange;
    // todo set correct game phase
    // todo set declearer
    // todo set contract
    // todo set current player
  } else {
    do {
      NextPlayer();
    } while (players_bids_.at(current_player_) == 0);
  }
}

bool TarokState::AllButCurrentPlayerPassedBidding() const {
  for (int i = 0; i < num_players_; i++) {
    if (i == current_player_) continue;
    if (players_bids_.at(i) != 0) return false;
  }
  return true;
}

void TarokState::NextPlayer() {
  current_player_ += 1;
  if (current_player_ == num_players_) current_player_ = 0;
}

}  // namespace tarok
