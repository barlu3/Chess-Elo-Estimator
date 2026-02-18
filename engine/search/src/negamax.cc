#include "Game.h"
#include "Board.h"
#include "Pieces.h"
#include "api/moveHistory.h"
#include "api/undoMove.h"
#include "api/convertMoveToString.h"

#include "search/header/negamax.h"
#include "eval/material.h"

#include <vector>

long long NegaMax::negamax(Game& game, moveHistory& history, int depth) {
    if (depth == 0) return Evaluator::evaluate(game);

    std::vector<Move> moves;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (!game.getBoard().isEmpty(r,c)) {
                auto pieceMoves = game.getBoard().getPiece(r,c)->generateMoves(game.getBoard(), false);
                moves.insert(moves.end(), pieceMoves.begin(), pieceMoves.end());
            }
        }
    }

    if (moves.empty()) {
        return game.getBoard().isInCheck(game.isWhiteTurn()) ? -100000 : 0;
    }

    long long best = -1e18;
    for (const Move& move : moves) {
        std::string record = convertMoveToString::moveAsString(move, game.getBoard());
        game.performMove(move);                                                                 //pseudo perform the move
        history.appendLatestMove(record);     //append latest move to move history
        long long score = -negamax(game, history, depth - 1);                                   //recursively calculate the best move
        undoMove::undoLatestMove(history, game.getBoard());                                     //undo all the pseudo moves
        best = std::max(best,score);                                                            //set best score
    }
    return best;
}