#include "Game.h"
#include "Pieces.h"
#include "api/Board.h"
#include "api/moveHistory.h"

#include "search/MoveExecutor.h"

#include "search/header/alphabeta.h"
#include "search/header/quiescence.h"
#include "search/header/ordering.h"
#include "eval/material.h"

#include <algorithm>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

long long AlphaBeta::alphabeta(Game& game, MoveHistory& history, int depth, long long alpha, long long beta) {
    nodes++;
    if (timeUp(nodes)) return 0;
    if (depth == 0) return quiescence ? Quiescence::quiescence(game, history, alpha, beta, 1) : Evaluator::evaluate(game);

    std::vector<Move> moves = MoveExecutor::generateMoves(game);

    if (moves.empty()) return game.getBoard().isInCheck(game.isWhiteTurn()) ? -100000 : 0;

    if (ordering) MoveOrder::orderMoves(moves, game.getBoard());
    
    for (const Move& move : moves) {
        if (!MoveExecutor::make(move, game, history)) continue;

        long long score = -alphabeta(game, history, depth - 1, -beta, -alpha);
        
        MoveExecutor::undo(game,history);

        if (pruning && score >= beta) return beta;     //opponent wont allow this move
        if (score > alpha) alpha = score;   //next best move for player
    }
    return alpha;
}

SearchResult AlphaBeta::search(Game& game, MoveHistory& history, int depth, int threads) {
    static const long long INF = 1e18;

    nodes = 0;
    Quiescence::nodes = 0;

    std::vector<Move> moves = MoveExecutor::generateMoves(game);

    if (ordering) MoveOrder::orderMoves(moves, game.getBoard());

    SearchResult result;
    result.score = -INF;
    result.bestMove = moves.empty() ? Move{-1, -1, -1, -1} : moves[0];

    //hardware_concurrency() may report 0, so clamp before trusting it
    unsigned int hw = std::thread::hardware_concurrency();
    if (hw == 0) hw = 1;

    int workerCount = threads;
    if (workerCount > static_cast<int>(hw)) workerCount = static_cast<int>(hw);
    if (workerCount > static_cast<int>(moves.size())) workerCount = static_cast<int>(moves.size());

    if (workerCount <= 1 || moves.empty()) {
        long long alpha = -INF;
        long long beta = INF;

        for (auto move : moves) {
            if (!MoveExecutor::make(move, game, history)) continue;

            long long score = -alphabeta(game, history, depth - 1, -beta, -alpha);

            MoveExecutor::undo(game, history);

            if (stopped.load(std::memory_order_relaxed)) break;   //score is from an aborted subtree, keep only the root moves finished so far
            if (score > result.score) {
                result.score = score;
                result.bestMove = move;
            }
            if (score > alpha) alpha = score;
        }

        result.mainNodes = nodes;
        result.quiescenceNodes = Quiescence::nodes;
        return result;
    }

    //Parallel root: move 0 is searched alone to seed the window, then workers take the rest
    //dynamically with a window narrowed by the shared best score. Each worker copies the Game
    //and builds its own MoveHistory, so make/undo never races.
    //Workers do not catch exceptions: like the sequential path, allocation failure is fatal.
    std::mutex resultMutex;
    long long bestScore = -INF;
    size_t bestIndex = moves.size();
    long long mainTotal = 0, quiescenceTotal = 0;

    if (MoveExecutor::make(moves[0], game, history)) {
        bestScore = -alphabeta(game, history, depth - 1, -INF, INF);
        MoveExecutor::undo(game, history);
        bestIndex = 0;
    }
    mainTotal = nodes;
    quiescenceTotal = Quiescence::nodes;

    std::atomic<size_t> next{1};
    auto worker = [&]() {
        for (size_t i = next++; i < moves.size(); i = next++) {
            Game localGame = game;
            MoveHistory localHistory;

            if (!MoveExecutor::make(moves[i], localGame, localHistory)) continue;

            long long lower;
            {
                std::lock_guard<std::mutex> lock(resultMutex);
                //earlier-index moves search one point wider so an exact tie is detected and the earliest root move wins
                lower = (i < bestIndex) ? bestScore - 1 : bestScore;
            }

            long long score = -alphabeta(localGame, localHistory, depth - 1, -INF, -lower);

            MoveExecutor::undo(localGame, localHistory);

            if (score <= lower) continue;   //fail-hard bound at or below the window, not an exact score

            std::lock_guard<std::mutex> lock(resultMutex);
            //tie-break on the earliest root move so scheduling cannot change the answer
            if (score > bestScore || (score == bestScore && i < bestIndex)) {
                bestScore = score;
                bestIndex = i;
            }
        }

        std::lock_guard<std::mutex> lock(resultMutex);
        mainTotal += nodes;                 //each worker is a fresh thread, so its counters start at 0
        quiescenceTotal += Quiescence::nodes;
    };

    std::vector<std::thread> pool;
    pool.reserve(workerCount);
    for (int id = 0; id < workerCount; id++) pool.emplace_back(worker);
    for (std::thread& t : pool) t.join();

    if (bestIndex < moves.size()) {
        result.score = bestScore;
        result.bestMove = moves[bestIndex];
    }

    result.mainNodes = mainTotal;
    result.quiescenceNodes = quiescenceTotal;
    return result;
}