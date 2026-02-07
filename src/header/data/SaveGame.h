#ifndef SAVEGAME_H
#define SAVEGAME_H

#include "user/FileManager.h"
#include "api/Board.h"

class SaveGame : public FileManager {
public:
    void save(const std::string& username, const std::string& filename, const Board& b, bool whiteTurn);
};

#endif
