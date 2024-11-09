#include "us.hpp"
#include <TFT_eSPI.h>
#include <SPI.h>

#define RED_DISPLAY

#include <SPIFFS.h>
#include "fastchess.hpp"
#include "chess.hpp"
#include "chess_pieces_bmps.h"

#include "tc_state.hpp"
#include "tc_mainmenu.hpp"
#include "tc_calibration.hpp"

#ifdef RED_DISPLAY

//#define TFT_RS 4
//#define TFT_CS 5
#define TS_CS 22

#else

#define TFT_DC 2
#define TFT_BL 21
#define TFT_CS 15

#define TS_CS 33
#define TS_OUT 39
#define TS_IN  32
#define TS_CLK 25

#endif

static SPIClass *vspi;
//static SPIClass hspi(HSPI);

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

static touch_chess::CalibrateState _cal_state;

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
    _tft->fillScreen(TFT_BLACK);

    if (__chessParty != nullptr)
      delete __chessParty;
    __chessParty = new ChessParty();
    __color = chess::Color_t::C_WHITE;

    drawBoard();
  }

  touch_chess::State_t step(unsigned current_time) override
  {
    _tft->fillScreen(TFT_BLACK);
    drawBoard();
    drawFigures();

    _tft->setCursor(10, 300);
    _tft->setTextColor(TFT_WHITE);
    if (__color == chess::Color_t::C_WHITE)
      _tft->print("White: thinking");
    else
      _tft->print("Black: thinking");

    chess::Move_t mov;
    if (__players[size_t(__color)]->getType() == Player_t::HUMAN) {
      while (true) {
        mov = __getMove();
        if (!__chessParty->isMoveValid(mov)) {
          _tft->fillRect(50,100, 140, 50, TFT_RED);
          _tft->setCursor(75, 110, TFT_YELLOW);
          _tft->print("Bad move!");
          delay(1000);
          drawBoard();
          drawFigures();
        } else
          break;
      };
      __chessParty->enterMove(mov);
    } else {
      TaskHandle_t th;
      task_parameters_t params;
      params.percent = 0;
      params.bot = __chessParty;
      params.finished = false;
      BaseType_t bt = xTaskCreatePinnedToCore(__aiMoveTask, nullptr, 8192, &params, 1, &th, 1);
      Serial.println("Task created");
      uint8_t last_pc = 0;
      _tft->fillRect(5, 5, 230, 5, TFT_BLACK);
      while (!params.finished) {
        Serial.println(params.percent, DEC);
        if (last_pc != params.percent) {
          last_pc = params.percent;
          _tft->fillRect(5, 5, 230*last_pc/100, 5, TFT_BLUE);
        }
        delay(300);
      }
      mov = params.mov;
    }
    __color = chess::Color_t(1-uint8_t(__color));

    _tft->fillRect(0, 270, 240, 50, TFT_BLACK);
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
  struct task_parameters_t
  {
    uint8_t percent;
    chess::Bot* bot;
    chess::Move_t mov;
    bool finished;
  };

  static void __aiMoveTask(void* p)
  {
    task_parameters_t* params = reinterpret_cast<task_parameters_t*>(p);
    params->mov = params->bot->makeBotMove(params->percent);
    params->finished = true;
    vTaskDelete(nullptr);
  }

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
    chess::Move_t result;
    int file_from, rank_from;
    int file_to, rank_to;
    // Get first cell
    while (true) {
      TS_Point p;
      if (getTouch(p.x, p.y)) {
        file_from = (p.x - 12) / 27;
        rank_from = 7 - (p.y - 42 - 13) / 27;
        if (file_from >= 0 && file_from < 8 && rank_from >= 0 && rank_from < 8) {
          chess::Piece_t p; chess::Color_t c;
          if (__chessParty->getCell(chess::Column_t(file_from), chess::Row_t(7-rank_from), p, c)) {
            if (c == __color) {
              _tft->fillScreen(TFT_BLACK);
              drawBoard();
              drawFigures();
              _tft->drawRect(12+27*file_from+1, 42+(7-rank_from)*27+1, 25, 25, TFT_BLUE);
              break;
            }
          }
        }
      }
      delay(100);
    }
    result.from = { chess::Column_t(file_from), chess::Row_t(rank_from) };

    while (true) {
      TS_Point p;
      if (getTouch(p.x, p.y)) {
        file_to = (p.x - 12) / 27;
        rank_to = 7 - (p.y - 42 - 13) / 27;
        if (file_to >= 0 && file_to < 8 && rank_to >= 0 && rank_to < 8) {
          chess::Piece_t p; chess::Color_t c;
          bool isfig = __chessParty->getCell(chess::Column_t(file_to), chess::Row_t(7-rank_to), p, c);
          //if ((isfig && (c != __color)) || !isfig) {
              _tft->fillScreen(TFT_BLACK);
              drawBoard();
              drawFigures();
              _tft->drawRect(12+27*file_from+1, 42+(7-rank_from)*27+1, 25, 25, TFT_BLUE);
              _tft->drawRect(12+27*file_to+1, 42+(7-rank_to)*27+1, 25, 25, TFT_DARKGREEN);
              break;
          //}
        }
      }
      delay(100);
    }
    result.to = { chess::Column_t(file_to), chess::Row_t(rank_to) };
    return result;
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
    _tft->fillScreen(RGB565_DARKERGREY);

    _tft->setCursor(35, 20, 2);
    _tft->print("White:");

    _tft->drawRoundRect(10, 40, 220, 25, 3, TFT_WHITE);
    _tft->setCursor(15, 45);
    _tft->print("<");
    _tft->setCursor(215, 45);
    _tft->print(">");
    _tft->setCursor(40, 45);
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
        _tft->fillRect(50, 41, 150, 23, RGB565_DARKERGREY);
        _tft->setCursor(40, 45);
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


void setup()
{
#ifndef RED_DISPLAY
  vspi = new SPIClass(VSPI);
  vspi->begin(TS_CLK, TS_OUT, TS_IN, TS_CS);
#endif

  _tscreen = new XPT2046_Touchscreen(TS_CS);
  _tft = new TFT_eSPI();

  _tft->init();
  _tft->fillScreen(TFT_BLUE);
#ifndef RED_DISPLAY
  _tft->invertDisplay(true);
#endif

#ifndef RED_DISPLAY
  _tscreen->begin(*vspi);
#else
  _tscreen->begin();
#endif
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
