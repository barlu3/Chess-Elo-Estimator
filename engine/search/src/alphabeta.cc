#include "Game.h"
#include "Pieces.h"
#include "api/Board.h"
#include "api/moveHistory.h"
#include "api/undoMove.h"
#include "api/convertMoveToString.h"

#include "search/header/alphabeta.h"
#include "search/header/quiescence.h"
#include "search/header/ordering.h"

#include <vector>

long long AlphaBeta::alphabeta(Game& game, moveHistory& history, int depth, long long alpha, long long beta) {
    if (depth == 0) return Quiescence::quiescence(game, history, alpha, beta, 1);

    std::vector<Move> moves;

    for (int r = 0 ; r < 8; r++) {
        for (int c = 0 ; c < 8 ; c++) {
            if (!game.getBoard().isEmpty(r,c)) {
                const Piece* piece = game.getBoard().getPiece(r,c);
                if(piece->getColor() != game.isWhiteTurn()) continue;
                auto pieceMoves = piece->generateMoves(game.getBoard(), false);
                moves.insert(moves.end(), pieceMoves.begin(), pieceMoves.end());
            }
        }
    }

    if (moves.empty()) return game.getBoard().isInCheck(game.isWhiteTurn()) ? -100000 : 0;

    MoveOrder::orderMoves(moves, game.getBoard());
    for (const Move& move : moves) {
        std::string record = convertMoveToString::moveAsString(move, game.getBoard());
        if (!game.performMove(move)) continue;
        history.appendLatestMove(record);

        long long score = -alphabeta(game, history, depth - 1, -beta, -alpha);

        undoMove::undoLatestMove(history, game.getBoard(), game);

        if (score >= beta) return beta;     //opponent wont allow this move
        if (score > alpha) alpha = score;   //next best move for player
    }
    return alpha;
}

SearchResult AlphaBeta::search(Game& game, moveHistory& history, int depth) {
    static const long long INF = 1e18;

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

    MoveOrder::orderMoves(moves, game.getBoard());

    SearchResult result;
    result.score = -INF;
    result.bestMove = moves.empty() ? Move{-1, -1, -1, -1} : moves[0];

    long long alpha = -INF;
    long long beta = INF;

    for (auto move : moves) {
        std::string record = convertMoveToString::moveAsString(move, game.getBoard());
        if (!game.performMove(move)) continue;
        history.appendLatestMove(record);
        
        long long score = -alphabeta(game, history, depth - 1, -beta, -alpha);

        undoMove::undoLatestMove(history, game.getBoard(), game);

        if (score > result.score) {
            result.score = score;
            result.bestMove = move;
        }
        if (score > alpha) alpha = score;
    }

    return result;
}