#include "tc_settingsstate.hpp"

#define CALIBRATION_POS 60
#define LANGUAGE_POS 100
#define BACK_POS 180
#define BUTTON_HEIGHT 30

namespace touch_chess
{

void Settings::enter()
{
  _tft->fillScreen(TFT_DARKGREY);

  _tft->fillRoundRect(10, CALIBRATION_POS, 220, BUTTON_HEIGHT, 3, TFT_NAVY);
  _tft->drawRoundRect(10, CALIBRATION_POS, 220, BUTTON_HEIGHT, 3, TFT_LIGHTGREY);
  _tft->setCursor(20, CALIBRATION_POS+20);
  _tft->print("CALIBRATION");

  _tft->setCursor(20, LANGUAGE_POS+20);
  _tft->print("LANGUAGE");
  _tft->fillRoundRect(10, LANGUAGE_POS+40, 220, BUTTON_HEIGHT, 3, TFT_BLACK);
  _tft->drawRoundRect(10, LANGUAGE_POS+40, 220, BUTTON_HEIGHT, 3, TFT_BLACK);
  _tft->setCursor(20, LANGUAGE_POS+60);
  _tft->print("<    English    >");

  _tft->fillRoundRect(10, BACK_POS, 220, BUTTON_HEIGHT, 3, TFT_NAVY);
  _tft->drawRoundRect(10, BACK_POS, 220, BUTTON_HEIGHT, 3, TFT_LIGHTGREY);
  _tft->setCursor(20, BACK_POS+20);
  _tft->print("BACK");
}

State_t Settings::step(unsigned dt)
{
  int16_t x,y;
  if (this->_getTouch(x, y)) {
    if (y>CALIBRATION_POS && y<CALIBRATION_POS+BUTTON_HEIGHT) {
      _tft->drawRect(11, CALIBRATION_POS+1, 218, BUTTON_HEIGHT-2, TFT_BLUE);
      delay(300);
      return State_t::CALIBRATION;
    } else if (y>BACK_POS && y<BACK_POS+BUTTON_HEIGHT) {
      _tft->drawRect(11, BACK_POS+1, 218, BUTTON_HEIGHT-2, TFT_BLUE);
      delay(300);
      return State_t::MAIN_MENU;
    }
  }
  return touch_chess::State_t::SETTINGS;
}

} // namespace touch_chess
