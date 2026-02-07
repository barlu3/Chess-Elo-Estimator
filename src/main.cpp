#include "header/Game.h"
#include "header/api/Board.h"
#include "header/interface/Menu.h"
#include "header/data/SaveGame.h"
#include "header/data/LoadGame.h"
#include "header/user/AccountManager.h"
#include <filesystem>

#include <iostream>

int main() {
    
    AccountManager accountManager;

    while (true) {
        std::cout << "1. Login\n";
        std::cout << "2. Register New Account\n";
        std::cout << "3. Continue as Guest\n";
        std::cout << "Enter choice: ";

        int choice;
        std::cin >> choice;

        while (std::cin.fail()) { //validating input
            std::cin.clear();
            std::cin.ignore(1000, '\n');
            std::cout << "Invalid input. Enter a number: ";
            std::cin >> choice;
        }

        if (choice == 3) {
            Menu menu("guest", true); // guest account 
            menu.run();
            return 0;
        }

        if (choice == 1) {
            std::string username, password;
            std::cout << "Username: ";
            std::cin >> username;
            std::cout << "Password: ";
            std::cin >> password;

            if (accountManager.login(username, password)) {
                std::cout << "Login successful!\n";
                Menu menu(username, false); //login with existing account
                menu.run();
                return 0;
            } else {
                std::cout << "Invalid username or password.\n";
            }

        } else if (choice == 2) {
            std::string username, password;
            std::cout << "Choose a username: ";
            std::cin >> username;
            std::cout << "Choose a password: ";
            std::cin >> password;

            if (accountManager.createAccount(username, password)) {
                std::cout << "Account created successfully!\n";
                Menu menu(username, false); //login with new account
                menu.run();
                return 0;
            } else {
                std::cout << "Username already taken.\n";
            }
        } else {
            std::cout << "Invalid option.\n";
        }
    }

    /*
    Game game;

    while (true) {
        UI::printBoard(game.getBoard());

        if (game.getBoard().isCheckMate(game.isWhiteTurn())) {
            std::cout << "Checkmate!\n";
            return 0;
        }

        if (game.getBoard().isStalemate(game.isWhiteTurn())) {
            std::cout << "Stalemate!\n";
            break;
        }

        std::cout << (game.isWhiteTurn() ? "White" : "Black") << " to move.\n";

        Move move = UI::selectMove(game);

        //sentinel, stop playing: user wanted to quit program (either through save or without)
        if (move.fromRow == -2) {
            std::cout << "Successfully quit the game!\n";
            return 0;
        }
        //sentinel, skip execution: a load happened
        if (move.fromRow == -1) continue; 

        game.performMove(move);
    }
        */

    return 0;
}
