#include "data/DeleteSaveFile.h"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

bool DeleteSaveFile::deleteFile(const std::string& username, const std::string& filename) {
    std::string fullName = filename;

    if (filename.size() < 4 || filename.substr(filename.size() - 4) != ".txt") {
        fullName += ".txt";
    }

    std::string path = "../saves/" + username + "/" + fullName;

    if (!fs::exists(path)) {
        return false;
    }

    // Try to delete the file
    std::error_code ec;
    bool removed = fs::remove(path, ec);

    if (ec) {
        std::cerr << "Error deleting file: " << ec.message() << std::endl;
        return false;
    }

    return removed;
}
