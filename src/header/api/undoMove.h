#ifndef UNDOMOVE_H
#define UNDOMOVE_H

#include "moveHistory.h"
#include "Game.h"

class undoMove {
    public:
        static void undoLatestMove(MoveHistory& history, Game& game) {
            if (history.currentBoardState < 1) {
                std::cout << "Too early to undo any move.\n";
                return;
            }

            MoveRecord& rec = history.top();
            Board& board = game.getBoard();

            game.setTurn(rec.whiteTurnBefore);

            // promotion: queen at dest is not the original piece — handle separately
            if (rec.wasPromotion) {
                board.removePiece(rec.move.toRow, rec.move.toCol);  // delete queen
                rec.promotedFromPiece->setPosition(rec.move.fromRow, rec.move.fromCol);
                board.setPieceRaw(rec.move.fromRow, rec.move.fromCol, rec.promotedFromPiece);
                rec.promotedFromPiece = nullptr;

                // restore any piece that was on the promotion square
                if (rec.capturedPiece) {
                    board.setPieceRaw(rec.move.toRow, rec.move.toCol, rec.capturedPiece);
                    rec.capturedPiece = nullptr;
                }

                history.pop();
                return;
            }

            // !!TODO!!: refactor, reduce code blaot
            // normal move: move piece back
            Piece* mp = const_cast<Piece*>(board.getPiece(rec.move.toRow, rec.move.toCol));
            mp->setPosition(rec.move.fromRow, rec.move.fromCol);
            if (!rec.movedFlagBefore) mp->unsetMoved();
            mp->setEnPassant(rec.enPassantBefore);
            board.setPieceRaw(rec.move.fromRow, rec.move.fromCol, mp);
            board.setPieceRaw(rec.move.toRow,   rec.move.toCol,   nullptr);

            // restore captured piece — transfer ownership, don't clone
            if (rec.capturedPiece && !rec.enPassantCapture) {
                board.setPieceRaw(rec.move.toRow, rec.move.toCol, rec.capturedPiece);
                rec.capturedPiece = nullptr;
            }

            // restore en passant captured pawn — transfer ownership
            if (rec.enPassantCapture && rec.epCapturedPiece) {
                board.setPieceRaw(rec.epCapturedRow, rec.epCapturedCol, rec.epCapturedPiece);
                rec.epCapturedPiece = nullptr;
            }

            // undo castling rook
            if (rec.rookMoved) {
                Piece* rook = const_cast<Piece*>(board.getPiece(rec.rookToRow, rec.rookToCol));
                rook->setPosition(rec.rookFromRow, rec.rookFromCol);
                if (!rec.rookMovedFlagBefore) rook->unsetMoved();
                board.setPieceRaw(rec.rookFromRow, rec.rookFromCol, rook);
                board.setPieceRaw(rec.rookToRow,   rec.rookToCol,   nullptr);
            }

            if (rec.enPassantClearedRow != -1) {
                Piece* p = const_cast<Piece*>(
                    board.getPiece(rec.enPassantClearedRow, rec.enPassantClearedCol));
                if (p) p->setEnPassant(true);
            }

            history.pop();
        }
};

#endif