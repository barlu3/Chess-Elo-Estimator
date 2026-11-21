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
