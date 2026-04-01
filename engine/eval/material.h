#ifndef MATERIAL_H
#define MATERIAL_H

#include "Game.h"

class Evaluator {
    public:
        static long long evaluate(const Game& game);
        static long long materialValue(char type) {
            char loweredType = std::tolower(static_cast<unsigned char>(type));

            switch (loweredType) {
                case 'p': return 100;
                case 'n': return 320;
                case 'b': return 330;
                case 'r': return 500;
                case 'q': return 900;
                case 'k': return 0;
            }
            return 0;
        }
};

#endif