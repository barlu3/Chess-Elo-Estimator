#ifndef ALPHABETA_H
#define ALPHABETA_H

#include "Game.h"
#include "api/moveHistory.h"
#include "Pieces.h"

struct SearchResult {
    Move bestMove;
    long long score;
    long long mainNodes = 0;        //alphabeta() calls during this search(), summed over all workers
    long long quiescenceNodes = 0;  //quiescence() calls during this search(), summed over all workers
};

class AlphaBeta {
    public:
        //benchmark switches: defaults are the normal engine; set them before search(), never during one
        static inline bool pruning = true;   //beta cutoff in alphabeta()
        static inline bool ordering = true;  //MoveOrder::orderMoves at the root and inside alphabeta()
        //per-thread call counter; search() sums the workers' counters into SearchResult
        static inline thread_local long long nodes = 0;

        static long long alphabeta(Game& game, MoveHistory& history, int depth, long long alpha, long long beta);

        //threads<=1 runs the sequential root; threads>1 splits the root moves across
        //workers, each with its own Game copy and its own MoveHistory. Parallel workers
        //cannot share the narrowing alpha, so they visit more nodes than the sequential
        //search, but agree on the returned score. Ties break on the earliest root move
        //so the result stays reproducible regardless of thread scheduling.
        static SearchResult search(Game& game, MoveHistory& history, int depth, int threads = 1);
};


#endif
