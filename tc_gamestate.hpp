/*
Copyright (c) 2024 Andrey V. Skvortsov

This work is licensed under the terms of the MIT license.  
For a copy, see LICENSE file.
*/

#pragma once

#include "tc_state.hpp"
#include "chess.hpp"
#include "fastchess.hpp"

namespace touch_chess
{

enum class Player_t: uint8_t
{
  AI = 1,
  HUMAN = 0,
};

class Player
{
public:
  Player_t getType() const
  {
    return this->__type;
  }
protected:
  Player(const char* name, Player_t type):
  __type(type)
  {
    strncpy(this->__name, name, 16);
  }
private:
  Player_t __type;
  char __name[17];
};

class Human: public Player
{
public:
  Human(const char* name):
  Player(name, Player_t::HUMAN)
  {
  }
};

class AIPlayer: public Player
{
public:
  AIPlayer(const char* name, uint8_t depth):
  Player(name, Player_t::AI), __depth(depth)
  {
  }
  uint8_t getDepth() const
  {
    return this->__depth;
  }
private:
  uint8_t __depth;
};

class ChessGame: public State
{
public:

  ChessGame();

  void setPlayer(chess::Color_t color, PlayerClass_t player);

  void enter() override;

  State_t step(unsigned current_time) override;

private:

  struct task_parameters_t
  {
    uint8_t percent;
    chess::Bot* bot;
    chess::Move_t mov;
    bool finished;
  };

  static void __aiMoveTask(void* p);

  uint8_t _getSaveSlot();

  void drawBoard();

  void drawFigures();

  chess::Move_t __getMove();

  void __addMove(chess::Move_t&);

  Player* __players[2];

  ChessParty* __chessParty;

  chess::Color_t __color;

  chess::Move_t __lastMove;

  uint16_t __gfx_colors[2];
  // Moves list
  chess::Move_t** __moves;

  size_t __movesCount;
};

} // namespace touch_chess
