#ifndef STOCKFISH_H
#define STOCKFISH_H

#include <string>
#include "api/Board.h"
#include "Game.h"
#include "interface/Menu.h"

class Stockfish {
public:
    // difficulty can range from 1-20
    static bool getBestMove(const Board& board, bool whiteTurn, int difficulty, Move& outMove);
};

#endif
