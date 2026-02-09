#include "api/Board.h"
#include "Game.h"
#include "material.h"

long long materialValue(char type) {
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

long long Evaluator::evaluate(const Game& game) {
    long long score = 0;

    for (const Piece* p : game.getBoard().Pieces()) {
        int value = materialValue(p->symbol());

        if (p->getColor()) { score += value; }
        else { score -= value; }
    }

    if (!game.isWhiteTurn()) { score = -score; }

    return score;
}