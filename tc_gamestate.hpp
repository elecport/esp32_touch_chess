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

  /**
   * @brief Set player for given color: human, bot, etc.
   * 
   * @param color
   * @param player
   */
  void setPlayer(chess::Color_t, PlayerClass_t);

  /**
   * @brief Load game from file
   * 
   * @param save file
   */
  void loadGame(const char*);

private:

  struct task_parameters_t
  {
    uint8_t percent;
    chess::Bot* bot;
    chess::Move_t mov;
    bool finished;
  };

  const char* __file2load;

  static void __aiMoveTask(void* p);

  int8_t _getSaveSlot();

  /**
   * @brief Redraw empty board
   */
  void __drawBoard();

  /**
   * @brief Draw current figures set
   */
  void __drawFigures();

  /**
   * #brief Get human player move through touch
   */
  chess::Move_t __getMove();

  /**
   * @brief Read possible menu buttons presses
   */
  void __getMenuPress();

  void __addMove(chess::Move_t&);

  Player* __players[2];

  PlayerClass_t __player_classes[2];

  ChessParty* __chessParty;

  chess::Color_t __color;

  chess::Move_t __lastMove;

  uint16_t __gfx_colors[2];
  // Moves list
  chess::Move_t** __moves;

  size_t __movesCount;

// State interface
public:

  void enter() override;

  State_t step(unsigned current_time) override;
};

} // namespace touch_chess
