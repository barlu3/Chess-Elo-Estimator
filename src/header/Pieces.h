#ifndef PIECES_H
#define PIECES_H

#include <vector>

class Board;

struct Move {
    int fromRow, fromCol;
    int toRow, toCol;
};

class Piece {
    protected:
        int row, col;
        bool isWhite;
    
    public:
        Piece(int row, int column, bool white) : row(row), col(column), isWhite(white) {}
        virtual ~Piece() = default;
        Piece(const Piece& other) : Piece(other.row, other.col, other.isWhite) {}

        bool getColor() const { return isWhite; }

        int getRow() const { return row; }

        int getCol() const { return col; }

        void setPosition(int row, int column) {
            this->row = row;
            this->col = column;
        }

        //for generating each piece's moves
        virtual std::vector<Move> generateMoves(const Board& board, bool pseudoLegal) const = 0;

        //printing board in terminal
        virtual char symbol() const = 0;

        //cloning pieces for loading and saving boards
        virtual Piece* clone() const = 0;

        //for checking if moved (pawn, rook, king)
        virtual bool hasMoved() const { return false; }
        virtual void setMoved() {}

        // //for checking if pawns can be en Passant'ed
        virtual bool enPassantable() const { return false; }
        virtual void setEnPassant(bool) {}
};

#endif