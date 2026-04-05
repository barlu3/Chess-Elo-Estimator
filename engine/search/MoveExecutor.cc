#include "MoveExecutor.h"
#include "api/Board.h"

#include <cstdlib>

bool MoveExecutor::make(const Move& move, Game& game, MoveHistory& history) {
    const Board& board = game.getBoard();
    const Piece* fromPos = board.getPiece(move.fromRow, move.fromCol);
    const Piece* toPos = board.getPiece(move.toRow, move.toCol);

    if (!fromPos) return false;

    MoveRecord rec;
    rec.move.fromRow = move.fromRow; rec.move.fromCol = move.fromCol;
    rec.move.toRow   = move.toRow;   rec.move.toCol   = move.toCol;
    rec.whiteTurnBefore    = game.isWhiteTurn();
    rec.movedFlagBefore    = fromPos->hasMoved();
    rec.enPassantBefore    = fromPos->enPassantable();
    rec.capturedPiece      = toPos ? toPos->clone() : nullptr;
    rec.enPassantCapture   = false;
    rec.epCapturedPiece    = nullptr;
    rec.rookMoved          = false;
    rec.wasPromotion       = false;

    char sym        = fromPos->symbol();
    bool isPawn     = (sym == 'P' || sym == 'p');
    bool isKing     = (sym == 'K' || sym == 'k');
    int  colDiff    = move.toCol - move.fromCol;

    if (isPawn && move.fromCol != move.toCol && !toPos) {
        rec.enPassantCapture = true;
        rec.epCapturedRow    = move.fromRow;
        rec.epCapturedCol    = move.toCol;
        const Piece* ep      = board.getPiece(move.fromRow, move.toCol);
        rec.epCapturedPiece  = ep ? ep->clone() : nullptr;
        rec.capturedPiece    = nullptr;
    }

    rec.wasPromotion = isPawn &&
                       ((sym == 'P' && move.toRow == 0) ||
                        (sym == 'p' && move.toRow == 7));

    if (isKing && std::abs(colDiff) == 2) {
        rec.rookMoved           = true;
        rec.rookFromRow         = move.fromRow;
        rec.rookToRow           = move.fromRow;
        rec.rookFromCol         = (colDiff == 2) ? 7 : 0;
        rec.rookToCol           = (colDiff == 2) ? 5 : 3;
        const Piece* rook       = board.getPiece(rec.rookFromRow, rec.rookFromCol);
        rec.rookMovedFlagBefore = rook ? rook->hasMoved() : false;
    }
    rec.promotedFromPiece = nullptr;
    if (rec.wasPromotion) {
        rec.promotedFromPiece = fromPos->clone();
    }
    if (!game.performMove(move)) {
        delete rec.capturedPiece;
        delete rec.epCapturedPiece;
        return false;
    }

    history.push(std::move(rec));
    return true;
}

void MoveExecutor::undo(Game& game, MoveHistory& history) {
    undoMove::undoLatestMove(history, game);
}

std::vector<Move> MoveExecutor::generateMoves(const Game& game) {
    std::vector<Move> moves;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (game.getBoard().isEmpty(r, c)) continue;
            const Piece* p = game.getBoard().getPiece(r, c);
            if (p->getColor() != game.isWhiteTurn()) continue;
            auto pm = p->generateMoves(game.getBoard(), false);
            moves.insert(moves.end(), pm.begin(), pm.end());
        }
    }
    return moves;
}