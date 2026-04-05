#include "Game.h"
#include "Pieces.h"
#include "api/Board.h"
#include "api/moveHistory.h"
#include "api/undoMove.h"
#include "api/convertMoveToString.h"

#include "search/MoveExecutor.h"

#include "search/header/negamax.h"
#include "eval/material.h"
#include "search/header/ordering.h"

#include <vector>

long long NegaMax::negamax(Game& game, MoveHistory& history, int depth) {
    if (depth == 0) return Evaluator::evaluate(game);

    std::vector<Move> moves = MoveExecutor::generateMoves(game);

    if (moves.empty()) {
        return game.getBoard().isInCheck(game.isWhiteTurn()) ? -100000 : 0;
    }

    // MoveOrder::orderMoves(moves, game.getBoard());

    long long best = -1e18;
    for (auto move : moves) {
        // std::string record = convertMoveToString::moveAsString(move, game.getBoard());
        if (!MoveExecutor::make(move, game, history)) continue;                                                                 //pseudo perform the move append latest move to move history

        best = std::max(best,-negamax(game, history, depth - 1));                                                            //set best score recursively calculate the best move
        
        MoveExecutor::undo(game,history);                                     //undo all the pseudo moves
    }
    return best;
}