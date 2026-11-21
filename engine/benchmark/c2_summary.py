#!/usr/bin/env python3
"""Summarize the C2 thread-scaling benchmark (stdlib only; run from repo root)."""
import csv
import math
import os
import statistics
import sys
from collections import defaultdict

RUNS = sys.argv[1] if len(sys.argv) > 1 else "engine/benchmark/results/c2_runs.csv"
SUMMARY = sys.argv[2] if len(sys.argv) > 2 else "engine/benchmark/results/c2_summary.csv"
SPLIT = sys.argv[3] if len(sys.argv) > 3 else "engine/benchmark/results/c2_split.csv"
THREADS = [1, 2, 4, 8]
PARALLEL = [2, 4, 8]


def check(cond, msg):
    # Explicit exit rather than assert so the checks survive python -O.
    if not cond:
        sys.stderr.write("CHECK failed: " + msg + "\n")
        sys.exit(2)


def table(header, rows):
    print("| " + " | ".join(header) + " |")
    print("|" + "|".join("---" for _ in header) + "|")
    for r in rows:
        print("| " + " | ".join(str(c) for c in r) + " |")
    print()


def geo(xs):
    return math.exp(sum(math.log(x) for x in xs) / len(xs))


# ---- load runs ----
with open(RUNS, newline="") as f:
    reader = csv.DictReader(f)
    need = ["rep", "id", "threads", "workers", "depth", "main_nodes", "ms", "best_move", "score"]
    check(reader.fieldnames is not None and all(c in reader.fieldnames for c in need),
          f"{RUNS}: header must contain {','.join(need)}")
    runs = []
    for row in reader:
        t = int(row["threads"])
        check(t in THREADS, f"unexpected threads={t} for id {row['id']}")
        runs.append({"id": row["id"], "threads": t, "workers": int(row["workers"]),
                     "ms": float(row["ms"]), "nodes": int(row["main_nodes"]),
                     "move": row["best_move"], "score": int(row["score"])})
check(runs, f"no runs in {RUNS}")

order = []  # ids in first-appearance order
cells = defaultdict(list)  # (id, threads) -> runs
for r in runs:
    if r["id"] not in order:
        order.append(r["id"])
    cells[(r["id"], r["threads"])].append(r)
for pid in order:
    for t in THREADS:
        check((pid, t) in cells, f"id {pid} has no runs at threads={t}")

# ---- A: per-cell stats and summary CSV ----
stats = {}
for key, rs in cells.items():
    ws = {r["workers"] for r in rs}
    check(len(ws) == 1, f"workers differ across runs of {key}: {sorted(ws)}")
    stats[key] = {"workers": ws.pop(), "runs": len(rs),
                  "median_ms": statistics.median(r["ms"] for r in rs),
                  "min_ms": min(r["ms"] for r in rs),
                  "max_ms": max(r["ms"] for r in rs),
                  "median_nodes": statistics.median(r["nodes"] for r in rs)}
for (pid, t), s in stats.items():
    base = stats[(pid, 1)]
    s["speedup"] = base["median_ms"] / s["median_ms"]
    s["inflation"] = s["median_nodes"] / base["median_nodes"]

with open(SUMMARY, "w", newline="") as f:
    w = csv.writer(f)
    w.writerow(["id", "threads", "workers", "runs", "median_ms", "min_ms", "max_ms",
                "median_main_nodes", "speedup", "node_inflation"])
    for pid in order:
        for t in THREADS:
            s = stats[(pid, t)]
            w.writerow([pid, t, s["workers"], s["runs"], f"{s['median_ms']:.3f}",
                        f"{s['min_ms']:.3f}", f"{s['max_ms']:.3f}",
                        f"{s['median_nodes']:.0f}", f"{s['speedup']:.4f}",
                        f"{s['inflation']:.4f}"])

# ---- B: same-move check and repetition/determinism facts ----
pairs = defaultdict(lambda: defaultdict(int))  # id -> (threads, move, score) -> runs
for r in runs:
    pairs[r["id"]][(r["threads"], r["move"], r["score"])] += 1
bad_move = [pid for pid in order if len({(m, s) for (_, m, s) in pairs[pid]}) != 1]
if not bad_move:
    print(f"same-move check: OK ({len(order)} positions, {len(runs)} runs, identical best "
          "move and score across 1/2/4/8 threads and all repetitions)\n")
else:
    print("same-move check: MISMATCH\n")
    table(["id", "threads", "best_move", "score", "runs"],
          [(pid, t, m, s, n) for pid in bad_move for (t, m, s), n in sorted(pairs[pid].items())])

counts = [s["runs"] for s in stats.values()]
print(f"repetitions per (position, thread count): min {min(counts)}, max {max(counts)}")
parallel_nodes = defaultdict(set)
for r in runs:
    if r["threads"] > 1:
        parallel_nodes[r["id"]].add(r["nodes"])
bad_nodes = [pid for pid in order if len(parallel_nodes[pid]) != 1]
if bad_nodes:
    print("main-node counts identical across all runs with threads>1 for the same id: "
          "no (ids: " + ", ".join(bad_nodes) + ")\n")
else:
    print("main-node counts identical across all runs with threads>1 for the same id: yes\n")

# ---- C: per-position tables ----
table(["id", "workers (2/4/8 threads)", "median ms: 1T", "2T", "4T", "8T",
       "speedup: 2T", "4T", "8T"],
      [[pid, "/".join(str(stats[(pid, t)]["workers"]) for t in PARALLEL)]
       + [f"{stats[(pid, t)]['median_ms']:.0f}" for t in THREADS]
       + [f"{stats[(pid, t)]['speedup']:.2f}" for t in PARALLEL] for pid in order])
table(["id", "main nodes 1T", "node inflation 2T", "4T", "8T"],
      [[pid, f"{stats[(pid, 1)]['median_nodes']:.0f}"]
       + [f"{stats[(pid, t)]['inflation']:.2f}" for t in PARALLEL] for pid in order])

# ---- D: aggregates per thread count ----
pooled = {}  # threads -> pooled speedup
rows = []
for t in THREADS:
    sp = [stats[(pid, t)]["speedup"] for pid in order]
    inf = [stats[(pid, t)]["inflation"] for pid in order]
    pooled[t] = (sum(stats[(pid, 1)]["median_ms"] for pid in order)
                 / sum(stats[(pid, t)]["median_ms"] for pid in order))
    pooled_inf = (sum(stats[(pid, t)]["median_nodes"] for pid in order)
                  / sum(stats[(pid, 1)]["median_nodes"] for pid in order))
    rows.append([t, f"{statistics.median(sp):.2f}", f"{geo(sp):.2f}", f"{pooled[t]:.2f}",
                 f"{statistics.median(inf):.2f}", f"{pooled_inf:.2f}"])
table(["threads", "median speedup", "geo-mean speedup", "pooled speedup",
       "median node inflation", "pooled node inflation"], rows)

# ---- E: speedup curve ----
table(["threads", "ideal", "pooled speedup", "efficiency (pooled/threads)"],
      [[t, f"{t:.2f}", f"{pooled[t]:.2f}", f"{pooled[t] / t:.2f}"] for t in THREADS])
print("```")
for t in THREADS:
    print(f"{t}T |{'#' * round(pooled[t] / 0.25)} {pooled[t]:.2f}x (ideal {t}x)")
print("```\n")

# ---- F: imbalance (only when the split file exists) ----
if not os.path.exists(SPLIT):
    print(f"imbalance section skipped: {SPLIT} not found\n")
else:
    with open(SPLIT, newline="") as f:
        reader = csv.DictReader(f)
        check(reader.fieldnames is not None and all(c in reader.fieldnames
                                                    for c in ["id", "index", "nodes"]),
              f"{SPLIT}: header must contain id,index,nodes")
        split = defaultdict(dict)  # id -> root-move index -> subtree nodes
        for row in reader:
            i = int(row["index"])
            check(i not in split[row["id"]], f"duplicate index {i} for id {row['id']}")
            split[row["id"]][i] = int(row["nodes"])
    for pid in order:
        check(split.get(pid), f"no split rows for id {pid}")

    imb = {}  # (id, threads) -> imbalance numbers for that cell
    for pid in order:
        for n in PARALLEL:
            w_eff = stats[(pid, n)]["workers"]
            loads = [0] * w_eff
            for i, nodes in split[pid].items():
                loads[i % w_eff] += nodes  # worker k gets root moves with index % w == k
            total = sum(split[pid].values())
            if w_eff > 1:
                check(total == stats[(pid, n)]["median_nodes"],
                      f"{pid} threads={n}: split total {total} != median main_nodes "
                      f"{stats[(pid, n)]['median_nodes']}")
            makespan = max(loads)
            imb[(pid, n)] = {"total": total, "makespan": makespan, "w": w_eff,
                             "balance": total / (w_eff * makespan),
                             "bound": stats[(pid, 1)]["median_nodes"] / makespan}

    rows = []
    for pid in order:
        row = [pid, "/".join(str(stats[(pid, n)]["workers"]) for n in PARALLEL)]
        for n in PARALLEL:
            d = imb[(pid, n)]
            row += [f"{d['balance']:.2f}", f"{d['bound']:.2f}",
                    f"{stats[(pid, n)]['speedup']:.2f}"]
        rows.append(row)
    table(["id", "workers (2/4/8 threads)", "2T balance", "2T node-bound speedup",
           "2T measured speedup", "4T balance", "4T node-bound speedup",
           "4T measured speedup", "8T balance", "8T node-bound speedup",
           "8T measured speedup"], rows)

    rows = []
    for n in PARALLEL:
        infl = (sum(stats[(pid, n)]["median_nodes"] for pid in order)
                / sum(stats[(pid, 1)]["median_nodes"] for pid in order))
        bal = (sum(imb[(pid, n)]["total"] for pid in order)
               / sum(imb[(pid, n)]["w"] * imb[(pid, n)]["makespan"] for pid in order))
        bound = (sum(stats[(pid, 1)]["median_nodes"] for pid in order)
                 / sum(imb[(pid, n)]["makespan"] for pid in order))
        rows.append([n, f"{infl:.2f}", f"{bal:.2f}", f"{bound:.2f}", f"{pooled[n]:.2f}",
                     f"{pooled[n] / bound:.2f}"])
    table(["threads", "pooled node inflation", "pooled balance", "pooled node-bound speedup",
           "pooled measured speedup", "residual (measured / node-bound)"], rows)

print(f"wrote {SUMMARY}")
sys.exit(1 if bad_move else 0)
