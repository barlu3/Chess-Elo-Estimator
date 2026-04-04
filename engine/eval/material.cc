#include "api/Board.h"
#include "Game.h"
#include "material.h"
#include "tables/header/pst.h"

long long Evaluator::evaluate(const Game& game) {
    long long score = 0;

    for (const Piece* p : game.getBoard().Pieces()) {
        long long value = Evaluator::materialValue(p->symbol()) + PST::pstValue(p->symbol(), p->getColor(), p->getRow(), p->getCol());

        if (p->getColor()) { score += value; }
        else { score -= value; }
    }

    if (!game.isWhiteTurn()) { score = -score; }

    return score;
}