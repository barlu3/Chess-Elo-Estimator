#ifndef ALPHABETA_H
#define ALPHABETA_H

#include "Game.h"
#include "api/moveHistory.h"
#include "Pieces.h"

struct SearchResult {
    Move bestMove;
    long long score;
};

class AlphaBeta {
    public:
        static long long alphabeta(Game& game, moveHistory& history, int depth, long long alpha, long long beta);

        static SearchResult search(Game& game, moveHistory& history, int depth);
};


#endif
