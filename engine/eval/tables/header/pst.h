#ifndef PST_H
#define PST_H

class PST {
    public: 
        static long long pstValue(char pieceType, bool isWhite, int row, int col);
    private:
        static const int pawnTable[8][8];
        static const int knightTable[8][8];
        static const int bishopTable[8][8];
        static const int rookTable[8][8];
        static const int queenTable[8][8];
        static const int kingTable[8][8];
};

#endif