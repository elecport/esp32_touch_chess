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

} // namespace chess
