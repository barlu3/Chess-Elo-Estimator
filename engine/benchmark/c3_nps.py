#!/usr/bin/env python3
"""C3: main-search nodes per second from c1.csv and c2_runs.csv (stdlib only; run from repo root:
python3 -I engine/benchmark/c3_nps.py)."""
import csv
import statistics as st
import sys
from collections import defaultdict

C1 = "engine/benchmark/results/c1.csv"
C2 = "engine/benchmark/results/c2_runs.csv"
MIN_MS = 50  # a cell whose (median) time is under this is dominated by thread startup

with open(C1, newline="") as f:
    c1 = {r["id"]: r for r in csv.DictReader(f)}
cells = defaultdict(list)  # (id, threads) -> [(nodes, ms)]
with open(C2, newline="") as f:
    for r in csv.DictReader(f):
        cells[(r["id"], int(r["threads"]))].append((int(r["main_nodes"]), float(r["ms"])))

ids = list(c1)
if len(ids) != 20 or any(len(cells[(i, t)]) != 7 for i in ids for t in (1, 2, 4, 8)):
    sys.exit("CHECK failed: expected 20 positions x 4 thread counts x 7 reps")
# The 1-thread search is deterministic, so c2's node counts must equal c1's depth-5 counts.
if any(st.median(n for n, _ in cells[(i, 1)]) != int(c1[i]["mvv5_main"]) for i in ids):
    sys.exit("CHECK failed: c2 1-thread nodes differ from c1 mvv5_main")


# label -> {id: (nps, nodes, seconds)}; c1 rows are single runs, c2 rows take the median of 7 runs
rows = {
    "depth 6, 1T (c1, one run)": {i: (int(c1[i]["mvv6_main"]) / (float(c1[i]["mvv6_ms"]) / 1000),
                                      int(c1[i]["mvv6_main"]), float(c1[i]["mvv6_ms"]) / 1000) for i in ids},
    "depth 5, 1T (c1, one run)": {i: (int(c1[i]["mvv5_main"]) / (float(c1[i]["mvv5_ms"]) / 1000),
                                      int(c1[i]["mvv5_main"]), float(c1[i]["mvv5_ms"]) / 1000) for i in ids},
}
for t in (1, 2, 4, 8):
    rows[f"depth 5, {t}T (c2, median of 7)"] = {
        i: (st.median(n / (ms / 1000) for n, ms in cells[(i, t)]),
            st.median(n for n, _ in cells[(i, t)]), st.median(ms for _, ms in cells[(i, t)]) / 1000)
        for i in ids}

summary = {}
for title, skip in (("all 20 positions", 0), (f"excluding cells under {MIN_MS} ms", MIN_MS)):
    print(f"{title}\n\n| run | positions | median NPS | min - max NPS | pooled NPS |\n|---|---|---|---|---|")
    for label, d in rows.items():
        keep = {i: v for i, v in d.items() if v[2] * 1000 >= skip}
        nps = [v[0] for v in keep.values()]
        pooled = sum(v[1] for v in keep.values()) / sum(v[2] for v in keep.values())
        summary[(label, skip)] = (st.median(nps), pooled)
        gone = ", ".join(sorted(set(d) - set(keep))) or "none"
        print(f"| {label} | {len(keep)} (excluded: {gone}) | {st.median(nps):,.0f} | "
              f"{min(nps):,.0f} - {max(nps):,.0f} | {pooled:,.0f} |")
    print()

a, b = "depth 5, 1T (c1, one run)", "depth 5, 1T (c2, median of 7)"
for skip in (0, MIN_MS):
    (m1, p1), (m2, p2) = summary[(a, skip)], summary[(b, skip)]
    ratio = [rows[b][i][0] / rows[a][i][0] for i in ids if rows[a][i][2] * 1000 >= skip and rows[b][i][2] * 1000 >= skip]
    print(f"cross-check ({'all' if not skip else f'>= {MIN_MS} ms'}): c2 / c1 1T depth 5 = "
          f"{m2 / m1:.3f} (median NPS), {p2 / p1:.3f} (pooled); per position median {st.median(ratio):.3f}, "
          f"range {min(ratio):.3f} - {max(ratio):.3f}")
