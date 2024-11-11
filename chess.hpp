/*
Copyright (c) 2024 Andrey V. Skvortsov

This work is licensed under the terms of the MIT license.  
For a copy, see LICENSE file.
*/

/**
 * @file chess.hpp
 * @brief Chess-domain related datatypes
 * 
 * Abstract classes and eumeratons for the Chess game related terms
 */

#pragma once

#include <inttypes.h>

namespace chess
{

/**
 * @brief Chess piece type
 */
enum class Piece_t: uint8_t
{
  P_PAWN = 0,
  P_ROOK = 1,
  P_KNIGHT = 2,
  P_BISHOP = 3,
  P_QUEEN = 4,
  P_KING = 5,
};

/**
 * @brief Piece color type
 */
enum class Color_t: uint8_t
{
  C_WHITE = 0,
  C_BLACK = 1
};

/**
 * @brief Rank (row) of the board cells
 */
enum class Rank_t : uint8_t
{
  R_1 = 0,
  R_2 = 1,
  R_3 = 2,
  R_4 = 3,
  R_5 = 4,
  R_6 = 5,
  R_7 = 6,
  R_8 = 7
};

/**
 * @brief File (column) of the board cells
 */
enum class File_t : uint8_t
{
  F_A = 0,
  F_B = 1,
  F_C = 2,
  F_D = 3,
  F_E = 4,
  F_F = 5,
  F_G = 6,
  F_H = 7
};

/**
 * @brif Board cell coordinates
 */
struct Cell_t
{
  File_t col;
  Rank_t row;
};

/**
 * @brief Move of the piece coordinates
 */
struct Move_t
{
  Cell_t from;
  Cell_t to;
};

enum class End_t: uint8_t
{
  CHECKMATE,
  STALLMATE,
  INSUFFICIENT_MAT,
  MOVES_EXPIRED
};

/**
 * @brief Chess bot
 * 
 * Abstract class for chess bot
 */
class Bot
{
public:

  virtual ~Bot() = default;

  /**
   * @brief Test if move is valid
   */
  virtual bool isMoveValid(Move_t) = 0;

  /**
   * @brief Make bots next move
   */
  virtual Move_t makeBotMove(uint8_t &percent) = 0;

  /**
   * @brief Enter next counterpart move
   */
  virtual void enterMove(Move_t) = 0;

  /**
   * @brief 
   */
  virtual bool partyEnded(End_t&) = 0;

  virtual bool getCell(File_t, Rank_t, Piece_t&, Color_t&) = 0;

  static void moveStr(chess::Move_t move, char *str)
  {
    str[0] = char(uint8_t(move.from.col)+'a');
    str[1] = char(uint8_t(move.from.row)+'1');
    str[2] = char(uint8_t(move.to.col)+'a');
    str[3] = char(uint8_t(move.to.row)+'1');
  }
protected:
  Bot() = default;
};

} // namespace chess
