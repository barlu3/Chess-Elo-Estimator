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
