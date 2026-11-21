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

### C3: nodes per second

Main-search nodes per second (static-eval leaves, one node = one `alphabeta()` call), computed from existing data, no
new runs: `python3 -I engine/benchmark/c3_nps.py`.

**Depth 6, 1 thread (C1, one run per position):** median 59,387 NPS over the 20 positions (min 19,295, max 291,122);
pooled (sum of nodes / sum of seconds) 44,859. Excluding eg3 and eg4 (under 50 ms): median 56,531 (min 19,295, max 124,103),
pooled 44,840.

**Depth 5, 1/2/4/8 threads (C2):** per (position, threads) the median over 7 runs of `main_nodes / seconds`, then the
median over positions; pooled = sum of median nodes / sum of median seconds. Total across all workers, not per thread.

| threads | median NPS (20 positions) | pooled NPS (20) | median NPS (excl. under 50 ms) | pooled NPS (excl.) |
|---|---|---|---|---|
| 1 | 86,572 | 71,630 | 81,489 (18) | 71,570 |
| 2 | 79,677 | 68,218 | 71,375 (18) | 68,167 |
| 4 | 141,316 | 131,629 | 136,085 (18) | 131,562 |
| 8 | 161,869 | 154,592 | 150,214 (17) | 154,045 |

Exclusion: a position is dropped from a row if its median time at that row's thread count is under 50 ms. That is eg3
(4 to 6 ms) and eg4 (1 to 2 ms) at every thread count, plus eg1 at 8 threads only (47.0 ms; its other cells are 54 to
113 ms), hence 17 positions in the last row. The pooled figures barely move because the pooled sum is dominated by the
heavy positions. The pooled ratios to 1 thread (0.95x, 1.84x, 2.16x) are the "pooled throughput vs 1T" column of C2.

**Cross-check against C1 (depth 5, 1 thread):** C1's single runs give a median of 81,803 NPS and a pooled 63,675. C2's
median of 7 is 5.8% higher on median NPS (11.7% excluding eg3 and eg4) and 12.5% higher pooled; the per-position ratio has
a median of 1.03 (range 0.94 to 1.66). The node counts are identical (checked in the script), so the gap is time: C1's
single runs were slower on mg3 (1.66x), mg7 (1.31x), mg4 (1.25x) and mg5 (1.24x), which dominate the pooled figure.
So the two agree to 3% per position at the median and 12% pooled; C1 is one run per cell, so the gap is probably
run-to-run noise (not tested).

Context: these count main-search nodes with static-eval leaves (quiescence off); the real engine with quiescence
visits more nodes per search, so its NPS differs. The low absolute speed (tens of thousands of nodes per second) is
attributed to the per-candidate `Board` copies in move generation (read from the code, not profiled; see point 4 of the
C2 diagnosis). Multi-thread NPS is the total over all workers, not per thread: 8 threads reach 2.2x the 1-thread pooled
NPS, about 0.27x per thread.

## C2: how does the search scale across threads?

**Metric: wall-clock speedup of one fixed-depth `AlphaBeta::search` on N threads versus 1 thread, depth 5, 20 positions.**
Configuration: ab+mvvlva (`pruning = true`, `ordering = true`) with static-eval leaves (`quiescence = false`), the same
as C1, on the same 20 positions (`positions.epd`). Every (position, thread count) is a separate process, 7 repetitions
each, 560 runs. Speedup = median 1-thread ms / median N-thread ms over the 7 repetitions. This is the search after the
parallel-root fix (commit `9832a84`); the original parallel root and why it was replaced are described under
"Before the fix" below.

**Depth 5, not depth 6.** The C1 depth-6 single-thread times add up to 237 s over the 20 positions. With the per-round
time ratios seen in the before-fix gate and in the identical-work thread test below (about 3.5 to 5 single-thread
sums per round), 7 rounds at depth 6 would take roughly 1.6 to 2.3 h, over the 1.5 h budget. The whole depth-5 run
took 10 min (single-thread sum 25.4 s). A firm after-fix depth-6 projection was not possible: the machine slept
(lid closed) during that attempt, so those timings were discarded.

### Setup

| | |
|---|---|
| Measured at commit | `aa7f76a` (clean tree; the result files were added afterwards) |
| Parallel root fix | commit `9832a84` |
| Run | 2026-11-20, 22:40:29 to 22:50:37 PST, 560 runs, `DEPTH=5 engine/benchmark/c2.sh 7` |
| CPU | Apple M5, 10 cores (4 performance + 6 efficiency); `hardware_concurrency()` = 10 |
| Workers | `min(threads, 10, number of root moves)`; the "workers" column below shows the effective count |
| Compiler and flags | Apple clang 21.0.0 (clang-2100.1.1.101), `-O3 -DNDEBUG` (CMake `Release`) |
| macOS | 26.6.2 (25G83) |
| Machine state | AC power (100% charged), lid open, no thermal warning recorded. About 17 GB of 17 to 18 GB swap was in use (other apps), and system daemons (`mobileassetd`, `modelcatalogd`, `WindowServer`) were active. Nothing else of mine ran during the timing. |
| Canary (mg4, depth 5, 1 thread) | 5.96, 6.27, 5.74, 5.82 s before the run; 5.73 s after (5.8 s earlier the same day) |
| Order | rep, then position, then thread count; the thread order alternates `1 2 4 8` and `8 4 2 1` on odd and even reps |
| Noise | (max - min) / median over the 7 reps, cells of at least 100 ms: median 10.6% at 1T, 26.5% at 2T, 20.9% at 4T, 19.6% at 8T; worst 137.5% (8T) |

### Same-move check

same-move check: OK (20 positions, 560 runs, identical best move and score across 1/2/4/8 threads and all repetitions)

Best move and score are identical for every position across 1, 2, 4 and 8 threads and all 7 repetitions: no
mismatch. (Tie-breaking on the earliest root move is covered by `ParallelSearchTest.MatchesSequentialMoveAndScore`.)

- repetitions per (position, thread count): min 7, max 7
- main-node count spread across repetitions (threads>1): max 11.40% (eg6, 8T)

Node counts for 2, 4 and 8 threads vary a little between runs because the window a root move gets depends on when
other workers finish; the tables use medians. The spread above is the largest across the 7 repetitions.

### Per-position results

Median wall ms over 7 repetitions, speedup against the 1-thread median, and the effective worker count at 2, 4 and 8
threads (a position with few root moves cannot use all threads):

| id | workers (2/4/8 threads) | median ms: 1T | 2T | 4T | 8T | speedup: 2T | 4T | 8T |
|---|---|---|---|---|---|---|---|---|
| op1 | 2/4/8 | 477 | 565 | 424 | 373 | 0.84 | 1.13 | 1.28 |
| op2 | 2/4/8 | 503 | 479 | 434 | 403 | 1.05 | 1.16 | 1.25 |
| op3 | 2/4/8 | 914 | 1142 | 876 | 974 | 0.80 | 1.04 | 0.94 |
| op4 | 2/4/8 | 3499 | 3750 | 2166 | 2489 | 0.93 | 1.62 | 1.41 |
| op5 | 2/4/8 | 5719 | 6327 | 3665 | 3996 | 0.90 | 1.56 | 1.43 |
| op6 | 2/4/8 | 876 | 887 | 674 | 618 | 0.99 | 1.30 | 1.42 |
| mg1 | 2/4/8 | 1121 | 1270 | 613 | 497 | 0.88 | 1.83 | 2.26 |
| mg2 | 2/4/5 | 290 | 425 | 395 | 388 | 0.68 | 0.73 | 0.75 |
| mg3 | 2/4/8 | 738 | 984 | 464 | 412 | 0.75 | 1.59 | 1.79 |
| mg4 | 2/4/8 | 5890 | 6043 | 3054 | 2456 | 0.97 | 1.93 | 2.40 |
| mg5 | 2/4/8 | 993 | 1202 | 566 | 468 | 0.83 | 1.75 | 2.12 |
| mg6 | 2/4/8 | 575 | 667 | 569 | 581 | 0.86 | 1.01 | 0.99 |
| mg7 | 2/4/8 | 2804 | 3432 | 1697 | 1431 | 0.82 | 1.65 | 1.96 |
| eg1 | 2/4/8 | 95 | 113 | 55 | 47 | 0.84 | 1.74 | 2.03 |
| eg2 | 2/4/8 | 69 | 130 | 100 | 58 | 0.53 | 0.69 | 1.18 |
| eg3 | 2/4/4 | 4 | 6 | 5 | 5 | 0.79 | 0.91 | 0.97 |
| eg4 | 2/3/3 | 1 | 2 | 2 | 2 | 0.78 | 0.80 | 0.79 |
| eg5 | 2/4/8 | 567 | 667 | 377 | 322 | 0.85 | 1.51 | 1.76 |
| eg6 | 2/4/8 | 95 | 136 | 75 | 78 | 0.70 | 1.28 | 1.23 |
| eg7 | 2/4/8 | 145 | 178 | 107 | 138 | 0.82 | 1.36 | 1.05 |

Node inflation = median N-thread main nodes / median 1-thread main nodes (the 1-thread counts equal C1's depth-5
`ab+mvvlva` counts):

| id | main nodes 1T | node inflation 2T | 4T | 8T |
|---|---|---|---|---|
| op1 | 51078 | 1.23 | 1.35 | 1.73 |
| op2 | 46534 | 1.00 | 1.00 | 1.00 |
| op3 | 57447 | 1.26 | 1.59 | 2.01 |
| op4 | 234236 | 1.14 | 1.34 | 1.60 |
| op5 | 412015 | 1.09 | 1.27 | 1.59 |
| op6 | 77920 | 1.00 | 1.00 | 1.00 |
| mg1 | 73672 | 1.00 | 1.00 | 1.00 |
| mg2 | 12837 | 1.20 | 1.79 | 1.79 |
| mg3 | 62199 | 1.00 | 1.00 | 1.06 |
| mg4 | 365714 | 1.00 | 1.00 | 1.00 |
| mg5 | 78165 | 1.00 | 1.00 | 1.00 |
| mg6 | 44328 | 1.00 | 1.38 | 1.74 |
| mg7 | 170285 | 1.05 | 1.13 | 1.17 |
| eg1 | 14857 | 1.00 | 1.00 | 1.00 |
| eg2 | 9995 | 1.23 | 1.72 | 1.84 |
| eg3 | 1554 | 1.00 | 1.00 | 1.00 |
| eg4 | 375 | 1.00 | 1.00 | 1.00 |
| eg5 | 70272 | 1.06 | 1.28 | 1.36 |
| eg6 | 14831 | 1.22 | 1.43 | 1.85 |
| eg7 | 19537 | 1.09 | 1.20 | 1.81 |

### Aggregates and speedup curve

| threads | median speedup | geo-mean speedup | pooled speedup | median node inflation | pooled node inflation | pooled speedup (min ms) | pooled throughput vs 1T |
|---|---|---|---|---|---|---|---|
| 1 | 1.00 | 1.00 | 1.00 | 1.00 | 1.00 | 1.00 | 1.00 |
| 2 | 0.83 | 0.82 | 0.89 | 1.02 | 1.07 | 0.95 | 0.95 |
| 4 | 1.33 | 1.27 | 1.56 | 1.16 | 1.18 | 1.82 | 1.84 |
| 8 | 1.34 | 1.37 | 1.61 | 1.27 | 1.34 | 1.74 | 2.16 |

Median and geometric mean are over the 20 per-position speedups; pooled = sum of the 1-thread medians / sum of the
N-thread medians (it is weighted by the heavy positions: mg4 5.9 s, op5 5.7 s, op4 3.5 s and mg7 2.8 s of the 25.4 s
total). "Pooled speedup (min ms)" repeats the pooled figure with the fastest of the 7 repetitions per cell, as a
check against interference. "Pooled throughput vs 1T" is main nodes per second summed over positions, relative to 1 thread.

| threads | ideal | pooled speedup | efficiency (pooled/threads) |
|---|---|---|---|
| 1 | 1.00 | 1.00 | 1.00 |
| 2 | 2.00 | 0.89 | 0.45 |
| 4 | 4.00 | 1.56 | 0.39 |
| 8 | 8.00 | 1.61 | 0.20 |

```
1T |#### 1.00x (ideal 1x)
2T |#### 0.89x (ideal 2x)
4T |###### 1.56x (ideal 4x)
8T |###### 1.61x (ideal 8x)
```

Reading the curve:

- 2 threads is slower than 1 thread on 19 of 20 positions (pooled 0.89x); only op2 is faster (1.05x).
- 4 threads: pooled 1.56x, median position 1.33x, geometric mean 1.27x; 4 of 20 positions are below 1x and 9 are at
  or above 1.5x (best mg4 1.93x).
- 8 threads: pooled 1.61x, median 1.34x, geometric mean 1.37x; 5 of 20 are below 1x (best mg4 2.40x). 4 and 8 threads
  are not distinguishable at this noise level: with the fastest repetition per cell the pooled speedup is 1.82x at 4T
  and 1.74x at 8T.
- Speedup is throughput divided by node inflation, to a good approximation: 0.95 / 1.07 = 0.89, 1.84 / 1.18 = 1.56,
  2.16 / 1.34 = 1.61.

### Why the curve is far below linear

Decomposition, pooled over the 20 positions. The node model asks: if the only losses were node inflation, the serial
seed move (root move 0, searched alone) and an even split of the rest over the effective workers, at equal speed per
node, how fast would the search be? Per position, critical path = seed nodes + (parallel-search nodes - seed nodes) /
workers, and the bound is 1-thread nodes / critical path, summed over positions. Seed nodes come from
`bench split engine/benchmark/positions.epd <id> 5` (full-window size of root move 0).

| threads | pooled node inflation | node-model bound | measured pooled speedup | residual (measured / bound) |
|---|---|---|---|---|
| 2 | 1.07 | 1.77 | 0.89 | 0.50 |
| 4 | 1.18 | 2.93 | 1.56 | 0.53 |
| 8 | 1.34 | 4.51 | 1.61 | 0.36 |

(a) **Root moves cannot share a full window with their siblings.** A worker that starts a root move before a good
score is known searches with a wide window and visits more nodes than the sequential search, which narrows alpha
as it goes. After the fix this costs 1.07x / 1.18x / 1.34x pooled at 2 / 4 / 8 threads (median position 1.02x / 1.16x /
1.27x, worst 2.01x on op3 at 8T), and 8 of 20 positions show no inflation even at 8 threads. Before the fix it was
2.60x on mg7 and 6.84x on mg4 at depth 6, which made two threads slower than one. It grows with the thread count
because more workers start a root move before the bound arrives. Inflation alone explains a loss of at most 1.34x at 8
threads, so it is not the main limit any more.

(b) **Uneven subtree sizes and the serial seed.** The original static round-robin split was measured to be only mildly
unbalanced (busiest worker over mean 1.10 / 1.19 / 1.29 at 2 / 4 / 8 workers, see the diagnosis), and the fix replaced
it with a shared counter, so what is left is granularity and the serial seed. The seed (one thread, full window) is
6.1% of the sequential nodes pooled, which costs little overall, but it is 33% to 37% on op2, op6, mg2 and eg4 (25% on
eg3), which caps those positions near 3x however many workers there are. The largest single root subtree (a full-window
upper bound from `bench split`) is a median 21% of the parallel node count, range 5% to 66%, which is more than an
even 1/8 share (12.5%) on 15 of 20 positions, so one root move can keep a worker busy while the others have finished.
The node model includes the seed but not this granularity, and per-worker idle time after the fix was not
instrumented, so how much of the residual is imbalance is not measured (hypothesis).

(c) **8 threads is more than the 4 performance cores.** Going from 4 to 8 threads adds only 3% to the pooled speedup
(1.56x to 1.61x; 1.82x to 1.74x by fastest repetition) and 17% to the pooled throughput (1.84x to 2.16x), although the
thread count doubles. The hardware-only measurement from the diagnosis agrees: N separate single-thread processes of
mg7 at depth 6 reached an aggregate 1.9x (N=2), 2.9x (N=4) and 3.0x (N=8) of one process, with each of the 8 running
at 0.37x of solo speed. Beyond the 4 performance cores the 6 efficiency cores add very little for this search.

(d) **Per-node speed drops when threads share a process (allocator or scheduler, unproven).** The residual of
0.50 / 0.53 / 0.36 is close to what the identical-work test in the diagnosis found for K threads in one process
(mg4, depth 6: aggregate 0.78x / 1.69x / 2.0x of one thread, that is 0.39 / 0.42 / 0.25 per thread for K = 2 / 4 / 8),
and below separate processes (0.82 / 0.57 / 0.51 per process). In other words two threads in one process do less
total work per second than one thread in the sequential path (measured pooled throughput 0.95x at 2 threads). The
hot path allocates heavily (every candidate move copies the whole Board, see the diagnosis), so allocator contention
is the leading hypothesis; `MallocNanoZone=0` made no consistent difference, and how the macOS scheduler treats a
multi-thread process is the alternative. Removing the Board copies from move generation would test it; that is a
Board representation change and was not attempted.

### After the fix versus before (indicative; the depths differ)

| position | before the fix (depth 6, one run) | after the fix (depth 5, median of 7) |
|---|---|---|
| mg7 speedup at 2 / 4 / 8 threads | 0.39 / 0.80 / 1.29 | 0.82 / 1.65 / 1.96 |
| mg7 node inflation | 2.60 / 2.60 / 2.60 | 1.05 / 1.13 / 1.17 (depth 6: 1.04 / 1.06 / 1.09) |
| mg4 speedup at 8 threads | 0.38 | 2.40 (2 / 4 threads: 0.97 / 1.93) |
| mg4 node inflation at 8 threads | 6.84 | 1.00 (depth 6: 2.51) |

The depth-6 after-fix node counts are exact counts from single runs; no valid after-fix depth-6 timing exists.

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
     per second (0.78x) than one thread. Separate processes reach 1.64x / 2.26x / 4.07x. (This test used a throwaway `bench thr` subcommand that is not committed: K `std::thread`s each
     running `AlphaBeta::search(..., 1)` on the same position.)
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

### Bullet

Reached 1.56× speedup on 4 threads at fixed depth 5 while returning the same move by **...**

Suggested phrasing for the missing clause: "by searching the first root move alone, then handing the other root
moves to workers one at a time with a window narrowed by the best score so far (same best move and score on all 20
positions, 560 runs)".

Qualifiers to keep next to the number: 1.56x is the pooled speedup over 20 positions (median position 1.33x), 2 threads is
slower than 1 thread (0.89x), and 8 threads is no better than 4 (1.61x). Depth 5 rather than 6, see the top of this
section.

### Caveats

- Depth 5, not C1's depth 6; the depth-6 after-fix timing could not be taken (the machine slept).
- Multi-thread cells are noisy (median 20% to 27% spread between the 7 repetitions, worst 137%) because the machine
  carried about 17 GB of swap and background daemons; medians of 7 and a fastest-repetition check are reported, and
  they differ by 0.1x to 0.3x at 4 and 8 threads.
- The pooled figures are dominated by four heavy positions (mg4, op5, op4, mg7); eg3 and eg4 take 1 to 5 ms and are
  dominated by thread start-up.
- Per-worker idle time and the cause of the in-process slowdown were not instrumented, so (b) after the fix and (d) are
  hypotheses backed by the numbers above, not proofs.
- Node counts for 2, 4 and 8 threads are not deterministic any more (largest spread across repetitions 11.4%, eg6 at
  8 threads); the sequential count and the returned move and score are.

### Files and reproduce

- `results/c2_runs.csv`: every run, `rep,id,threads,workers,depth,main_nodes,ms,best_move,score` (560 rows; C3 can
  compute nodes per second as `main_nodes / ms`).
- `results/c2_summary.csv`: per (position, thread count) medians, min and max ms, median nodes, speedup and node
  inflation.

From the repository root (about 10 min at depth 5, about 2 h at depth 6; needs AC power and an open lid, `c2.sh`
refuses to run otherwise and keeps the machine awake; it appends to `c2_runs.csv`, so delete that file first for a
fresh run; do not compile or run anything else while it runs):

```
cmake -S test -B build-bench -DCMAKE_BUILD_TYPE=Release && cmake --build build-bench --target bench -j8 && DEPTH=5 engine/benchmark/c2.sh && python3 -I engine/benchmark/c2_summary.py
```

`c2_summary.py` prints the tables above, the same-move check (exit status 1 on a mismatch) and writes
`c2_summary.csv`. The seed-move node counts in (b) come from `bench split engine/benchmark/positions.epd <id> 5` for
each id in `positions.epd`.
