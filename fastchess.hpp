#include "fast-chess.h"

enum class Player_t: uint8_t
{
  AI = 1,
  HUMAN = 0,
};

enum class Color_t: uint8_t
{
  CWHITE = 0,
  CBLACK = 1
};

class Player
{
public:
  Player_t getType() const
  {
    return this->__type;
  }
protected:
  Player(const char* name, Player_t type):
  __type(type)
  {
    strncpy(this->__name, name, 16);
  }
private:
  Player_t __type;
  char __name[17];
};

class Human: public Player
{
public:
  Human(const char* name):
  Player(name, Player_t::HUMAN)
  {
  }
};

class AIPlayer: public Player
{
public:
  AIPlayer(const char* name, uint8_t depth):
  Player(name, Player_t::AI), __depth(depth)
  {
  }
  uint8_t getDepth() const
  {
    return this->__depth;
  }
private:
  uint8_t __depth;
};

class ChessParty
{
public:
  ChessParty(Player* players[2])
  {
    getInitialGame(&__game);
    __players[0] = players[0];
    __players[1] = players[1];
    __color = Color_t::CWHITE;
  }

  Color_t color() const { return __color; }
  Game& game() { return __game; }

  bool step()
  {
    size_t i = size_t(__color);
    __color = Color_t(1-i);

    printBoard(&(__game.position.board));
    if (hasGameEnded(&__game.position)) {
      printOutcome(&__game.position);
      return false;
    }
    Move m;
    if (__players[i]->getType() == Player_t::HUMAN)
      m = __getPlayerMove();
    else {
      AIPlayer* aip = static_cast<AIPlayer*>(__players[i]);
      m = getAIMove(&__game, aip->getDepth());
    }
    makeMove(&__game, m);

    return true;
  }

  void getBoard(char (&board)[8][8])
  {
    for (uint8_t rank=0; rank<8; rank++) {
      for (uint8_t file=0; file<8; file++) {
        board[rank][file] = getPieceChar( (FILES_BB[file] & RANKS_BB[7-rank]), &__game.position.board);
      }
    }
  }

private:

  Move __getPlayerMove()
  {
    Move *moves = new Move[MAX_BRANCHING_FACTOR];
    int moveCount = legalMoves(moves, &(__game.position), __game.position.toMove);
    do {
      char input[5];
      while (!Serial.available()) delay(10);
      Serial.readBytesUntil('\n', input, 5);
      Move pm = parseMove(input);
      for (int i=0; i<moveCount; ++i){
        if (pm == moves[i]) {
          delete [] moves;
          return pm;
        }
      }
      puts("Illegal move!");
    } while (true);
  }

  Color_t __color;
  Game __game;
  Player* __players[2];
};

/*
static ChessParty* _party = nullptr;

void setup()
{
  disableCore0WDT();
  disableCore1WDT();
  Serial.begin(115200);
  Serial.setTimeout(2000);
}

void loop()
{
  if (_party == nullptr) {
    Player *k = new AIPlayer("Kolya", 1);
    Player *p = new AIPlayer("Papa", 1);
    Player* players[2];
    players[size_t(Color_t::CWHITE)] = k;
    players[size_t(Color_t::CBLACK)] = p;
    _party = new ChessParty(players);
  }
  bool party_cnt = _party->step();
  if (!party_cnt) {
    delete _party;
  }
}
*/
