#include "Game.h"
#include "Pieces.h"
#include "api/Board.h"
#include "api/moveHistory.h"
#include "api/undoMove.h"
#include "api/convertMoveToString.h"

#include "search/header/alphabeta.h"
#include "search/header/quiescence.h"

#include <vector>

long long AlphaBeta::alphabeta(Game& game, moveHistory& history, int depth, long long alpha, long long beta) {
    if (depth == 0) return Quiescence::quiescence(game);

    std::vector<Move> moves;
    for (int r = 0 ; r < 8; r++) {
        for (int c = 0 ; c < 8 ; c++) {
            if (!game.getBoard().isEmpty(r,c)) {
                const Piece* piece = game.getBoard().getPiece(r,c);
                if(piece->getColor() != game.isWhiteTurn()) continue;
                auto pieceMoves = piece->generateMoves(game.getBoard(), false);
            }
        }
    }

    if (moves.empty()) return game.getBoard().isInCheck(game.isWhiteTurn()) ? -100000 : 0;

    for (const Move& move : moves) {
        std::string record = convertMoveToString::moveAsString(move, game.getBoard());
        if (!game.performMove(move)) continue;
        history.appendLatestMove(record);

        long long score = -alphabeta(game, history, depth - 1, -beta, -alpha);

        undoMove::undoLatestMove(history, game.getBoard());

        if (score >= beta) return beta;     //opponent wont allow this move
        if (score > alpha) alpha = score;   //next best move for player
    }
    return alpha;
}