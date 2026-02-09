#ifndef MATERIAL_H
#define MATERIAL_H

#include "api/Board.h"
#include "Game.h"

class Evaluator {
    public:
        static long long evaluate(const Game& game);
};

#endif