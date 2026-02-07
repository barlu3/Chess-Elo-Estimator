#ifndef BISHOP_H
#define BISHOP_H

#include "../../Pieces.h"
#include <vector>

class Bishop : public Piece {
    public:
        Bishop(int row, int column, bool white) : Piece(row, column, white) {}
        std::vector<Move> generateMoves(const Board& board, bool pseudoLegal) const override;
        char symbol() const override {return isWhite ? 'B' : 'b';}
        Piece* clone() const override {return new Bishop(*this);}
};

#endif