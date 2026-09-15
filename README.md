# Sudoku Solver Implementations

**Author:** Sitan Chen

This repository compares two C implementations for solving a 9 × 9 Sudoku. Both programs accept the same puzzle input and print the first valid solution. The difference is the order in which they explore the search tree.

## Repository layout

- [`sudoku-bt.c`](sudoku-bt.c) — the baseline recursive backtracking solver.
- [`sudoku-mrv.c`](sudoku-mrv.c) — a solver using candidate domains and the Minimum Remaining Values (MRV) heuristic.
- [`examples/wikipedia.txt`](examples/wikipedia.txt) — a complete example puzzle.
- [`examples/wikipedia-solution.txt`](examples/wikipedia-solution.txt) — the expected solution for the example.
- [`scripts/test-example.sh`](scripts/test-example.sh) — compiles both solvers and verifies the example output.

## Research paper

[Solving Sudoku: From Generic Backtracking to CSP Optimizations](paper/solving-sudoku-backtracking-to-csp-optimizations.pdf) is the six-page research paper accompanying this repository. It documents the original methodology, algorithm diagrams, and an earlier experimental evaluation.


## Build and run

Compile with GCC or Clang:

```bash
gcc -O2 -std=c11 -o sudoku-bt sudoku-bt.c
gcc -O2 -std=c11 -o sudoku-mrv sudoku-mrv.c
```

Each known digit is a separate three-digit argument in the form `row-column-number`, using 1-based positions. For example, `115` places digit 5 at row 1, column 1.

Run both solvers on the included complete example:

```bash
./sudoku-bt $(grep -v '^#' examples/wikipedia.txt)
./sudoku-mrv $(grep -v '^#' examples/wikipedia.txt)
```

To compile and check both implementations automatically:

```bash
./scripts/test-example.sh
```

## 1. Baseline solver: `sudoku-bt.c`

The baseline uses depth-first search with recursive backtracking.

1. `find_empty()` scans the board from top-left to bottom-right and returns the first cell containing `0`.
2. `solve()` tries digits 1 through 9 in that cell.
3. `is_valid()` rejects a digit that already appears in the same row, column, or 3 × 3 box.
4. For a legal digit, the program writes the value and recursively solves the remaining empty cells.
5. If the recursive branch fails, the value is reset to `0` and the next digit is tried.
6. If every digit fails, `solve()` returns `false`, causing the preceding recursive call to backtrack.

This implementation is intentionally direct and serves as the reference solver. Its weakness is its fixed cell order: it may make several guesses in a loosely constrained region before reaching a contradiction elsewhere. If there are `E` empty cells, the naïve search space has a worst-case upper bound of `O(9^E)`, though Sudoku constraints prune many branches in practice.

## 2. MRV solver: `sudoku-mrv.c`

The MRV version stores both a cell value and a set of legal candidates:

```c
typedef struct {
    int value;       // 0 means the cell is empty
    int candidates;  // a bit mask for the possible digits 1 through 9
} Cell;
```

`ALL_CANDIDATES` is `0x1ff`: its lowest nine bits are all set. Bit 0 represents digit 1, bit 1 represents digit 2, and bit 8 represents digit 9. If digit 5 is impossible in a cell, the solver clears bit 4.

### Candidate-domain pruning

`update_candidates()` recomputes the domain of every empty cell from the current board:

1. Start with every digit from 1 through 9 enabled.
2. Clear digits already used in the cell's row.
3. Clear digits already used in its column.
4. Clear digits already used in its 3 × 3 box.

A cell with no remaining candidate is contradictory. The solver can reject that branch rather than attempting digits that cannot lead to a valid board.

### Minimum Remaining Values

`find_mrv_cell()` counts the enabled bits in every empty cell with `__builtin_popcount()` and chooses the cell with the smallest count. This is the Minimum Remaining Values heuristic.

Choosing the most restricted cell first often exposes a contradiction near the top of the search tree. It changes the **order** of search, not the Sudoku rules or the definition of a valid solution.

### Backtracking state restoration

Before trying a candidate, the solver saves every candidate mask in `saved_candidates`. If the recursive branch fails, it clears the chosen value and restores those masks before trying the next candidate.

This restoration is necessary because candidate masks describe one particular partial board. Without it, a value ruled out by a failed branch could incorrectly remain unavailable in a different branch.

### Scope of the propagation step

`propagate_constraints()` recomputes candidate masks until they stop changing and recognizes contradictions created during recomputation. This is lightweight candidate-domain pruning, often called forward checking. It does not automatically fill single-candidate cells, and it does not implement hidden singles, pairs, or AC-3 queue propagation.

## 3. Reproducible check

The example test script compiles both programs with `cc -O2 -std=c11`, runs them on `examples/wikipedia.txt`, removes terminal colour escape codes, and verifies the solved 9 × 9 grid against `examples/wikipedia-solution.txt`.

For a fair comparison, use the same complete input, compiler, optimization flags, and machine for both programs. The timer inside each program uses C's `clock()`, so its output is CPU time rather than wall-clock time.

## 4. Representative benchmark

The following result was obtained from 20 runs of the harder example in `examples/hard.txt` on an ARM64 Mac using Apple Clang 21.0.0 with `-O2 -std=c11`. Values are the mean CPU time reported by the programs.

| Solver | Mean CPU time | Relative to baseline |
| --- | ---: | ---: |
| `sudoku-bt.c` | 1.457 ms | 1.00× |
| `sudoku-mrv.c` | 8.447 ms | 0.17× |

For this implementation and test case, the MRV version is slower. It recomputes candidate masks for the whole board and copies all masks at each branch, so its additional bookkeeping can outweigh the reduced search. MRV is a useful search heuristic, but this code should not be presented as having a fixed speed advantage for every puzzle. Profiling on multiple puzzle sets is required before making broader performance claims.

## Limitations

- The programs assume valid three-digit arguments and do not yet provide full input validation.
- They stop at the first solution and do not test whether that solution is unique.

