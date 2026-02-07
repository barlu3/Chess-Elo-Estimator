#include "api/undoMove.h"
#include "api/sub/Rook.h"
#include "api/sub/Queen.h"
#include "api/sub/Pawn.h"
#include "api/sub/Knight.h"
#include "api/sub/King.h"
#include "api/sub/Bishop.h"

#include <iostream>

void undoMove::undoLatestMove(moveHistory& currentGame, Board& currentBoard) {
    if (currentGame.currentBoardState >= 1) {
        std::string lastMove = currentGame.moveHistoryVector.back();
        
        //Determining what was last move
        char movedPiece = lastMove.at(0);
        int previousX = static_cast<int>(lastMove.at(1));
        int previousY = static_cast<int>(lastMove.at(2));
        int currentX = static_cast<int>(lastMove.at(3));
        int currentY = static_cast<int>(lastMove.at(4));
        char capturedPiece = lastMove.at(5);
        char specialMoveFlag = lastMove.at(6);

        //Special cases
        switch (specialMoveFlag) {
            //en Passant with black pawn
            case 'p': {
                Pawn* newBlackPawn = new Pawn(previousX, previousY, false);
                Pawn* newWhitePawn = new Pawn(currentX, previousY, true);
                
                newWhitePawn->setEnPassant(true);

                currentBoard.setPiece(previousX, previousY, newBlackPawn);
                currentBoard.removePiece(currentX, currentY);
                currentBoard.setPiece(currentX, previousY, newWhitePawn);

                currentGame.currentBoardState -= 1;
                currentGame.moveHistoryVector.pop_back();
                
                return;
            }
            //en Passant with white pawn
            case 'P': {
                Pawn* newWhitePawn = new Pawn(previousX, previousY, true);
                Pawn* newBlackPawn = new Pawn(currentX, previousY, false);
                
                newBlackPawn->setEnPassant(true);

                currentBoard.setPiece(previousX, previousY, newWhitePawn);
                currentBoard.removePiece(currentX, currentY);
                currentBoard.setPiece(currentX, previousY, newBlackPawn);

                currentGame.currentBoardState -= 1;
                currentGame.moveHistoryVector.pop_back();

                return;
            }
            //king-side castle
            case 'c':
                //Black king
                if (movedPiece == 'k') {
                    King* newBlackKing = new King(previousX, previousY, false);
                    Rook* newBlackRook = new Rook(7, 7, false);

                    currentBoard.removePiece(7, 5);
                    currentBoard.removePiece(7, 6);

                    currentBoard.setPiece(7, 4, newBlackKing);
                    currentBoard.setPiece(7, 7, newBlackRook);

                    currentGame.currentBoardState -= 1;
                    currentGame.moveHistoryVector.pop_back();

                    return;
                    break;
                }
                //White king
                else if (movedPiece == 'K') {
                    King* newWhiteKing = new King(previousX, previousY, true);
                    Rook* newWhiteRook = new Rook(0, 7, true);

                    currentBoard.removePiece(0, 5);
                    currentBoard.removePiece(0, 6);

                    currentBoard.setPiece(0, 4, newWhiteKing);
                    currentBoard.setPiece(0, 7, newWhiteRook);

                    currentGame.currentBoardState -= 1;
                    currentGame.moveHistoryVector.pop_back();

                    return;
                    break;
                }
            //Queen-side castle
            case 'C':
                //Black king
                if (movedPiece == 'k') {
                    King* newBlackKing = new King(previousX, previousY, false);
                    Rook* newBlackRook = new Rook(7, 0, false);

                    currentBoard.removePiece(7, 1);
                    currentBoard.removePiece(7, 2);

                    currentBoard.setPiece(7, 4, newBlackKing);
                    currentBoard.setPiece(7, 0, newBlackRook);

                    currentGame.currentBoardState -= 1;
                    currentGame.moveHistoryVector.pop_back();

                    return;
                    break;
                }
                //White king
                else if (movedPiece == 'K') {
                    King* newWhiteKing = new King(previousX, previousY, true);
                    Rook* newWhiteRook = new Rook(0, 0, true);

                    currentBoard.removePiece(0, 1);
                    currentBoard.removePiece(0, 2);

                    currentBoard.setPiece(0, 4, newWhiteKing);
                    currentBoard.setPiece(0, 0, newWhiteRook);

                    currentGame.currentBoardState -= 1;
                    currentGame.moveHistoryVector.pop_back();

                    return;
                    break;
                }
            default:
                break;
        }

        //Typical cases
        switch (movedPiece) {
            // pawn
            case 'p': {
                Pawn* movedPiecePtr = new Pawn(previousX, previousY, false);
                currentBoard.setPiece(previousX, previousY, movedPiecePtr);
                break;
            }
            case 'P': {
                Pawn* movedPiecePtr = new Pawn(previousX, previousY, true);
                currentBoard.setPiece(previousX, previousY, movedPiecePtr);
                break;
            }

            // rook
            case 'r': {
                Rook* movedPiecePtr = new Rook(previousX, previousY, false);
                currentBoard.setPiece(previousX, previousY, movedPiecePtr);
                break;
            }
            case 'R': {
                Rook* movedPiecePtr = new Rook(previousX, previousY, true);
                currentBoard.setPiece(previousX, previousY, movedPiecePtr);
                break;
            }

            // bishop
            case 'b': {
                Bishop* movedPiecePtr = new Bishop(previousX, previousY, false);
                currentBoard.setPiece(previousX, previousY, movedPiecePtr);
                break;
            }
            case 'B': {
                Bishop* movedPiecePtr = new Bishop(previousX, previousY, true);
                currentBoard.setPiece(previousX, previousY, movedPiecePtr);
                break;
            }

            // knight
            case 'n': {
                Knight* movedPiecePtr = new Knight(previousX, previousY, false);
                currentBoard.setPiece(previousX, previousY, movedPiecePtr);
                break;
            }
            case 'N': {
                Knight* movedPiecePtr = new Knight(previousX, previousY, true);
                currentBoard.setPiece(previousX, previousY, movedPiecePtr);
                break;
            }

            // queen
            case 'q': {
                Queen* movedPiecePtr = new Queen(previousX, previousY, false);
                currentBoard.setPiece(previousX, previousY, movedPiecePtr);
                break;
            }
            case 'Q': {
                Queen* movedPiecePtr = new Queen(previousX, previousY, true);
                currentBoard.setPiece(previousX, previousY, movedPiecePtr);
                break;
            }

            // king
            case 'k': {
                King* movedPiecePtr = new King(previousX, previousY, false);
                currentBoard.setPiece(previousX, previousY, movedPiecePtr);
                break;
            }
            case 'K': {
                King* movedPiecePtr = new King(previousX, previousY, true);
                currentBoard.setPiece(previousX, previousY, movedPiecePtr);
                break;
            }
        }

        currentBoard.removePiece(currentX, currentY);

        switch (capturedPiece) {
            // pawn
            case 'p': {
                Pawn* capturedPiecePtr = new Pawn(currentX, currentY, false);
                currentBoard.setPiece(currentX, currentY, capturedPiecePtr);
                break;
            }
            case 'P': {
                Pawn* capturedPiecePtr = new Pawn(currentX, currentY, true);
                currentBoard.setPiece(currentX, currentY, capturedPiecePtr);
                break;
            }

            // rook
            case 'r': {
                Rook* capturedPiecePtr = new Rook(currentX, currentY, false);
                currentBoard.setPiece(currentX, currentY, capturedPiecePtr);
                break;
            }
            case 'R': {
                Rook* capturedPiecePtr = new Rook(currentX, currentY, true);
                currentBoard.setPiece(currentX, currentY, capturedPiecePtr);
                break;
            }

            // bishop
            case 'b': {
                Bishop* capturedPiecePtr = new Bishop(currentX, currentY, false);
                currentBoard.setPiece(currentX, currentY, capturedPiecePtr);
                break;
            }
            case 'B': {
                Bishop* capturedPiecePtr = new Bishop(currentX, currentY, true);
                currentBoard.setPiece(currentX, currentY, capturedPiecePtr);
                break;
            }

            // knight
            case 'n': {
                Knight* capturedPiecePtr = new Knight(currentX, currentY, false);
                currentBoard.setPiece(currentX, currentY, capturedPiecePtr);
                break;
            }
            case 'N': {
                Knight* capturedPiecePtr = new Knight(currentX, currentY, true);
                currentBoard.setPiece(currentX, currentY, capturedPiecePtr);
                break;
            }

            // queen
            case 'q': {
                Queen* capturedPiecePtr = new Queen(currentX, currentY, false);
                currentBoard.setPiece(currentX, currentY, capturedPiecePtr);
                break;
            }
            case 'Q': {
                Queen* capturedPiecePtr = new Queen(currentX, currentY, true);
                currentBoard.setPiece(currentX, currentY, capturedPiecePtr);
                break;
            }

            // king
            case 'k': {
                King* capturedPiecePtr = new King(currentX, currentY, false);
                currentBoard.setPiece(currentX, currentY, capturedPiecePtr);
                break;
            }
            case 'K': {
                King* capturedPiecePtr = new King(currentX, currentY, true);
                currentBoard.setPiece(currentX, currentY, capturedPiecePtr);
                break;
            }
        }

        currentGame.currentBoardState -= 1;
        currentGame.moveHistoryVector.pop_back();

    } else {
        std::cout << "To early to undo any move.\n";
    }
}
