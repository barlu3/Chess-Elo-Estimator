#ifndef CONVERTMOVETOSTRING_H
#define CONVERTMOVETOSTRING_H

#include <string>

#include "Board.h"
#include "Pieces.h"

class convertMoveToString {
    public:
        convertMoveToString() = default;
        std::string moveAsString(Move move, Board currentBoard);
};

#endif
