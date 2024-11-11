/*
Copyright (c) 2024 Andrey V. Skvortsov

This work is licensed under the terms of the MIT license.  
For a copy, see LICENSE file.
*/

#include "tc_gamesetupstate.hpp"
#include "tc_gamestate.hpp"

#include <TFT_eSPI.h>

namespace touch_chess
{

GameSetup::GameSetup(ChessGame& cgs): __playersHumanFlag{false, false}, __chess_game_state(cgs)
{
}

void GameSetup::enter()
{
  _tft->fillScreen(RGB565_DARKERGREY);

  _tft->setCursor(35, 20, 2);
  _tft->print("White:");

  _tft->fillRoundRect(10, 40, 220, 25, 3, TFT_BLACK);
  _tft->drawRoundRect(10, 40, 220, 25, 3, TFT_WHITE);
  _tft->setCursor(15, 45);
  _tft->print("<");
  _tft->setCursor(215, 45);
  _tft->print(">");
  _tft->setCursor(40, 45);
  _tft->print("fastchess");

  _tft->setCursor(35, 100);
  _tft->print("Black:");

  _tft->fillRoundRect(10, 120, 220, 25, 3, TFT_BLACK);
  _tft->drawRoundRect(10, 120, 220, 25, 3, TFT_WHITE);
  _tft->setCursor(15, 125);
  _tft->print("<");
  _tft->setCursor(215, 125);
  _tft->print(">");
  _tft->setCursor(40, 125);
  _tft->print("fastchess");

  _tft->fillRoundRect(10, 285, 220, 30, 3, TFT_BLACK);
  _tft->drawRoundRect(10, 285, 220, 30, 3, TFT_WHITE);
  _tft->setCursor(60, 290);
  _tft->print("Start game");
}

State_t GameSetup::step(unsigned current_time)
{
  delay(200);
  int16_t x,y;
  if (this->_getTouch(x, y)) {
    // Start game button
    if (y>290 && y<320) {
      this->__chess_game_state.setPlayer(
        chess::Color_t::C_WHITE,
        __playersHumanFlag[size_t(chess::Color_t::C_WHITE)]? touch_chess::PlayerClass_t::HUMAN: touch_chess::PlayerClass_t::FASTCHESS_1
      );
      this->__chess_game_state.setPlayer(
        chess::Color_t::C_BLACK,
        __playersHumanFlag[size_t(chess::Color_t::C_BLACK)]? touch_chess::PlayerClass_t::HUMAN: touch_chess::PlayerClass_t::FASTCHESS_1
      );
      return touch_chess::State_t::CHESS_GAME;
    } else if (y>40 && y<65) {
      _tft->fillRect(20, 41, 200, 23, TFT_BLACK);
      _tft->setCursor(40, 45);
      __playersHumanFlag[size_t(chess::Color_t::C_WHITE)] = !__playersHumanFlag[size_t(chess::Color_t::C_WHITE)];
      if (__playersHumanFlag[size_t(chess::Color_t::C_WHITE)])
        _tft->print("human");
      else
        _tft->print("fastchess");
    } else if (y>120 && y<145) {
      _tft->fillRect(20, 121, 200, 23, TFT_BLACK);
      _tft->setCursor(40, 125);
      __playersHumanFlag[size_t(chess::Color_t::C_BLACK)] = !__playersHumanFlag[size_t(chess::Color_t::C_BLACK)];
      if (__playersHumanFlag[size_t(chess::Color_t::C_BLACK)])
        _tft->print("human");
      else
        _tft->print("fastchess");
    }
  }
  return touch_chess::State_t::GAME_SETUP;
}

} // namespace touch_chess
