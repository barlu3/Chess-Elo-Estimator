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
