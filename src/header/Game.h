#ifndef GAME_H
#define GAME_H

#include "Player.h"
#include "api/Board.h"

#include <cmath>
#include <stdexcept>

static double expectedScore(int ratingA, int ratingB) {
    return 1.0 / (1.0 + std::pow(10.0, (ratingB - ratingA) / 400.0));
}

class Game {
    private:
        Board board;
        bool whiteTurn = true;
    
    public:
        Game() : board() {}

        //const method
        const Board& getBoard() const {
            return board;
        }
        //non-const method
        Board& getBoard() {
            return board;
        }

        //const method
        bool& isWhiteTurn() {
            return whiteTurn;
        }

        bool performMove(const Move& move) {
            const Piece* piece = board.getPiece(move.fromRow, move.fromCol);

            if (piece == nullptr) throw std::runtime_error("No piece on square");
            
            if (piece->getColor() != whiteTurn) {
                throw std::runtime_error("Wrong player's turn");
            }

            auto legalMoves = piece->generateMoves(board, false);

            bool legal = false;
            for (const auto& m : legalMoves) {
                if (move.toRow == m.toRow && move.toCol == m.toCol) {
                    legal = true;
                    break;
                }
            }

            if (!legal) return false;

            board.movePiece(move);
            whiteTurn = !whiteTurn;
            return true;
        }

        void updateElo(Player& white, Player& black, bool whiteWon, bool draw) {
            double EA = expectedScore(white.getRating(), black.getRating());
            double EB = expectedScore(black.getRating(), white.getRating());

            double SA = draw ? 0.5 : (whiteWon ? 1.0 : 0.0);
            double SB = 1.0 - SA;

            const int K = 32;

            white.setRating(white.getRating() + int(K * (SA - EA)));
            black.setRating(black.getRating() + int(K * (SB - EB)));
        }

        void setBoard(const Board& b) {
            board = b;
        }

        void setTurn(bool turn) {
            whiteTurn = turn;
        }
};


#endif