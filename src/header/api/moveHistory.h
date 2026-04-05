#ifndef MOVEHISTORY_H
#define MOVEHISTORY_H

#include "Board.h"
#include "Pieces.h"

#include <string>
#include <vector>

// class moveHistory {
//     public:
//         int currentBoardState;
//         std::vector<std::string> moveHistoryVector;

//         void appendLatestMove(std::string move);
//         moveHistory();
// };

//temp class to see if this fixes undomove leakage
struct MoveRecord {
    Move move;

    Piece* capturedPiece;

    bool movedFlagBefore;
    bool enPassantBefore;

    bool rookMoved;
    int rookFromRow, rookFromCol;
    int rookToRow, rookToCol;
    bool rookMovedFlagBefore;

    bool enPassantCapture;
    int epCapturedRow, epCapturedCol;
    Piece* epCapturedPiece;

    bool wasPromotion;
    Piece* promotedFromPiece;

    bool whiteTurnBefore;
};

class MoveHistory {
    public:
        int currentBoardState = 0;
        std::vector<MoveRecord> moveHistoryVector;

        void push(MoveRecord rec) {
            moveHistoryVector.push_back(std::move(rec));
            currentBoardState++;
        }

        MoveRecord& top() {
            return moveHistoryVector.back();
        }

        void pop() {
            if (!moveHistoryVector.empty()) {
                delete moveHistoryVector.back().capturedPiece;
                delete moveHistoryVector.back().epCapturedPiece;
                delete moveHistoryVector.back().promotedFromPiece;

                moveHistoryVector.pop_back();
                currentBoardState--;
            }
        }

        MoveHistory()=default;
        ~MoveHistory() {
            while (!moveHistoryVector.empty()) pop();
        }

        MoveHistory(const MoveHistory&)     =delete;
        MoveHistory& operator=(const MoveHistory&) =delete;
};

#endif