#ifndef ALPHABETA_H
#define ALPHABETA_H

#include "Game.h"
#include "api/moveHistory.h"
#include "Pieces.h"

#include <atomic>
#include <chrono>

struct SearchResult {
    Move bestMove;
    long long score;
    long long mainNodes = 0;        //alphabeta() calls during this search(), summed over all workers
    long long quiescenceNodes = 0;  //quiescence() calls during this search(), summed over all workers
    int depth = 0;                  //IterDeep: depth of the returned result, 0 if no depth completed; AlphaBeta::search leaves it 0
};

class AlphaBeta {
    public:
        //benchmark switches: defaults are the normal engine; set them before search(), never during one
        static inline bool pruning = true;   //beta cutoff in alphabeta()
        static inline bool ordering = true;  //MoveOrder::orderMoves at the root and inside alphabeta()
        static inline bool quiescence = true; //false: depth-0 nodes return Evaluator::evaluate instead of calling quiescence
        //per-thread call counter; search() sums the workers' counters into SearchResult
        static inline thread_local long long nodes = 0;

        //hard deadline, set only by IterDeep (global, so one IterDeep search at a time like the switches above).
        //time_point::max() means no deadline, so a plain search() is unchanged. After stopped is set, alphabeta() and
        //quiescence() return 0 at once and the search output must be discarded by the caller.
        static inline std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::time_point::max();
        static inline std::atomic<bool> stopped{false};

        //called with the thread's own node counter right after incrementing it: reads the flag every node (so everything
        //unwinds fast) and the clock only every 256 nodes: a node costs about 10 us, so that bounds the overrun to a few ms (1024 overran by 13 ms median)
        static bool timeUp(long long counter) {
            if (stopped.load(std::memory_order_relaxed)) return true;
            if ((counter & 255) == 0 && std::chrono::steady_clock::now() >= deadline) {
                stopped.store(true, std::memory_order_relaxed);
                return true;
            }
            return false;
        }

        static long long alphabeta(Game& game, MoveHistory& history, int depth, long long alpha, long long beta);

        //threads<=1 runs the sequential root; threads>1 seeds with root move 0, then
        //workers take the remaining root moves dynamically, each with a window narrowed
        //by the shared best score. The returned move and score equal the sequential
        //search (ties go to the earliest root move); the node count can differ.
        //Each worker copies the Game and uses its own MoveHistory.
        static SearchResult search(Game& game, MoveHistory& history, int depth, int threads = 1);
};


#endif
