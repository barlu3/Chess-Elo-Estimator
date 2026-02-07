#ifndef MENU_H
#define MENU_H

#include "api/Board.h"
#include <string>

class Menu {
private:
    std::string username;
    bool isGuest;

public:
    Menu(const std::string& username, bool isGuest);

    void run(); // Main menu loop

private:
    void displayMainMenu();
    void newGameMenu();

    void startLocalGame();
    void startBotGame(int difficulty);
    void startBotGame(int difficulty, const Board& loadedBoard, bool whiteTurn);

    void loadFilePrompt();
    void deleteFilePrompt();
    void saveFilePrompt(const Board& board, bool isWhite);
    void showLeaderboard(); 
};

#endif
