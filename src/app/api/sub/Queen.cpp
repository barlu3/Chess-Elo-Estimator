#include "api/sub/Queen.h"
#include "api/Board.h"

#include <vector>

std::vector<Move> Queen::generateMoves(const Board& board, bool pseudoLegal) const {
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

        while (board.inBounds(r, c)) {
            Move m = {row, col, r, c};
            if (board.isEmpty(r,c)) {
                if (pseudoLegal) {
                    moves.push_back(m);
                }
                else if (!board.isInCheckAfterMove(m, getColor())) {
                    moves.push_back(m);
                }
            }
            else {
                if (board.isEnemy(r,c, this->getColor())) {
                    if (pseudoLegal) {
                        moves.push_back(m);
                    }
                    else if (!board.isInCheckAfterMove(m, getColor())) {
                        moves.push_back(m);
                    }
                }
                break;
            }
            r += d[0];
            c += d[1];
        }
    }
    return moves;
}
