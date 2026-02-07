#include "api/sub/Pawn.h"
#include "api/Board.h"

#include <vector>

std::vector<Move> Pawn::generateMoves(const Board& board, bool pseudoLegal) const {
    std::vector<Move> moves;

    //should never be a nullptr so no need to check (we can check before we call generateMoves)
    int forward = (getColor()) ? -1 : 1;

    int r1 = row + forward;
    int c1 = col;
    
    //foward 1 step
    if (board.inBounds(r1, c1) && board.isEmpty(r1, c1)) {
        Move m = {row, col, r1, c1};
        if (pseudoLegal || !board.isInCheckAfterMove(m, getColor())) {
            moves.push_back(m);
        }
        //forward 2 step
        int startRow = (board.getPiece(row,col)->getColor()) ? 6 : 1;
        int r2 = row + 2 * forward;
        if (row == startRow && board.isEmpty(r2, c1)) {
            if (pseudoLegal || !board.isInCheckAfterMove(m, getColor())) {
                moves.push_back({row, col, r2, c1});
            }
        }
    }

    //attacking left
    int cLeft = col - 1;
    if (board.inBounds(r1, cLeft)) {
        if (!(board.isEmpty(r1, cLeft)) && board.isEnemy(r1, cLeft, this->getColor())) {
            Move m = {row, col, r1, cLeft};
            if (pseudoLegal || !board.isInCheckAfterMove(m, getColor())){
                moves.push_back({row, col, r1, cLeft});
            }
        }
        //en Passant
        if (board.isEmpty(r1, cLeft) && 
            !(board.isEmpty(row, cLeft)) && 
            board.isEnemy(row, cLeft, this->getColor()) &&
            board.getPiece(row, cLeft)->enPassantable())
        {
            Move m = {row, col, r1, cLeft};
            if (pseudoLegal || !board.isInCheckAfterMove(m, getColor())){
                moves.push_back({row, col, r1, cLeft});
            }
        }
    }

    //attacking right
    int cRight = col + 1;
    if (board.inBounds(r1, cRight)){
        if (!(board.isEmpty(r1, cRight)) && board.isEnemy(r1, cRight, this->getColor())) {
            Move m = {row, col, r1, cRight};
            if (pseudoLegal || !board.isInCheckAfterMove(m, getColor())) {
                moves.push_back({row, col, r1, cRight});
            }
        }
        //en Passant
        if (board.isEmpty(r1, cRight) && 
            !(board.isEmpty(row, cRight)) && 
            board.isEnemy(row, cRight, this->getColor()) &&
            board.getPiece(row, cRight)->enPassantable())
        {
            Move m = {row, col, r1, cRight};
            if (pseudoLegal || !board.isInCheckAfterMove(m, getColor())){
                moves.push_back({row, col, r1, cRight});
            }
        }
    }

    return moves;
}
