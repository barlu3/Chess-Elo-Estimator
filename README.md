[![CI](https://github.com/barlu3/lab-5-github-actions-jrodr675-lbari008/actions/workflows/main.yml/badge.svg)](https://github.com/barlu3/lab-5-github-actions-jrodr675-lbari008/actions/workflows/main.yml)

# Chess Game
Authors: Andres Solorio https://github.com/Andres-55, Temuulen Tserenchimed https://github.com/ttser002, Jeremiah Rodriguez https://github.com/JeremiahDRodriguez, Luke Baring https://github.com/barlu3

## Project Description

This project is interesting to us because it will allow us to apply our coding skills and learn how to work as a team on an interesting project like chess.
We are currently planning on only using c++ and doing everything from the terminal. We will have different files for each chess piece and for the different options like undoing a move.
The input of our project would include choosing to either play with another player or with the computer. The output of this would be to start the game. However, if there was a game in progress, then we will implement a way to ask if the user wants to continue their previous game. During the game we will ask the user what move they want to choose and the project should display the move and continue to the next turn. Additionally, in the background, the game would have a way to keep track of all changes so when the user chooses to undo a move the game will successfully do it. The features that this project will provide are being able to save and load games, the option to play with another player or computer, and a way to undo any moves.

## User Interface Specification


### Navigation Diagram
<img width="1040" height="735" alt="image" src="https://github.com/user-attachments/assets/84345cef-7052-42ef-aa61-f249b41228bc" />


### Screen Layouts
> * The game will start with a login page. You will need to either use a guest account, create a new account, or sign into a preexisting account using a username and password. 
> * After signing in, there will be a main menu with a leaderboard, a way to log out, and multiple options to play.
>   * Choosing to look at the leaderboard will show the players with the highest elo at the top and the least at the bottom.
>   * Choosing to log out will put the user at the login page.
>   * There are also two ways to start a game. The user will have an option if they want to choose a game mode or continue a saved game. 
>     * If they choose to select a game mode they will be given an option to play against a computer at a chosen difficulty or with another player. If they choose to play with another player the screen will change to ask for the login details for the second player or play as guest
>     * If they choose to continue a saved game they would be given the option to choose a save file and either continue the game or delete the save file.
> * At this point the player will see the chess board and the pieces on the terminal. The pieces will have their own unique names and spots on the board. If the user continues a save file the board will look similar except that the pieces will be in different spots. 
> * To play the game one side starts by choosing if they want to make a move, undo a move, or save and exit.
>   * If they choose to make a move they have to write the pieces unique name and the spot they want to move it to
>   * If they choose to undo a move the board will revert to how it looked in the previous move.
>   * If they choose to save and exit the game will ask to pick a name for the file and verify the name doesn't already exist. Then it will automatically save and the screen will change to the main menu.
> * Then the next player will choose their move with the board oriented in their view. This will repeat until there is a stalemate or checkmate
> * After the game is over the screen will go to a different screen saying who won or if there was a stalemate. There would also be a prompt to return to the main menu restarting the whole thing.


### Class Diagram
<img width="1640" height="1312" alt="Image" src="https://github.com/user-attachments/assets/6b7992d9-48f6-4cd7-9208-13f3639aa3b6" />

### Updates
> * Bot opponent class, initially part of chessGame class, replaced with StockfishAI to reflect functionality using Stockfish submodule. Follows the Single Responsibility principle, being to generate a moved based on the selected difficulty. Allows for independent development of both features.
> * Seperated chessGame into Board, Game, and Player classes to follow the Single Responsibility Principle. Board handles piece location and movement, Game handles conveying and intercepting gameplay information from the user, and Player allows for turn order and verification of inputed moves via color specification.
> * userClasses have been turned into Player class and AccountManager class. Player class stores relevant user information such as name, rating, and wins. AccountManager now handles the creation, manipulation, and deletion of user accounts and their respective player objects. 
> * Applied greater polymorphism to the chessPiece class, now called Piece, to remove redundant identifiers. Every Piece child class follows the Liskov Substitution Principle as before. Piece follows the Open-Closed Principle as theoretically, new pieces with different movesets can be added without modifying existing code.
> * Pawn class has been updated to account for piece promotion and en Passant.
> * King and Rook classes have been updated to account for castling functionality.
> * Added moveHistory, gameHistory, undoMove, convertMoveToString classes to handle undo functionality and viewing past games. moveHistory stores all moves made by both players in order. undoMove navigates backward through the moveHistory of a game while deleting said moves. gameHistory allows for navigation/viewing of a completed game's moves. convertMoveToString assists moveHistory and gameHistory by creating interpretable strings for their respective arrays. All follow the Single Responsibility Principle.
> * Menu class has been created. This class handles the general navigation into subsets of the program, such as the leaderboard, player account manager, player save files, and creating new games.
> * UI class has been implemented to interact with an active game. Players can see the board state and choose to move a piece, undo a move (if facing a bot), save and quit, or quit without saving.
> * fileManager, loadGame, and saveGame classes added to account for previously only implied game file management system. loadGame and saveGame follow Single Responsibility Principle with loadGame reading preexisting .txt files and saveGame creating said .txt files.
> * leaderboard and gameHistory classes are to be completed during next sprint.

## Screenshots
<img width="363" height="1348" alt="Image" src="https://github.com/user-attachments/assets/f245dd7f-6578-400f-b51f-6105a889d146" />
 
<img width="307" height="1214" alt="Image" src="https://github.com/user-attachments/assets/3e57ac75-8241-4624-b1eb-dc37872d4440" />
 
<img width="326" height="1024" alt="Image" src="https://github.com/user-attachments/assets/26872f0f-4c94-4a5a-b795-885c4ec91274" />

<img width="535" height="945" alt="image" src="https://github.com/user-attachments/assets/6ee7d9b3-e822-457a-8faa-95c36d7b792c" />

<img width="378" height="787" alt="image" src="https://github.com/user-attachments/assets/b29137e4-72ab-4c78-92b2-7456764794ae" />

## Installation/Usage
> * This Application is compiled in Linux, in 64-bits. Emulation may be required to run it.
### Prerequisites
> * CMake Version 3.20 or higher.
> * C++17 or higher.
### To Run this Application
> 1. Clone this repository inside your IDE using ``git clone --recursive https://github.com/cs100/final-project-lbari008-jrodr675-ttser002-asolo061.git``
> 2. Run ``git submodules update --init --recursive``
> 3. Navigate to Stockfish/src/
> 4. Run ``git checkout sf_14.1``
> 5. Compile using ``make build ARCH=x86-64``
> 6. Navigate to ../../test/
> 7. Compile using ``make``
> 8. Run ``./bin/runChess``

## Testing
> * Our project was tested and validated through Continuous Integration with a repeatedly updated main.yml file to run all googletests after each input. Pull requests were used to evaluate code quality and manual testing on individual machines was also performed.
 
