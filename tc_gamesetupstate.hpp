/*
Copyright (c) 2024 Andrey V. Skvortsov

This work is licensed under the terms of the MIT license.  
For a copy, see LICENSE file.
*/

#pragma once

#include "tc_state.hpp"

namespace touch_chess
{

class ChessGame;

class GameSetup: public touch_chess::State
{
public:

  GameSetup(ChessGame&);

  ~GameSetup() = default;

private:
  bool __playersHumanFlag[2];

  ChessGame& __chess_game_state;

// Interface of the State
  void enter() override;

  State_t step(unsigned current_time) override;
};

} // namespace touch_chess
