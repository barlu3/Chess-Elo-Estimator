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
        static inline bool quiescence = true; //false: depth-0 nodes return Evaluator::evaluate instead of calling quiescence
        //per-thread call counter; search() sums the workers' counters into SearchResult
        static inline thread_local long long nodes = 0;

        static long long alphabeta(Game& game, MoveHistory& history, int depth, long long alpha, long long beta);

        //threads<=1 runs the sequential root; threads>1 seeds with root move 0, then
        //workers take the remaining root moves dynamically, each with a window narrowed
        //by the shared best score. The returned move and score equal the sequential
        //search (ties go to the earliest root move); the node count can differ.
        //Each worker copies the Game and uses its own MoveHistory.
        static SearchResult search(Game& game, MoveHistory& history, int depth, int threads = 1);
};


#endif
