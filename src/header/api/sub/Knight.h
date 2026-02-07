#ifndef KNIGHT_H
#define KNIGHT_H

#include "../../Pieces.h"
#include <vector>

class Knight : public Piece {
    public:
        Knight(int row, int column, bool white) : Piece(row, column, white) {}
        std::vector<Move> generateMoves(const Board& board, bool pseudoLegal) const override;
        char symbol() const override { return isWhite ? 'N' : 'n';}
        Piece* clone() const override {return new Knight(*this);}
};

#endif 