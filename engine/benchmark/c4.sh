#!/bin/bash
# C4: Win At Chess solve rate at 100 ms and 1000 ms per move, 1 thread, one bench process per time control.
set -u
cd "$(dirname "$0")/../.." || exit 1

BENCH=build-bench/bin/bench
EPD=engine/benchmark/wac.epd
OUT=engine/benchmark/results

[ -x "$BENCH" ] || { echo "bench not executable: $BENCH" >&2; exit 1; }

# Timings on battery are not comparable with AC runs, so refuse to time.
pmset -g batt | head -1 | grep -q "AC Power" || { echo "not on AC power, refusing to time" >&2; exit 1; }
# A closed lid sleeps the machine, which would corrupt the timings.
ioreg -r -k AppleClamshellState | grep -q '"AppleClamshellState" = Yes' && { echo "lid is closed, the machine will sleep; refusing to time" >&2; exit 1; }
# keep the machine awake for the whole run; idle sleep freezes the benchmark and corrupts wall times
caffeinate -ims -w $$ &

mkdir -p "$OUT"

# Record machine state (battery, thermal and swap) before the runs for later interpretation of the timings.
date >&2
pmset -g batt 2>&1 | head -2 >&2
pmset -g therm >&2
sysctl vm.swapusage >&2

for ms in 100 1000; do "$BENCH" wac "$EPD" "$ms" "$OUT/c4_${ms}ms.csv" || { echo "bench wac failed at $ms ms" >&2; exit 1; }; done

date >&2
sysctl vm.swapusage >&2
