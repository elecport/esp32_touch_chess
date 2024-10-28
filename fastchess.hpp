#pragma once

#include "chess.hpp"
#include "fast-chess.h"

enum class Player_t: uint8_t
{
  AI = 1,
  HUMAN = 0,
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

class ChessParty: public chess::Bot
{
public:
  ChessParty(Player* players[2])
  {
    getInitialGame(&__game);
    __players[0] = players[0];
    __players[1] = players[1];
    __color = chess::Color_t::C_WHITE;
  }

  ~ChessParty() override = default;

  chess::Color_t color() const { return __color; }
  Game& game() { return __game; }

  bool step()
  {
    size_t i = size_t(__color);
    __color = chess::Color_t(1-i);

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

  static chess::Move_t toMove(Move move)
  {
    chess::Move_t res;

    res.from.col = chess::Column_t(uint8_t(getFile(getFrom(move))-'a'));
    res.from.row = chess::Row_t(uint8_t(getRank(getFrom(move))-'1'));

    res.to.col = chess::Column_t(uint8_t(getFile(getTo(move))-'a'));
    res.to.row = chess::Row_t(uint8_t(getRank(getTo(move))-'1'));

    return res;
  }

  chess::Color_t __color;
  Game __game;
  uint8_t __depth = 1;
  Player* __players[2];

  // Bot interface
public:
  bool isMoveValid(chess::Move_t move) override
  {
    Move *moves = new Move[MAX_BRANCHING_FACTOR];
    int moveCount = legalMoves(moves, &(__game.position), __game.position.toMove);
    for (int i=0; i<moveCount; ++i) {
      char str[4];
      moveStr(move, str);
;     Move pm = parseMove(str);
      if (pm == moves[i]) {
        delete [] moves;
        return true;
      }
    }
    return false;
  }

  chess::Move_t makeBotMove() override
  {
    Move m = getAIMove(&__game, __depth);
    makeMove(&__game, m);
    return toMove(m);
  }

  void enterMove(chess::Move_t move) override
  {
    char str[4];
    moveStr(move, str);
    Move m = parseMove(str);
    makeMove(&__game, m);
  }

  bool partyEnded() override
  {
    return hasGameEnded(&__game.position);
  }
};
