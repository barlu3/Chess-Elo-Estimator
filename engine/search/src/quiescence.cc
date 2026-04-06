#include "Game.h"
#include "Pieces.h"
#include "api/Board.h"
#include "api/moveHistory.h"
#include "api/undoMove.h"
#include "api/convertMoveToString.h"

#include "search/MoveExecutor.h"

#include "search/header/quiescence.h"
#include "eval/material.h"

#include <vector>

//ensures that we care about captures instead of moves that have little to no affect on the game
long long Quiescence::quiescence(Game& game, MoveHistory& history, long long alpha, long long beta, int checkDepth) {
    bool inCheck = game.getBoard().isInCheck(game.isWhiteTurn());
    
    //only take standPat if not in check, otherwise we need to find a legal move to get out
    if (!inCheck) {
        long long standPat = Evaluator::evaluate(game);

        if (standPat >= beta) return beta;
        if (standPat > alpha) alpha = standPat;
    }   

    auto moves = MoveExecutor::generateMoves(game);
    std::vector<Move> candidates;
    for (const Move& m : moves) {
        bool isCapture = game.getBoard().isEnemy(m.toRow, m.toCol, game.isWhiteTurn());
        bool givesCheck = (checkDepth > 0) && game.getBoard().isInCheckAfterMove(m, !game.isWhiteTurn());

        if (isCapture || inCheck || givesCheck) {
            candidates.push_back(m);
        }
    }

    //if we're in check with no moves, its a checkmate
    if (inCheck && candidates.empty()) return -100000;

    for (const Move& move : candidates) {
        
        if (!MoveExecutor::make(move, game, history)) continue;

        long long score = -quiescence(game, history, -beta, -alpha, checkDepth - 1);

        MoveExecutor::undo(game, history);
       
        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}