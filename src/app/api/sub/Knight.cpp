#include "api/sub/Knight.h"
#include "api/Board.h"

#include <vector>

std::vector<Move> Knight::generateMoves(const Board& board, bool pseudoLegal) const {
    std::vector<Move> moves;

    const static int directions[8][2] = {
        {2, 1}, //down-right
        {2, -1}, //down-left
        {-2, 1}, //up-right
        {-2, -1}, //up-left
        {1, 2}, 
        {-1, 2},
        {1, -2},
        {-1, -2}
    };

    for (auto& d : directions) {
        int r = row + d[0];
        int c = col + d[1];
        //if out of bounds, try another direction
        if (!board.inBounds(r,c)) continue;
        //if square is empty, add to moves
        Move m = {row, col, r, c};
        if (board.isEmpty(r,c)) {
            if (pseudoLegal) {
                moves.push_back(m);
            }
            else if (!board.isInCheckAfterMove(m, getColor())) {
                moves.push_back(m);
            }
        }
        //if square is enemy, add capture
        else {
            if (board.isEnemy(r, c, this->getColor())) {
                if (pseudoLegal){
                    moves.push_back(m);
                }
                else if (!board.isInCheckAfterMove(m, getColor())) {
                    moves.push_back(m);
                }
            }
        }
    }
    return moves;
}
