#ifndef ORDERING_H
#define ORDERING_H

#include "Game.h"
#include "api/Board.h"
#include "Pieces.h"
#include <vector>

class MoveOrder {
    public: 
        //sorts move list in place in the form of captures first then quiet moves
        static void orderMoves(std::vector<Move>& moves, const Board& board);
    private: 
        //returns the MVV-LVA score for a capture move
        //the greather the MVV-LVA score, the earlier we search it
        static long long captureScore(const Move& move, const Board& board);
};

#endif