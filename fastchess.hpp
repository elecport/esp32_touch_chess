/*
Copyright (c) 2024 Andrey V. Skvortsov

This work is licensed under the terms of the MIT license.  
For a copy, see LICENSE file.
*/

#pragma once

#include "chess.hpp"
#include "fast-chess.h"

class ChessParty: public chess::Bot
{
public:
  ChessParty()
  {
    getInitialGame(&__game);
    __color = chess::Color_t::C_WHITE;
  }

  ~ChessParty() override = default;

  chess::Color_t color() const { return __color; }
  Game& game() { return __game; }

  void getBoard(char (&board)[8][8])
  {
    for (uint8_t rank=0; rank<8; rank++) {
      for (uint8_t file=0; file<8; file++) {
        board[rank][file] = getPieceChar( (FILES_BB[file] & RANKS_BB[7-rank]), &__game.position.board);
      }
    }
  }

private:

  static chess::Move_t toMove(Move move)
  {
    chess::Move_t res;

    res.from.col = chess::Column_t(uint8_t(getFile(getFrom(move))-'a'));
    res.from.row = chess::Row_t(uint8_t(getRank(getFrom(move))-'1'));

    res.to.col = chess::Column_t(uint8_t(getFile(getTo(move))-'a'));
    res.to.row = chess::Row_t(uint8_t(getRank(getTo(move))-'1'));

    return res;
  }

  chess::Color_t __color;
  Game __game;
  uint8_t __depth = 1;

  // Bot interface
public:
  bool isMoveValid(chess::Move_t move) override
  {
    Move *moves = new Move[MAX_BRANCHING_FACTOR];
    int moveCount = legalMoves(moves, &(__game.position), __game.position.toMove);
    for (int i=0; i<moveCount; ++i) {
      char str[4];
      moveStr(move, str);
;     Move pm = parseMove(str);
      if (pm == moves[i]) {
        delete [] moves;
        return true;
      }
    }
    return false;
  }

  chess::Move_t makeBotMove(uint8_t &percent) override
  {
    Move m = getAIMove(&__game, __depth, &percent);
    makeMove(&__game, m);
    return toMove(m);
  }

  void enterMove(chess::Move_t move) override
  {
    char str[4];
    moveStr(move, str);
    Move m = parseMove(str);
    makeMove(&__game, m);
  }

  bool partyEnded() override
  {
    return hasGameEnded(&__game.position);
  }

  bool getCell(
    chess::Column_t col, chess::Row_t row, chess::Piece_t& p, chess::Color_t& c
  ) override
  {
    char _c = getPieceChar(
      (FILES_BB[uint8_t(col)] & RANKS_BB[7-uint8_t(row)]), &__game.position.board
    );
    if (_c == 'P') {
      p = chess::Piece_t::P_PAWN;
      c = chess::Color_t::C_WHITE;
    } else if (_c == 'p') {
      p = chess::Piece_t::P_PAWN;
      c = chess::Color_t::C_BLACK;
    } else if (_c == 'R') {
      p = chess::Piece_t::P_ROOK;
      c = chess::Color_t::C_WHITE;
    } else if (_c == 'r') {
      p = chess::Piece_t::P_ROOK;
      c = chess::Color_t::C_BLACK;
    } else if (_c == 'N') {
      p = chess::Piece_t::P_KNIGHT;
      c = chess::Color_t::C_WHITE;
    } else if (_c == 'n') {
      p = chess::Piece_t::P_KNIGHT;
      c = chess::Color_t::C_BLACK;
    } else if (_c == 'B') {
      p = chess::Piece_t::P_BISHOP;
      c = chess::Color_t::C_WHITE;
    } else if (_c == 'b') {
      p = chess::Piece_t::P_BISHOP;
      c = chess::Color_t::C_BLACK;
    } else if (_c == 'Q') {
      p = chess::Piece_t::P_QUEEN;
      c = chess::Color_t::C_WHITE;
    } else if (_c == 'q') {
      p = chess::Piece_t::P_QUEEN;
      c = chess::Color_t::C_BLACK;
    } else if (_c == 'K') {
      p = chess::Piece_t::P_KING;
      c = chess::Color_t::C_WHITE;
    } else if (_c == 'k') {
      p = chess::Piece_t::P_KING;
      c = chess::Color_t::C_BLACK;
    }
    return _c!='.';
  }
};
