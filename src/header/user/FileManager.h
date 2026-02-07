#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <string>
#include <vector>

// This is the parent class of SaveGame and LoadGame. They could both list out the files they saved or want to load. 
// Added a way to check if the file exists but might not use it. 
class FileManager {
public:
    virtual bool fileExists(const std::string& filename) const;
    virtual std::vector<std::string> listSaves(const std::string& username) const;
};

#endif
