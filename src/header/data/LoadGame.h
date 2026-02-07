#ifndef LOADGAME_H
#define LOADGAME_H

#include "user/FileManager.h"
#include "api/Board.h"
#include <string>

class LoadGame : public FileManager {
public:
    bool load(const std::string& username, const std::string& filename, Board& b, bool& whiteTurn);
};

#endif
