#ifndef MOVEHISTORY_H
#define MOVEHISTORY_H

#include <string>
#include <vector>

#include "Board.h"

// class moveHistory {
//     public:
//         int currentBoardState;
//         std::vector<std::string> moveHistoryVector;

//         void appendLatestMove(std::string move);
//         moveHistory();
// };

//temp class to see if this fixes undomove leakage
struct BoardSnapshot {
    Board board;
    bool whiteTurn;
};

class moveHistory {
    public:
        int currentBoardState = 0;
        std::vector<BoardSnapshot> moveHistoryVector;

        void appendSnapshot(const Board& board, bool whiteTurn) {
            moveHistoryVector.push_back({board, whiteTurn});
            currentBoardState++;
        }

        void popSnapshot() {
            if (!moveHistoryVector.empty()) {
                moveHistoryVector.pop_back();
                currentBoardState--;
            }
        }

        moveHistory()=default;

};

#endif