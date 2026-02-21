#include "Game.h"
#include "Pieces.h"
#include "api/Board.h"
#include "api/moveHistory.h"
#include "api/undoMove.h"
#include "api/convertMoveToString.h"

#include "search/header/quiescence.h"
#include "eval/material.h"

#include <vector>

//ensures that we care about captures instead of moves that have little to no affect on the game
long long Quiescence::quiescence(Game& game, moveHistory& history, long long alpha, long long beta) {
    long long standPat = Evaluator::evaluate(game);

    if (standPat >= beta) return beta;
    if (standPat > alpha) alpha = standPat;

    std::vector<Move> captures;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (!game.getBoard().isEmpty(r, c)) {
                const Piece* piece = game.getBoard().getPiece(r,c);
                if (piece->getColor() != game.isWhiteTurn()) continue;
                auto pieceMoves = piece->generateMoves(game.getBoard(), false);
                for (const Move& m : pieceMoves) {
                    if (game.getBoard().isEnemy(m.toRow, m.toCol, game.isWhiteTurn())) {
                        captures.push_back(m);
                    }
                }
            }
        }
    }

    for (const Move& move : captures) {
        std::string record = convertMoveToString::moveAsString(move, game.getBoard());
        if (!game.performMove(move)) continue;
        history.appendLatestMove(record);

        long long score = -quiescence(game, history, -beta, -alpha);

        undoMove::undoLatestMove(history, game.getBoard());

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}