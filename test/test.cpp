#include <filesystem>
#include <gtest/gtest.h>
#include <algorithm> 
#include <fstream>


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

TEST(MenuTest, DisplaysMainMenuCorrectly)
{
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

TEST(MenuTest, GuestCannotLoadGames)
{
    Menu menu("Guest", true);

    CinRedirect input("2\n5\n");
    testing::internal::CaptureStdout();

    menu.run();

    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("Guests cannot load games."), std::string::npos);
}

TEST(MenuTest, DisplaySubMenu)
{
    Menu menu("Bob", false);

    CinRedirect input("1\n3\n5\n");

    testing::internal::CaptureStdout();
    menu.run();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("--- New Game ---"), std::string::npos);
    EXPECT_NE(output.find("Local 2-Player"), std::string::npos);
    EXPECT_NE(output.find("Play Against Bot"), std::string::npos);
}

TEST(MenuTest, DisplayRemovingFile)
{
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
    Board newBoard;
    moveHistory moveHistory;
    undoMove undo;

    std::stringstream ss;
    std::streambuf* oldCout = std::cout.rdbuf();
    std::cout.rdbuf(ss.rdbuf());

    undo.undoLatestMove(moveHistory, newBoard);

    std::cout.rdbuf(oldCout);

    EXPECT_EQ(ss.str(), "To early to undo any move.\n");
}

TEST(MoveHistoryVectorTest, undoFirstTwoMoves) {
    Board newBoardWithMoves;
    moveHistory moveHistoryDefault;
    moveHistory moveHistoryWithMoves;
    undoMove undo;

    Move move1 = {0, 1, 0, 2};
    Move move2 = {7, 6, 7, 5};

    convertMoveToString converter;

    std::string moveStr1 = converter.moveAsString(move1, newBoardWithMoves);
    newBoardWithMoves.movePiece(move1);
    moveHistoryWithMoves.appendLatestMove(moveStr1);

    std::string moveStr2 = converter.moveAsString(move2, newBoardWithMoves);
    newBoardWithMoves.movePiece(move2);
    moveHistoryWithMoves.appendLatestMove(moveStr2);

    ASSERT_EQ(moveHistoryWithMoves.moveHistoryVector.size(), 2);
    undo.undoLatestMove(moveHistoryWithMoves, newBoardWithMoves);
    undo.undoLatestMove(moveHistoryWithMoves, newBoardWithMoves);

    EXPECT_EQ(moveHistoryWithMoves.moveHistoryVector.size(), moveHistoryDefault.moveHistoryVector.size());
}

TEST(MoveHistoryVectorTest, undoThreeTurns) {
    Board newBoard;
    Board newBoardWithMoves;
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

    std::string moveStr1 = converter.moveAsString(move1, newBoard);
    moveHistoryDefault.appendLatestMove(moveStr1);

    moveStr1 = converter.moveAsString(move1, newBoardWithMoves);
    moveHistoryWithMoves.appendLatestMove(moveStr1);
    std::string moveStr2 = converter.moveAsString(move2, newBoardWithMoves);
    moveHistoryWithMoves.appendLatestMove(moveStr2);
    std::string moveStr3 = converter.moveAsString(move3, newBoardWithMoves);
    moveHistoryWithMoves.appendLatestMove(moveStr3);
    std::string moveStr4 = converter.moveAsString(move4, newBoardWithMoves);
    moveHistoryWithMoves.appendLatestMove(moveStr4);
    std::string moveStr5 = converter.moveAsString(move5, newBoardWithMoves);
    moveHistoryWithMoves.appendLatestMove(moveStr5);
    std::string moveStr6 = converter.moveAsString(move6, newBoardWithMoves);
    moveHistoryWithMoves.appendLatestMove(moveStr6);
    std::string moveStr7 = converter.moveAsString(move7, newBoardWithMoves);
    moveHistoryWithMoves.appendLatestMove(moveStr7);

    for (unsigned i = 0; i < 6; ++i) {
        undo.undoLatestMove(moveHistoryWithMoves, newBoardWithMoves);
    }

    EXPECT_EQ(moveHistoryWithMoves.moveHistoryVector.size(), moveHistoryDefault.moveHistoryVector.size());
}