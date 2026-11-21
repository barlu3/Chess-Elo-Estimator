#ifndef EPD_H
#define EPD_H

#include "Game.h"
#include "Pieces.h"
#include <string>
#include <vector>

struct EpdEntry {
    std::string fen;                 //first 4 EPD fields, loadable by Game::loadFEN
    std::string id;                  //id operand without quotes
    std::vector<std::string> bm, am; //SAN moves exactly as written in the file
};

class Epd {
    public:
        enum class San { Ok, NoMatch, Underpromotion };

        //parses "<4 fen fields> op operands; op operands;"; false if the line has fewer than 4 fields
        static bool parse(const std::string& line, EpdEntry& out);

        //matches san against the legal moves of game. Ok sets move. NoMatch: unknown, illegal or ambiguous san.
        //Underpromotion: san promotes to R, B or N, which the engine cannot play (it always promotes to a queen)
        static San sanToMove(const Game& game, const std::string& san, Move& move);
};

#endif
