#ifndef MOVEEXECUTOR_H
#define MOVEEXECUTOR_H

#include "Game.h"
#include "api/moveHistory.h"
#include "api/undoMove.h"
#include "Pieces.h"

class MoveExecutor {
    public:
        static bool make(const Move& move, Game& game, MoveHistory& history);

        static void undo(Game& game, MoveHistory& history);

        static std::vector<Move> generateMoves(const Game& game);
};

#endif