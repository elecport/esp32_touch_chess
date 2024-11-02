#include <SPIFFS.h>
#include <Fonts/FreeMonoBold9pt7b.h>

#include "main_menu.hpp"

namespace touch_chess
{

void MainMenu::enter()
{
  if (!SPIFFS.exists("/calibration.conf")) {
    File f = SPIFFS.open("/calibration.conf", "w");
    f.close();
  } else {
    File f = SPIFFS.open("/calibration.conf", "r");
    touch_chess::ts_x0 = f.parseInt();
    touch_chess::ts_y0 = f.parseInt();
    touch_chess::ts_dx = f.parseInt();
    touch_chess::ts_dy = f.parseInt();
    f.close();
    this->__noSpiffsConf = false;
  }

  _tft.fillScreen(ILI9341_BLACK);
  _tft.setTextSize(1);
  _tft.setFont(&FreeMonoBold9pt7b);

  _tft.drawRect(10, 60, 220, 30, ILI9341_WHITE);
  _tft.setCursor(20, 75);
  _tft.print("   PLAY CHESS");

  _tft.drawRect(10, 100, 220, 30, ILI9341_WHITE);
  _tft.setCursor(20, 115);
  _tft.print("  CALIBRATION");
}

} // namespace touch_chess
