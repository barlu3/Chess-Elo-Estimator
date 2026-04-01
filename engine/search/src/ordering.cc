#include "api/Board.h"
#include "Pieces.h"
#include "eval/material.h"
#include "search/header/ordering.h"

#include <vector>
#include <algorithm>

long long MoveOrder::captureScore(const Move& move, const Board& board) {
    const Piece* attacker = board.getPiece(move.fromRow, move.fromCol);
    const Piece* victim = board.getPiece(move.toRow, move.toCol);

    if (!attacker || !victim) return 0;

    long long attackerValue = Evaluator::materialValue(attacker->symbol());
    long long victimValue = Evaluator::materialValue(victim->symbol());

    return victimValue - attackerValue;
}

void MoveOrder::orderMoves(std::vector<Move>& moves, const Board& board) {
    auto partitionPoint = std::stable_partition(moves.begin(), moves.end(), [&board, &moves](const Move& m) {
            const Piece* attacker = board.getPiece(m.fromRow, m.fromCol); 
            if (!attacker) return false;
            return board.isEnemy(m.toRow, m.toCol, attacker->getColor());
        }
    );

    std::sort(
        moves.begin(), partitionPoint, 
        [&board](const Move& a, const Move& b) {
            return captureScore(a, board) > captureScore(b, board);
        }
    );
}

