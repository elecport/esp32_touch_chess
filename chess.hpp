#pragma once

#include <inttypes.h>

namespace chess
{

enum class Piece_t: uint8_t
{
  P_PAWN = 0,
  P_ROOK = 1,
  P_KNIGHT = 2,
  P_BISHOP = 3,
  P_QUEEN = 4,
  P_KING = 5,
};

enum class Color_t: uint8_t
{
  C_WHITE = 0,
  C_BLACK = 1
};

enum class Row_t : uint8_t
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

enum class Column_t : uint8_t
{
  R_A = 0,
  R_B = 1,
  R_C = 2,
  R_D = 3,
  R_E = 4,
  R_F = 5,
  R_G = 6,
  R_H = 7
};

struct Cell_t
{
  Column_t col;
  Row_t row;
};

struct Move_t
{
  Cell_t from;
  Cell_t to;
};

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
  virtual Move_t makeBotMove() = 0;

  /**
   * @brief Enter next counterpart move
   */
  virtual void enterMove(Move_t) = 0;

  virtual bool partyEnded() = 0;

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
