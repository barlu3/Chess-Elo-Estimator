#!/bin/bash
# Thread-scaling benchmark: each (position, threads) run is a separate bench process.
set -u
cd "$(dirname "$0")/../.." || exit 1

BENCH=build-bench/bin/bench
EPD=engine/benchmark/positions.epd
OUT=${OUT:-engine/benchmark/results/c2_runs.csv}
DEPTH=${DEPTH:-6}
REPS=${1:-7}
THREADS="1 2 4 8"

[ -x "$BENCH" ] || { echo "bench not executable: $BENCH" >&2; exit 1; }

# Timings on battery are not comparable with AC runs, so refuse to time.
pmset -g batt | head -1 | grep -q "AC Power" || { echo "not on AC power, refusing to time" >&2; exit 1; }
# A closed lid sleeps the machine, which would corrupt the timings.
ioreg -r -k AppleClamshellState | grep -q '"AppleClamshellState" = Yes' && { echo "lid is closed, the machine will sleep; refusing to time" >&2; exit 1; }
# keep the machine awake for the whole run; idle sleep freezes the benchmark and corrupts wall times
caffeinate -ims -w $$ &

IDS=$(awk -F';' '/^#/ || /^[[:space:]]*$/ {next} {gsub(/ /,"",$2); if (n++) printf " "; printf "%s", $2}' "$EPD")

# Create the output directory up front so a long run does not fail on its first write.
mkdir -p "$(dirname "$OUT")"
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

date >&2
