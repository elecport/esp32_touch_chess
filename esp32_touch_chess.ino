#include <TFT_eSPI.h>
#include <SPI.h>
//#include "Adafruit_GFX.h"


//#include <Fonts/FreeMono9pt7b.h>
//#include <Fonts/FreeMonoBold9pt7b.h>
//#include <Fonts/Picopixel.h>

#include <SPIFFS.h>
#include "fastchess.hpp"
#include "chess.hpp"
#include "chess_pieces_bmps.h"

#include "tc_state.hpp"
#include "tc_mainmenu.hpp"

#define TFT_DC 2
#define TFT_BL 21
#define TFT_CS 15

#define TS_CS 33
#define TS_OUT 39
#define TS_IN  32
#define TS_CLK 25

static SPIClass *vspi;
//static SPIClass hspi(HSPI);

//Adafruit_ILI9341 *_tft;
TFT_eSPI* _tft;
XPT2046_Touchscreen *_tscreen;


static const uint8_t* pieces_bmps[][2] = {
  {chess_pawn_bitmap, chess_pawn_bitmap_filled},
  {chess_rook_bitmap, chess_rook_bitmap_filled},
  {chess_knight_bitmap, chess_knight_bitmap_filled},
  {chess_bishop_bitmap, chess_bishop_bitmap_filled},
  {chess_qeen_bitmap, chess_qeen_bitmap_filled},
  {chess_king_bitmap, chess_king_bitmap_filled}
};


touch_chess::State* _currentState;
static touch_chess::State_t _currentStateType = touch_chess::State_t::MAIN_MENU;
static touch_chess::State_t _previousStateType = touch_chess::State_t::S_EMPTY;

class CalibrateState: public touch_chess::State
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
    touch_chess::Keeper k(__PRETTY_FUNCTION__);

    _tft->fillScreen(TFT_BLUE);
    _tft->setCursor(20,20,2);
    _tft->print("Calibration of the\n         screen");
    delay(1000);
    m_step = 0;
    m_waitTouch = false;
    _tft->fillScreen(TFT_BLACK);
  }

  touch_chess::State_t step(unsigned current_time) override
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

private:

  uint8_t m_step;

  bool m_waitTouch;

  uint16_t m_coords[4][2];
};

static CalibrateState _cal_state;

class ChessGame: public touch_chess::State
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

  void setPlayer(chess::Color_t color, PlayerClass_t player)
  {
    if (__players[size_t(color)] != nullptr)
      delete __players[size_t(color)];
    if (player == PlayerClass_t::HUMAN)
      __players[size_t(color)] = new Human("");
    else {
      uint8_t level=0;
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
    _tft->fillScreen(ILI9341_BLACK);

    if (__chessParty != nullptr)
      delete __chessParty;
    __chessParty = new ChessParty(__players);
    __color = chess::Color_t::C_WHITE;

    drawBoard();
  }

  touch_chess::State_t step(unsigned current_time) override
  {
    _tft->fillScreen(ILI9341_BLACK);
    drawBoard();
    drawFigures();

    _tft->setCursor(10, 300);
    _tft->setTextColor(ILI9341_WHITE);
    if (__color == chess::Color_t::C_WHITE)
      _tft->print("White: thinking");
    else
      _tft->print("Black: thinking");

    chess::Move_t mov;
    if (__players[size_t(__color)]->getType() == Player_t::HUMAN) {
      mov = __getMove();
    } else {
      mov = __chessParty->makeBotMove();
    }
    __color = chess::Color_t(1-uint8_t(__color));

    _tft->fillRect(0, 270, 240, 50, ILI9341_BLACK);
    _tft->setCursor(10, 280);
    _tft->setTextColor(RGB565_IVORY);

    //_tft->setFont(&FreeMonoBold9pt7b);
    if (__color == chess::Color_t::C_WHITE)
      _tft->print("White: ");
    else
      _tft->print("Black: ");

    // Print last move
    char mstr[5];
    chess::Bot::moveStr(mov, mstr);
    mstr[4] = '\0';
    _tft->print(mstr);
    //printf("%x %x %x %x %s\n", int(mstr[0]), int(mstr[1]), int(mstr[2]), int(mstr[3]), mstr);
    bool party_cnt = __chessParty->partyEnded();

    if (party_cnt) {
      delete __chessParty;
      __chessParty = nullptr;
      return touch_chess::State_t::MAIN_MENU;
    }

    return touch_chess::State_t::CHESS_GAME;
  }

private:

  void drawBoard()
  {
    //_tft->setFont(&Picopixel);
    for (int i=0; i<8; ++i) {
      _tft->drawChar(22+27*i, 33, char('a'+i), __gfx_colors[0], __gfx_colors[1], 1);
      _tft->drawChar(22+27*i, 260, char('a'+i), __gfx_colors[0], __gfx_colors[1], 1);

      _tft->drawChar(5,55+25*i, char('8'-i), __gfx_colors[0], __gfx_colors[1], 1);
      _tft->drawChar(232,55+25*i, char('8'-i), __gfx_colors[0], __gfx_colors[1], 1);
    }
    for (int i=0; i<8; ++i) {
      int flag = i%2?1:0;
      for (int j=0; j<8; ++j){
        flag = 1-flag;
        _tft->fillRect(12+j*27, 42+i*27, 27, 27, flag?__gfx_colors[0]:__gfx_colors[1]);
      }
    }
    _tft->drawRect(0,30, 240, 240, __gfx_colors[0]);
    _tft->drawRect(12,43, 216, 216, __gfx_colors[0]);
  }

  void drawFigures()
  {
    char board[8][8];
    __chessParty->getBoard(board);

    //_tft->setFont(&FreeMonoBold9pt7b);
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
          _tft->drawBitmap(14+27*j, 44+27*i, pieces_bmps[figure_index][fill_index], 24, 24, fg);
        }
      }
    }
  }

  chess::Move_t __getMove()
  {
    // Get first cell
    while (true) {
      TS_Point p;
      if (getTouch(p.x, p.y)) {
        int file = (p.x - 12) / 27;
        int rank = 7 - (p.y - 42 - 13) / 27;
        printf("%d %d (%d %d)\n", p.x, p.y, file, rank);
        if (file >= 0 && file < 8 && rank >= 0 && file < 8) {
          chess::Piece_t p; chess::Color_t c;
          if (__chessParty->getCell(chess::Column_t(file), chess::Row_t(7-rank), p, c)) {
            printf("Colors: %d, %d\n", int(c), int(__color));
            if (c == __color) {
              _tft->fillScreen(ILI9341_BLACK);
              drawBoard();
              drawFigures();
              _tft->drawRect(12+27*file+1, 42+(7-rank)*27+1, 25, 25, ILI9341_BLUE);
            }
          }
        }
      }
      delay(100);
    }
  }

  Player* __players[2];
  ChessParty* __chessParty;
  chess::Color_t __color;
  chess::Move_t __lastMove;
  uint16_t __gfx_colors[2];
};

ChessGame _chess_game_state;


class GameSetup: public touch_chess::State
{
public:

  GameSetup(): __playersHumanFlag{false, false}
  {
  }

  ~GameSetup() = default;

  // Interface of the State
  void enter() override
  {
    _tft->fillScreen(TFT_BLACK);

    _tft->setCursor(35, 20, 2);
    _tft->print("White:");

    _tft->drawRoundRect(10, 40, 220, 25, 3, TFT_WHITE);
    _tft->setCursor(15, 50);
    _tft->print("<");
    _tft->setCursor(215, 50);
    _tft->print(">");
    _tft->setCursor(40, 50);
    _tft->print("fastchess");

    _tft->setCursor(35, 100);
    _tft->print("Black:");

    _tft->drawRoundRect(10, 120, 220, 25, 3, TFT_WHITE);
    _tft->setCursor(15, 125);
    _tft->print("<");
    _tft->setCursor(215, 125);
    _tft->print(">");
    _tft->setCursor(40, 125);
    _tft->print("fastchess");

    _tft->drawRect(10, 285, 220, 30, TFT_WHITE);
    _tft->setCursor(60, 300);
    _tft->print("Start game");
  }

  touch_chess::State_t step(unsigned current_time) override
  {
    delay(200);
    int16_t x,y;
    if (getTouch(x, y)) {
      // Start game button
      if (y>290 && y<310) {
        _chess_game_state.setPlayer(
          chess::Color_t::C_WHITE,
          __playersHumanFlag[size_t(chess::Color_t::C_WHITE)]?ChessGame::PlayerClass_t::HUMAN:ChessGame::PlayerClass_t::FASTCHESS_1
        );
        _chess_game_state.setPlayer(
          chess::Color_t::C_BLACK,
          __playersHumanFlag[size_t(chess::Color_t::C_BLACK)]?ChessGame::PlayerClass_t::HUMAN:ChessGame::PlayerClass_t::FASTCHESS_1
        );
        return touch_chess::State_t::CHESS_GAME;
      } else if (y>40 && y<65) {
        _tft->fillRect(50, 41, 150, 23, TFT_BLACK);
        _tft->setCursor(40, 50);
        __playersHumanFlag[size_t(chess::Color_t::C_WHITE)] = !__playersHumanFlag[size_t(chess::Color_t::C_WHITE)];
        if (__playersHumanFlag[size_t(chess::Color_t::C_WHITE)])
          _tft->print("human");
        else
          _tft->print("fastchess");
      } else if (y>120 && y<145) {
        _tft->fillRect(50, 121, 150, 23, TFT_BLACK);
        _tft->setCursor(40, 130);
        __playersHumanFlag[size_t(chess::Color_t::C_BLACK)] = !__playersHumanFlag[size_t(chess::Color_t::C_BLACK)];
        if (__playersHumanFlag[size_t(chess::Color_t::C_BLACK)])
          _tft->print("human");
        else
          _tft->print("fastchess");
      }
    }
    return touch_chess::State_t::GAME_SETUP;
  }
private:
  bool __playersHumanFlag[2];
};

static GameSetup _game_setup_state;
static touch_chess::MainMenu _main_menu_state;


void setup() {
  //disableCore0WDT();
  //disableCore1WDT();

  vspi = new SPIClass(VSPI);
  vspi->begin(TS_CLK, TS_OUT, TS_IN, TS_CS);

  _tscreen = new XPT2046_Touchscreen(TS_CS);
  _tft = new TFT_eSPI();

  _tft->init();
  _tft->fillScreen(TFT_BLUE);
  _tft->invertDisplay(true);
  
  _tscreen->begin(*vspi);
  _tscreen->setRotation(2);
  delay(300);
  Serial.begin(115200);
  if (!SPIFFS.begin(true)) {
    perror("SPIFFS error");
    exit(1);
  }
  puts("SPIFFS initialized");
}

void loop() {
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
