# Benchmark results

## C1: how many nodes does pruning save?

**Metric: main-search nodes, fixed depth 5, static-eval leaves (quiescence off in all modes).**
A main-search node is one `alphabeta()` call, depth-0 calls included. Quiescence is switched off
(`AlphaBeta::quiescence = false`) in every mode, so there are no quiescence nodes to report. The real engine
adds quiescence on top of this search.

Depth 5 is the comparison depth because full-width depth 6 is 1 to 8 billion nodes per middlegame, which is
infeasible here.

Modes, all at depth 5 on the same 20 positions (`positions.epd`: 6 opening, 7 middlegame, 7 endgame):

- **negamax**: the exact full-width tree, counted with a node-only walk (make/undo, no evaluation).
- **ab**: alpha-beta with the beta cutoff on and move ordering off (moves in generation order).
- **ab+mvvlva**: the default engine search (beta cutoff on, captures first by MVV-LVA).

### Setup

| | |
|---|---|
| Measured at commit | `af1bf0b` (clean tree) |
| CPU | Apple M5, 10 cores (4 performance + 6 efficiency) |
| Compiler | Apple clang 21.0.0 (clang-2100.1.1.101) |
| Flags | `-O3 -DNDEBUG` (CMake `Release`) |
| Threads | search runs single-threaded; the negamax tree count uses 10 threads |
| Timing | one timed run per mode per position (raw milliseconds are in `results/c1.csv`); a repeat of ab+mvvlva on the first 3 positions asserts that node counts are deterministic |

### Checks (all asserted in `sim.cc`, the run aborts on failure)

- **Identity check:** a real pruning-off, ordering-off, quiescence-off `AlphaBeta::search` visits exactly
  the node-only tree count, at depth 3 on all 20 positions and at depth 4 on 5 positions (op1, mg6, eg2, eg3,
  eg4). This shows the negamax baseline is the tree the search would visit. At the same depths the scores of
  the pruning-off, ab and ab+mvvlva searches also agree (no mismatches).
- **Root score check:** the depth-5 root score of ab and ab+mvvlva is identical on all 20 positions
  (`mismatches: 0`). The negamax column is a node count only, so it has no score of its own.

### Per-position results

Main-search nodes, fixed depth 5, static-eval leaves (quiescence off in all modes).
The depth 6 column is ab+mvvlva only; ab at depth 6 omitted: up to 22 min/position.

| id | category | negamax | ab | ab+mvvlva | % saved ab | % saved ab+mvvlva | d6 ab+mvvlva |
|---|---|---|---|---|---|---|---|
| op1 | opening | 5073731 | 64877 | 51078 | 98.72 | 98.99 | 473543 |
| op2 | opening | 10199645 | 101538 | 46534 | 99.00 | 99.54 | 706199 |
| op3 | opening | 28747855 | 482976 | 57447 | 98.32 | 99.80 | 1044896 |
| op4 | opening | 42968762 | 723616 | 234236 | 98.32 | 99.45 | 839477 |
| op5 | opening | 70190663 | 1150550 | 412015 | 98.36 | 99.41 | 1912882 |
| op6 | opening | 39908304 | 280041 | 77920 | 99.30 | 99.80 | 158477 |
| mg1 | middlegame | 197239343 | 7627464 | 73672 | 96.13 | 99.96 | 194707 |
| mg2 | middlegame | 10048076 | 338048 | 12837 | 96.64 | 99.87 | 185145 |
| mg3 | middlegame | 74419192 | 242718 | 62199 | 99.67 | 99.92 | 167293 |
| mg4 | middlegame | 168062332 | 3241163 | 365714 | 98.07 | 99.78 | 1407833 |
| mg5 | middlegame | 178094660 | 3425404 | 78165 | 98.08 | 99.96 | 287102 |
| mg6 | middlegame | 72018034 | 1056921 | 44328 | 98.53 | 99.94 | 121868 |
| mg7 | middlegame | 139640141 | 2525720 | 170285 | 98.19 | 99.88 | 2744459 |
| eg1 | endgame | 720900 | 75081 | 14857 | 89.59 | 97.94 | 22151 |
| eg2 | endgame | 595282 | 19224 | 9995 | 96.77 | 98.32 | 19509 |
| eg3 | endgame | 7145 | 1901 | 1554 | 73.39 | 78.25 | 4722 |
| eg4 | endgame | 2594 | 375 | 375 | 85.54 | 85.54 | 589 |
| eg5 | endgame | 3597435 | 222867 | 70272 | 93.80 | 98.05 | 250868 |
| eg6 | endgame | 645795 | 33197 | 14831 | 94.86 | 97.70 | 40099 |
| eg7 | endgame | 976847 | 50609 | 19537 | 94.82 | 98.00 | 65683 |

### Aggregates (% of main-search nodes saved versus negamax, depth 5)

| mode | median of per-position % | pooled % (1 - sum nodes / sum negamax) | min | max |
|---|---|---|---|---|
| ab | 98.07 | 97.92 | 73.39 | 99.67 |
| ab+mvvlva | 99.50 | 99.83 | 78.25 | 99.96 |

Pooled over all 20 positions: negamax 1,043,156,736 nodes, ab 21,664,290, ab+mvvlva 1,817,851. Ordering
alone cuts ab's remaining nodes by a further factor of about 11.9. By category (pooled % saved, ab / ab+mvvlva):
opening 98.58 / 99.55, middlegame 97.80 / 99.90, endgame 93.84 / 97.99. ab+mvvlva never searched more nodes
than ab; on eg4 (Fine #70, a locked pawn ending with no captures) they tie because there is nothing to order.

### Bullet

Cut nodes searched 99.8% at depth 5 versus plain negamax by **...**

Suggested phrasing: "Cut nodes searched 99.8% at depth 5 versus plain negamax by combining alpha-beta
pruning with MVV-LVA move ordering (20 positions, static-eval leaves; 97.9% from alpha-beta alone)."
(99.8% is the pooled figure; the median per position is 99.5%.)

### Caveats

- The move generator accepts a few illegal moves (22 extra depth-4 leaves from the start position versus
  standard perft). All three modes use the same generator, so the comparison is still fair.
- The real engine adds quiescence on top of this search, so its node counts are higher; this table isolates
  the pruning and ordering of the main search.
- The engine promotes only to a queen, so perft positions 4 and 5 (mg2, mg3) differ from standard perft counts.
- ab depth 6 omitted: up to 22 min/position (measured on two heavy middlegames: Kiwipete 119,985,849 nodes
  in 1311 s, perft position 6 35,789,528 nodes in 386 s). Depth-6 ab+mvvlva counts are in the table and CSV.
- ab uses generation order (a board scan), not a random order, so its numbers depend on that order.

### Reproduce

From the repository root (rebuilds in `build-bench`, which is git-ignored, and reruns everything above; about 1h40m
on the machine above, dominated by the negamax tree count):

```
mkdir -p engine/benchmark/results && cmake -S test -B build-bench -DCMAKE_BUILD_TYPE=Release && cmake --build build-bench --target bench -j8 && build-bench/bin/bench run engine/benchmark/positions.epd engine/benchmark/results/c1.csv
```

`bench` prints the compiler and flags, the identity-check progress, and the markdown table and aggregates shown
above. Keep the machine otherwise idle while it runs, because the timed searches share the CPU.

## C2: how does the search scale across threads?

Status: the diagnosis and the fix below are done. The 1/2/4/8-thread measurement over the 20 positions
(`c2.sh`, `c2_summary.py`) has not been run yet: the machine went to sleep (lid closed, on battery) and wall times
taken in that state are meaningless. `c2.sh` now refuses to run on battery or with the lid closed.

### Before the fix (indicative, one run per cell, AC power, lid open)

Configuration: ab+mvvlva (`pruning = true`, `ordering = true`), static-eval leaves (`quiescence = false`), depth 6,
the same as C1's depth-6 measurement. The original parallel root gave every root move a full window and dealt moves
round-robin (`i += workerCount`).

| position | threads | wall s | speedup | main nodes | node inflation |
|---|---|---|---|---|---|
| mg7 | 1 | 70.9 | 1.00 | 2,744,459 | 1.00 |
| mg7 | 2 | 183.9 | 0.39 | 7,121,575 | 2.60 |
| mg7 | 4 | 89.1 | 0.80 | 7,121,575 | 2.60 |
| mg7 | 8 | 55.0 | 1.29 | 7,121,575 | 2.60 |
| mg4 | 1 | 29.2 | 1.00 | 1,407,833 | 1.00 |
| mg4 | 8 | 75.9 | 0.38 | 9,623,312 | 6.84 |

Best move and score were identical at every thread count (mg7 `43-21` -80, mg4 `64-74` -110).

### Diagnosis (before the fix)

1. **Load imbalance is small.** `bench split` (full-window subtree size per ordered root move, depth 6) gives the
   per-worker node load under the round-robin split. Busiest worker divided by mean worker: mg7 1.10 / 1.19 / 1.29
   and mg4 1.10 / 1.20 / 1.31 at 2 / 4 / 8 workers. The largest single root subtree is 4.7% of mg7's total, so no
   root move dominates.
2. **Node inflation dominates.** Full-window root searches visit 2.59x (mg7) and 6.84x (mg4) the sequential nodes.
   Even with perfect per-node speed and the measured imbalance, the node-bound speedup (sequential nodes divided by
   the busiest worker's nodes) is 0.70 / 1.29 / 2.38 on mg7 and 0.26 / 0.49 / 0.89 on mg4. Two threads cannot beat
   one on either position.
3. **Per-node speed also drops inside one process.**
   - Hardware alone, mg7 depth 6, N separate 1-thread processes started together: nodes per second per process was
     38.7k solo, 36.6k at N=2 (0.94x), 28.1k at N=4 (0.73x), 14.4k at N=8 (0.37x).
   - The in-process workers' busiest-worker rate, corrected for imbalance, was 21.4k / 23.9k / 20.9k at 2 / 4 / 8
     threads (0.55x / 0.62x / 0.54x of solo).
   - Controlled test with identical work per thread (mg4 depth 6, K independent sequential searches as K threads in
     one process versus K separate processes, per-thread wall time): solo 28.7 s (thread) and 30.8 s (process);
     K=2 73.9 s versus 37.5 s; K=4 68.0 s versus 54.5 s; K=8 114.4 s versus 60.5 s. Putting threads in one process
     costs 1.97x / 1.25x / 1.89x on top of the hardware effect, and K=2 threads in one process finish less total work
     per second (0.78x) than one thread. Separate processes reach 1.64x / 2.26x / 4.07x.
   - `MallocNanoZone=0` did not change this consistently (mg4 depth 5: K=2 15.3 s to 12.7 s, K=4 12.0 s to 13.0 s,
     solo 5.8 s), so the cause is not specific to the nano zone. Whether it is the allocator or how the scheduler
     treats a multi-thread process is not established (hypothesis).
4. **The hot path allocates heavily (read from the code, not profiled).** `Board::isInCheckAfterMove` copies the
   whole Board, and the Board copy constructor clones every piece (up to 32 `new` and 32 `delete`). It runs for every
   pseudo-legal candidate in `MoveExecutor::generateMoves` (every interior node) and again inside
   `Game::performMove` (every make). `std::vector<Move>` is built per piece and per node, and captures clone the
   captured piece. That is about 1,000 or more allocations per interior node, which is why the search runs at only
   about 40k nodes per second.

Verdict: node inflation (2.6x to 6.8x) is the dominant cause of the sub-linear and negative scaling; in-process
slowdown (about 1.3x to 2x per thread) is second; imbalance is minor. Removing the in-process slowdown would mean
removing Board copies from the hot path (a Board representation change), which is outside this task.

### The fix

`AlphaBeta::search`, threads > 1 path only (the threads <= 1 path is unchanged): root move 0 is searched alone with the
full window to seed the best score, then workers take the remaining root moves from a shared atomic index and search
each with a window narrowed by the best score so far. Earlier-index moves use a window one point wider, so exact ties
are still detected and the earliest root move wins, matching the sequential search.

Checks after the fix (node counts are not timings, so they are valid even though the later timings are not):

- 1-thread node counts are unchanged: mg7 2,744,459 and mg4 1,407,833 at depth 6 as in C1, and op1, mg2 and eg5 equal
  C1's depth-5 and depth-6 counts. C1 used 1 thread, so it is unaffected.
- Best move and score are unchanged at 2, 4 and 8 threads: mg7 `43-21` -80, mg4 `64-74` -110.
- Node inflation at depth 6 dropped from 2.60x to 1.04x (2 threads), 1.06x (4 threads) and 1.09x (8 threads) on mg7
  (2,855,567 / 2,907,095 / 2,986,938 nodes), and from 6.84x to 2.51x at 8 threads on mg4 (3,528,930 nodes). Node
  counts now vary slightly between runs because the window a root move gets depends on when other workers finish.
- Tests: `ParallelSearchTest.MatchesSequentialMoveAndScore` compares move and score for 2, 4 and 8 threads against the
  sequential search on five positions, one with tied root scores (two mating rook moves). All 129 tests pass, and
  ThreadSanitizer reports no race in `ParallelSearchTest.*`.
- Kiwipete was dropped from that test: its sequential reference search with quiescence takes over a minute even at
  depth 2.
