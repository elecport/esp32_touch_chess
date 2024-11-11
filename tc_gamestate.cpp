/*
Copyright (c) 2024 Andrey V. Skvortsov

This work is licensed under the terms of the MIT license.  
For a copy, see LICENSE file.
*/

#include "tc_gamestate.hpp"

// Size of the chess board cell in pixels
#define BCELL_SIZE 27
// BoardPosition
#define BOARD_POSITION_TOP 30

namespace touch_chess
{

ChessGame::ChessGame():
__players{nullptr, nullptr},
__chessParty(nullptr),
__gfx_colors{RGB565_IVORY, RGB565_CHOCOLATE}
{
}

void ChessGame::setPlayer(chess::Color_t color, PlayerClass_t player)
{
  if (__players[size_t(color)] != nullptr)
    delete __players[size_t(color)];
  if (player == PlayerClass_t::HUMAN)
    __players[size_t(color)] = new Human("");
  else {
    uint8_t level=0;
    if (player == PlayerClass_t::FASTCHESS_1)
      level = 1;
    else if (player == PlayerClass_t::FASTCHESS_2)
      level = 2;
    else if (player == PlayerClass_t::FASTCHESS_3)
      level = 3;
    __players[size_t(color)] = new AIPlayer("", level);
  }
}

void ChessGame::enter()
{
  _tft->fillScreen(TFT_BLACK);

  if (__chessParty != nullptr)
    delete __chessParty;
  __chessParty = new ChessParty();
  __color = chess::Color_t::C_WHITE;

  drawBoard();
}

State_t ChessGame::step(unsigned current_time)
{
  _tft->fillRect(0, 0, 240, 240+BOARD_POSITION_TOP, TFT_BLACK);
  drawBoard();
  drawFigures();

  _tft->setCursor(10, 300);
  _tft->setTextColor(TFT_WHITE);
  if (__color == chess::Color_t::C_WHITE)
    _tft->print("White: thinking");
  else
    _tft->print("Black: thinking");

  chess::Move_t mov;
  if (__players[size_t(__color)]->getType() == Player_t::HUMAN) {
    while (true) {
      mov = __getMove();
      if (!__chessParty->isMoveValid(mov)) {
        this->_messageBox("Bad move!");
        drawBoard();
        drawFigures();
      } else
        break;
    };
    __chessParty->enterMove(mov);
  } else {
    TaskHandle_t th;
    task_parameters_t params;
    params.percent = 0;
    params.bot = __chessParty;
    params.finished = false;
    BaseType_t bt = xTaskCreatePinnedToCore(__aiMoveTask, nullptr, 8192, &params, 1, &th, 1);
    Serial.println("Task created");
    uint8_t last_pc = 0;
    _tft->fillRect(5, 5, 230, 5, TFT_BLACK);
    while (!params.finished) {
      Serial.println(params.percent, DEC);
      if (last_pc != params.percent) {
        last_pc = params.percent;
        _tft->fillRect(5, 5, 230*last_pc/100, 5, TFT_BLUE);
      }
      delay(300);
    }
    mov = params.mov;
  }
  __color = chess::Color_t(1-uint8_t(__color));

  _tft->fillRect(0, 270, 240, 50, TFT_BLACK);
  _tft->setCursor(10, 280);
  _tft->setTextColor(RGB565_IVORY);

  if (__color == chess::Color_t::C_BLACK)
    _tft->print("White: ");
  else
    _tft->print("Black: ");

  // Print last move
  char mstr[5];
  chess::Bot::moveStr(mov, mstr);
  mstr[4] = '\0';
  _tft->print(mstr);
  chess::End_t endgame;
  bool party_cnt = __chessParty->partyEnded(endgame);
  if (party_cnt) {
    delete __chessParty;
    __chessParty = nullptr;
    if (endgame == chess::End_t::CHECKMATE) {
      char msg[64] = "Checkmate!       wins!";
      if (__color == chess::Color_t::C_BLACK)
        strcpy(msg+11, "Black");
      else
        strcpy(msg+11, "White");
      this->_messageBox(msg);
    } else if (endgame == chess::End_t::STALLMATE) {
      this->_messageBox("Stallmate! Draw");
    }
    return touch_chess::State_t::MAIN_MENU;
  }

  return touch_chess::State_t::CHESS_GAME;
}

void ChessGame::__aiMoveTask(void* p)
{
  task_parameters_t* params = reinterpret_cast<task_parameters_t*>(p);
  params->mov = params->bot->makeBotMove(params->percent);
  params->finished = true;
  vTaskDelete(nullptr);
}

void ChessGame::drawBoard()
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
  _tft->drawRect(0, BOARD_POSITION_TOP, 240, 240, __gfx_colors[0]);
  _tft->drawRect(12, BOARD_POSITION_TOP+12, 216, 216, __gfx_colors[0]);
}

void ChessGame::drawFigures()
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

chess::Move_t ChessGame::__getMove()
{
  chess::Move_t result;
  int file_from, rank_from;
  int file_to, rank_to;
  // Get first cell
  while (true) {
    TS_Point p;
    if (this->_getTouch(p.x, p.y)) {
      file_from = (p.x - 12) / BCELL_SIZE;
      rank_from = 7 - (p.y - 42 - 13) / BCELL_SIZE;
      if (file_from >= 0 && file_from < 8 && rank_from >= 0 && rank_from < 8) {
        chess::Piece_t p; chess::Color_t c;
        if (__chessParty->getCell(chess::File_t(file_from), chess::Rank_t(7-rank_from), p, c)) {
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
  result.from = { chess::File_t(file_from), chess::Rank_t(rank_from) };

  while (true) {
    TS_Point p;
    if (this->_getTouch(p.x, p.y)) {
      file_to = (p.x - 12) / BCELL_SIZE;
      rank_to = 7 - (p.y - 42 - 13) / BCELL_SIZE;
      if (file_to >= 0 && file_to < 8 && rank_to >= 0 && rank_to < 8) {
        chess::Piece_t p; chess::Color_t c;
        bool isfig = __chessParty->getCell(chess::File_t(file_to), chess::Rank_t(7-rank_to), p, c);
          _tft->fillScreen(TFT_BLACK);
          drawBoard();
          drawFigures();
          _tft->drawRect(12+BCELL_SIZE*file_from+1, 42+(7-rank_from)*BCELL_SIZE+1, 25, 25, TFT_BLUE);
          _tft->drawRect(12+BCELL_SIZE*file_to+1, 42+(7-rank_to)*BCELL_SIZE+1, 25, 25, TFT_DARKGREEN);
          break;
      }
    }
    delay(100);
  }
  result.to = { chess::File_t(file_to), chess::Rank_t(rank_to) };
  return result;
}

} // namespace touch_chess
