#include "Game.h"
#include "api/moveHistory.h"
#include "search/header/alphabeta.h"
#include "search/header/iterdeep.h"

#include <chrono>

SearchResult IterDeep::search(Game& game, MoveHistory& history, int timeLimitMs, int minDepth, int maxDepth, int threads) {
    auto startTime = std::chrono::steady_clock::now();

    auto elapsedMs = [&]() -> int {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
    };

    //Sentinel -- replaced as soon as a depth completes or the first depth is cut off
    SearchResult best;
    best.score = -1e18;
    best.bestMove = {-1,-1,-1,-1};

    //every depth, including the first, aborts at the hard deadline; the clock is polled inside the search
    AlphaBeta::deadline = startTime + std::chrono::milliseconds(timeLimitMs);

    for (int depth = minDepth; depth <= maxDepth; depth++) {
        SearchResult result = AlphaBeta::search(game, history, depth, threads);

        if (AlphaBeta::stopped) {
            //depth was cut off, so keep the last completed depth; if none completed, the cut-off search's partial best is still a legal move
            if (best.bestMove.fromRow == -1) best = result;
            break;
        }

        best = result;
        best.depth = depth;

        //after completing iteration, if we exceeded time limit break
        if (elapsedMs() >= timeLimitMs) break;
    }

    AlphaBeta::deadline = std::chrono::steady_clock::time_point::max();
    AlphaBeta::stopped = false;
    return best;
}