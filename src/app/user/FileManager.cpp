#include "user/FileManager.h"
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <vector>
#include <string>

bool FileManager::fileExists(const std::string& filename) const {
    std::ifstream file(filename);
    return file.is_open();
}

std::vector<std::string> FileManager::listSaves(const std::string& username) const {
    std::vector<std::string> saves;

    std::string path = "../saves/" + username;
    DIR* dir = opendir(path.c_str());
    struct dirent* ent;

    if (dir != nullptr) {
        while ((ent = readdir(dir)) != nullptr) {
            std::string name = ent->d_name;
            if (name.size() > 4 && name.substr(name.size() - 4) == ".txt" && name != "password.txt") 
                saves.push_back(name);
        }
        closedir(dir);
    }
    return saves;
}
