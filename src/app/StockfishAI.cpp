#include "StockfishAI.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <sstream>
#include <vector>

bool Stockfish::getBestMove(const Board& board, bool whiteTurn, int difficulty, Move& outMove) {
    std::string fen = board.toFEN(whiteTurn);
    std::string path = "../Stockfish/src/stockfish"; //finds the path to the stockfish executable through real path.
    // MAKE SURE TO MAKE THE EXECUTABLE IN Stockfish/src WITH THE COMMAND make

    int inPipe[2];  // initiating pipe that goes from parent to stockfish
    int outPipe[2]; // initiating pipe that goes from stockfish to parent

    if (pipe(inPipe) == -1 || pipe(outPipe) == -1) {
        std::cerr << "Stockfish pipe failure.\n";
        return false;
    }

    pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "Stockfish fork failure.\n";
        return false;
    }

    if (pid == 0) {
        dup2(inPipe[0], STDIN_FILENO); //stockfish will read from this pipe
        dup2(outPipe[1], STDOUT_FILENO); //stockfish will write into this pipe
        close(inPipe[0]);
        close(inPipe[1]);
        close(outPipe[0]);
        close(outPipe[1]);

        execl(path.c_str(), path.c_str(), nullptr); // executes stockfish engine
        _exit(1);
    }

    close(inPipe[0]); //closes unused ends by parent
    close(outPipe[1]);

    auto sendCmd = [&](const std::string& cmd) {
        write(inPipe[1], (cmd + "\n").c_str(), cmd.size() + 1); //function to send commands in stockfish
    };

    auto readLine = [&]() -> std::string { //function to read output from stockfish
        char c;
        std::string line;
        while (read(outPipe[0], &c, 1) == 1) {
            if (c == '\n') break;
            line += c;
        }
        return line;
    };

    //sending commands to stockfish here
    sendCmd("uci"); 
    sendCmd("setoption name Skill Level value " + std::to_string(difficulty)); //input difficulty for stockfish
    sendCmd("position fen " + fen); //inputs fen positions 
    sendCmd("go movetime 1000"); //1000ms thinking time

    std::string bestmove;
    while (true) {
        std::string line = readLine();
        if (line.find("bestmove") != std::string::npos) { //collecting only the information we need
            std::istringstream iss(line);
            std::string token;
            iss >> token;
            iss >> bestmove; 
            break;
        }
    }

    close(inPipe[1]); //closes pipes once done
    close(outPipe[0]);

    int status;
    waitpid(pid, &status, 0); 

    if (bestmove.size() < 4) {
        std::cerr << "Stockfish generation failure\n";
        return false;
    }

    // prepares uci output for move
    int fromCol = bestmove[0] - 'a';
    int fromRow = 8 - (bestmove[1] - '0');
    int toCol = bestmove[2] - 'a';
    int toRow = 8 - (bestmove[3] - '0');

    outMove = Move{fromRow, fromCol, toRow, toCol}; 
    return true;
}
