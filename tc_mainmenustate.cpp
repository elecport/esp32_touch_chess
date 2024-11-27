/*
Copyright (c) 2024 Andrey V. Skvortsov

This work is licensed under the terms of the MIT license.  
For a copy, see LICENSE file.
*/

#include <SPIFFS.h>

#include "tc_mainmenustate.hpp"
#include "Free_Fonts.h"

#define SCROLL_STEP_X 1
#define SCROLL_STEP_Y 2
#define CELL_SIZE 32

#define NEW_GAME_POS 60
#define LOAD_GAME_POS 100
#define SETTINGS_POS 140
#define BUTTON_HEIGHT 30

namespace touch_chess
{

void MainMenu::enter()
{
  Keeper k(__PRETTY_FUNCTION__);

  if (!SPIFFS.exists("/calibration.conf")) {
    puts("No file /calibration.conf, creating");
    File f = SPIFFS.open("/calibration.conf", "w");
    f.close();
  } else {
    puts("Opening file /calibration.conf");
    File f = SPIFFS.open("/calibration.conf", "r");
    touch_chess::ts_x0 = f.parseInt();
    touch_chess::ts_y0 = f.parseInt();
    touch_chess::ts_dx = f.parseInt();
    touch_chess::ts_dy = f.parseInt();
    f.close();
    this->__noSpiffsConf = false;
  }

  _tft->fillScreen(RGB565_DARKERGREY);
  _tft->setTextColor(TFT_LIGHTGREY);

  __drawButtons();
}

void MainMenu::__drawButtons()
{
  _tft->setFreeFont(FSSO9);
  
  _tft->fillRoundRect(10, NEW_GAME_POS, 220, BUTTON_HEIGHT, 3, TFT_NAVY);
  _tft->drawRoundRect(10, NEW_GAME_POS, 220, BUTTON_HEIGHT, 3, TFT_LIGHTGREY);
  _tft->setCursor(20, NEW_GAME_POS+20);
  _tft->print("NEW GAME");

  _tft->fillRoundRect(10, LOAD_GAME_POS, 220, BUTTON_HEIGHT, 3, TFT_NAVY);
  _tft->drawRoundRect(10, LOAD_GAME_POS, 220, BUTTON_HEIGHT, 3, TFT_LIGHTGREY);
  _tft->setCursor(20, LOAD_GAME_POS+20);
  _tft->print("LOAD GAME");

  _tft->fillRoundRect(10, SETTINGS_POS, 220, BUTTON_HEIGHT, 3, TFT_NAVY);
  _tft->drawRoundRect(10, SETTINGS_POS, 220, BUTTON_HEIGHT, 3, TFT_LIGHTGREY);
  _tft->setCursor(20, SETTINGS_POS+20);
  _tft->print("SETTINGS");
}

State_t MainMenu::step(unsigned current_time)
{
  if (this->__noSpiffsConf)
    return State_t::CALIBRATION;
  int16_t x,y;
  if (this->_getTouch(x, y)) {
    if (y>SETTINGS_POS && y<SETTINGS_POS+BUTTON_HEIGHT) {
      _tft->drawRect(11, SETTINGS_POS+1, 218, BUTTON_HEIGHT-2, TFT_BLUE);
      delay(300);
      return State_t::SETTINGS;
    } else if (y>NEW_GAME_POS && y<90) {
      _tft->drawRect(11, NEW_GAME_POS+1, 218, BUTTON_HEIGHT-2, TFT_BLUE);
      delay(300);
      return State_t::GAME_SETUP;
    }
  }
  delay(200);
  if ((__step+=3) > CELL_SIZE)
    __step -= CELL_SIZE;
  for (int16_t i=0; i<240/CELL_SIZE+1; ++i) {
    bool _state = i%2==0;
    for (uint16_t j=0; j<320/CELL_SIZE+1; ++j) {
      _state = !_state;
      _tft->fillRect(i*CELL_SIZE+__step-CELL_SIZE, j*CELL_SIZE+__step-CELL_SIZE, CELL_SIZE, CELL_SIZE, _state?TFT_LIGHTGREY:RGB565_DARKERGREY);
    }
  }
  //if (__step % 2 == 0)
    __drawButtons();
  return State_t::MAIN_MENU;
}

} // namespace touch_chess
