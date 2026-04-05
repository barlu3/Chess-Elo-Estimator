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
long long Quiescence::quiescence(Game& game, moveHistory& history, long long alpha, long long beta, int checkDepth) {
    bool inCheck = game.getBoard().isInCheck(game.isWhiteTurn());
    
    //only take standPat if not in check, otherwise we need to find a legal move to get out
    if (!inCheck) {
        long long standPat = Evaluator::evaluate(game);

        if (standPat >= beta) return beta;
        if (standPat > alpha) alpha = standPat;
    }   

    std::vector<Move> moves;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (!game.getBoard().isEmpty(r, c)) {
                const Piece* piece = game.getBoard().getPiece(r,c);
                if (piece->getColor() != game.isWhiteTurn()) continue;
                auto pieceMoves = piece->generateMoves(game.getBoard(), false);
                for (const Move& m : pieceMoves) {
                    bool isCapture = game.getBoard().isEnemy(m.toRow, m.toCol, game.isWhiteTurn());
                    //include checks within checkDepth budget (if checkDepth = 0 we cant consider checks)
                    bool givesCheck = (checkDepth > 0) && game.getBoard().isInCheckAfterMove(m, !game.isWhiteTurn());
                    if (isCapture || inCheck || givesCheck) {
                        moves.push_back(m);
                    }
                }
            }
        }
    }

    //if we're in check with no moves, its a checkmate
    if (inCheck && moves.empty()) return -100000;

    for (auto move : moves) {
        // std::string record = convertMoveToString::moveAsString(move, game.getBoard());
        Board snap = game.getBoard();
        bool turn = game.isWhiteTurn();
        if (!game.performMove(move)) continue;
        history.appendSnapshot(snap, turn);

        long long score = -quiescence(game, history, -beta, -alpha, checkDepth - 1);

        undoMove::undoLatestMove(history, game);

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}