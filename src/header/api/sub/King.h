#ifndef KING_H
#define KING_H

#include "../../Pieces.h"
#include <vector>

class King : public Piece {
    private:
        bool moved = false;
    public:
        King(int row, int column, bool white) : Piece(row, column, white) {}
        std::vector<Move> generateMoves(const Board& board, bool pseudoLegal) const override;
        char symbol() const override { return isWhite ? 'K' : 'k';}
        Piece* clone() const override {return new King(*this);}

        void setMoved() override { moved = true; }
        bool hasMoved() const override { return moved; }
};

#endif