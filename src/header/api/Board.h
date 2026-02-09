#ifndef BOARD_H
#define BOARD_H

#include "Pieces.h"
#include <vector>
#include <string>

class Board {
    private:
        Piece* board[8][8];

    public:
        Board();
        ~Board();
        Board(const Board& other);
        Board& operator=(const Board& other);
        
        const std::vector<Piece*> Pieces() const;
        const Piece* getPiece(int row, int column) const;
        void setPiece(int row, int column, Piece* piece);
        void movePiece(const Move& move);
        void removePiece(int row, int column);

        void enPassantClear(bool color);

        //helper funcs
        bool inBounds(int row, int column) const;
        bool isEmpty(int row, int column) const;
        bool isFriendly(int row, int column, bool isWhite) const;
        bool isEnemy(int row, int column, bool isWhite) const;
        bool isInCheck(bool isWhite) const;
        bool isAttacked(int row, int column, bool isWhite) const;
        bool isInCheckAfterMove(const Move& move, bool isWhite) const;
        bool isCheckMate(bool isWhite) const;
        bool isStalemate(bool isWhite) const;
        bool hasLegalMoves(bool isWhite) const;
        std::string toFEN(bool isWhite) const;
};

#endif