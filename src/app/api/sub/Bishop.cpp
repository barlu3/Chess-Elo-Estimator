#include "api/sub/Bishop.h"
#include "api/Board.h"

#include <vector>

std::vector<Move> Bishop::generateMoves(const Board& board, bool pseudoLegal) const {
    std::vector<Move> moves;

    static const int directions[4][2] = {
        {1, 1}, //down-right
        {1, -1}, //down-left
        {-1, 1}, //up-right
        {-1, -1} //up-left
    };

    for (auto& d : directions) {
        int r = row + d[0];
        int c = col + d[1];

        while (board.inBounds(r,c)) {
            Move m = {row, col, r, c};
            //if square is empty or enemy, try to add to possible moves
            if (board.isEmpty(r, c)) {
                if (pseudoLegal) {
                    moves.push_back(m);
                }
                else if (!board.isInCheckAfterMove(m, getColor())) {
                    moves.push_back(m);
                }
            }
            else {
                //if square is an enemy, add capture move then stop
                if (board.isEnemy(r,c, this->getColor())) {
                    if (pseudoLegal) {
                        moves.push_back(m);
                    }
                    else if (!board.isInCheckAfterMove(m, getColor())) {
                        moves.push_back(m);
                    }
                }
                //either enemy or friendly, stop adding these moves
                break;
            }
            r += d[0];
            c += d[1];
        }
    }
    return moves;
}
