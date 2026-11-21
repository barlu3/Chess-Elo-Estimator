#include "api/Board.h"

#include "api/sub/Rook.h"
#include "api/sub/Queen.h"
#include "api/sub/Pawn.h"
#include "api/sub/Knight.h"
#include "api/sub/King.h"
#include "api/sub/Bishop.h"
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
//implement current game state

Board::Board() {
    //construct a blank board
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            board[r][c] = nullptr;
        }
    }

    //initialize white pieces
    board[7][0] = new Rook(7, 0, true);
    board[7][1] = new Knight(7, 1, true);
    board[7][2] = new Bishop(7, 2, true);
    board[7][3] = new Queen(7, 3, true);
    board[7][4] = new King(7, 4, true);
    board[7][5] = new Bishop(7, 5, true);
    board[7][6] = new Knight(7, 6, true);
    board[7][7] = new Rook(7, 7, true);

    for (int c = 0; c < 8; c++) {
        board[6][c] = new Pawn(6, c, true);
    }

    //initialize black pieces
    board[0][0] = new Rook(0, 0, false);
    board[0][1] = new Knight(0, 1, false);
    board[0][2] = new Bishop(0, 2, false);
    board[0][3] = new Queen(0, 3, false);
    board[0][4] = new King(0, 4, false);
    board[0][5] = new Bishop(0, 5, false);
    board[0][6] = new Knight(0, 6, false);
    board[0][7] = new Rook(0, 7, false);

    for (int c = 0; c < 8; c++) {
        board[1][c] = new Pawn(1, c, false);
    }
}

Board::~Board() {
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (!isEmpty(r,c)) {
                removePiece(r,c);
            }
        }
    }
}

Board::Board(const Board& other) {
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (!other.isEmpty(r,c)) {
                board[r][c] = other.board[r][c]->clone();
            }
            else {
                board[r][c] = nullptr;
            }
        }
    }
}

Board& Board::operator=(const Board& other) {
    if (this == &other) return *this;

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            removePiece(r,c);
        }
    }

    for (int r = 0; r < 8; r++) {
        for (int c= 0; c < 8; c++) {
            if (!other.isEmpty(r,c)) {
                board[r][c] = other.board[r][c]->clone();
            }
            else {
                board[r][c] = nullptr;
            }
        }
    }
    return *this;
}

const vector<Piece*> Board::Pieces() const {
    std::vector<Piece*> pieces;
    for (int row = 0 ; row < 8; row++) {
        for (int col = 0 ; col < 8; col++) {
            if (board[row][col]) {
                pieces.push_back(board[row][col]);
            }
        }
    }
    return pieces;
}
const Piece* Board::getPiece(int row, int column) const {
    return board[row][column];
}
Piece* Board::getPiece(int row, int column) {
    return board[row][column];
}
void Board::setPiece(int row, int column, Piece* piece) {
    removePiece(row,column);
    board[row][column] = piece;
}
// !!VOLATILE!! make sure you are using this properly
void Board::setPieceRaw(int row, int column, Piece* piece) {
    board[row][column] = piece;
}
void Board::movePiece(const Move& move) {
    Piece* piece = board[move.fromRow][move.fromCol];
    //detect castling, is this piece a king
    if (King* k = dynamic_cast<King*>(piece)) {
        int colDiff = move.toCol - move.fromCol;

        // king side castle
        if (colDiff == 2) {
            Piece* rook = board[move.fromRow][7];
            board[move.fromRow][5] = rook;
            board[move.fromRow][7] = nullptr;
            rook->setPosition(move.fromRow, 5);
            rook->setMoved();
        }

        // queen side castle
        if (colDiff == -2) {
            Piece* rook = board[move.fromRow][0];
            board[move.fromRow][3] = rook;
            board[move.fromRow][0] = nullptr;
            rook->setPosition(move.fromRow, 3);
            rook->setMoved();
        }
    }
    //detect promotion, is this piece a pawn
    if (Pawn* p = dynamic_cast<Pawn*>(piece)) {
        
        // En Passant flag setting
        int rowsMoved = abs(move.toRow - move.fromRow);
        if (rowsMoved == 2) {
            piece->setEnPassant(true);
        }

        // En Passant capture
        // If a pawn moves diagonally into an empty square, it must be an en passant capture.
        if (move.fromCol != move.toCol && board[move.toRow][move.toCol] == nullptr) {
            removePiece(move.fromRow, move.toCol);
        }

        // Promotion
        bool whitePromo = p->getColor() && move.toRow == 0;
        bool blackPromo = (!p->getColor()) && move.toRow == 7;
        
        if (whitePromo || blackPromo) {
            // FIX: If capturing during promotion, remove the target piece first!
            if (board[move.toRow][move.toCol] != nullptr) {
                removePiece(move.toRow, move.toCol);
            }
            
            removePiece(move.fromRow, move.fromCol);
            board[move.toRow][move.toCol] = new Queen(move.toRow, move.toCol, whitePromo);
            return; // We return early because promotion handles the actual board placement
        }
    }

    //regular piece
    removePiece(move.toRow, move.toCol);
    board[move.toRow][move.toCol] = piece;
    board[move.fromRow][move.fromCol] = nullptr;

    piece->setPosition(move.toRow, move.toCol);
    piece->setMoved();
}

void Board::removePiece(int row, int column) {
    if (!(isEmpty(row, column))) {
        delete board[row][column];
        board[row][column] = nullptr;
    }
}

void Board::enPassantClear(bool color) {
    for (int row = 0; row <= 7; ++row) {
        for (int col = 0; col <= 7; ++col) {
            if (this->board[row][col] != nullptr) {
                if (this->board[row][col]->getColor() == color) {this->board[row][col]->setEnPassant(false);};
            }
        }
    }
}

//helper funcs
bool Board::inBounds(int row, int column) const {
    return row >= 0 && row < 8 && column >= 0 && column < 8;
}

bool Board::isEmpty(int row, int column) const {
    return board[row][column] == nullptr;
}
bool Board::isFriendly(int row, int column, bool isWhite) const {
    return board[row][column] != nullptr && board[row][column]->getColor() == isWhite;
}
bool Board::isEnemy(int row, int column, bool isWhite) const {
    return board[row][column] != nullptr && board[row][column]->getColor() != isWhite;
}
bool Board::isInCheck(bool white) const {
    int kingRow = -1, kingCol = -1;

    //find the king
    for (int r = 0; r< 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (!isEmpty(r,c) && board[r][c]->getColor() == white) {
                if (dynamic_cast<King*>(board[r][c])) {
                    kingRow = r;
                    kingCol = c;
                    break;
                }
            }
        }
    }
    //safety check
    if (kingRow == -1) return false;

    return isAttacked(kingRow, kingCol, white);
}

bool Board::isAttacked(int row, int column, bool white) const {
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (!isEmpty(r,c) && board[r][c]->getColor() != white) {
                auto moves = board[r][c]->generateMoves(*this, true);
                for (const auto& m : moves) {
                    if (m.toRow == row && m.toCol == column) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool Board::isInCheckAfterMove(const Move& move, bool white) const {
    Board temp = *this;
    temp.movePiece(move);
    return temp.isInCheck(white);
}

bool Board::hasLegalMoves(bool white) const {
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (!isEmpty(r,c) && getPiece(r,c)->getColor() == white) {
                auto moves = getPiece(r,c)->generateMoves(*this, false);
                if (!moves.empty()) return true;
            }
        }
    }
    return false;
}

bool Board::isCheckMate(bool white) const {
    //we are in check and we have no more moves left
    return isInCheck(white) && !hasLegalMoves(white);
}

bool Board::isStalemate(bool white) const {
    //we are not in check but we have no more moves left
    return !isInCheck(white) && !hasLegalMoves(white);
}

std::string Board::toFEN(bool white) const {
    //this is a function to convert our board into a FEN string for Stockfish
    std::string fen = "";

    for (int r = 0; r < 8; r++) {
        int empty = 0;

        for (int c = 0; c < 8; c++) {
            const Piece* p = board[r][c];

            if (!p) {
                empty++;
                continue;
            }

            if (empty > 0) {
                fen += std::to_string(empty);
                empty = 0;
            }

            char ch = p->symbol();
            fen += ch;
        }

        if (empty > 0) fen += std::to_string(empty);
        if (r != 7) fen += "/";
    }

    fen += white ? " w " : " b ";

    fen += "- - 0 1";

    return fen;
}

bool Board::loadFEN(const std::string& fen, bool& isWhite) {
    istringstream in(fen);
    string placement, side, castling, ep;
    if (!(in >> placement >> side >> castling >> ep)) return false;

    if (side != "w" && side != "b") return false;
    if (castling != "-" && castling.find_first_not_of("KQkq") != string::npos) return false;
    if (ep != "-" && (ep.size() != 2 || ep[0] < 'a' || ep[0] > 'h' || (ep[1] != '3' && ep[1] != '6'))) return false;

    //built aside so a malformed FEN never touches this board
    Piece* next[8][8] = {};
    auto fail = [&]() {
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                delete next[r][c];
            }
        }
        return false;
    };

    const string pieceChars = "PNBRQKpnbrqk";
    int row = 0, col = 0;
    for (char ch : placement) {
        if (ch == '/') {
            if (col != 8 || row >= 7) return fail();
            row++;
            col = 0;
        } else if (ch >= '1' && ch <= '8') {
            col += ch - '0';
            if (col > 8) return fail();
        } else {
            size_t idx = pieceChars.find(ch);
            if (idx == string::npos || col >= 8) return fail();
            bool white = idx < 6;
            Piece* p = nullptr;
            switch (idx % 6) {
                case 0: p = new Pawn(row, col, white); break;
                case 1: p = new Knight(row, col, white); break;
                case 2: p = new Bishop(row, col, white); break;
                case 3: p = new Rook(row, col, white); break;
                case 4: p = new Queen(row, col, white); break;
                default: p = new King(row, col, white); break;
            }
            next[row][col] = p;
            col++;
        }
    }
    if (row != 7 || col != 8) return fail();

    //engine castling reads only the moved flags and corner squares, so a king or rook without its right must be marked moved
    auto has = [&](char right) { return castling.find(right) != string::npos; };
    int whiteKings = 0, blackKings = 0;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (next[r][c] == nullptr) continue;
            char s = next[r][c]->symbol();
            if (s == 'K') whiteKings++;
            if (s == 'k') blackKings++;
            if (s != 'K' && s != 'k' && s != 'R' && s != 'r') continue;

            bool keeps =
                (s == 'K' && r == 7 && c == 4 && (has('K') || has('Q'))) ||
                (s == 'k' && r == 0 && c == 4 && (has('k') || has('q'))) ||
                (s == 'R' && r == 7 && c == 7 && has('K')) ||
                (s == 'R' && r == 7 && c == 0 && has('Q')) ||
                (s == 'r' && r == 0 && c == 7 && has('k')) ||
                (s == 'r' && r == 0 && c == 0 && has('q'));
            if (!keeps) next[r][c]->setMoved();
        }
    }
    if (whiteKings != 1 || blackKings != 1) return fail();

    if (ep != "-") {
        int epCol = ep[0] - 'a';
        bool whiteDoubled = ep[1] == '3';
        Piece* pawn = next[whiteDoubled ? 4 : 3][epCol];
        if (pawn == nullptr || pawn->symbol() != (whiteDoubled ? 'P' : 'p')) return fail();
        pawn->setEnPassant(true);
    }

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            removePiece(r, c);
            board[r][c] = next[r][c];
        }
    }
    isWhite = (side == "w");
    return true;
}
