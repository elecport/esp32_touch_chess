/*
Copyright (c) 2024 Andrey V. Skvortsov

This work is licensed under the terms of the MIT license.  
For a copy, see LICENSE file.
*/

#pragma once

#include "tc_state.hpp"
#include "chess.hpp"
#include "fastchess.hpp"

// Size of the chess board cell in pixels
#define BCELL_SIZE 27

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

  void drawBoard()
  {
    for (int i=0; i<8; ++i) {
      _tft->drawChar(22+BCELL_SIZE*i, 33, char('a'+i), __gfx_colors[0], __gfx_colors[1], 1);
      _tft->drawChar(22+BCELL_SIZE*i, 260, char('a'+i), __gfx_colors[0], __gfx_colors[1], 1);

      _tft->drawChar(5,55+BCELL_SIZE*i, char('8'-i), __gfx_colors[0], __gfx_colors[1], 1);
      _tft->drawChar(232,55+BCELL_SIZE*i, char('8'-i), __gfx_colors[0], __gfx_colors[1], 1);
    }
    for (int i=0; i<8; ++i) {
      int flag = i%2?1:0;
      for (int j=0; j<8; ++j){
        flag = 1-flag;
        _tft->fillRect(12+j*BCELL_SIZE, 42+i*BCELL_SIZE, BCELL_SIZE, BCELL_SIZE, flag?__gfx_colors[0]:__gfx_colors[1]);
      }
    }
    _tft->drawRect(0,30, 240, 240, __gfx_colors[0]);
    _tft->drawRect(12,43, 216, 216, __gfx_colors[0]);
  }

  void drawFigures()
  {
    char board[8][8];
    __chessParty->getBoard(board);

    //_tft->setFont(&FreeMonoBold9pt7b);
    for (uint8_t i=0; i<8; ++i) {
      int flag = i%2;
      for (uint8_t j=0; j<8; ++j) {
        flag = 1-flag;
        if (board[i][j] == '.')
          continue;
        else {
          uint16_t fg; uint8_t fill_index;
          uint8_t figure_index;
          if (flag == 1) {
            fg = this->__gfx_colors[1];
            fill_index = 0;
          } else {
            fg = this->__gfx_colors[0];
            fill_index = 1;
          }
          if (board[i][j] >= 'a') {
            figure_index = board[i][j] - 'a'+'A';
            fill_index = 1-fill_index;
          } else
            figure_index = board[i][j];
            
          switch (figure_index) {
            case 'P': figure_index = uint8_t(chess::Piece_t::P_PAWN); break;
            case 'R': figure_index = uint8_t(chess::Piece_t::P_ROOK); break;
            case 'N': figure_index = uint8_t(chess::Piece_t::P_KNIGHT); break;
            case 'B': figure_index = uint8_t(chess::Piece_t::P_BISHOP); break;
            case 'Q': figure_index = uint8_t(chess::Piece_t::P_QUEEN); break;
            case 'K': figure_index = uint8_t(chess::Piece_t::P_KING); break;
            default: figure_index = 0;
          }
          _tft->drawBitmap(14+BCELL_SIZE*j, 44+BCELL_SIZE*i, pieces_bmps[figure_index][fill_index], 24, 24, fg);
        }
      }
    }
  }

  chess::Move_t __getMove()
  {
    chess::Move_t result;
    int file_from, rank_from;
    int file_to, rank_to;
    // Get first cell
    while (true) {
      TS_Point p;
      if (getTouch(p.x, p.y)) {
        file_from = (p.x - 12) / BCELL_SIZE;
        rank_from = 7 - (p.y - 42 - 13) / BCELL_SIZE;
        if (file_from >= 0 && file_from < 8 && rank_from >= 0 && rank_from < 8) {
          chess::Piece_t p; chess::Color_t c;
          if (__chessParty->getCell(chess::Column_t(file_from), chess::Row_t(7-rank_from), p, c)) {
            if (c == __color) {
              _tft->fillScreen(TFT_BLACK);
              drawBoard();
              drawFigures();
              _tft->drawRect(12+BCELL_SIZE*file_from+1, 42+(7-rank_from)*BCELL_SIZE+1, 25, 25, TFT_BLUE);
              break;
            }
          }
        }
      }
      delay(100);
    }
    result.from = { chess::Column_t(file_from), chess::Row_t(rank_from) };

    while (true) {
      TS_Point p;
      if (getTouch(p.x, p.y)) {
        file_to = (p.x - 12) / BCELL_SIZE;
        rank_to = 7 - (p.y - 42 - 13) / BCELL_SIZE;
        if (file_to >= 0 && file_to < 8 && rank_to >= 0 && rank_to < 8) {
          chess::Piece_t p; chess::Color_t c;
          bool isfig = __chessParty->getCell(chess::Column_t(file_to), chess::Row_t(7-rank_to), p, c);
          //if ((isfig && (c != __color)) || !isfig) {
              _tft->fillScreen(TFT_BLACK);
              drawBoard();
              drawFigures();
              _tft->drawRect(12+BCELL_SIZE*file_from+1, 42+(7-rank_from)*BCELL_SIZE+1, 25, 25, TFT_BLUE);
              _tft->drawRect(12+BCELL_SIZE*file_to+1, 42+(7-rank_to)*BCELL_SIZE+1, 25, 25, TFT_DARKGREEN);
              break;
          //}
        }
      }
      delay(100);
    }
    result.to = { chess::Column_t(file_to), chess::Row_t(rank_to) };
    return result;
  }

  Player* __players[2];
  ChessParty* __chessParty;
  chess::Color_t __color;
  chess::Move_t __lastMove;
  uint16_t __gfx_colors[2];
};

} // namespace touch_chess
