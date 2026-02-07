#include "api/moveHistory.h"
#include "api/Board.h"
#include "Pieces.h"

moveHistory::moveHistory() {
    this->currentBoardState = 0;
}

// "move" format: <movingPieceChar><movingPieceXY><movingPieceDestinationXY><capturedPieceChar>
void moveHistory::appendLatestMove(std::string move) {
    this->currentBoardState += 1;
    this->moveHistoryVector.push_back(move);
}
