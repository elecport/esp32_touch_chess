#include <SPIFFS.h>
//#include <Fonts/FreeMonoBold9pt7b.h>

#include "tc_mainmenu.hpp"

#define SCROLL_STEP_X 1
#define SCROLL_STEP_Y 2
#define CELL_SIZE 32

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
  _tft->fillRoundRect(10, 60, 220, 30, 3, TFT_NAVY);
  _tft->drawRoundRect(10, 60, 220, 30, 3, TFT_LIGHTGREY);
  _tft->setCursor(20, 70, 2);
  _tft->print("   PLAY CHESS");

  _tft->fillRoundRect(10, 100, 220, 30, 3, TFT_NAVY);
  _tft->drawRoundRect(10, 100, 220, 30, 3, TFT_LIGHTGREY);
  _tft->setCursor(20, 110);
  _tft->print("  CALIBRATION");
}

State_t MainMenu::step(unsigned current_time)
{
  if (this->__noSpiffsConf)
    return State_t::CALIBRATION;
  int16_t x,y;
  if (getTouch(x, y)) {
    if (y>100 && y<130) {
      _tft->drawRect(11, 101, 218, 28, ILI9341_BLUE);
      delay(300);
      return State_t::CALIBRATION;
    } else if (y>60 && y<90) {
      _tft->drawRect(11, 61, 218, 28, ILI9341_BLUE);
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
