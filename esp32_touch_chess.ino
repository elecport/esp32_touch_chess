/*
Copyright (c) 2024 Andrey V. Skvortsov

This work is licensed under the terms of the MIT license.  
For a copy, see LICENSE file.
*/

/**
 * @file esp32_touch_chess
 * @brief Main source file for an Arduino IDE
 */

#include "us.hpp"
#include <TFT_eSPI.h>
#include <SPI.h>

// If defined, you can compile to wide range of custom configurations
// based on the "red-board" SPI-tft screens with SPI resistive touch
// screen and SPI SD card reader

//#define RED_DISPLAY

#include <SPIFFS.h>
#include "fastchess.hpp"
#include "chess.hpp"

#include "tc_state.hpp"
#include "tc_mainmenustate.hpp"
#include "tc_calibrationstate.hpp"
#include "tc_gamestate.hpp"

#ifdef RED_DISPLAY

#define TS_CS 22

#else

#define TFT_DC 2
#define TFT_BL 21
#define TFT_CS 15

#define TS_CS 33
#define TS_OUT 39
#define TS_IN  32
#define TS_CLK 25

// VSPI object
static SPIClass *vspi;

#endif

// global tft-interface object
TFT_eSPI* _tft;
// global touch screen object
XPT2046_Touchscreen *_tscreen;


// Current "state" object (menu, game, setup etc)
touch_chess::State* _currentState;

// Current and previous states of the software
static touch_chess::State_t _currentStateType = touch_chess::State_t::MAIN_MENU;
static touch_chess::State_t _previousStateType = touch_chess::State_t::S_EMPTY;

// Calibration of the screen state object
static touch_chess::CalibrateState _cal_state;

static touch_chess::ChessGame _chess_game_state;


class GameSetup: public touch_chess::State
{
public:

  GameSetup(): __playersHumanFlag{false, false}
  {
  }

  ~GameSetup() = default;

  // Interface of the State
  void enter() override
  {
    _tft->fillScreen(RGB565_DARKERGREY);

    _tft->setCursor(35, 20, 2);
    _tft->print("White:");

    _tft->drawRoundRect(10, 40, 220, 25, 3, TFT_WHITE);
    _tft->setCursor(15, 45);
    _tft->print("<");
    _tft->setCursor(215, 45);
    _tft->print(">");
    _tft->setCursor(40, 45);
    _tft->print("fastchess");

    _tft->setCursor(35, 100);
    _tft->print("Black:");

    _tft->drawRoundRect(10, 120, 220, 25, 3, TFT_WHITE);
    _tft->setCursor(15, 125);
    _tft->print("<");
    _tft->setCursor(215, 125);
    _tft->print(">");
    _tft->setCursor(40, 125);
    _tft->print("fastchess");

    _tft->drawRect(10, 285, 220, 30, TFT_WHITE);
    _tft->setCursor(60, 300);
    _tft->print("Start game");
  }

  touch_chess::State_t step(unsigned current_time) override
  {
    delay(200);
    int16_t x,y;
    if (getTouch(x, y)) {
      // Start game button
      if (y>290 && y<310) {
        _chess_game_state.setPlayer(
          chess::Color_t::C_WHITE,
          __playersHumanFlag[size_t(chess::Color_t::C_WHITE)]? touch_chess::PlayerClass_t::HUMAN: touch_chess::PlayerClass_t::FASTCHESS_1
        );
        _chess_game_state.setPlayer(
          chess::Color_t::C_BLACK,
          __playersHumanFlag[size_t(chess::Color_t::C_BLACK)]? touch_chess::PlayerClass_t::HUMAN: touch_chess::PlayerClass_t::FASTCHESS_1
        );
        return touch_chess::State_t::CHESS_GAME;
      } else if (y>40 && y<65) {
        _tft->fillRect(50, 41, 150, 23, RGB565_DARKERGREY);
        _tft->setCursor(40, 45);
        __playersHumanFlag[size_t(chess::Color_t::C_WHITE)] = !__playersHumanFlag[size_t(chess::Color_t::C_WHITE)];
        if (__playersHumanFlag[size_t(chess::Color_t::C_WHITE)])
          _tft->print("human");
        else
          _tft->print("fastchess");
      } else if (y>120 && y<145) {
        _tft->fillRect(50, 121, 150, 23, TFT_BLACK);
        _tft->setCursor(40, 130);
        __playersHumanFlag[size_t(chess::Color_t::C_BLACK)] = !__playersHumanFlag[size_t(chess::Color_t::C_BLACK)];
        if (__playersHumanFlag[size_t(chess::Color_t::C_BLACK)])
          _tft->print("human");
        else
          _tft->print("fastchess");
      }
    }
    return touch_chess::State_t::GAME_SETUP;
  }
private:
  bool __playersHumanFlag[2];
};

static GameSetup _game_setup_state;
static touch_chess::MainMenu _main_menu_state;


void setup()
{
#ifndef RED_DISPLAY
  vspi = new SPIClass(VSPI);
  vspi->begin(TS_CLK, TS_OUT, TS_IN, TS_CS);
#endif

  _tscreen = new XPT2046_Touchscreen(TS_CS);
  _tft = new TFT_eSPI();

  _tft->init();
  _tft->fillScreen(TFT_BLUE);
#ifndef RED_DISPLAY
  _tft->invertDisplay(true);
#endif

#ifndef RED_DISPLAY
  _tscreen->begin(*vspi);
#else
  _tscreen->begin();
#endif
  _tscreen->setRotation(2);
  delay(300);
  Serial.begin(115200);
  if (!SPIFFS.begin(true)) {
    perror("SPIFFS error");
    exit(1);
  }
  puts("SPIFFS initialized");
}

void loop() {
  if (_currentStateType != _previousStateType) {
    _previousStateType = _currentStateType;
    if (_currentStateType == touch_chess::State_t::MAIN_MENU) {
      _currentState = &_main_menu_state;
    } else if (_currentStateType == touch_chess::State_t::CALIBRATION) {
      Serial.println("Choose calibration state");
      _currentState = &_cal_state;
    } else if (_currentStateType == touch_chess::State_t::CHESS_GAME) {
      _currentState = &_chess_game_state;
    } else if (_currentStateType == touch_chess::State_t::GAME_SETUP) {
      _currentState = &_game_setup_state;
    }
    _currentState->enter();
  }
  _currentStateType = _currentState->step(millis());
}
