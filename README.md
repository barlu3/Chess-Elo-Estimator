[![CI](https://github.com/barlu3/Chess-Elo-Estimator/actions/workflows/main.yml/badge.svg)](https://github.com/barlu3/Chess-Elo-Estimator/actions/workflows/main.yml)

# Chess Elo Estimator
Author: Luke Baring https://github.com/barlu3

## Project Description

This project is built off a chess engine I built for my CS100 project at UCR along with several other engineers (Andres Solorio https://github.com/Andres-55, Temuulen Tserenchimed https://github.com/ttser002, Jeremiah Rodriguez https://github.com/JeremiahDRodriguez). 

In this, I design a Chess MiniMax algorithm that uses alpha-beta pruning and quiescence searching to optimize game state evaluation. I also explore data collection by fixing time constraints per engine move (1ms, 50ms, 100ms) and benchmarking this engine against established Chess bots/engines (Stockfish at different levels of difficulty).

Elo estimation is planned for this project once benchmarking has been finished.

## Installation/Usage
> * This Application is compiled in Linux, in 64-bits. Emulation may be required to run it.
### Prerequisites
> * CMake Version 3.20 or higher.
> * C++17 or higher.
### To Run this Application
> 1. Clone this repository inside your IDE using ``git clone --recursive https://github.com/barlu3/Chess-Elo-Estimator.git``
> 2. Run ``git submodules update --init --recursive``
> 3. Navigate to Stockfish/src/
> 4. Run ``git checkout sf_14.1``
> 5. Compile using ``make build ARCH=x86-64``
> 6. Navigate to ../../test/
> 8. Compile using ``cmake .`` then ``make``
> 9. Run ``./bin/runChess``

## Testing
> * This project was tested and validated through Continuous Integration with a repeatedly updated main.yml file to run all googletests after each input. Manual testing on MacOS and Windows (using WSL) machines was also performed.
 


## Benchmark facts

* **C1 (nodes saved by pruning):** main-search nodes, fixed depth 5, static-eval leaves (quiescence off in all modes), alpha-beta and alpha-beta + MVV-LVA ordering against plain negamax.
  * Comparison depth: 5, because full-width depth 6 is 1 to 8 billion nodes per middlegame, which is infeasible here. Depth-6 raw counts for alpha-beta + MVV-LVA are recorded in `engine/benchmark/results/c1.csv`; alpha-beta without ordering at depth 6 is omitted (up to 22 min per position).
  * Positions: `engine/benchmark/positions.epd`, 20 positions (6 opening, 7 middlegame, 7 endgame).
  * Metric: main-search nodes (each `alphabeta()` call, depth-0 calls included). Quiescence is off in every mode, so there are no quiescence nodes; the real engine adds quiescence on top.
  * CPU: Apple M5, 10 cores (4 performance + 6 efficiency).
  * Compiler and flags: Apple clang 21.0.0 (clang-2100.1.1.101), `-O3 -DNDEBUG` (CMake `Release`).
  * Measured at commit `af1bf0b`.
  * Results and the one-command rerun: `engine/benchmark/RESULTS.md`.

* **C2 (thread scaling):** wall-clock speedup of one fixed-depth `AlphaBeta::search` on 1, 2, 4 and 8 threads, alpha-beta + MVV-LVA ordering (`pruning = true`, `ordering = true`) with static-eval leaves (`quiescence = false`), the same configuration as C1.
  * Depth 5, because 7 repetitions at depth 6 projected to roughly 1.6 to 2.3 h, over the 1.5 h budget. All 20 positions of `engine/benchmark/positions.epd`, 7 repetitions per (position, thread count), 560 runs.
  * Method: every run is a separate process (`bench time`), interleaved (repetition, then position, then thread count, with the thread order alternating), run by `engine/benchmark/c2.sh`; speedup is median 1-thread ms over median N-thread ms. Effective workers are `min(threads, 10, root moves)`.
  * Result: pooled speedup 0.89x at 2 threads, 1.56x at 4 and 1.61x at 8 (median position 0.83x, 1.33x, 1.34x); best move and score identical across 1, 2, 4 and 8 threads on all 20 positions and all repetitions.
  * CPU: Apple M5, 10 cores (4 performance + 6 efficiency).
  * Compiler and flags: Apple clang 21.0.0 (clang-2100.1.1.101), `-O3 -DNDEBUG` (CMake `Release`).
  * macOS 26.6.2 (25G83).
  * Machine state: AC power, lid open, about 17 GB of 17 to 18 GB swap in use (other apps) and system daemons active during the run, so multi-thread cells are noisy (median spread between repetitions 20% to 27%).
  * Measured at commit `aa7f76a`. The parallel root search changed at commit `9832a84` (seed root move 0, then dynamic hand-out with narrowed windows; before it, node counts were 2.6x to 6.8x the sequential search at depth 6). C1 is unaffected because it used 1 thread (1-thread node counts re-checked identical on op1, mg2, eg5, mg4 and mg7).
  * Raw runs: `engine/benchmark/results/c2_runs.csv`; summary `c2_summary.csv`; results, explanation and the one-command rerun: `engine/benchmark/RESULTS.md`.

* **C3 (nodes per second):** main-search nodes / wall-clock seconds, computed from existing data with `python3 -I engine/benchmark/c3_nps.py`, no new runs; same configuration as C1 and C2 (alpha-beta + MVV-LVA, static-eval leaves, so a node is one `alphabeta()` call and real-engine quiescence is not counted).
  * Sources and depths: depth 6, 1 thread from `engine/benchmark/results/c1.csv` (`mvv6_main / mvv6_ms`, one run per position); depth 5, 1/2/4/8 threads from `engine/benchmark/results/c2_runs.csv` (7 runs per cell), with C1's depth-5 1-thread figure (`mvv5_main / mvv5_ms`) as a cross-check.
  * Median: per (position, threads) the median over the 7 runs of per-run `main_nodes / seconds`, then the median over the 20 positions; pooled = sum of median nodes / sum of median seconds. Multi-thread NPS is the total across workers.
  * Result: depth 6, 1 thread: median 59,387 NPS (min 19,295, max 291,122), pooled 44,859. Depth 5 median / pooled at 1, 2, 4, 8 threads: 86,572 / 71,630, 79,677 / 68,218, 141,316 / 131,629, 161,869 / 154,592. C2's 1-thread depth-5 figures are 5.8% (median) and 12.5% (pooled) above C1's single runs.
  * Exclusion rule: also reported without cells whose median time is under 50 ms (eg3 and eg4 at every thread count, eg1 at 8 threads only, 47.0 ms), because thread start-up dominates those; the pooled figures change by under 0.6%, the medians by 4% to 10%.
  * Results and notes: `engine/benchmark/RESULTS.md`.

* **C4 (Win At Chess solve rate):** % of the 300 WAC positions where the engine's move is a listed best move (`bm`, and not an `am`), one `IterDeep::search` per position at a hard per-move deadline, scored by `bench wac`.
  * Suite: `engine/benchmark/wac.epd`, 300 positions (`WAC.001` to `WAC.300`), SHA-256 `9bfb7b42cf5bbc7f3d60ff767ca3be460e0ae4d9110ef9a71ddf0bd7071d4d46`. Source: `https://raw.githubusercontent.com/jdart1/arasan-chess/207a88358c414c15eec7933774d25ffe24fd4598/tests/wacnew.epd` (Arasan chess engine repo, `tests/wacnew.epd`). Edition: `wacnew`, the Fred Wilson 2001 revision in algebraic notation as identified by the chessprogramming wiki page "Win at Chess" (its "EPD Wacnew" record matches all 300 lines; the file itself has no version text). It lists alternative best moves: 37 positions have more than one `bm`, WAC.274 has an `am`.
  * Checks: all 300 FENs load through `Game::loadFEN`; every `bm` and `am` resolves to one legal engine move; no underpromotion positions (the file has no promotions), nothing unresolved or unsolvable.
  * Time controls: 100 ms and 1000 ms per move; one run per control.
  * Engine: `IterDeep::search(game, history, ms, 1, 64, 1)`, threads = 1, `pruning`, `ordering` and `quiescence` all on (defaults), `maxDepth` 64 so the clock ends the search.
  * Deadline: `alphabeta()` and `quiescence()` read a shared stop flag every node and the clock every 256 nodes; an aborted depth is discarded and the last completed depth is returned. Depth 1 is not exempt (at HEAD its depth-1 search with quiescence took up to 34 s on WAC positions); if it is cut off, the best of the root moves searched so far is returned. No deadline set leaves node counts unchanged (checked against `c1.csv`).
  * Result: 80/300 (26.7%) at 100 ms and 123/300 (41.0%) at 1 s. Median completed depth 1 and 3; no completed depth in 69 and 10 positions. Time per move: median 103.6 ms and 1003.5 ms, max 118.1 ms and 1012.5 ms (limit plus at most 18.1 ms). Against the older Bergbom `wac.epd` (fewer accepted moves) the same moves score 77/300 and 119/300.
  * CPU: Apple M5, 10 cores (4 performance + 6 efficiency), one thread used. macOS 26.6.2 (25G83). Compiler and flags: Apple clang 21.0.0 (clang-2100.1.1.101), `-O3 -DNDEBUG` (CMake `Release`).
  * Machine state: AC power (100% charged), lid open, no thermal warning recorded, `caffeinate -ims` held by `c4.sh`; about 17.0 GB of 18.4 GB swap in use and Chrome, Discord and Claude Code running in the background.
  * Measured at commit `ed0d72d`.
  * Raw rows: `engine/benchmark/results/c4_100ms.csv` and `c4_1000ms.csv`; results, caveats (including the move-generator perft caveat) and the one-command rerun: `engine/benchmark/RESULTS.md`.
