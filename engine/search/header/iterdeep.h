#ifndef ITERDEEP_H
#define ITERDEEP_H

#include "Game.h"
#include "api/moveHistory.h"
#include "search/header/alphabeta.h"

class IterDeep {
    public:
        //searches to increasing depths until timeLimitMs; a depth still running at the deadline is aborted
        //(alphabeta() and quiescence() poll the clock, on every thread), and the best move of the last COMPLETED depth is returned,
        //i.e. if time runs out during the 5th depth return the 4th depth best move
        //if even the first depth is cut off, the partial best of that depth is returned (a legal move, never the sentinel when one exists), with depth == 0
        //minDepth is only the starting depth, it is no longer guaranteed to complete
        //threads is forwarded to the root search at each depth; see AlphaBeta::search
        static SearchResult search(Game& game, MoveHistory& history, int timeLimitMs, int minDepth = 1, int maxDepth = 8, int threads = 1);
};

#endif