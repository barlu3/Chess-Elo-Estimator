[![CI](https://github.com/barlu3/lab-5-github-actions-jrodr675-lbari008/actions/workflows/main.yml/badge.svg)](https://github.com/barlu3/lab-5-github-actions-jrodr675-lbari008/actions/workflows/main.yml)

# Chess Elo Estimator
Author: Luke Baring https://github.com/barlu3

## Project Description

This project is built off a chess engine I built for my CS100 project at UCR along with several other engineers (Andres Solorio https://github.com/Andres-55, Temuulen Tserenchimed https://github.com/ttser002, Jeremiah Rodriguez https://github.com/JeremiahDRodriguez). In this, I design a Chess MiniMax algorithm that uses alpha-beta pruning and quiescence searching to optimize game state evaluation. I also explore data collection by fixing time constraints per engine move (1ms, 50ms, 100ms) and benchmarking this engine against established Chess bots/engines (Stockfish at different levels of difficulty).

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
> 8. Compile using ``cmake . | make``
> 9. Run ``./bin/runChess``

## Testing
> * This project was tested and validated through Continuous Integration with a repeatedly updated main.yml file to run all googletests after each input. Manual testing on MacOS and Windows machines was also performed.
 
