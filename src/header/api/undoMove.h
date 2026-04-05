#ifndef UNDOMOVE_H
#define UNDOMOVE_H

#include "moveHistory.h"
#include "Game.h"

// class undoMove {
//     public:
//         static void undoLatestMove(moveHistory& currentGame, Board& currentBoard, Game& game);
// };

//temp class to see if fixes undoMove leakage
class undoMove {
    public:
        static void undoLatestMove(moveHistory& history, Game& game) {
            if (history.currentBoardState < 1) {
                std::cout << "Too early to undo any move.\n";
                return;
            }

            const BoardSnapshot& snap = history.moveHistoryVector.back();
            game.setBoard(snap.board);
            game.setTurn(snap.whiteTurn);
            history.popSnapshot();
        }
};

#endif