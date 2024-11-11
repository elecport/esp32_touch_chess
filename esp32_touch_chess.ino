/*
Copyright (c) 2024 Andrey V. Skvortsov

This work is licensed under the terms of the MIT license.  
For a copy, see LICENSE file.
*/

/**
 * @file esp32_touch_chess
 * @brief Main source file for an Arduino IDE
 * 
 * Initialization of the libraries and HW
 */

//#include "us.hpp"
#include <TFT_eSPI.h>
#include <SPI.h>

// If defined, you can compile to wide range of custom configurations
// based on the "red-board" SPI-tft screens with SPI resistive touch
// screen and SPI SD card reader

#define RED_DISPLAY

#include <SPIFFS.h>
#include "fastchess.hpp"
#include "chess.hpp"

#include "tc_state.hpp"
#include "tc_mainmenustate.hpp"
#include "tc_calibrationstate.hpp"
#include "tc_gamestate.hpp"
#include "tc_gamesetupstate.hpp"

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

// Main chess game state
static touch_chess::ChessGame _chess_game_state;

// Game setup state object
static touch_chess::GameSetup _game_setup_state(_chess_game_state);

// Main menu state object
static touch_chess::MainMenu _main_menu_state;

/**
 * @brief Standart Arduino platform setup function
 */
void setup()
{
#ifndef RED_DISPLAY
  // On "yellow" boards with separate SPI for TFT and Touchscreen
  // separate SPI-object for vspi interface, used by the touchscreen
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

  // Initialize debug serial interface
  Serial.begin(115200);

  // Initialize SPFFS file system
  if (!SPIFFS.begin(true)) {
    perror("SPIFFS error");
    exit(1);
  }
  puts("SPIFFS initialized");
}

// Main loop function, used by the Arduino platform
void loop() {
  // If the state was changed, do initialization ad change work
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
