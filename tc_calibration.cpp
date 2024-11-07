#include "tc_calibration.hpp"

namespace touch_chess
{

void CalibrateState::enter()
{
  touch_chess::Keeper k(__PRETTY_FUNCTION__);

  _tft->fillScreen(TFT_BLUE);
  _tft->setCursor(20,20,2);
  _tft->print("Calibration of the\n         screen");
  delay(1000);
  m_step = 0;
  m_waitTouch = false;
  _tft->fillScreen(TFT_BLACK);
}

State_t CalibrateState::step(unsigned current_time)
{
  delay(100);
  if (m_step > 3) {
    Serial.print(m_coords[0][0], DEC);
    Serial.print(":");
    Serial.println(m_coords[2][0], DEC);
    int x0 = (m_coords[0][0] + m_coords[2][0]) / 2;
    Serial.print(m_coords[1][0], DEC);
    Serial.print(":");
    Serial.println(m_coords[3][0], DEC);
    int xf = (m_coords[1][0] + m_coords[3][0]) / 2;
    Serial.print(x0, DEC);
    Serial.print(":");
    Serial.println(xf, DEC);
    Serial.println();

    Serial.print(m_coords[0][1], DEC);
    Serial.print(":");
    Serial.println(m_coords[1][1], DEC);
    int y0 = (m_coords[0][1] + m_coords[1][1]) / 2;
    Serial.print(m_coords[2][1], DEC);
    Serial.print(":");
    Serial.println(m_coords[3][1], DEC);
    int yf = (m_coords[2][1] + m_coords[3][1]) / 2;
    Serial.print(y0, DEC);
    Serial.print(":");
    Serial.println(yf, DEC);
    Serial.println();

    int dx = (xf - x0) / 230;
    int dy = (yf - y0) / 310;

    touch_chess::ts_dx = dx;
    touch_chess::ts_dy = dy;
    touch_chess::ts_x0 = x0;
    touch_chess::ts_y0 = y0;

    puts("Making a file /calibration.conf");
    fs::File f = SPIFFS.open("/calibration.conf", "w");
    f.println(touch_chess::ts_x0, DEC);
    f.println(touch_chess::ts_y0, DEC);
    f.println(touch_chess::ts_dx, DEC);
    f.println(touch_chess::ts_dy, DEC);
    f.close();

    return touch_chess::State_t::MAIN_MENU;
  }
  if (m_waitTouch == false) {
    m_waitTouch = true;
    if (m_step == 0) {
      _tft->drawRect(4, 4, 3, 3, ILI9341_WHITE);
    } else if (m_step == 1) {
      _tft->drawRect(234, 4, 3, 3, ILI9341_WHITE);
    } else if (m_step == 2) {
      _tft->drawRect(4, 314, 3, 3, ILI9341_WHITE);
    } else if (m_step == 3) {
      _tft->drawRect(234, 314, 3, 3, ILI9341_WHITE);
    }
  } else {
    if (_tscreen->touched()) {
      m_waitTouch = false;
      TS_Point p = _tscreen->getPoint();
      m_coords[m_step][0] = p.x;
      m_coords[m_step][1] = p.y;
      _tft->setCursor(20,50+15*m_step);
      _tft->print(p.x, DEC);
      _tft->print(":");
      _tft->print(p.y, DEC);
      m_step++;
    }
  }
  return touch_chess::State_t::CALIBRATION;
}

} // namespace touch_chess
