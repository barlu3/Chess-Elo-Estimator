#ifndef QUIESCENCE_H
#define QUIESCENCE_H

#include "Game.h"
#include "api/moveHistory.h"

class Quiescence {
    public:
        static long long quiescence(Game& game, MoveHistory& history, long long alpha, long long beta, int checkDepth);

        //per-thread call counter for benchmarks; search() sums these so the hot path has no shared writes
        static inline thread_local long long nodes = 0;
};

#endif