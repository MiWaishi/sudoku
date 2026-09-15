# Sudoku Solver Implementations

This page documents the two C implementations used for the Sudoku-solving study. The existing `sudoku.c` remains in this repository as the earlier prototype.

## Files

- [`sudoku-bt.c`](sudoku-bt.c): a plain recursive backtracking solver. It selects the first empty cell in row-major order and tests digits 1 through 9.
- [`sudoku-mrv.c`](sudoku-mrv.c): an improved solver that recomputes candidate domains, selects the cell with the Minimum Remaining Values (MRV) heuristic, and restores candidate masks after a failed branch.

## Build

```bash
gcc -O2 -std=c11 -o sudoku-bt sudoku-bt.c
gcc -O2 -std=c11 -o sudoku-mrv sudoku-mrv.c
```

## Input

Each given number is passed as a three-digit argument in the form `row-column-number`, using 1-based positions. For example, `115` places 5 at row 1, column 1.

```bash
./sudoku-bt 115 123 157 216 241 259 265 329 338
./sudoku-mrv 115 123 157 216 241 259 265 329 338
```

Use the same complete puzzle input for both programs when comparing execution time. The MRV implementation uses candidate-domain pruning, a lightweight form of constraint propagation; it does not implement advanced human-style Sudoku deductions.
