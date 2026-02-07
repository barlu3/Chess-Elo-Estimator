#include <filesystem>
#include "user/AccountManager.h"
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

AccountManager::AccountManager() {
    currentPath = "../saves/guest/";
    currentUsername = "guest";
    if(!fs::exists(currentPath))
        fs::create_directories(currentPath);
}

std::string AccountManager::getPassword() {
    std::ifstream file(currentPath + "password.txt");
    std::string password;
    if(file.is_open())
        std::getline(file, password);
    return password;
}

void AccountManager::savePassword(const std::string& password) {
    std::ofstream file (currentPath + "password.txt", std::ios::trunc);
    if(file.is_open())
        file << password;
}

std::string AccountManager::getCurrentPath() const {
    return currentPath;
}

std::string AccountManager::getCurrentUsername() const {
    return currentUsername;
}

bool AccountManager::createAccount(const std::string& username, const std::string& password) {
    std::string newUserFolder = "../saves/" + username + "/";
    if(fs::exists(newUserFolder))
        return false;

    fs::create_directories(newUserFolder);
    currentPath = newUserFolder;
    currentUsername = username;
    savePassword(password);
    return true;
}

bool AccountManager::login(const std::string& username, const std::string& password) {
    std::string newUserFolder = "../saves/" + username + "/";

    //checks username
    if(!fs::exists(newUserFolder)) 
        return false;
    
    std::string oldPath = currentPath;
    currentPath = newUserFolder;
    std::string storedPassword = getPassword();
    currentPath = oldPath;

    //checks password
    if(storedPassword != password) 
        return false;

    currentPath = newUserFolder;
    currentUsername = username;
    return true;
}

void AccountManager::logout() {
    currentPath = "../saves/guest/";
    currentUsername = "guest";
}

std::vector<std::string> AccountManager::usernameList() const {
    std::vector<std::string> usernames;
    std::string savesFolder = "../saves/";

    if(fs::exists(savesFolder)) {
        for(const auto& userFolder : fs::directory_iterator(savesFolder)) {
            if(userFolder.is_directory()) {
                usernames.push_back(userFolder.path().filename().string());
            }
        }
    }
    return usernames;
}

bool AccountManager::deleteAccount(const std::string& username, const std::string& password) {
    std::string userFolder = "../saves/" + username + "/";

    if(!fs::exists(userFolder)) 
        return false;

    std::string oldPath = currentPath;
    currentPath = userFolder;
    std::string storedPassword = getPassword();
    currentPath = oldPath;

    if(storedPassword != password) 
        return false;
    
    fs::remove_all(userFolder);

    logout();

    return true;
}

int AccountManager::getElo(const std::string& name) const {
    std::string filePath = "../saves/" + name + "/rating.txt";
    std::ifstream file(filePath);
    int elo = 1200;
    if(file.is_open()) {
        file >> elo;
    }
    return elo;
}

void AccountManager::setElo(const std::string& name,int elo) {
    std::string filePath = "../saves/" + name + "/rating.txt";
    std::ofstream file(filePath, std::ios::trunc);
    if (file.is_open()) {
        file << elo;
    }
}

void AccountManager::saveStats(const std::string& name,int& wins, int& losses) {
    std::string filePath = "../saves/" + name + "/stats.txt";
    std::ofstream file(filePath, std::ios::trunc);
    if(file.is_open()) {
        file << wins << " " << losses;
    }
}

void AccountManager::loadStats(const std::string& name,int& wins, int& losses) {
    std::string filePath = "../saves/" + name + "/stats.txt";
    wins = 0; losses = 0;
    std::ifstream file(filePath);
    if(file.is_open()) {
        file >> wins >> losses;
    }
}