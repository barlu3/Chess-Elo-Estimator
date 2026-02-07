#include "api/sub/King.h"
#include "api/sub/Rook.h"
#include "api/Board.h"

#include <vector>
#include <iostream>

using namespace std;

std::vector<Move> King::generateMoves(const Board& board, bool pseudoLegal) const {
    std::vector<Move> moves;

    static const int directions[8][2] = {
        {1, 0}, //down
        {-1, 0}, //up
        {0, 1}, //right
        {0, -1}, //left
        {1, 1}, //down-right
        {1, -1}, //down-left
        {-1, 1}, //up-right
        {-1, -1} //up-left
    };

    for (auto& d : directions) {
        int r = row + d[0];
        int c = col + d[1];

        //if next 'square' is out of bounds, try another square
        if (!board.inBounds(r,c)) continue;
        //if square is empty or enemy, try to add to move set
        if (board.isEmpty(r,c) || board.isEnemy(r,c,isWhite)) {
            Move m = {row, col, r, c};
            
            if (pseudoLegal) {
                moves.push_back(m);
            }
            //doesnt cause king to be in check, add to move set
            else if (!board.isInCheckAfterMove(m, isWhite)) {
                moves.push_back(m);
            }
        }
    }

    bool canCastle = (!hasMoved() && !pseudoLegal && !board.isInCheck(getColor()));

    if (canCastle) {

        //king side castle
        if (!board.isEmpty(row, 7)) {
            //has the original king side rook moved
            if (dynamic_cast<const Rook*>(board.getPiece(row, 7)) && !board.getPiece(row, 7)->hasMoved()) {
                //is the rest of the row between king and rook empty
                if (board.isEmpty(row, 5) && board.isEmpty(row, 6)) {
                    //are any of the squares being attacked
                    if (!board.isAttacked(row, 5, getColor()) && !board.isAttacked(row, 6, getColor())) {
                        moves.push_back({row, col, row, 6});
                    }
                }
            }
        }
        //queen side castle
        if (!board.isEmpty(row, 0)) {
            //has the original queen side rook moved
            if (dynamic_cast<const Rook*>(board.getPiece(row, 0)) && !board.getPiece(row, 0)->hasMoved()) {
                //is the rest of the row between king and rook empty
                if (board.isEmpty(row, 1) && board.isEmpty(row, 2) && board.isEmpty(row, 3)) {
                    //are any squares being attacked
                    if (!board.isAttacked(row, 1, getColor()) && !board.isAttacked(row, 2, getColor()) && !board.isAttacked(row, 3, getColor())) {
                        moves.push_back({row, col, row, 2});
                    }
                }
            }
        }
    }    

    return moves;
}
