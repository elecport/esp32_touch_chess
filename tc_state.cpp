#include "tc_state.hpp"

namespace touch_chess
{

int ts_dx = 0;
int ts_dy = 0;
int ts_x0 = 0;
int ts_y0 = 0;

bool State::getTouch(int &x, int &y)
{
  if (tscreen.touched()) {
    TS_Point p = tscreen.getPoint();

    x = (p.x - ts_x0) / ts_dx + 5;
    y = (p.y - ts_y0) / ts_dy + 5;

    return true;
  }
  return false;
}

} // touch_schess
