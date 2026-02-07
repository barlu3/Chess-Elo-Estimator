#ifndef UNDOMOVE_H
#define UNDOMOVE_H

#include "moveHistory.h"
#include "Game.h"

class undoMove {
    public:
        void undoLatestMove(moveHistory& currentGame, Board& currentBoard);
};

#endif