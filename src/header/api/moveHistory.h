#ifndef MOVEHISTORY_H
#define MOVEHISTORY_H

#include "Board.h"
#include "Pieces.h"

#include <string>
#include <vector>

struct MoveRecord {
    Move move;

    Piece* capturedPiece = nullptr;

    bool movedFlagBefore;
    bool enPassantBefore;

    bool rookMoved;
    int rookFromRow, rookFromCol;
    int rookToRow, rookToCol;
    bool rookMovedFlagBefore;

    bool enPassantCapture;
    int epCapturedRow, epCapturedCol;
    int enPassantClearedRow = -1;
    int enPassantClearedCol = -1;
    Piece* epCapturedPiece = nullptr;

    bool wasPromotion;
    Piece* promotedFromPiece = nullptr;

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