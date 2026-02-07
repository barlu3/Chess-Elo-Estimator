#include "api/convertMoveToString.h"

#include <string>

std::string convertMoveToString::moveAsString(Move move, Board currentBoard) {
    int movingPieceX = move.fromRow;
    int movingPieceY = move.fromCol;
    int destinationX = move.toRow;
    int destinationY = move.toCol;

    char movingPiece = ' ';
    if (currentBoard.getPiece(movingPieceX, movingPieceY) != nullptr) {
        movingPiece = currentBoard.getPiece(movingPieceX, movingPieceY)->symbol();
    }

    char capturedPiece = ' ';
    if (currentBoard.getPiece(destinationX, destinationY) != nullptr) {
        capturedPiece = currentBoard.getPiece(destinationX, destinationY)->symbol();
    }

    char specialMoveFlag = ' ';
    switch (movingPiece) {
        //Had to split cases to avoid "same label"
        case ('p'):
            // Detecting en Passant, black
            if (abs(destinationY - movingPieceY) == 1 && capturedPiece == ' ') {
                specialMoveFlag = movingPiece;
            }
            break;
        case ('P'):
            // Detecting en Passant, white
            if (abs(destinationY - movingPieceY) == 1 && capturedPiece == ' ') {
                specialMoveFlag = movingPiece;
            }
            break;
        case ('k'):
            // King-side castle, black
            if (abs(destinationX - movingPieceX) == 2) {
                specialMoveFlag = 'c';
            }
            // Queen-side castle, black
            else if (abs(destinationX - movingPieceX) == 3) {
                specialMoveFlag = 'C';
            }
        case ('K'):
            // King-side castle, white
            if (abs(destinationX - movingPieceX) == 2) {
                specialMoveFlag = 'c';
            }
            // Queen-side castle, white
            else if (abs(destinationX - movingPieceX) == 3) {
                specialMoveFlag = 'C';
            }
        default:
            break;
    }

    std::string finalString;
    finalString.push_back(movingPiece);
    finalString.push_back(movingPieceX);
    finalString.push_back(movingPieceY);
    finalString.push_back(destinationX);
    finalString.push_back(destinationY);
    finalString.push_back(capturedPiece);
    finalString.push_back(specialMoveFlag);

    return finalString;
}
