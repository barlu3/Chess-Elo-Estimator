#ifndef PAWN_H
#define PAWN_H

#include "../../Pieces.h"
#include <vector>

class Pawn : public Piece {
    private:
        bool enPassantBool = false;
        bool moved = false;
    public:
        Pawn(int row, int column, bool white) : Piece(row, column, white) {}
        std::vector<Move> generateMoves(const Board& board, bool pseudoLegal) const override;
        char symbol() const override { return isWhite ? 'P' : 'p';}
        Piece* clone() const override {return new Pawn(*this);}

        void promote() {};

        bool enPassantable() const override {return enPassantBool;};
        void setEnPassant(bool set) override {enPassantBool = set;};
        bool hasMoved() const override {return moved;};
        void setMoved() override {moved = true;}
};

#endif