#ifndef MOVEHISTORY_H
#define MOVEHISTORY_H

#include <string>
#include <vector>

#include "Board.h"

class moveHistory {
    public:
        int currentBoardState;
        std::vector<std::string> moveHistoryVector;

        void appendLatestMove(std::string move);
        moveHistory();
};

#endif