#include <filesystem>
#include <gtest/gtest.h>
#include <algorithm> 
#include <fstream>
#include <iostream>


#include "../src/header/api/Board.h"
#include "../src/header/api/moveHistory.h"
#include "../src/header/api/undoMove.h"
#include "../src/header/api/convertMoveToString.h"
#include "../src/header/Game.h"
#include "../src/header/Pieces.h"
#include "../src/header/Player.h"
#include "../src/header/data/SaveGame.h"
#include "../src/header/data/DeleteSaveFile.h"
#include "../src/header/data/LoadGame.h"
#include "../src/header/interface/UI.h"
#include "../src/header/interface/Menu.h"
#include "../src/header/user/AccountManager.h"

#include "../engine/eval/material.h"
#include "../engine/search/header/negamax.h"
#include "../engine/search/header/alphabeta.h"
#include "../engine/search/header/ordering.h"
#include "../engine/search/header/iterdeep.h"

#include "../src/header/api/sub/Pawn.h"
#include "../src/header/api/sub/Rook.h"
#include "../src/header/api/sub/Knight.h"
#include "../src/header/api/sub/Bishop.h"
#include "../src/header/api/sub/Queen.h"
#include "../src/header/api/sub/King.h"

#define WHITE true
#define BLACK false

// remove added files
void removeTestFiles() {
    DeleteSaveFile del;

    del.deleteFile("guest", "testSave.txt");
    del.deleteFile("guest", "testLoad.txt");
    del.deleteFile("guest", "testList.txt");
}

void removeAccount(const std::string& name) {
    std::string path = "../saves/" + name + "/";
    std::filesystem::remove_all(path);
}

TEST(BoardTest, PiecesPlacedCorrectly) {
    Board b;

    for (int c = 0; c < 8; c++) {
        // White pieces
        EXPECT_NE(b.getPiece(7,c), nullptr);
        EXPECT_TRUE(b.getPiece(7,c)->getColor());
        // Black pieces
        EXPECT_NE(b.getPiece(0,c), nullptr);
        EXPECT_FALSE(b.getPiece(0,c)->getColor());
    }
    // Pawns
    for (int c = 0; c < 8; c++) {
        //white pawns
        EXPECT_NE(b.getPiece(6,c), nullptr);
        EXPECT_TRUE(b.getPiece(6,c)->getColor());
        //black pawns
        EXPECT_NE(b.getPiece(1,c), nullptr);
        EXPECT_FALSE(b.getPiece(1,c)->getColor());
    }

    // Empty squares
    for (int r = 2; r < 6; r++) {
        for (int c = 0; c < 8; c++) {
            EXPECT_TRUE(b.isEmpty(r,c));
        }
    }
}

TEST(BoardTest, SetAndGetPieceWorks) {
    Board b;

    b.setPiece(4,4, new Queen(4,4, WHITE));
    EXPECT_FALSE(b.isEmpty(4,4));

    const Piece* p = b.getPiece(4,4);
    EXPECT_TRUE(p->getColor());
    EXPECT_EQ(p->getRow(), 4);
    EXPECT_EQ(p->getCol(), 4);
}

TEST(BoardTest, MovePieceSimple) {
    Board b;

    Piece* p = new Rook(5, 5, WHITE);
    b.setPiece(5,5, p);

    Move m{5,5, 3,5};   // move up

    b.movePiece(m);

    EXPECT_TRUE(b.isEmpty(5,5));
    EXPECT_FALSE(b.isEmpty(3,5));

    const Piece* moved = b.getPiece(3,5);
    EXPECT_EQ(moved->getRow(), 3);
    EXPECT_EQ(moved->getCol(), 5);
}

TEST(BoardTest, MovePieceCapture) {
    Board b;

    Piece* attacker = new Rook(5,5, WHITE);
    Piece* victim   = new Knight(3,5, BLACK);

    b.setPiece(5,5, attacker);
    b.setPiece(3,5, victim);

    Move m{5,5, 3,5};

    b.movePiece(m);

    EXPECT_TRUE(b.isEmpty(5,5));
    EXPECT_FALSE(b.isEmpty(3,5));

    // captured piece should be replaced by attacker
    const Piece* p = b.getPiece(3,5);
    EXPECT_TRUE(p->getColor());
}

TEST(BoardTest, StandOffNoCheck) {
    Board b; 
    
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            b.removePiece(r, c);

    //place just the kings
    b.setPiece(7,4,new King(7,4,true));
    b.setPiece(0,4,new King(0,4,false));

    EXPECT_FALSE(b.isInCheck(true));  // white not in check
    EXPECT_FALSE(b.isInCheck(false)); // black not in check
}

TEST(BoardTest, CheckBlockedByPiece) {
    Board b;

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            b.removePiece(r,c);
        }
    }

    b.setPiece(7,4,new King(7,4,true)); //white king
    b.setPiece(0,4,new Rook(0,4,false)); //black rook
    b.setPiece(3,4,new Pawn(3,4,true)); //blocks rook

    EXPECT_FALSE(b.isInCheck(true)); //white king is safe
}

TEST(BoardTest, CheckByDiagonalQueen) {
    Board b;

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            b.removePiece(r,c);
        }
    }

    b.setPiece(0,0, new King(0,0,false)); //black king
    b.setPiece(3,3, new Queen(3,3,true)); //white queen

    EXPECT_TRUE(b.isInCheck(false)); //black king in check
}

TEST(BoardTest, RookCheck) {
    Board b;
    
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            b.removePiece(r,c);
        }
    }

    b.setPiece(7,4,new King(7,4,true)); //white king
    b.setPiece(0,4, new Rook(0,4,false)); //black rook

    EXPECT_TRUE(b.isInCheck(true));
}

TEST(BoardTest, FoolsMate) {
    Board b;
    
    //move white f2 pawn to f3
    b.movePiece({6, 5, 5, 5});
    //move black e7 pawn to e6
    b.movePiece({1, 4, 2, 4});
    //move white g2 pawn to g4
    b.movePiece({6, 6, 4, 6});
    //move black queen at d8 to h4
    b.movePiece({0, 3, 4, 7});

    EXPECT_TRUE(b.isCheckMate(true));
}

TEST(BoardTest, Stalemate) {
    Board b;

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            b.removePiece(r,c);
        }
    }

    b.setPiece(0,0,new King(0,0,true));
    b.setPiece(2,1, new Queen(2,1,false));
    b.setPiece(2,0, new King(0,0,false));

    EXPECT_TRUE(b.isStalemate(true));
}

TEST(BoardTest, CopyConstructorDeepCopy) {
    Board b1;

    // Modify b1
    b1.setPiece(4,4, new Queen(4,4, WHITE));

    Board b2 = b1; // copy

    // Contents must be equal
    EXPECT_FALSE(b2.isEmpty(4,4));

    // But pointers must NOT match → deep copy
    EXPECT_NE(b1.getPiece(4,4), b2.getPiece(4,4));

    // Changing b2 should not affect b1
    b2.removePiece(4,4);
    EXPECT_FALSE(b1.isEmpty(4,4));
    EXPECT_TRUE(b2.isEmpty(4,4));
}

TEST(BoardTest, AssignmentOperatorDeepCopy) {
    Board b1, b2;

    b1.setPiece(4,4, new Queen(4,4, WHITE));

    b2 = b1;

    EXPECT_FALSE(b2.isEmpty(4,4));
    EXPECT_NE(b1.getPiece(4,4), b2.getPiece(4,4));

    b2.removePiece(4,4);
    EXPECT_FALSE(b1.isEmpty(4,4));
    EXPECT_TRUE(b2.isEmpty(4,4));
}

TEST(BoardTest, BoundsCheck) {
    Board b;
    EXPECT_TRUE(b.inBounds(0,0));
    EXPECT_TRUE(b.inBounds(7,7));
    EXPECT_FALSE(b.inBounds(-1,3));
    EXPECT_FALSE(b.inBounds(8,0));
}

TEST(BoardTest, FriendEnemyCheck) {
    Board b;

    b.setPiece(5,5, new Pawn(5,5, WHITE));
    b.setPiece(5,6, new Pawn(5,6, BLACK));

    EXPECT_TRUE(b.isFriendly(5,5, WHITE));
    EXPECT_FALSE(b.isFriendly(5,6, WHITE));

    EXPECT_TRUE(b.isEnemy(5,6, WHITE));
    EXPECT_FALSE(b.isEnemy(5,5, WHITE));
}

TEST(King, FreeRowOfUnmovedKingAnd2RooksCanCastle) {
    Board b;
    for (int r=0; r<8; r++) {
        for (int c = 0; c<8; c++) {
            b.removePiece(r,c);
        }
    }

    b.setPiece(0,4, new King(0,4,true));
    b.setPiece(0,0, new Rook(0,0, true));
    b.setPiece(0,7, new Rook(0,7, true));

    auto moves = b.getPiece(0,4)->generateMoves(b, false);
    //can move around it (5) and castle (+2)
    EXPECT_EQ(moves.size(), 7);
}

TEST(Queen, StartingPositionHasNoMoves) {
    Board b;

    // white queen at (7,3)
    const Piece* q = b.getPiece(7,3);
    auto moves = q->generateMoves(b, false);

    EXPECT_EQ(moves.size(), 0);  // blocked by friendly pieces
}

TEST(Queen, OpenCenterGenerates27Moves) {
    Board b;

    // clear center
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            b.removePiece(r,c);

    // place queen at center
    b.setPiece(4,4, new Queen(4,4, true));

    const Piece* q = b.getPiece(4,4);
    auto moves = q->generateMoves(b, false);

    EXPECT_EQ(moves.size(), 27); // queen moves = rook moves (14) + bishop moves (13)
}

TEST(Queen, QueenPinnedHasSomeMoves) {
     Board b;
    for (int r=0; r<8; r++) {
        for (int c = 0; c<8; c++) {
            b.removePiece(r,c);
        }
    }
    b.setPiece(0,4, new King(0,4,true));
    b.setPiece(1,4, new Queen(1,4,true));
    b.setPiece(4,4, new Rook(4,4, false));

    auto moves = b.getPiece(1,4)->generateMoves(b, false);
    //can only move on the row and capture piece
    EXPECT_EQ(moves.size(), 3);
}

TEST(Rook, StartingPositionHasNoMoves) {
    Board b;
    const Piece* rook = b.getPiece(7,0); // white rook
    EXPECT_EQ(rook->generateMoves(b, false).size(), 0);
}

TEST(Rook, OpenRookGenerates14Moves) {
    Board b;

    // clear board
    for (int r=0; r<8; r++)
        for (int c=0; c<8; c++)
            b.removePiece(r,c);

    b.setPiece(4,4, new Rook(4,4, true));

    const Piece* r = b.getPiece(4,4);
    EXPECT_EQ(r->generateMoves(b, false).size(), 14); // 7 up + 7 down/left/right combinations
}

TEST(Rook, RookPinnedHasSomeMoves) {
     Board b;
    for (int r=0; r<8; r++) {
        for (int c = 0; c<8; c++) {
            b.removePiece(r,c);
        }
    }
    b.setPiece(0,4, new King(0,4,true));
    b.setPiece(1,4, new Rook(1,4,true));
    b.setPiece(4,4, new Rook(4,4, false));

    auto moves = b.getPiece(1,4)->generateMoves(b, false);
    //can move along row and capture the rook
    EXPECT_EQ(moves.size(), 3);
}

TEST(Bishop, StartingPositionHasNoMoves) {
    Board b;
    const Piece* bishop = b.getPiece(7,2); // white bishop
    EXPECT_EQ(bishop->generateMoves(b, false).size(), 0);
}

TEST(Bishop, OpenBishopGenerates13Moves) {
    Board b;
    for (int r=0; r<8; r++)
        for (int c=0; c<8; c++)
            b.removePiece(r,c);

    b.setPiece(4,4, new Bishop(4,4, true));

    const Piece* bs = b.getPiece(4,4);
    EXPECT_EQ(bs->generateMoves(b, false).size(), 13);
}

TEST(Bishop, BishopPinnedHasNoMoves) {
     Board b;
    for (int r=0; r<8; r++) {
        for (int c = 0; c<8; c++) {
            b.removePiece(r,c);
        }
    }
    b.setPiece(0,4, new King(0,4,true));
    b.setPiece(1,4, new Bishop(1,4,true));
    b.setPiece(4,4, new Rook(4,4, false));

    auto moves = b.getPiece(1,4)->generateMoves(b, false);
    EXPECT_EQ(moves.size(), 0);
}

TEST(Knight, StartingKnightHasTwoMoves) {
    Board b;
    const Piece* knight = b.getPiece(7,1); // white knight

    auto moves = knight->generateMoves(b, false);

    EXPECT_EQ(moves.size(), 2); // Knights hop over pieces
}

TEST(Knight, KnightInCenterHasEightMoves) {
    Board b;
    for (int r=0; r<8; r++)
        for (int c=0; c<8; c++)
            b.removePiece(r,c);

    b.setPiece(4,4, new Knight(4,4, true));

    auto moves = b.getPiece(4,4)->generateMoves(b, false);
    EXPECT_EQ(moves.size(), 8);
}

TEST(Knight, KnightPinnedHasNoMoves) {
    Board b;
    for (int r=0; r<8; r++) {
        for (int c = 0; c<8; c++) {
            b.removePiece(r,c);
        }
    }
    b.setPiece(0,4, new King(0,4,true));
    b.setPiece(1,4, new Knight(1,4,true));
    b.setPiece(4,4, new Rook(4,4, false));

    auto moves = b.getPiece(1,4)->generateMoves(b, false);
    EXPECT_EQ(moves.size(), 0);
}

TEST(Pawn, PawnStartingHasTwoForwardMovesIfEmpty) {
    Board b;

   //generate moves from white pawn at [6][0]
    auto moves = b.getPiece(6,0)->generateMoves(b, false);

    // Should have 1-step and 2-step
    EXPECT_EQ(moves.size(), 2);
}

TEST(Pawn, PawnCanCaptureDiagonally) {
    Board b;

    // enemy on diagonal
    b.setPiece(5,1, new Pawn(5,1,false));

    Pawn* p = (Pawn*)b.getPiece(6,0);
    auto moves = p->generateMoves(b, false);

    bool foundCapture = false;
    for (auto& m : moves) {
        if (m.toRow == 5 && m.toCol == 1)
            foundCapture = true;
    }

    EXPECT_TRUE(foundCapture);
}

TEST(Pawn, PawnPinnedHasNoMoves) {
    Board b;
    for (int r=0; r<8; r++) {
        for (int c = 0; c<8; c++) {
            b.removePiece(r,c);
        }
    }
    b.setPiece(0,4, new King(0,4,true));
    b.setPiece(1,4, new Pawn(1,4,true));
    b.setPiece(4,4, new Rook(4,4, false));

    auto moves = b.getPiece(1,4)->generateMoves(b, false);
    EXPECT_EQ(moves.size(), 0);
}

TEST(Pawn, PawnPromoOnEmptyBoardHas21Moves) {
    Board b;
    for (int r=0; r<8; r++) {
        for (int c = 0; c<8; c++) {
            b.removePiece(r,c);
        }
    }
    b.setPiece(1,4, new Pawn(1,4,true));
    b.movePiece({1,4,0,4});
    auto movesWhite = b.getPiece(0,4)->generateMoves(b, false);
    //white pawn should be a queen now
    EXPECT_EQ(movesWhite.size(), 21);

    b.setPiece(6,4, new Pawn(6,4,false));
    b.movePiece({6,4,7,4});

    auto movesBlack = b.getPiece(7,4)->generateMoves(b, false);
    //black pawn should be a queen now
    EXPECT_EQ(movesBlack.size(), 21);
}

TEST(Pawn, enPassantCapture) {
    Board b;

    Piece* attacker = new Pawn(4,4, WHITE);
    Piece* victim   = new Pawn(4,5, BLACK);
    victim->setEnPassant(true);

    b.setPiece(4, 4, attacker);
    b.setPiece(4, 5, victim);

    Move m{4, 4, 5, 5};

    b.movePiece(m);

    EXPECT_TRUE(b.isEmpty(4,4));
    EXPECT_TRUE(b.isEmpty(4,5));
    EXPECT_FALSE(b.isEmpty(5,5));
}

TEST(Pawn, enPassantTimingDecay) {
    Game g;

    Piece* attacker = new Pawn(4,4, WHITE);
    Piece* victim   = new Pawn(4,5, BLACK);
    victim->setEnPassant(true);

    g.getBoard().setPiece(4, 4, attacker);
    g.getBoard().setPiece(4, 5, victim);

    g.getBoard().enPassantClear(BLACK);

    Move m{4, 4, 5, 5};

    EXPECT_EQ(false, g.performMove(m));
}

bool whiteTurn = true;

TEST(testGameFiles, testSaveFile) {
    std::filesystem::create_directories("../saves/guest");
    SaveGame saver;
    Board b;
    saver.save("guest", "testSave", b, whiteTurn);

    std::vector<std::string> saves = saver.listSaves("guest");

    EXPECT_NE(std::find(saves.begin(), saves.end(), "testSave.txt"), saves.end()); 
}

TEST(testGameFiles, testLoadFile) {
    SaveGame saver;
    Board b;
    Board copyBoard;
    Piece* piece = new King(4, 4, true);

    b.setPiece(6, 7, piece);

    saver.save("guest", "testLoad", b, whiteTurn);

    bool copyWhiteTurn = false;
    LoadGame loader;
    bool isLoaded = loader.load("guest", "testLoad.txt", copyBoard, copyWhiteTurn); 

    EXPECT_TRUE(isLoaded);
    EXPECT_EQ(copyWhiteTurn, whiteTurn);

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            const Piece* p1 = b.getPiece(i,j);
            const Piece* p2 = copyBoard.getPiece(i,j);

            if (p1 == nullptr && p2 == nullptr) continue;
            ASSERT_NE(p1, nullptr);
            ASSERT_NE(p2, nullptr);
            
            EXPECT_EQ(typeid(*p1), typeid(*p2));
            EXPECT_EQ(p1->getColor(), p2->getColor());
        }
    }
}

TEST(testGameFiles, testLoadFileList) {
    SaveGame saver;
    Board b;
    saver.save("guest", "testList", b, whiteTurn);

    LoadGame loader;
    std::vector<std::string> saves = loader.listSaves("guest");

    EXPECT_NE(std::find(saves.begin(), saves.end(), "testList.txt"), saves.end());
}

TEST(testAccountManager, testCreateAccount) {
    AccountManager am;

    removeAccount("testUser1");
    bool created = am.createAccount("testUser1", "123");
    
    EXPECT_TRUE(created);

    EXPECT_TRUE(std::filesystem::exists("../saves/testUser1/password.txt"));

    EXPECT_EQ(am.getCurrentUsername(), "testUser1");
    EXPECT_EQ(am.getCurrentPath(), "../saves/testUser1/");
    removeAccount("testUser1");
}

TEST(testAccountManager, testDuplicateAccount) {
    AccountManager am;

    removeAccount("testUser2");
    am.createAccount("testUser2", "abc");
    bool tryDuplicate = am.createAccount("testUser2", "abc");

    EXPECT_FALSE(tryDuplicate);
    removeAccount("testUser2");
}

TEST(testAccountManager, testLogin) {
    AccountManager am;

    removeAccount("testUser3");
    am.createAccount("testUser3", "1234");

    bool loggedIn = am.login("testUser3", "1234");

    EXPECT_TRUE(loggedIn);
    EXPECT_EQ(am.getCurrentUsername(), "testUser3");
    EXPECT_EQ(am.getCurrentPath(), "../saves/testUser3/");
    removeAccount("testUser3");
}

TEST(testAccountManager, testWrongPassword) {
    AccountManager am;

    removeAccount("testUser4");
    am.createAccount("testUser4", "pass");

    bool loggedIn = am.login("testUser4", "wrongpass");

    EXPECT_FALSE(loggedIn);
    removeAccount("testUser4");
}

TEST(testAccountManager, testLogout) {
    AccountManager am;

    removeAccount("testUser5");
    am.createAccount("testUser5", "1234");
    am.logout();

    EXPECT_EQ(am.getCurrentUsername(), "guest");
    EXPECT_EQ(am.getCurrentPath(), "../saves/guest/");
    removeAccount("testUser5");
}

TEST(testAccountManager, testUsernameList) {
    AccountManager am;

    removeAccount("account1");
    removeAccount("account2");

    am.createAccount("account1", "p1");
    am.logout();
    am.createAccount("account2", "p2");

    std::vector<std::string> names = am.usernameList();

    EXPECT_NE(std::find(names.begin(), names.end(), "account1"), names.end());
    EXPECT_NE(std::find(names.begin(), names.end(), "account2"), names.end());

    removeAccount("account1");
    removeAccount("account2");
}

TEST(testAccountManager, testDeleteAccount) {
    AccountManager am;

    removeAccount("trash");
    am.createAccount("trash", "password");

    bool deleted = am.deleteAccount("trash", "password");

    EXPECT_TRUE(deleted);
    EXPECT_FALSE(std::filesystem::exists("../saves/trash/"));
    removeAccount("trash");
}

TEST(testDeletSaveFile, testDeleteFile) {
    DeleteSaveFile del;
    SaveGame saver;
    Board b;

    saver.save("guest", "deleteFile", b, whiteTurn);

    //make sure the file exists
    std::vector<std::string> saves = del.listSaves("guest");
    EXPECT_NE(std::find(saves.begin(), saves.end(), "deleteFile.txt"), saves.end()); 

    //delete and check it was deleted
    del.deleteFile("guest", "deleteFile");
    saves = del.listSaves("guest");
    EXPECT_EQ(std::find(saves.begin(), saves.end(), "deleteFile.txt"), saves.end()); 
}

// -------------------------
// UI PARSING TESTS
// -------------------------

TEST(UITest, ParseSquareValid) {
    int r, c;
    
    EXPECT_TRUE(UI::parseSquare("a8", r, c));
    EXPECT_EQ(r, 0);
    EXPECT_EQ(c, 0);
    
    EXPECT_TRUE(UI::parseSquare("h1", r, c));
    EXPECT_EQ(r, 7);
    EXPECT_EQ(c, 7);
    
    EXPECT_TRUE(UI::parseSquare("e2", r, c));
    EXPECT_EQ(r, 6);
    EXPECT_EQ(c, 4);
}

TEST(UITest, ParseSquareInvalid) {
    int r, c;
    
    EXPECT_FALSE(UI::parseSquare("z9", r, c));
    EXPECT_FALSE(UI::parseSquare("a9", r, c));
    EXPECT_FALSE(UI::parseSquare("i1", r, c));
    EXPECT_FALSE(UI::parseSquare("aa", r, c));
    EXPECT_FALSE(UI::parseSquare("e", r, c));
}

TEST(PlayerTest, InitPlayer) {
    AccountManager am;
    am.createAccount("testUser", "1234");
    
    Player p("testUser", am, true);
    
    EXPECT_EQ(p.getName(), "testUser");
    EXPECT_TRUE(p.getColor());
    EXPECT_EQ(p.getRating(), 1200);

    removeAccount("testUser");
}

TEST(PlayerTest, EloPersistence) {
    AccountManager am;
    am.createAccount("eloUser", "pass");
    
    Player p("eloUser", am, true);
    
    p.setRating(1500);
    
    // Create a new Player object using the same account manager
    Player p2("eloUser",am, true);
    
    EXPECT_EQ(p2.getRating(), 1500); // Elo should persist

    removeAccount("eloUser");
}

TEST(PlayerTest, StatsPersistence) {
    AccountManager am;
    am.createAccount("statsUser", "pass");
    
    Player p("statsUser", am, true);
    
    int wins, losses;
    am.loadStats(p.getName(), wins,losses);
    
    EXPECT_EQ(wins, 0);
    EXPECT_EQ(losses, 0);
    
    p.win();
    p.lose();
    
    am.loadStats(p.getName(), wins,losses);
    EXPECT_EQ(wins, 1);
    EXPECT_EQ(losses, 1);

    removeAccount("statsUser");
}

TEST(PlayerTest, MultiplePlayersElo) {
    AccountManager am;
    am.createAccount("player1", "pass");
    am.createAccount("player2", "pass");
    
    Player p1("player1",am, true);
    Player p2("player2",am, false);
    
    p1.setRating(1400);
    p2.setRating(1600);
    
    EXPECT_EQ(p1.getRating(), 1400);
    EXPECT_EQ(p2.getRating(), 1600);

    removeAccount("player1");
    removeAccount("player2");
}

// used to simulate cin in test
class CinRedirect {
public:
    CinRedirect(const std::string& input) : cinBuffer(std::cin.rdbuf()) {
        cinStream.str(input);
        std::cin.rdbuf(cinStream.rdbuf());
    }
    ~CinRedirect() {
        std::cin.rdbuf(cinBuffer);
    }

private:
    std::stringstream cinStream;
    std::streambuf* cinBuffer;
};

TEST(MenuTest, DisplaysMainMenuCorrectly) {
    Menu menu("Bob", false);

    CinRedirect input("5\n"); //exiting program
    testing::internal::CaptureStdout(); //starts capturing output

    menu.run();

    std::string output = testing::internal::GetCapturedStdout(); //ends output capture

    EXPECT_NE(output.find("Main Menu"), std::string::npos);
    EXPECT_NE(output.find("Bob"), std::string::npos);
    EXPECT_NE(output.find("1. Start New Game"), std::string::npos);
    EXPECT_NE(output.find("5. Exit"), std::string::npos);
}

TEST(MenuTest, GuestCannotLoadGames) {
    Menu menu("Guest", true);

    CinRedirect input("2\n5\n");
    testing::internal::CaptureStdout();

    menu.run();

    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("Guests cannot load games."), std::string::npos);
}

TEST(MenuTest, DisplaySubMenu) {
    Menu menu("Bob", false);

    CinRedirect input("1\n3\n5\n");

    testing::internal::CaptureStdout();
    menu.run();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("--- New Game ---"), std::string::npos);
    EXPECT_NE(output.find("Local 2-Player"), std::string::npos);
    EXPECT_NE(output.find("Play Against Bot"), std::string::npos);
}

TEST(MenuTest, DisplayRemovingFile) {
    std::filesystem::create_directories("../saves/testUser");
    SaveGame saver;
    Board b;
    saver.save("testUser", "deleteSave", b, whiteTurn);

    Menu menu("testUser", false);

    CinRedirect input("3\n1\n5\n");

    testing::internal::CaptureStdout();
    menu.run();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("Save deleted successfully."), std::string::npos);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    removeTestFiles(); 

    return RUN_ALL_TESTS();
}

// moveHistory and undoMoves
TEST(MoveHistoryVectorTest, appendNewMovesTest) {
    Board newBoard;
    moveHistory moveHistory;

    Move move1 = {0, 1, 0, 2};
    Move move2 = {7, 6, 7, 5};

    convertMoveToString converter;

    std::string moveStr1 = converter.moveAsString(move1, newBoard);
    newBoard.movePiece(move1);
    moveHistory.appendLatestMove(moveStr1);

    std::string moveStr2 = converter.moveAsString(move2, newBoard);
    newBoard.movePiece(move2);
    moveHistory.appendLatestMove(moveStr2);

    EXPECT_EQ(moveHistory.moveHistoryVector.size(), 2);
}

TEST(MoveHistoryVectorTest, undoDefaultBoardTest) {
    Game game;
    moveHistory moveHistory;
    undoMove undo;

    std::stringstream ss;
    std::streambuf* oldCout = std::cout.rdbuf();
    std::cout.rdbuf(ss.rdbuf());

    undo.undoLatestMove(moveHistory, game.getBoard(), game);

    std::cout.rdbuf(oldCout);

    EXPECT_EQ(ss.str(), "To early to undo any move.\n");
}

TEST(MoveHistoryVectorTest, undoFirstTwoMoves) {
    Game game;
    moveHistory moveHistoryDefault;
    moveHistory moveHistoryWithMoves;
    undoMove undo;

    Move move1 = {0, 1, 0, 2};
    Move move2 = {7, 6, 7, 5};

    convertMoveToString converter;

    std::string moveStr1 = converter.moveAsString(move1, game.getBoard());
    game.getBoard().movePiece(move1);
    moveHistoryWithMoves.appendLatestMove(moveStr1);

    std::string moveStr2 = converter.moveAsString(move2, game.getBoard());
    game.getBoard().movePiece(move2);
    moveHistoryWithMoves.appendLatestMove(moveStr2);

    ASSERT_EQ(moveHistoryWithMoves.moveHistoryVector.size(), 2);
    undo.undoLatestMove(moveHistoryWithMoves, game.getBoard(), game);
    undo.undoLatestMove(moveHistoryWithMoves, game.getBoard(), game);

    EXPECT_EQ(moveHistoryWithMoves.moveHistoryVector.size(), moveHistoryDefault.moveHistoryVector.size());
}

TEST(MoveHistoryVectorTest, undoThreeTurns) {
    Game game1;
    Game game2;
    moveHistory moveHistoryDefault;
    moveHistory moveHistoryWithMoves;
    undoMove undo;

    Move move1 = {0, 1, 0, 2};
    Move move2 = {7, 6, 7, 5};
    Move move3 = {1, 0, 2, 2};
    Move move4 = {3, 6, 3, 4};
    Move move5 = {3, 1, 3, 2};
    Move move6 = {2, 7, 7, 2};
    Move move7 = {2, 0, 4, 2};

    convertMoveToString converter;

    std::string moveStr1 = converter.moveAsString(move1, game1.getBoard());
    moveHistoryDefault.appendLatestMove(moveStr1);

    moveStr1 = converter.moveAsString(move1, game2.getBoard());
    moveHistoryWithMoves.appendLatestMove(moveStr1);
    std::string moveStr2 = converter.moveAsString(move2, game2.getBoard());
    moveHistoryWithMoves.appendLatestMove(moveStr2);
    std::string moveStr3 = converter.moveAsString(move3, game2.getBoard());
    moveHistoryWithMoves.appendLatestMove(moveStr3);
    std::string moveStr4 = converter.moveAsString(move4, game2.getBoard());
    moveHistoryWithMoves.appendLatestMove(moveStr4);
    std::string moveStr5 = converter.moveAsString(move5, game2.getBoard());
    moveHistoryWithMoves.appendLatestMove(moveStr5);
    std::string moveStr6 = converter.moveAsString(move6, game2.getBoard());
    moveHistoryWithMoves.appendLatestMove(moveStr6);
    std::string moveStr7 = converter.moveAsString(move7, game2.getBoard());
    moveHistoryWithMoves.appendLatestMove(moveStr7);

    for (unsigned i = 0; i < 6; ++i) {
        undo.undoLatestMove(moveHistoryWithMoves, game2.getBoard(), game2);
    }

    EXPECT_EQ(moveHistoryWithMoves.moveHistoryVector.size(), moveHistoryDefault.moveHistoryVector.size());
}

//---------------------
//EVALUATOR TESTS
//---------------------

TEST(EvaluatorTest, startingValueIsZero) {
    Game game;

    EXPECT_EQ(Evaluator::evaluate(game), 0);
}

TEST(EvaluatorTest, PerspectiveFlipsWithSideToMove) {
    Game game;

    game.setTurn(false);
    EXPECT_EQ(Evaluator::evaluate(game), 0);
}

TEST(EvaluatorTest, WhiteUpQueen) {
    Game game;
    game.getBoard().removePiece(0,3);

    EXPECT_EQ(Evaluator::evaluate(game), 900);
}

TEST(EvaluatorTest, PerspectiveWithMaterialAdvantage) {
    Game game;
    game.getBoard().removePiece(0,3);
    game.setTurn(false);

    EXPECT_EQ(Evaluator::evaluate(game), -900);
}

// ─────────────────────────────────────────────
// Helper: count material on board for one color
// Used to verify board state is fully restored
// ─────────────────────────────────────────────
static long long countMaterial(const Board& board, bool white) {
    long long total = 0;
    for (const Piece* p : board.Pieces()) {
        if (p->getColor() == white) {
            char t = std::tolower(static_cast<unsigned char>(p->symbol()));
            switch (t) {
                case 'p': total += 100; break;
                case 'n': total += 320; break;
                case 'b': total += 330; break;
                case 'r': total += 500; break;
                case 'q': total += 900; break;
            }
        }
    }
    return total;
}

// Helper: build an empty board with all squares cleared
static Board emptyBoard() {
    Board b;
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            b.removePiece(r, c);
    return b;
}

// ─────────────────────────────────────────────
// Depth-0 Tests
// ─────────────────────────────────────────────

// At depth 0, negamax must return exactly what Evaluator::evaluate returns.
// No moves should be searched — validates the base case short-circuit.
TEST(NegaMaxTest, DepthZeroMatchesEvaluator) {
    Game game;
    moveHistory history;

    long long evalScore   = Evaluator::evaluate(game);
    long long searchScore = NegaMax::negamax(game, history, 0);

    EXPECT_EQ(evalScore, searchScore);
}

// Starting position is material-equal so depth-0 score must be exactly 0.
TEST(NegaMaxTest, DepthZeroStartingPositionIsZero) {
    Game game;
    moveHistory history;

    EXPECT_EQ(NegaMax::negamax(game, history, 0), 0LL);
}

// ─────────────────────────────────────────────
// Perspective / Symmetry Tests
// ─────────────────────────────────────────────

// Removing black's queen gives white a +900 material advantage.
// With white to move the score should be positive.
TEST(NegaMaxTest, WhiteUpQueenIsPositive) {
    Game game;
    moveHistory history;

    game.getBoard().removePiece(0, 3);  // remove black queen at d8

    long long score = NegaMax::negamax(game, history, 0);
    EXPECT_EQ(score, 900LL);
}

// Same imbalance but black to move — score must flip to negative.
// This validates the side-to-move normalization in the evaluator.
TEST(NegaMaxTest, PerspectiveFlipsWithSideToMove) {
    Game game;
    moveHistory history;

    game.getBoard().removePiece(0, 3);  // remove black queen
    game.setTurn(false);                // black to move

    long long score = NegaMax::negamax(game, history, 0);
    EXPECT_EQ(score, -900LL);
}

// Starting position with black to move should still be 0 (symmetric material).
TEST(NegaMaxTest, StartingPositionSymmetricForBlack) {
    Game game;
    moveHistory history;

    game.setTurn(false);

    EXPECT_EQ(NegaMax::negamax(game, history, 0), 0LL);
}

// ─────────────────────────────────────────────
// Terminal Position Tests
// ─────────────────────────────────────────────

// Stalemate: no legal moves, king NOT in check → must return exactly 0.
// Position: white king b6, white queen b1, black king a8 — black to move.
TEST(NegaMaxTest, StalemateReturnsZero) {
    Board b = emptyBoard();

   b.setPiece(0, 7, new King(0, 7, false));  // black king  h8 — row 0, col 7
    b.setPiece(1, 5, new Queen(1, 5, true));  // white queen f7 — row 1, col 5
    b.setPiece(2, 6, new King(2, 6, true));   // white king  g6 — row 2, col 6

    Game game;
    game.setBoard(b);
    game.setTurn(false);  // black to move

    ASSERT_TRUE(game.getBoard().isStalemate(false))
        << "Setup error: position should be stalemate for black";

    moveHistory history;
    long long score = NegaMax::negamax(game, history, 1);
    EXPECT_EQ(score, 0LL);
}

// Checkmate: no legal moves, king IS in check → must return large negative.
// Position: white king b6, white queen a1, black king a8 — black to move.
TEST(NegaMaxTest, CheckmateReturnsLargeNegative) {
    Board b = emptyBoard();

    // White queen at a1 (row 7, col 0) covers a-file → checks black king
    // White king at b6 (row 2, col 1) covers escape squares
    // Black king at a8 (row 0, col 0) — no legal moves
    b.setPiece(0, 0, new King(0, 0, false));
    b.setPiece(2, 2, new King(2, 2, true));
    b.setPiece(1, 1, new Queen(1, 1, true));

    Game game;
    game.setBoard(b);
    game.setTurn(false);  // black to move — checkmated

    ASSERT_TRUE(game.getBoard().isCheckMate(false))
        << "Setup error: position should be checkmate for black";

    moveHistory history;
    long long score = NegaMax::negamax(game, history, 1);
    EXPECT_LT(score, -10000LL);
}

// ─────────────────────────────────────────────
// Move Quality Tests
// ─────────────────────────────────────────────

// A free queen capture must be found at depth 1.
TEST(NegaMaxTest, FindsFreeQueenCapture) {
    Board b = emptyBoard();

    // Bare minimum: both kings + white pawn + undefended black queen
    b.setPiece(7, 4, new King(7, 4, true));   // white king e1
    b.setPiece(0, 4, new King(0, 4, false));  // black king e8
    b.setPiece(4, 3, new Pawn(4, 3, true));   // white pawn d4
    b.setPiece(3, 4, new Queen(3, 4, false)); // black queen e5 — pawn can capture, nothing defends

    Game game;
    game.setBoard(b);
    game.setTurn(true);

    moveHistory history;
    long long score = NegaMax::negamax(game, history, 1);
    EXPECT_GE(score, 100LL);
}

// ─────────────────────────────────────────────
// Board & History Integrity Tests
// ─────────────────────────────────────────────

// After negamax completes, white and black material must be identical to before.
// This is the most important correctness test — catches any make/undo mismatch.
TEST(NegaMaxTest, BoardMaterialRestoredAfterSearch) {
    Game game;
    moveHistory history;

    long long whiteBefore = countMaterial(game.getBoard(), true);
    long long blackBefore = countMaterial(game.getBoard(), false);

    NegaMax::negamax(game, history, 2);

    long long whiteAfter = countMaterial(game.getBoard(), true);
    long long blackAfter = countMaterial(game.getBoard(), false);

    EXPECT_EQ(whiteBefore, whiteAfter)
        << "White material changed after negamax — undoMove is not restoring correctly";
    EXPECT_EQ(blackBefore, blackAfter)
        << "Black material changed after negamax — undoMove is not restoring correctly";
}

// moveHistory must be in the exact same state before and after search.
// Validates that appendLatestMove and undoLatestMove are perfectly balanced.
TEST(NegaMaxTest, HistorySizeRestoredAfterSearch) {
    Game game;
    moveHistory history;

    size_t sizeBefore  = history.moveHistoryVector.size();
    int    stateBefore = history.currentBoardState;

    NegaMax::negamax(game, history, 2);

    EXPECT_EQ(history.moveHistoryVector.size(), sizeBefore)
        << "moveHistoryVector leaked entries after negamax";
    EXPECT_EQ(history.currentBoardState, stateBefore)
        << "currentBoardState was not restored after negamax";
}

// The whiteTurn flag on Game must not be mutated by search.
TEST(NegaMaxTest, TurnUnchangedAfterSearch) {
    Game game;
    moveHistory history;

    bool turnBefore = game.isWhiteTurn();
    NegaMax::negamax(game, history, 2);

    EXPECT_EQ(game.isWhiteTurn(), turnBefore)
        << "whiteTurn flag was not restored after negamax";
}

// Running negamax twice on the same unchanged position must return the same score.
// Non-determinism here means search is silently corrupting state between calls.
TEST(NegaMaxTest, DeterministicOnSamePosition) {
    Game game1, game2;
    moveHistory h1, h2;

    long long score1 = NegaMax::negamax(game1, h1, 2);
    long long score2 = NegaMax::negamax(game2, h2, 2);

    EXPECT_EQ(score1, score2)
        << "negamax returned different scores on identical positions — state is leaking";
}

//ALPHA BETA TEST SUITES
TEST(AlphaBetaTest, MatchesNegamaxScore) {
    Game g1, g2;
    moveHistory h1, h2;

    long long negamaxScore  = NegaMax::negamax(g1, h1, 3);
    long long alphabetaScore = AlphaBeta::alphabeta(g2, h2, 3, -1e18, 1e18);

    EXPECT_EQ(negamaxScore, alphabetaScore)
        << "Alpha-beta returned different score than negamax on same position";
}
static const long long INF = 1e18;
// ─────────────────────────────────────────────
// Correctness: must match negamax exactly
// ─────────────────────────────────────────────

// Alpha-beta must return the same score as negamax at every depth.
// If any of these fail, the cutoff logic is wrong.
TEST(AlphaBetaTest, MatchesNegamaxDepth1) {
    Game g1, g2;
    moveHistory h1, h2;

    EXPECT_EQ(
        NegaMax::negamax(g1, h1, 1),
        AlphaBeta::alphabeta(g2, h2, 1, -INF, INF)
    );
}

TEST(AlphaBetaTest, MatchesNegamaxDepth2) {
    Game g1, g2;
    moveHistory h1, h2;

    EXPECT_EQ(
        NegaMax::negamax(g1, h1, 2),
        AlphaBeta::alphabeta(g2, h2, 2, -INF, INF)
    );
}

TEST(AlphaBetaTest, MatchesNegamaxDepth3) {
    Game g1, g2;
    moveHistory h1, h2;

    EXPECT_EQ(
        NegaMax::negamax(g1, h1, 3),
        AlphaBeta::alphabeta(g2, h2, 3, -INF, INF)
    );
}

// Same correctness check but with black to move.
TEST(AlphaBetaTest, MatchesNegamaxBlackToMove) {
    Game g1, g2;
    moveHistory h1, h2;

    g1.setTurn(false);
    g2.setTurn(false);

    EXPECT_EQ(
        NegaMax::negamax(g1, h1, 2),
        AlphaBeta::alphabeta(g2, h2, 2, -INF, INF)
    );
}

// ─────────────────────────────────────────────
// Depth-0 base case
// ─────────────────────────────────────────────

TEST(AlphaBetaTest, DepthZeroMatchesEvaluator) {
    Game game;
    moveHistory history;

    EXPECT_EQ(
        Evaluator::evaluate(game),
        AlphaBeta::alphabeta(game, history, 0, -INF, INF)
    );
}

TEST(AlphaBetaTest, DepthZeroStartingPositionIsZero) {
    Game game;
    moveHistory history;

    EXPECT_EQ(AlphaBeta::alphabeta(game, history, 0, -INF, INF), 0LL);
}

// ─────────────────────────────────────────────
// Terminal positions
// ─────────────────────────────────────────────

// FEN: 7k/5Q2/6K1/8/8/8/8/8 b - - 0 1 — stalemate for black
TEST(AlphaBetaTest, StalemateReturnsZero) {
    Board b = emptyBoard();
    b.setPiece(0, 7, new King(0, 7, false));
    b.setPiece(1, 5, new Queen(1, 5, true));
    b.setPiece(2, 6, new King(2, 6, true));

    Game game;
    game.setBoard(b);
    game.setTurn(false);

    ASSERT_TRUE(game.getBoard().isStalemate(false))
        << "Setup error: position should be stalemate for black";

    moveHistory history;
    EXPECT_EQ(AlphaBeta::alphabeta(game, history, 1, -INF, INF), 0LL);
}

// FEN: 6k1/5QK1/8/8/8/8/8/8 b - - 0 1 — checkmate for black
TEST(AlphaBetaTest, CheckmateReturnsLargeNegative) {
    Board b = emptyBoard();
    b.setPiece(0, 6, new King(0, 6, false));
    b.setPiece(1, 5, new Queen(1, 5, true));
    b.setPiece(1, 6, new King(1, 6, true));

    Game game;
    game.setBoard(b);
    game.setTurn(false);

    ASSERT_TRUE(game.getBoard().isCheckMate(false))
        << "Setup error: position should be checkmate for black";

    moveHistory history;
    EXPECT_LT(AlphaBeta::alphabeta(game, history, 1, -INF, INF), -10000LL);
}

// ─────────────────────────────────────────────
// Pruning-specific behavior
// ─────────────────────────────────────────────

// If alpha >= beta on entry, the node should be pruned immediately.
// Score returned must be <= beta (the upper bound we passed in).
TEST(AlphaBetaTest, ImmediateBetaCutoffWhenWindowClosed) {
    Game game;
    moveHistory history;

    // Pass a closed window: alpha == beta == 0
    // Any score found will trigger cutoff immediately
    long long score = AlphaBeta::alphabeta(game, history, 2, 0, 0);

    // With a zero window, we expect beta returned (0) or lower
    EXPECT_LE(score, 0LL);
}

// A very high alpha (better than anything reachable) should cause
// immediate cutoffs everywhere — score must be <= alpha passed in.
TEST(AlphaBetaTest, HighAlphaCausesImmediateCutoffs) {
    Game game;
    moveHistory history;

    long long score = AlphaBeta::alphabeta(game, history, 2, 100000LL, INF);

    // Nothing can beat alpha=100000 from starting position, so beta cutoff
    EXPECT_LE(score, 100000LL);
}

// ─────────────────────────────────────────────
// Integrity: board and history must be restored
// ─────────────────────────────────────────────

TEST(AlphaBetaTest, BoardMaterialRestoredAfterSearch) {
    Game game;
    moveHistory history;

    long long whiteBefore = countMaterial(game.getBoard(), true);
    long long blackBefore = countMaterial(game.getBoard(), false);

    AlphaBeta::alphabeta(game, history, 2, -INF, INF);

    EXPECT_EQ(countMaterial(game.getBoard(), true),  whiteBefore)
        << "White material changed — undoMove is leaking";
    EXPECT_EQ(countMaterial(game.getBoard(), false), blackBefore)
        << "Black material changed — undoMove is leaking";
}

TEST(AlphaBetaTest, HistorySizeRestoredAfterSearch) {
    Game game;
    moveHistory history;

    size_t sizeBefore  = history.moveHistoryVector.size();
    int    stateBefore = history.currentBoardState;

    AlphaBeta::alphabeta(game, history, 2, -INF, INF);

    EXPECT_EQ(history.moveHistoryVector.size(), sizeBefore)
        << "moveHistoryVector leaked entries";
    EXPECT_EQ(history.currentBoardState, stateBefore)
        << "currentBoardState was not restored";
}

TEST(AlphaBetaTest, TurnUnchangedAfterSearch) {
    Game game;
    moveHistory history;

    bool turnBefore = game.isWhiteTurn();
    AlphaBeta::alphabeta(game, history, 2, -INF, INF);

    EXPECT_EQ(game.isWhiteTurn(), turnBefore)
        << "whiteTurn flag was not restored after search";
}

TEST(AlphaBetaTest, DeterministicOnSamePosition) {
    Game g1, g2;
    moveHistory h1, h2;

    long long score1 = AlphaBeta::alphabeta(g1, h1, 3, -INF, INF);
    long long score2 = AlphaBeta::alphabeta(g2, h2, 3, -INF, INF);

    EXPECT_EQ(score1, score2)
        << "alphabeta is non-deterministic on identical positions";
}

// ─────────────────────────────────────────────
// MOVE ORDERING TESTS
// ─────────────────────────────────────────────

// Helper: returns true if all captures precede all quiet moves in the list
static bool capturesBeforeQuiets(const std::vector<Move>& moves, const Board& board) {
    bool seenQuiet = false;
    for (const Move& m : moves) {
        const Piece* attacker = board.getPiece(m.fromRow, m.fromCol);
        if (attacker == nullptr) continue;
        bool isCapture = board.isEnemy(m.toRow, m.toCol, attacker->getColor());
        if (isCapture && seenQuiet) return false;
        if (!isCapture) seenQuiet = true;
    }
    return true;
}

// After ordering, no moves should be added or dropped
TEST(MoveOrderTest, SizeUnchangedAfterOrdering) {
    Board b = emptyBoard();
    b.setPiece(4, 4, new Queen(4, 4, true));
    b.setPiece(7, 4, new King(7, 4, true));
    b.setPiece(0, 4, new King(0, 4, false));
    b.setPiece(2, 4, new Pawn(2, 4, false));
    b.setPiece(3, 3, new Knight(3, 3, false));

    std::vector<Move> moves;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (!b.isEmpty(r, c) && b.getPiece(r, c)->getColor() == true) {
                auto pieceMoves = b.getPiece(r, c)->generateMoves(b, false);
                moves.insert(moves.end(), pieceMoves.begin(), pieceMoves.end());
            }
        }
    }

    size_t sizeBefore = moves.size();
    MoveOrder::orderMoves(moves, b);
    EXPECT_EQ(moves.size(), sizeBefore)
        << "orderMoves dropped or added moves";
}

// All captures must appear before any quiet move
TEST(MoveOrderTest, CapturesBeforeQuiets) {
    Board b = emptyBoard();
    b.setPiece(7, 4, new King(7, 4, true));
    b.setPiece(0, 4, new King(0, 4, false));
    b.setPiece(4, 4, new Queen(4, 4, true));
    b.setPiece(4, 6, new Pawn(4, 6, false));    // capturable by queen
    b.setPiece(4, 2, new Knight(4, 2, false));  // capturable by queen

    std::vector<Move> moves;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (!b.isEmpty(r, c) && b.getPiece(r, c)->getColor() == true) {
                auto pieceMoves = b.getPiece(r, c)->generateMoves(b, false);
                moves.insert(moves.end(), pieceMoves.begin(), pieceMoves.end());
            }
        }
    }

    MoveOrder::orderMoves(moves, b);
    EXPECT_TRUE(capturesBeforeQuiets(moves, b))
        << "A quiet move appeared before a capture after ordering";
}

// Pawn takes queen must be ordered before queen takes pawn (MVV-LVA)
TEST(MoveOrderTest, MVVLVAOrderedCorrectly) {
    Board b = emptyBoard();
    b.setPiece(7, 4, new King(7, 4, true));
    b.setPiece(0, 4, new King(0, 4, false));

    // white pawn at d4 can take black queen at e5 — score = 900 - 100 = 800
    b.setPiece(4, 3, new Pawn(4, 3, true));
    b.setPiece(3, 4, new Queen(3, 4, false));

    // white queen at a1 can take black pawn at a5 — score = 100 - 900 = -800
    b.setPiece(7, 0, new Queen(7, 0, true));
    b.setPiece(3, 0, new Pawn(3, 0, false));

    std::vector<Move> moves;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (!b.isEmpty(r, c)) {
                auto pieceMoves = b.getPiece(r, c)->generateMoves(b, false);
                moves.insert(moves.end(), pieceMoves.begin(), pieceMoves.end());
            }
        }
    }

    MoveOrder::orderMoves(moves, b);

    // Find the index of pawn-takes-queen and queen-takes-pawn in the ordered list
    int pawnTakesQueenIdx = -1;
    int queenTakesPawnIdx = -1;
    for (int i = 0; i < (int)moves.size(); i++) {
        const Piece* attacker = b.getPiece(moves[i].fromRow, moves[i].fromCol);
        const Piece* victim   = b.getPiece(moves[i].toRow,   moves[i].toCol);
        if (attacker == nullptr || victim == nullptr) continue;

        char a = std::tolower(attacker->symbol());
        char v = std::tolower(victim->symbol());

        if (a == 'p' && v == 'q') pawnTakesQueenIdx = i;
        if (a == 'q' && v == 'p') queenTakesPawnIdx = i;
    }

    ASSERT_NE(pawnTakesQueenIdx, -1) << "pawn-takes-queen move not found";
    ASSERT_NE(queenTakesPawnIdx, -1) << "queen-takes-pawn move not found";
    EXPECT_LT(pawnTakesQueenIdx, queenTakesPawnIdx)
        << "pawn-takes-queen should appear before queen-takes-pawn (MVV-LVA)";
}

// Position with no captures at all — ordering must not crash and list is valid
TEST(MoveOrderTest, NoCapturesStable) {
    Board b = emptyBoard();
    b.setPiece(7, 4, new King(7, 4, true));
    b.setPiece(0, 4, new King(0, 4, false));
    // only kings, no captures possible for white

    std::vector<Move> moves;
    auto kingMoves = b.getPiece(7, 4)->generateMoves(b, false);
    moves.insert(moves.end(), kingMoves.begin(), kingMoves.end());

    size_t sizeBefore = moves.size();
    EXPECT_NO_THROW(MoveOrder::orderMoves(moves, b));
    EXPECT_EQ(moves.size(), sizeBefore);
}

// Equal-value captures (rook takes rook) should both appear before quiet moves
TEST(MoveOrderTest, EqualValueCapturesBeforeQuiets) {
    Board b = emptyBoard();
    b.setPiece(7, 4, new King(7, 4, true));
    b.setPiece(0, 4, new King(0, 4, false));
    b.setPiece(4, 0, new Rook(4, 0, true));   // white rook
    b.setPiece(4, 7, new Rook(4, 7, false));  // black rook — capturable

    std::vector<Move> moves;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (!b.isEmpty(r, c) && b.getPiece(r, c)->getColor() == true) {
                auto pieceMoves = b.getPiece(r, c)->generateMoves(b, false);
                moves.insert(moves.end(), pieceMoves.begin(), pieceMoves.end());
            }
        }
    }

    MoveOrder::orderMoves(moves, b);
    EXPECT_TRUE(capturesBeforeQuiets(moves, b));
}

// ─────────────────────────────────────────────
// AlphaBeta::search() TESTS
// ─────────────────────────────────────────────

// search() must return a valid move, not the sentinel {-1,-1,-1,-1}
TEST(SearchTest, ReturnsValidMove) {
    Game game;
    moveHistory history;

    SearchResult result = AlphaBeta::search(game, history, 1);

    EXPECT_NE(result.bestMove.fromRow, -1) << "search() returned sentinel move";
    EXPECT_NE(result.bestMove.fromCol, -1);
    EXPECT_NE(result.bestMove.toRow,   -1);
    EXPECT_NE(result.bestMove.toCol,   -1);
}

// search() must find the only mating move in a mate-in-one position.
// FEN: 6k1/5QK1/8/8/8/8/8/8 w - - 0 1
// White queen on f7 delivers checkmate — any other move lets black survive.
TEST(SearchTest, FindsMateInOne) {
    
    Game game;
    game.setBoard(emptyBoard());
    game.setTurn(true);

    game.getBoard().setPiece(0, 6, new King(0, 6, false));   // black king  g8
    game.getBoard().setPiece(2, 5, new Queen(2, 5, true));   // white queen f7
    game.getBoard().setPiece(2, 6, new King(2, 6, true));    // white king  g7

    moveHistory history;

    SearchResult result = AlphaBeta::search(game, history, 1);          

    // After playing the best move, black must be in checkmate
    game.performMove(result.bestMove);

    EXPECT_TRUE(game.getBoard().isCheckMate(false))
        << "search() did not find the mating move";
}

// search() must prefer capturing a free queen over any quiet move
// passes
TEST(SearchTest, PrefersFreeCaptureOverQuiet) {
    Board b = emptyBoard();
    b.setPiece(7, 4, new King(7, 4, true));
    b.setPiece(0, 4, new King(0, 4, false));
    b.setPiece(4, 3, new Pawn(4, 3, true));    // white pawn d4
    b.setPiece(3, 4, new Queen(3, 4, false));  // undefended black queen e5

    Game game;
    game.setBoard(b);
    game.setTurn(true);

    moveHistory history;
    SearchResult result = AlphaBeta::search(game, history, 1);

    // Best move must be pawn captures queen: (4,3) -> (3,4)
    EXPECT_EQ(result.bestMove.fromRow, 4);
    EXPECT_EQ(result.bestMove.fromCol, 3);
    EXPECT_EQ(result.bestMove.toRow,   3);
    EXPECT_EQ(result.bestMove.toCol,   4)
        << "search() did not capture the free queen";
}

// Board and history must be fully restored after search()
TEST(SearchTest, BoardAndHistoryRestoredAfterSearch) {
    Game game;
    moveHistory history;

    long long whiteBefore = countMaterial(game.getBoard(), true);
    long long blackBefore = countMaterial(game.getBoard(), false);
    size_t    sizeBefore  = history.moveHistoryVector.size();
    int       stateBefore = history.currentBoardState;
    bool      turnBefore  = game.isWhiteTurn();

    AlphaBeta::search(game, history, 2);

    EXPECT_EQ(countMaterial(game.getBoard(), true),  whiteBefore);
    EXPECT_EQ(countMaterial(game.getBoard(), false), blackBefore);
    EXPECT_EQ(history.moveHistoryVector.size(), sizeBefore);
    EXPECT_EQ(history.currentBoardState, stateBefore);
    EXPECT_EQ(game.isWhiteTurn(), turnBefore);
}

// search() score must be deterministic across two identical positions
TEST(SearchTest, DeterministicOnSamePosition) {
    Game g1, g2;
    moveHistory h1, h2;

    SearchResult r1 = AlphaBeta::search(g1, h1, 2);
    SearchResult r2 = AlphaBeta::search(g2, h2, 2);

    EXPECT_EQ(r1.score, r2.score)
        << "search() returned different scores on identical positions";
}

// ─────────────────────────────────────────────
// IterDeep::search() TESTS
// ─────────────────────────────────────────────

// IterDeep must always return a valid move even with a very tight time limit
TEST(IterDeepTest, AlwaysReturnsValidMove) {
    Game game;
    moveHistory history;

    // minDepth=1 guarantees at least one completed search
    SearchResult result = IterDeep::search(game, history, 1, 1);

    EXPECT_NE(result.bestMove.fromRow, -1) << "IterDeep returned sentinel move";
    EXPECT_NE(result.bestMove.fromCol, -1);
    EXPECT_NE(result.bestMove.toRow,   -1);
    EXPECT_NE(result.bestMove.toCol,   -1);
}

// On a simple position IterDeep and AlphaBeta::search must agree on score at depth 1
//passes
TEST(IterDeepTest, MatchesAlphaBetaAtDepth1) {
    Board b = emptyBoard();
    b.setPiece(7, 4, new King(7, 4, true));
    b.setPiece(0, 4, new King(0, 4, false));
    b.setPiece(4, 3, new Pawn(4, 3, true));
    b.setPiece(3, 4, new Queen(3, 4, false));

    Game g1, g2;
    g1.setBoard(b); g1.setTurn(true);
    g2.setBoard(b); g2.setTurn(true);

    moveHistory h1, h2;

    SearchResult abResult    = AlphaBeta::search(g1, h1, 1);
    // generous time limit so iterdeep completes depth 1 comfortably
    SearchResult iterResult  = IterDeep::search(g2, h2, 5000, 1, 1);

    EXPECT_EQ(abResult.score, iterResult.score)
        << "IterDeep and AlphaBeta::search disagree on score at depth 1";
}

// Board and history must be fully restored after IterDeep::search()
// does not pass all the time, suspecting seg error
TEST(IterDeepTest, BoardAndHistoryRestoredAfterSearch) {
    Game game;
    moveHistory history;

    long long whiteBefore = countMaterial(game.getBoard(), true);
    long long blackBefore = countMaterial(game.getBoard(), false);
    size_t    sizeBefore  = history.moveHistoryVector.size();
    int       stateBefore = history.currentBoardState;
    bool      turnBefore  = game.isWhiteTurn();

    IterDeep::search(game, history, 100, 1);

    EXPECT_EQ(countMaterial(game.getBoard(), true),  whiteBefore);
    EXPECT_EQ(countMaterial(game.getBoard(), false), blackBefore);
    EXPECT_EQ(history.moveHistoryVector.size(), sizeBefore);
    EXPECT_EQ(history.currentBoardState, stateBefore);
    EXPECT_EQ(game.isWhiteTurn(), turnBefore);
}