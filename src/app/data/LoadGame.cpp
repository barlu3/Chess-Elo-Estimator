#include "data/LoadGame.h"
#include "api/Board.h"
#include "api/sub/Rook.h"
#include "api/sub/Queen.h"
#include "api/sub/Pawn.h"
#include "api/sub/Knight.h"
#include "api/sub/King.h"
#include "api/sub/Bishop.h"

#include <fstream>

bool LoadGame::load(const std::string& username, const std::string& filename, Board& b, bool& whiteTurn) {
    std::string fullName = filename;

    if (filename.size() < 4 || filename.substr(filename.size() - 4) != ".txt") {
        fullName += ".txt";
    }

    std::ifstream file("../saves/" + username + "/" + fullName);
    if (!file.is_open()) return false;
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            std::string cell;
            file >> cell;
            
            if(cell == "0") {
                b.setPiece(i, j, nullptr);
                continue;
            }

            char type = cell[0];
            char color = cell[1];
            bool isWhite = (color == 'W');

            Piece* p = nullptr;

            switch(type) {
                case 'P': p = new Pawn(i, j, isWhite); break;
                case 'R': p = new Rook(i, j, isWhite); break;
                case 'N': p = new Knight(i, j, isWhite); break;
                case 'B': p = new Bishop(i, j, isWhite); break;
                case 'Q': p = new Queen(i, j, isWhite); break;
                case 'K': p = new King(i, j, isWhite); break;
            }

            b.setPiece(i, j, p);
        }   
    }

    int turn;
    file >> turn;
    whiteTurn = (turn != 0);
    file.close();
    return true;
}
