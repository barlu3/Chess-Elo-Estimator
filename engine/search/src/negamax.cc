#include "Game.h"
#include "Pieces.h"
#include "api/Board.h"
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
                const Piece* piece = game.getBoard().getPiece(r,c);
                if (piece->getColor() != game.isWhiteTurn()) continue;
                auto pieceMoves = piece->generateMoves(game.getBoard(), false);
                moves.insert(moves.end(), pieceMoves.begin(), pieceMoves.end());
            }
        }
    }

    if (moves.empty()) {
        return game.getBoard().isInCheck(game.isWhiteTurn()) ? -100000 : 0;
    }

    long long best = -1e18;
    for (auto move : moves) {
        std::string record = convertMoveToString::moveAsString(move, game.getBoard());
        if (!game.performMove(move)) continue;                                                                 //pseudo perform the move
        history.appendLatestMove(record);     //append latest move to move history

        long long score = -negamax(game, history, depth - 1);                                   //recursively calculate the best move
        
        undoMove::undoLatestMove(history, game.getBoard(), game);                                     //undo all the pseudo moves
        best = std::max(best,score);                                                            //set best score
    }
    return best;
}