#include "data/SaveGame.h"
#include "api/Board.h"
#include "api/sub/Rook.h"
#include "api/sub/Queen.h"
#include "api/sub/Pawn.h"
#include "api/sub/Knight.h"
#include "api/sub/King.h"
#include "api/sub/Bishop.h"

#include <fstream>
#include <sys/stat.h> 


void SaveGame::save(const std::string& username, const std::string& filename, const Board& b, bool whiteTurn) {
    #ifdef _WIN32
        mkdir("../saves/"); 
    #else
        mkdir("../saves/", 0777); 
    #endif
    
    std::string fullName = filename;
    if (filename.size() < 4 || filename.substr(filename.size() - 4) != ".txt") {
        fullName += ".txt";
    }

    std::ofstream file("../saves/" + username + "/" + fullName, std::ios::trunc);
    if (!file.is_open()) throw std::runtime_error("Failed to open file for saving.");



    //saves data from board
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            const Piece* p = b.getPiece(i, j);
            if(p == nullptr) {
                file << "0 ";
                continue;
            }

            char type;
            if(dynamic_cast<const Pawn*>(p)) type = 'P';
            else if(dynamic_cast<const Rook*>(p)) type = 'R';
            else if(dynamic_cast<const Knight*>(p)) type = 'N';
            else if(dynamic_cast<const Bishop*>(p)) type = 'B';
            else if(dynamic_cast<const Queen*>(p)) type = 'Q';
            else if(dynamic_cast<const King*>(p)) type = 'K';

            char color = p->getColor() ? 'W' : 'B';

            file << type << color << " ";
        }
        file << "\n";
    }

    file << whiteTurn << "\n";
    file.close();
}
