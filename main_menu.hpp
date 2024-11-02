#pragma once

#include "tc_state.hpp"

namespace touch_chess
{

class MainMenu: public State
{
public:
  MainMenu() = default;

  ~MainMenu() = default;

  // Interface of the State
  void enter() override;

  State_t step(unsigned current_time) override
  {
    if (this->__noSpiffsConf)
      return State_t::CALIBRATION;
    int16_t x,y;
    if (getTouch(x, y)) {
      if (y>100 && y<130) {
        _tft.drawRect(11, 101, 218, 28, ILI9341_BLUE);
        delay(300);
        return State_t::CALIBRATION;
      } else if (y>60 && y<90) {
        _tft.drawRect(11, 61, 218, 28, ILI9341_BLUE);
        delay(300);
        return State_t::GAME_SETUP;
      }
    }
    return State_t::MAIN_MENU;
  }
private:
  bool __noSpiffsConf = true;
};

} // namespace touch_chess
