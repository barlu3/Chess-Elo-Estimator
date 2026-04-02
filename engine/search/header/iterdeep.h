#ifndef ITERDEEP_H
#define ITERDEEP_H

#include "Game.h"
#include "api/moveHistory.h"
#include "search/header/alphabeta.h"

class IterDeep {
    public:
        //searches to increasing depths until timeLimitMs is exceeded
        //always** returns the best move from the last fully completed depth
        //i.e. if time exceeds during 5th depth return the 4th depth best move
        static SearchResult search(Game& game, moveHistory& history, int timeLimitMs, int minDepth = 1, int maxDepth = 64);
};

#endif