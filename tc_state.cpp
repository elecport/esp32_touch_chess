/*
Copyright (c) 2024 Andrey V. Skvortsov

This work is licensed under the terms of the MIT license.  
For a copy, see LICENSE file.
*/

#include "tc_state.hpp"
#include "chess_pieces_bmps.h"

namespace touch_chess
{

unsigned Keeper::_level = 0;

int ts_dx = 0;
int ts_dy = 0;
int ts_x0 = 0;
int ts_y0 = 0;

const uint8_t* pieces_bmps[][2] = {
  {chess_pawn_bitmap, chess_pawn_bitmap_filled},
  {chess_rook_bitmap, chess_rook_bitmap_filled},
  {chess_knight_bitmap, chess_knight_bitmap_filled},
  {chess_bishop_bitmap, chess_bishop_bitmap_filled},
  {chess_qeen_bitmap, chess_qeen_bitmap_filled},
  {chess_king_bitmap, chess_king_bitmap_filled}
};

State::State()
{
  __lastTouched = millis();
}

bool State::_getTouch(int16_t &x, int16_t &y)
{
  unsigned t = millis();
  if (_tscreen->touched()) {
    if ((t-__lastTouched) < 300) {
      __lastTouched = t;
      return false;
    }
    __lastTouched = t;
    TS_Point p = _tscreen->getPoint();

    x = (p.x - ts_x0) / ts_dx + 5;
    y = (p.y - ts_y0) / ts_dy + 5;

    return true;
  }
  return false;
}

void State::_messageBox(const char* text)
{
  _tft->fillRoundRect(20, 50, 200, 50, 3, TFT_RED);
  _tft->drawRoundRect(20, 50, 200, 50, 3, TFT_WHITE);
  _tft->setCursor(25, 55);
  _tft->print(text);
  _tft->drawRoundRect(40, 80, 160, 15, 3, TFT_WHITE);
  _tft->setCursor(100, 80);
  _tft->print("OK");
  while (true) {
    int16_t x, y;
    if (this->_getTouch(x, y)) {
      if (x>40 && x<200 && y>80 && y<95)
        break;
    }
  }
}

} // touch_schess
