#!/usr/bin/env bash
set -euo pipefail

repository_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
build_dir="$(mktemp -d)"
trap 'rm -rf "$build_dir"' EXIT

cells=()
while IFS= read -r cell; do
  case "$cell" in
    ''|'#'*) continue ;;
  esac
  cells+=("$cell")
done < "$repository_dir/examples/wikipedia.txt"

for solver in sudoku-bt sudoku-mrv; do
  cc -O2 -std=c11 -o "$build_dir/$solver" "$repository_dir/$solver.c"
  "$build_dir/$solver" "${cells[@]}" > "$build_dir/$solver.out"
  perl -pe 's/\e\[[0-9;]*m//g' "$build_dir/$solver.out" > "$build_dir/$solver.clean"

  awk '
    /Solution (Found|found):/ { reading_solution = 1; next }
    reading_solution && /^\|/ {
      line = $0
      gsub(/[| ]/, "", line)
      print line
      if (++rows == 9) exit
    }
  ' "$build_dir/$solver.clean" > "$build_dir/$solver.solution"

  diff -u "$repository_dir/examples/wikipedia-solution.txt" "$build_dir/$solver.solution"
  printf '%s: example passed\n' "$solver"
done
