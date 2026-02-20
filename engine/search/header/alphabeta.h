#ifndef ALPHABETA_H
#define ALPHABETA_H

#include "Game.h"
#include "api/moveHistory.h"

class AlphaBeta {
    public:
        static long long alphabeta(Game& game, moveHistory& history, int depth, long long alpha, long long beta);
};


#endif
