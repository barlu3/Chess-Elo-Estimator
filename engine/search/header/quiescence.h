#ifndef QUIESCENCE_H
#define QUIESCENCE_H

#include "Game.h"
#include "api/moveHistory.h"

class Quiescence {
    public:
        static long long quiescence(Game& game, MoveHistory& history, long long alpha, long long beta, int checkDepth);
};

#endif