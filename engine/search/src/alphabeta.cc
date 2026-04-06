#include "Game.h"
#include "Pieces.h"
#include "api/Board.h"
#include "api/moveHistory.h"
#include "api/undoMove.h"
#include "api/convertMoveToString.h"

#include "search/MoveExecutor.h"

#include "search/header/alphabeta.h"
#include "search/header/quiescence.h"
#include "search/header/ordering.h"

#include <vector>

long long AlphaBeta::alphabeta(Game& game, MoveHistory& history, int depth, long long alpha, long long beta) {
    if (depth == 0) return Quiescence::quiescence(game, history, alpha, beta, 1);

    std::vector<Move> moves = MoveExecutor::generateMoves(game);

    if (moves.empty()) return game.getBoard().isInCheck(game.isWhiteTurn()) ? -100000 : 0;

    MoveOrder::orderMoves(moves, game.getBoard());
    
    for (const Move& move : moves) {
        if (!MoveExecutor::make(move, game, history)) continue;

        long long score = -alphabeta(game, history, depth - 1, -beta, -alpha);
        
        MoveExecutor::undo(game,history);

        if (score >= beta) return beta;     //opponent wont allow this move
        if (score > alpha) alpha = score;   //next best move for player
    }
    return alpha;
}

SearchResult AlphaBeta::search(Game& game, MoveHistory& history, int depth) {
    static const long long INF = 1e18;

    std::vector<Move> moves = MoveExecutor::generateMoves(game);

    MoveOrder::orderMoves(moves, game.getBoard());

    SearchResult result;
    result.score = -INF;
    result.bestMove = moves.empty() ? Move{-1, -1, -1, -1} : moves[0];

    long long alpha = -INF;
    long long beta = INF;

    for (auto move : moves) {
        if (!MoveExecutor::make(move, game, history)) continue;
        
        long long score = -alphabeta(game, history, depth - 1, -beta, -alpha);
        
        MoveExecutor::undo(game, history);

        if (score > result.score) {
            result.score = score;
            result.bestMove = move;
        }
        if (score > alpha) alpha = score;
    }

    return result;
}