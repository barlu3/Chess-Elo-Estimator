#ifndef QUEEN_H
#define QUEEN_H

#include "../../Pieces.h"
#include <vector>

class Queen : public Piece {
    public: 
        Queen(int row, int column, bool white) : Piece(row, column, white) {}
        std::vector<Move> generateMoves(const Board& board, bool pseudoLegal) const override;
        char symbol() const override { return isWhite ? 'Q' : 'q';}
        Piece* clone() const override {return new Queen(*this);}
};

#endif