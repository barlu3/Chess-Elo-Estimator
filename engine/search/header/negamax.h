#ifndef NEGAMAX_H
#define NEGAMAX_H

#include "Game.h"
#include "api/moveHistory.h"

class NegaMax {
    public:
        static long long negamax(Game& game, MoveHistory& history, int depth);
};

#endif