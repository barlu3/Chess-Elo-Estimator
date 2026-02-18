#ifndef NEGAMAX_H
#define NEGAMAX_H

#include "Game.h"
#include "api/moveHistory.h"

class NegaMax {
    public:
        long long negamax(Game& game, moveHistory& history, int depth);
};

#endif