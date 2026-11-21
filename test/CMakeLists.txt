CMAKE_MINIMUM_REQUIRED(VERSION 3.20)
project(ChessProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

include_directories( ../src/header ../src/header/sub ../engine)

ADD_SUBDIRECTORY(googletest)

add_executable(runAllTests
    test.cpp
    ../src/app/api/sub/Queen.cpp
    ../src/app/api/sub/Rook.cpp
    ../src/app/api/sub/Pawn.cpp
    ../src/app/api/sub/Knight.cpp
    ../src/app/api/sub/King.cpp
    ../src/app/api/sub/Bishop.cpp

    ../src/app/api/Board.cpp
    ../src/app/api/moveHistory.cpp
    ../src/app/api/convertMoveToString.cpp

    ../src/app/data/SaveGame.cpp
    ../src/app/data/LoadGame.cpp
    ../src/app/data/DeleteSaveFile.cpp

    ../src/app/interface/Menu.cpp
    ../src/app/interface/UI.cpp

    ../src/app/user/FileManager.cpp
    ../src/app/user/AccountManager.cpp

    ../src/app/StockfishAI.cpp

    ../engine/eval/material.cc
    ../engine/eval/tables/src/pst.cc

    ../engine/search/MoveExecutor.cc

    ../engine/search/src/negamax.cc
    ../engine/search/src/alphabeta.cc
    ../engine/search/src/quiescence.cc
    ../engine/search/src/ordering.cc
    ../engine/search/src/iterdeep.cc
    ../engine/search/src/epd.cc

)

add_executable(runChess
    ../src/main.cpp

    ../src/app/api/sub/Queen.cpp
    ../src/app/api/sub/Rook.cpp
    ../src/app/api/sub/Pawn.cpp
    ../src/app/api/sub/Knight.cpp
    ../src/app/api/sub/King.cpp
    ../src/app/api/sub/Bishop.cpp

    ../src/app/api/Board.cpp
    ../src/app/api/moveHistory.cpp
    ../src/app/api/convertMoveToString.cpp

    ../src/app/data/SaveGame.cpp
    ../src/app/data/LoadGame.cpp
    ../src/app/data/DeleteSaveFile.cpp

    ../src/app/interface/Menu.cpp
    ../src/app/interface/UI.cpp

    ../src/app/user/FileManager.cpp
    ../src/app/user/AccountManager.cpp

    ../src/app/StockfishAI.cpp
    
    ../engine/eval/material.cc
    ../engine/eval/tables/src/pst.cc

    ../engine/search/MoveExecutor.cc

    ../engine/search/src/negamax.cc
    ../engine/search/src/alphabeta.cc
    ../engine/search/src/quiescence.cc
    ../engine/search/src/ordering.cc
    ../engine/search/src/iterdeep.cc


)

string(TOUPPER "${CMAKE_BUILD_TYPE}" BENCH_BUILD_TYPE)
add_executable(bench
    ../engine/benchmark/sim.cc

    ../src/app/api/sub/Queen.cpp
    ../src/app/api/sub/Rook.cpp
    ../src/app/api/sub/Pawn.cpp
    ../src/app/api/sub/Knight.cpp
    ../src/app/api/sub/King.cpp
    ../src/app/api/sub/Bishop.cpp

    ../src/app/api/Board.cpp
    ../src/app/api/moveHistory.cpp

    ../engine/eval/material.cc
    ../engine/eval/tables/src/pst.cc

    ../engine/search/MoveExecutor.cc

    ../engine/search/src/negamax.cc
    ../engine/search/src/alphabeta.cc
    ../engine/search/src/quiescence.cc
    ../engine/search/src/ordering.cc
    ../engine/search/src/iterdeep.cc
    ../engine/search/src/epd.cc
)
target_compile_definitions(bench PRIVATE BENCH_FLAGS="${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${BENCH_BUILD_TYPE}}" BENCH_BUILD_TYPE_NAME="${CMAKE_BUILD_TYPE}")

find_package(Threads REQUIRED)

target_link_libraries(runChess Threads::Threads)
target_link_libraries(bench Threads::Threads)
target_link_libraries(runAllTests gtest gtest_main Threads::Threads)

#only link stdc++fs for old GCC
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
    AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
    target_link_libraries(runChess stdc++fs)
    target_link_libraries(runAllTests stdc++fs)
endif()

enable_testing()
add_test(NAME runAllTests COMMAND runAllTests)