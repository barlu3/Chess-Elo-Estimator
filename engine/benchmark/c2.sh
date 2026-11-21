#!/bin/bash
# Thread-scaling benchmark: each (position, threads) run is a separate bench process.
set -u
cd "$(dirname "$0")/../.." || exit 1

BENCH=build-bench/bin/bench
EPD=engine/benchmark/positions.epd
OUT=${OUT:-engine/benchmark/results/c2_runs.csv}
SPLIT=${SPLIT:-engine/benchmark/results/c2_split.csv}
DEPTH=${DEPTH:-6}
REPS=${1:-7}
THREADS="1 2 4 8"

[ -x "$BENCH" ] || { echo "bench not executable: $BENCH" >&2; exit 1; }

IDS=$(awk -F';' '/^#/ || /^[[:space:]]*$/ {next} {gsub(/ /,"",$2); if (n++) printf " "; printf "%s", $2}' "$EPD")

# Create output directories up front so a long run does not fail at the split step.
mkdir -p "$(dirname "$OUT")" "$(dirname "$SPLIT")"
[ -e "$OUT" ] || echo "rep,id,threads,workers,depth,main_nodes,ms,best_move,score" > "$OUT"

# Record machine state (battery and thermal) for later interpretation of the timings.
date >&2
pmset -g batt 2>&1 | head -2 >&2 || true
pmset -g therm >&2 || true

# Interleave reps, positions and thread counts so thermal drift spreads evenly.
for rep in $(seq 1 "$REPS"); do
  # Alternate thread order so heat build-up does not always favour the same thread count.
  if [ $((rep % 2)) -eq 1 ]; then order=$THREADS; else order="8 4 2 1"; fi
  for id in $IDS; do
    for t in $order; do
      line=$("$BENCH" time "$EPD" "$id" "$DEPTH" "$t") || { echo "bench failed: $id t=$t" >&2; exit 1; }
      echo "$line" | awk -F, 'NF!=8{exit 1}' || { echo "bad output: $id t=$t: $line" >&2; exit 1; }
      echo "$rep,$line" >> "$OUT"
      echo "rep $rep/$REPS $id t=$t" >&2
    done
  done
done

# Node counts are deterministic, so they are collected once after timing and not timed.
echo "id,index,nodes" > "$SPLIT"
for id in $IDS; do
  "$BENCH" split "$EPD" "$id" "$DEPTH" >> "$SPLIT" || { echo "split failed: $id" >&2; exit 1; }
done

date >&2
