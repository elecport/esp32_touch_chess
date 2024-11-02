#pragma once

#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>

extern Adafruit_ILI9341 _tft;
extern XPT2046_Touchscreen tscreen;

namespace touch_chess
{

extern int ts_dx;
extern int ts_dy;
extern int ts_x0;
extern int ts_y0;

enum class State_t
{
  S_EMPTY = 0,
  CALIBRATION,
  TEST_CALIBRATION,
  MAIN_MENU,
  GAME_SETUP,
  CHESS_GAME
};

class State
{
public:
  State() = default;
  virtual ~State() = default;

  virtual void enter() = 0;
  virtual State_t step(unsigned) = 0;
protected:
  bool getTouch(int16_t &x, int16_t &y);
};

} // namespace touch_chess
