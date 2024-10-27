#include <SPI.h>
#include "Adafruit_GFX.h"
#include <Adafruit_ILI9341.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/Picopixel.h>
#include <XPT2046_Touchscreen.h>
#include <SPIFFS.h>
#include "fastchess.hpp"
#include "chess.hpp"
#include "chess_pieces_bmps.h"

#define TFT_DC 2
#define TFT_RS 4
#define TFT_CS 15
#define TS_CS 22

#define RGB565_IVORY 0xFFF5
#define RGB565_CHOCOLATE 0x4980

static Adafruit_ILI9341 _tft(TFT_CS, TFT_DC, TFT_RS);
static XPT2046_Touchscreen tscreen(TS_CS);
static int ts_dx = 0;
static int ts_dy = 0;
static int ts_x0 = 0;
static int ts_y0 = 0;

static const uint8_t* pieces_bmps[][2] = {
  {chess_pawn_bitmap, chess_pawn_bitmap_filled},
  {chess_rook_bitmap, chess_rook_bitmap_filled},
  {chess_knight_bitmap, chess_knight_bitmap_filled},
  {chess_bishop_bitmap, chess_bishop_bitmap_filled},
  {chess_qeen_bitmap, chess_qeen_bitmap_filled},
  {chess_king_bitmap, chess_king_bitmap_filled}
};

class State
{
public:
  State() = default;
  virtual void enter() = 0;
  virtual void step(unsigned) = 0;
protected:
  bool getTouch(int &x, int &y)
  {
    if (tscreen.touched()) {
      TS_Point p = tscreen.getPoint();

      x = (p.x - ts_x0) / ts_dx + 5;
      y = (p.y - ts_y0) / ts_dy + 5;

      return true;
    }
    return false;
  }
};

enum class State_t
{
  S_EMPTY = 0,
  CALIBRATION,
  TEST_CALIBRATION,
  MAIN_MENU,
  GAME_SETUP,
  CHESS_GAME
};

State* _currentState;


static State_t _currentStateType = State_t::MAIN_MENU;
static State_t _previousStateType = State_t::S_EMPTY;

class CalibrateState: public State
{
public:
  CalibrateState()
  { 
  }

  ~CalibrateState()
  { 
  }

  // Interface of the State
  void enter() override
  {
    _tft.fillScreen(ILI9341_BLACK);
    _tft.setTextSize(1);
    _tft.setCursor(20,20);
    _tft.setFont(&FreeMonoBold9pt7b);
    _tft.print("Calibration of the\n         screen");
    delay(1000);
    m_step = 0;
    m_waitTouch = false;
    _tft.fillScreen(ILI9341_BLACK);
  }

  void step(unsigned current_time) override
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

      ts_dx = dx;
      ts_dy = dy;
      ts_x0 = x0;
      ts_y0 = y0;

      File f = SPIFFS.open("/calibration.conf", "w");
      f.println(ts_x0, DEC);
      f.println(ts_y0, DEC);
      f.println(ts_dx, DEC);
      f.println(ts_dy, DEC);
      f.close();

      _currentStateType = State_t::MAIN_MENU;
      return;

      Serial.print(dx, DEC);
      Serial.print(":");
      Serial.println(dy, DEC);
      Serial.println();

      if (tscreen.touched()) {
        _tft.fillScreen(ILI9341_BLACK);
        TS_Point p = tscreen.getPoint();

        _tft.setCursor(10,50);
        _tft.print(p.x, DEC);
        _tft.print(":");
        _tft.print(p.y, DEC);
        int x = (p.x - x0) / dx + 5;
        int y = (p.y - y0) / dy + 5;
        _tft.setCursor(10,80);
        _tft.print(x, DEC);
        _tft.print(":");
        _tft.println(y, DEC);
      }
      return;
    }
    if (m_waitTouch == false) {
      m_waitTouch = true;
      if (m_step == 0) {
        _tft.drawRect(4, 4, 3, 3, ILI9341_WHITE);
      } else if (m_step == 1) {
        _tft.drawRect(234, 4, 3, 3, ILI9341_WHITE);
      } else if (m_step == 2) {
        _tft.drawRect(4, 314, 3, 3, ILI9341_WHITE);
      } else if (m_step == 3) {
        _tft.drawRect(234, 314, 3, 3, ILI9341_WHITE);
      }
    } else {
      if (tscreen.touched()) {
        m_waitTouch = false;
        TS_Point p = tscreen.getPoint();
        m_coords[m_step][0] = p.x;
        m_coords[m_step][1] = p.y;
        _tft.setCursor(20,50+15*m_step);
        _tft.print(p.x, DEC);
        _tft.print(":");
        _tft.print(p.y, DEC);
        m_step++;
      }
    }
  }

private:

  uint8_t m_step;

  bool m_waitTouch;

  uint16_t m_coords[4][2];
};

static CalibrateState _cal_state;

class ChessGame: public State
{
public:

  enum class PlayerClass_t
  {
    HUMAN = 0,
    FASTCHESS_1,
    FASTCHESS_2,
    FASTCHESS_3
  };

  ChessGame():
  __players{nullptr, nullptr},
  __chessParty(nullptr),
  __gfx_colors{RGB565_IVORY, RGB565_CHOCOLATE}
  {
  }

  void setPlayer(Color_t color, PlayerClass_t player)
  {
    if (__players[size_t(color)] != nullptr)
      delete __players[size_t(color)];
    if (player == PlayerClass_t::HUMAN)
      __players[size_t(color)] = new Human("");
    else {
      uint8_t level;
      if (player == PlayerClass_t::FASTCHESS_1)
        level = 1;
      else if (player == PlayerClass_t::FASTCHESS_2)
        level = 2;
      else if (player == PlayerClass_t::FASTCHESS_3)
        level = 3;
      __players[size_t(color)] = new AIPlayer("", level);
    }
  }

  void enter() override
  {
    _tft.fillScreen(ILI9341_BLACK);

    if (__chessParty != nullptr)
      delete __chessParty;
    __chessParty = new ChessParty(__players);

    drawBoard();
  }

  void step(unsigned current_time) override
  {
    _tft.fillRect(0, 270, 240, 50, ILI9341_BLACK);
    auto c = __chessParty->color();
    if (__chessParty->game().moveListLen > 0) {
      _tft.setCursor(10, 290);
      _tft.setTextColor(RGB565_IVORY);
      Move m = __chessParty->game().moveList[__chessParty->game().moveListLen-1];
      if (c == Color_t::CBLACK)
        _tft.print("White: ");
      else
        _tft.print("Black:");
      char mstr[] = {getFile(getFrom(m)), getRank(getFrom(m)), getFile(getTo(m)), getRank(getTo(m)), '\0'};
      _tft.print(mstr);
    }
    _tft.setTextColor(ILI9341_WHITE);
    _tft.setCursor(10, 310);
    if (c == Color_t::CWHITE)
      _tft.print("White: thinking");
    else
      _tft.print("Black: thinking");
    bool party_cnt = __chessParty->step();
    drawBoard();
    char board[8][8];
    __chessParty->getBoard(board);
    drawFigures(board);

    _tft.setCursor(10, 310);
    _tft.fillRect(0, 270, 240, 50, ILI9341_BLACK);
    if (c == Color_t::CWHITE)
      _tft.print("White: ");
    else
      _tft.print("Black: ");
    Move m = __chessParty->game().moveList[__chessParty->game().moveListLen-1];
    char mstr[] = {getFile(getFrom(m)), getRank(getFrom(m)), getFile(getTo(m)), getRank(getTo(m)), '\0'};
    _tft.print(mstr);
    delay(5000);

    if (!party_cnt) {
      delete __chessParty;
      __chessParty = nullptr;
      _currentStateType = State_t::MAIN_MENU;
    }
  }

private:

  void drawBoard()
  {
    _tft.setFont(&Picopixel);
    for (int i=0; i<8; ++i) {
      _tft.drawChar(22+27*i, 38, char('a'+i), __gfx_colors[0], __gfx_colors[1], 1);
      _tft.drawChar(22+27*i, 265, char('a'+i), __gfx_colors[0], __gfx_colors[1], 1);

      _tft.drawChar(5,55+27*i, char('1'+i), __gfx_colors[0], __gfx_colors[1], 1);
      _tft.drawChar(232,55+27*i, char('1'+i), __gfx_colors[0], __gfx_colors[1], 1);
    }
    for (int i=0; i<8; ++i) {
      int flag = i%2?1:0;
      for (int j=0; j<8; ++j){
        flag = 1-flag;
        _tft.fillRect(12+j*27, 42+i*27, 27, 27, flag?__gfx_colors[0]:__gfx_colors[1]);
      }
    }
    _tft.drawRect(0,30, 240, 240, __gfx_colors[0]);
    _tft.drawRect(12,43, 216, 216, __gfx_colors[0]);
  }

  void drawFigures(char board[8][8])
  {
    _tft.setFont(&FreeMonoBold9pt7b);
    for (uint8_t i=0; i<8; ++i) {
      int flag = i%2;
      for (uint8_t j=0; j<8; ++j) {
        flag = 1-flag;
        if (board[i][j] == '.')
          continue;
        else {
          uint16_t fg; uint8_t fill_index;
          uint8_t figure_index;
          if (flag == 1) {
            fg = this->__gfx_colors[1];
            fill_index = 0;
          } else {
            fg = this->__gfx_colors[0];
            fill_index = 1;
          }
          if (board[i][j] >= 'a') {
            figure_index = board[i][j] - 'a'+'A';
            fill_index = 1-fill_index;
          } else
            figure_index = board[i][j];
            
          switch (figure_index) {
            case 'P': figure_index = uint8_t(chess::Piece_t::P_PAWN); break;
            case 'R': figure_index = uint8_t(chess::Piece_t::P_ROOK); break;
            case 'N': figure_index = uint8_t(chess::Piece_t::P_KNIGHT); break;
            case 'B': figure_index = uint8_t(chess::Piece_t::P_BISHOP); break;
            case 'Q': figure_index = uint8_t(chess::Piece_t::P_QUEEN); break;
            case 'K': figure_index = uint8_t(chess::Piece_t::P_KING); break;
            default: figure_index = 0;
          }
          _tft.drawBitmap(14+27*j, 44+27*i, pieces_bmps[figure_index][fill_index], 24, 24, fg);
        }
      }
    }
  }

  Player* __players[2];
  ChessParty* __chessParty;
  uint16_t __gfx_colors[2];
};

ChessGame _chess_game_state;


class GameSetup: public State
{
public:

  GameSetup()
  {
  }

  ~GameSetup()
  {
  }

  // Interface of the State
  void enter() override
  {
    _tft.fillScreen(ILI9341_BLACK);

    _tft.setCursor(35, 30);
    _tft.print("White:");

    _tft.drawRect(10, 40, 220, 25, ILI9341_WHITE);
    _tft.setCursor(15, 55);
    _tft.print("<");
    _tft.setCursor(215, 55);
    _tft.print(">");
    _tft.setCursor(40, 55);
    _tft.print("fastchess");


    _tft.setCursor(35, 110);
    _tft.print("Black:");

    _tft.drawRect(10, 120, 220, 25, ILI9341_WHITE);
    _tft.setCursor(15, 135);
    _tft.print("<");
    _tft.setCursor(215, 135);
    _tft.print(">");
    _tft.setCursor(40, 135);
    _tft.print("fastchess");

    _tft.drawRect(10, 285, 220, 30, ILI9341_WHITE);
    _tft.setCursor(60, 305);
    _tft.print("Start game");
  }

  void step(unsigned current_time) override
  {
    int x,y;
    if (getTouch(x, y)) {
      // Start game button
      if (y>290 && y<310) {
        _chess_game_state.setPlayer(Color_t::CWHITE, ChessGame::PlayerClass_t::FASTCHESS_1);
        _chess_game_state.setPlayer(Color_t::CBLACK, ChessGame::PlayerClass_t::FASTCHESS_1);
        _currentStateType = State_t::CHESS_GAME;
      }
    }
  }
};


static GameSetup _game_setup_state;


class MainMenu: public State
{
public:
  MainMenu()
  {
  }

  ~MainMenu()
  {
  }

  // Interface of the State
  void enter() override
  {
    if (!SPIFFS.exists("/calibration.conf")) {
      File f = SPIFFS.open("/calibration.conf", "w");
      f.close();
      _currentStateType = State_t::CALIBRATION;
      return;
    } else {
      File f = SPIFFS.open("/calibration.conf", "r");
      ts_x0 = f.parseInt();
      ts_y0 = f.parseInt();
      ts_dx = f.parseInt();
      ts_dy = f.parseInt();
      f.close();
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

  void step(unsigned current_time) override
  {
    int x,y;
    if (getTouch(x, y)) {
      if (y>100 && y<130) {
        _tft.drawRect(11, 101, 218, 28, ILI9341_BLUE);
        delay(300);
        _currentStateType = State_t::CALIBRATION;
      } else if (y>60 && y<90) {
        _tft.drawRect(11, 61, 218, 28, ILI9341_BLUE);
        delay(300);
        _currentStateType = State_t::GAME_SETUP;
      }
    }
  }
};

static MainMenu _main_menu_state;


void setup() {
  disableCore0WDT();
  disableCore1WDT();

  _tft.begin();
  _tft.fillScreen(ILI9341_BLACK);
  tscreen.begin();
  tscreen.setRotation(2);
  Serial.begin(115200);
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS error");
    exit(1);
  }
}

void loop() {
  if (_currentStateType != _previousStateType) {
    _previousStateType = _currentStateType;
    if (_currentStateType == State_t::MAIN_MENU) {
      _currentState = &_main_menu_state;
    } else if (_currentStateType == State_t::CALIBRATION) {
      _currentState = &_cal_state;
    } else if (_currentStateType == State_t::CHESS_GAME) {
      _currentState = &_chess_game_state;
    } else if (_currentStateType == State_t::GAME_SETUP) {
      _currentState = &_game_setup_state;
    }
    _currentState->enter();
  }
  _currentState->step(millis());
}
