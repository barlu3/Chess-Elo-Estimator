#ifndef ROOK_H
#define ROOK_H

#include "../../Pieces.h"
#include <vector>

class Rook : public Piece {
    private:
        bool moved = false;
    public:
        Rook(int row, int column, bool white) : Piece(row, column, white) {}
        std::vector<Move> generateMoves(const Board& board, bool pseudoLegal) const override;
        char symbol() const override { return isWhite ? 'R' : 'r'; }
        Piece* clone() const override {return new Rook(*this);}

        bool hasMoved() const override { return moved; }
        void setMoved() override { moved = true; }
};

#endif