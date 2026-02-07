#include "interface/UI.h"
#include "data/SaveGame.h"
#include <iostream>
#include <cctype>

namespace UI {
    void printBoard(const Board& board) {
        std::cout << "\n  a b c d e f g h\n\n";

        for (int row = 0; row < 8; row++) {
            std::cout << 8 - row << " ";
            for (int col = 0; col < 8; col++) {
                const Piece* piece = board.getPiece(row, col);

                if (piece == nullptr) std::cout << ". ";
                else {
                    char symbol = piece->symbol();
                    std::cout << symbol << " ";
                }
            }
            std::cout << 8 - row << "\n";
        }
        std::cout << "\n  a b c d e f g h\n\n";
    }

    bool parseSquare(const std::string& string, int& row, int& col) {
        if (string.size() != 2) return false;
        
        char file = std::tolower(string[0]);
        char rank = string[1];
        
        if (file < 'a' || file > 'h') return false;
        if (rank < '1' || rank > '8') return false;

        col = file - 'a';
        row = 8 - (rank - '0');

        return true;
    }

    void printMoves(const std::vector<Move>& moves) {
        for (const auto& m : moves) {
            char file = 'a' + m.toCol;
            char row = '8' - m.toRow;
            std::cout << file << row << " ";
        }
        std::cout << "\n";
    }

    Move selectMove(Game& game, const std::string& username) {
        
        while (true) {
            char choice;
            std::cout << "\n1. Play move\n2. Undo move (Bot only)\n3. Save and Quit\n4. Quit (without saving)\nPlease enter 1, 2, 3, or 4 for your choice: ";
            std::cin >> choice;

            if (choice == '4') {
                return Move{-2,-2,-2,-2};
            }

            if (choice == '3') {
                SaveGame saver;
                std::string filename;

                std::cout << "Enter the name of the save: ";
                std::cin >> filename;

                try {
                    saver.save(username, filename, game.getBoard(), game.isWhiteTurn());
                    std::cout << "Game saved!\n";
                }
                catch (const std::exception& e){
                std::cout << "Failed to save. Exception: " << e.what() << "\n";
                }

                return Move{-2,-2,-2,-2};
            }

            if (choice == '2') {
                return Move{-3,-3,-3,-3};
            }

            if (choice != '1') {
                std::cout << "Invalid choice. Please type 1 or 2.\n"; 
                continue;
            }

            game.getBoard().enPassantClear(game.isWhiteTurn());

            while (true) {
                std::string from;
                std::cout << "\nSelect a piece (e.g. e2):\n";
                std::cin >> from;

                int fr, fc;
                if (!parseSquare(from, fr, fc)) {
                    std::cout << "Invalid square.\n";
                    continue;
                }

                const Piece* piece = game.getBoard().getPiece(fr,fc);
                if (piece == nullptr) {
                    std::cout << "No piece at this square.\n";
                    continue;
                }

                if (piece->getColor() != game.isWhiteTurn()) {
                    std::cout << "Not your piece.\n";
                    continue;
                }

                auto legalMoves = piece->generateMoves(game.getBoard(), false);
                if (legalMoves.empty()) {
                    std::cout << "That piece has no legal moves.\n";
                    continue;
                }

                std::cout << "Legal moves: ";
                printMoves(legalMoves);

                std::string to;
                std::cout << "Select destination: ";
                std::cin >> to;

                int tr, tc;
                if (!parseSquare(to, tr, tc)) {
                    std::cout << "Invalid square.\n";
                    continue;
                }
                for (const auto& m : legalMoves) {
                    if (m.toRow == tr && m.toCol == tc) {
                        return m;
                    }
                }

                std::cout << "That square is not a legal move.\n";
            }
        }
    }
}