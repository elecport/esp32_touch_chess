#pragma once

//#include <Adafruit_ILI9341.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

//extern Adafruit_ILI9341 *_tft;
extern TFT_eSPI* _tft;
extern XPT2046_Touchscreen *_tscreen;

#define RGB565_IVORY 0xFFF5
#define RGB565_CHOCOLATE 0x4980
#define RGB565_DARKERGREY 0x39E7

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

class Keeper
{
public:
  Keeper(const char* _name): m_name(_name)
  {
    _millis = millis();
    for (unsigned i=0; i<_level; ++i)
      putchar(' ');
    ++_level;
    printf("%s heap %u\n", m_name, ESP.getFreeHeap());
  }
  ~Keeper()
  {
    unsigned elapsed = millis() - _millis;
    --_level;
    for (unsigned i=0; i<_level; ++i)
      putchar(' ');
    printf("%s heap %u elapsed %u\n", m_name, ESP.getFreeHeap(), elapsed);
  }
  const char* m_name;
  unsigned _millis;
  static unsigned _level;
};

class State
{
public:
  State();
  virtual ~State() = default;

  virtual void enter() = 0;
  virtual State_t step(unsigned) = 0;
protected:
  bool getTouch(int16_t &x, int16_t &y);
private:
  unsigned __lastTouched;
};

} // namespace touch_chess
