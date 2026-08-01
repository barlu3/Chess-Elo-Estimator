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
        static long long alphabeta(Game& game, MoveHistory& history, int depth, long long alpha, long long beta);

        //threads<=1 runs the sequential root; threads>1 splits the root moves across
        //workers, each with its own Game copy and its own MoveHistory. Parallel workers
        //cannot share the narrowing alpha, so they visit more nodes than the sequential
        //search, but agree on the returned score. Ties break on the earliest root move
        //so the result stays reproducible regardless of thread scheduling.
        static SearchResult search(Game& game, MoveHistory& history, int depth, int threads = 1);
};


#endif
