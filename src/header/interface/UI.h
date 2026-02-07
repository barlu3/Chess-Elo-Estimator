#ifndef UI_H
#define UI_H

#include "Game.h"
#include "api/Board.h"
#include "data/LoadGame.h"
#include "data/SaveGame.h"
#include <vector>

namespace UI {
    void printBoard(const Board& board);
    bool parseSquare(const std::string&, int&, int&);
    void printMoves(const std::vector<Move>& moves);
    Move selectMove(Game& game, const std::string& username);
}

#endif