/*
Copyright (c) 2024 Andrey V. Skvortsov

This work is licensed under the terms of the MIT license.  
For a copy, see LICENSE file.
*/

#include "tc_gamestate.hpp"

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
  _tft->fillScreen(TFT_BLACK);
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
        _tft->fillRect(50,100, 140, 50, TFT_RED);
        _tft->setCursor(75, 110, TFT_YELLOW);
        _tft->print("Bad move!");
        delay(1000);
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

  if (__color == chess::Color_t::C_WHITE)
    _tft->print("White: ");
  else
    _tft->print("Black: ");

  // Print last move
  char mstr[5];
  chess::Bot::moveStr(mov, mstr);
  mstr[4] = '\0';
  _tft->print(mstr);
  bool party_cnt = __chessParty->partyEnded();

  if (party_cnt) {
    delete __chessParty;
    __chessParty = nullptr;
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

} // namespace touch_chess
