#include "interface/Menu.h"
#include "data/SaveGame.h"
#include "data/LoadGame.h"
#include "data/DeleteSaveFile.h"
#include "api/Board.h"
#include "interface/UI.h"
#include "Game.h"
#include "StockfishAI.h"
#include "api/moveHistory.h"
#include "api/undoMove.h"
#include "api/convertMoveToString.h"

#include <iostream>

Menu::Menu(const std::string& username, bool isGuest): username(username), isGuest(isGuest) {}

void Menu::run() {
    int choice = 0;

    while (choice != 5) {
        displayMainMenu();
        std::cout << "Enter choice: ";
        std::cin >> choice;

        while (std::cin.fail()) { //validating input
            std::cin.clear();
            std::cin.ignore(1000, '\n');
            std::cout << "Invalid input. Enter a number: ";
            std::cin >> choice;
        }

        switch (choice) {
            case 1:
                newGameMenu();
                break;
            case 2:
                loadFilePrompt();
                break;
            case 3:
                deleteFilePrompt();
                break;
            case 4:
                showLeaderboard();
                break;
            case 5:
                std::cout << "Exiting.\n"; //this while loop will loop unless 4 is chosen.
                break;
            default:
                std::cout << "Invalid option. Try again.\n";
        }
    }
}

// Main menu
void Menu::displayMainMenu() {
    std::cout << "\n==============================\n";
    std::cout << "           Main Menu\n";
    std::cout << "==============================\n";
    std::cout << "Player: " << username << (isGuest ? " Guest Account" : "") << "\n";
    std::cout << "1. Start New Game\n";
    std::cout << "2. Load Game\n";
    std::cout << "3. Delete Save File\n";
    std::cout << "4. Leaderboard\n";
    std::cout << "5. Exit\n";
}

// Sub-menu for Gamemode selection
void Menu::newGameMenu() {
    int choice;
    std::cout << "\n--- New Game ---\n";
    std::cout << "1. Local 2-Player\n";
    std::cout << "2. Play Against Bot\n";
    std::cout << "3. Back\n";
    std::cout << "Choose mode: ";
    std::cin >> choice;

    switch (choice) {
        case 1:
            startLocalGame();
            break;

        case 2: {
            int difficulty;
            std::cout << "Select bot difficulty (1-20): ";
            std::cin >> difficulty;

            if (difficulty < 1 || difficulty > 20)
                difficulty = 1;

            startBotGame(difficulty);
            break;
        }
        case 3:
            return;

        default:
            std::cout << "Invalid choice.\n";
    }
}

void  Menu::startLocalGame() {
    std::cout << "\nStarting local 2-player game...\n";

    Game game;

    while (true) {
        UI::printBoard(game.getBoard());

        if (game.getBoard().isCheckMate(game.isWhiteTurn())) {
            std::cout << "Checkmate!\n";
            break;
        }

        if (game.getBoard().isStalemate(game.isWhiteTurn())) {
            std::cout << "Stalemate!\n";
            break;
        }

        std::cout << (game.isWhiteTurn() ? "White" : "Black") << " to move.\n";

        Move move = UI::selectMove(game, this->username);

        //sentinel, stop playing: user wanted to quit program (either through save or without)
        if (move.fromRow == -2) {
            std::cout << "Successfully quit the game!\n";
            return;
        }
        //sentinel, skip execution: a load happened
        if (move.fromRow == -1) continue; 

        //Undo move, for player
        if (move.fromRow == -3) {
            std::cout << "Cannot undo a move when facing a player\n";
        }
        else {
            game.performMove(move);
        }
    }
}

void Menu::startBotGame(int difficulty) {
    std::cout << "\nStarting game vs Bot (Difficulty " << difficulty << ")...\n";

    Game game;
    moveHistory moveHistory;
    convertMoveToString converter;
    undoMove undoMove;

    while (true) {
        UI::printBoard(game.getBoard());

        if (game.getBoard().isCheckMate(game.isWhiteTurn())) {
            std::cout << "Checkmate!\n";
            break;
        }

        if (game.getBoard().isStalemate(game.isWhiteTurn())) {
            std::cout << "Stalemate!\n";
            break;
        }

        if (game.isWhiteTurn()) {
            std::cout << "White to move.\n";
            Move move = UI::selectMove(game, username);

            if (move.fromRow == -2) {
                std::cout << "Successfully quit the game!\n";
                return;
            }

            if (move.fromRow == -3) {
                if (moveHistory.moveHistoryVector.size() >= 2) {
                    undoMove.undoLatestMove(moveHistory, game.getBoard());
                    undoMove.undoLatestMove(moveHistory, game.getBoard());
                }
                else {
                    std::cout << "To early to undo move.\n";
                }
            }
            else {
                game.performMove(move);
                moveHistory.appendLatestMove(converter.moveAsString(move, game.getBoard()));
            }
        } else {
            std::cout << "Black (Stockfish) is thinking...\n";
            Move sfMove;
            if (Stockfish::getBestMove(game.getBoard(), game.isWhiteTurn(), difficulty, sfMove)) {
                std::cout << "Stockfish plays: " << char('a'+sfMove.fromCol) << 8-sfMove.fromRow << char('a'+sfMove.toCol) << 8-sfMove.toRow << "\n";
                game.performMove(sfMove);
                moveHistory.appendLatestMove(converter.moveAsString(sfMove, game.getBoard()));
            } else {
                std::cerr << "Stockfish failed to make a move. Ending game.\n";
                return;
            }
        }
    }

}

void Menu::startBotGame(int difficulty, const Board& loadedBoard, bool whiteTurn) {
    std::cout << "\nStarting game vs Bot (Difficulty " << difficulty << ")...\n";

    Game game;
    game.setBoard(loadedBoard);
    game.setTurn(whiteTurn);

    while (true) {
        UI::printBoard(game.getBoard());

        if (game.getBoard().isCheckMate(game.isWhiteTurn())) {
            std::cout << "Checkmate!\n";
            break;
        }

        if (game.getBoard().isStalemate(game.isWhiteTurn())) {
            std::cout << "Stalemate!\n";
            break;
        }

        if (game.isWhiteTurn()) {
            std::cout << "White to move.\n";
            Move move = UI::selectMove(game, username);

            if (move.fromRow == -2) {
                std::cout << "Successfully quit the game!\n";
                return;
            }

            game.performMove(move);
        } else {
            std::cout << "Black (Stockfish) is thinking...\n";
            Move sfMove;
            if (Stockfish::getBestMove(game.getBoard(), game.isWhiteTurn(), difficulty, sfMove)) {
                std::cout << "Stockfish plays: " << char('a'+sfMove.fromCol) << 8-sfMove.fromRow << char('a'+sfMove.toCol) << 8-sfMove.toRow << "\n";
                game.performMove(sfMove);
            } else {
                std::cerr << "Stockfish failed to make a move. Ending game.\n";
                return;
            }
        }
    }

}

// Function for loading existing game, also a sub menu to list files.
void Menu::loadFilePrompt() {
    if (isGuest) {
        std::cout << "Guests cannot load games.\n";
        return;
    }
    
    std::string filename;
    LoadGame loader;

    std::vector<std::string> saves = loader.listSaves(username);

    if (saves.empty()) {
        std::cout << "No saves found.\n";
        return;
    } else {
        std::cout << "--- Choose a save file to load ---\n";
        std::cout << "Available saves:\n";
        for (int i = 0; i < saves.size(); ++i) {
            std::cout << (i+1) << ". " << saves[i] << "\n";
        }
    }
    
    int choice;
    std::cout << "Enter the number of the file to load: ";
    std::cin >> choice;
    choice--;

    if (choice >= 0 && choice < saves.size()) {
        filename = saves[choice];
    } else {
        std::cout << "Invalid choice!\n";
    }

    Board board;
    bool whiteTurn;

    if (loader.load(username, filename, board, whiteTurn)) {
        std::cout << "Game loaded successfully.\n";
        startBotGame(5, board, whiteTurn);
    } 
    else {
        std::cout << "Failed to load game.\n";
    }
}

// Function for deleting existing save file, also a sub menu to list files.
void Menu::deleteFilePrompt() {
    std::string filename;
    DeleteSaveFile deleter;

    std::vector<std::string> saves = deleter.listSaves(username);

    if (saves.empty()) {
        std::cout << "No saves found.\n";
        return;
    } else {
        std::cout << "--- Choose a save file to delete ---\n";
        std::cout << "Available saves:\n";
        for (int i = 0; i < saves.size(); ++i) {
            std::cout << (i+1) << ". " << saves[i] << "\n";
        }
    }
    
    int choice;
    std::cout << "Enter the number of the file to delete: ";
    std::cin >> choice;
    choice--;

    if (choice >= 0 && choice < saves.size()) {
        filename = saves[choice];
    } else {
        std::cout << "Invalid choice!\n";
    }

    if (deleter.deleteFile(username, filename)) {
        std::cout << "Save deleted successfully.\n";
    } 
    else {
        std::cout << "Failed to delete save.\n";
    }
}

// Save File function after finishing game.
void Menu::saveFilePrompt(const Board& board, bool isWhite){
    if (isGuest) {
        std::cout << "Guests cannot save games.\n";
        return;
    }
    
    std::string filename;
    std::cout << "Enter save file name: ";
    std::cin >> filename;  
    SaveGame saver;

    try{
        saver.save(username, filename, board, isWhite);
        std::cout << "Game saved!\n";
    }
    catch (const std::exception& e){
        std::cout << "Failed to save. Exception: " << e.what() << "\n";
    }
}

// Display Leaderboard -- Not implemented yet
void Menu::showLeaderboard() {
    std::cout << "\n--- Leaderboard ---\n";
    std::cout << "PLACEHOLDER\n";
}
