#include "Game.h"
#include "api/moveHistory.h"
#include "search/header/alphabeta.h"
#include "search/header/iterdeep.h"

#include <chrono>

SearchResult IterDeep::search(Game& game, moveHistory& history, int timeLimitMs, int minDepth, int maxDepth) {
    auto startTime = std::chrono::steady_clock::now();

    auto elapsedMs = [&]() -> int {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
    };

    //Sentinel -- replaced after minDepth is guaranteed to complete
    SearchResult best;
    best.score = -1e18;
    best.bestMove = {-1,-1,-1,-1};

    for (int depth = minDepth; depth <= maxDepth; depth++) {
        SearchResult result = AlphaBeta::search(game, history, depth);

        //if we completed the result accept the result always
        best = result;
        
        //after completing iteration, if we exceeded time limit break
        if (depth >= minDepth && elapsedMs() >= timeLimitMs) break;
    }

    return best;
}