# Sudoku Solver Implementations

**Author:** Sitan Chen

This repository compares two C implementations for solving a 9 × 9 Sudoku. Both programs accept the same puzzle input and print the first valid solution. The difference is the order in which they explore the search tree.

## Repository layout

- [`sudoku-bt.c`](sudoku-bt.c) — the baseline recursive backtracking solver.
- [`sudoku-mrv.c`](sudoku-mrv.c) — a solver using candidate domains and the Minimum Remaining Values (MRV) heuristic.

## Research paper

[Solving Sudoku: From Generic Backtracking to CSP Optimizations](paper/solving-sudoku-backtracking-to-csp-optimizations.pdf) is the six-page research paper accompanying this repository. It documents the original methodology, algorithm diagrams, and an earlier experimental evaluation.


## Build and run

Compile with GCC or Clang:

```bash
gcc -O2 -std=c11 -o sudoku-bt sudoku-bt.c
gcc -O2 -std=c11 -o sudoku-mrv sudoku-mrv.c
```

Each known digit is a separate three-digit argument in the form `row-column-number`, using 1-based positions. For example, `115` places digit 5 at row 1, column 1.

After compilation, pass every given cell of the chosen puzzle as a separate row-column-number argument.

```bash
./sudoku-bt 115 123 157
./sudoku-mrv 115 123 157
```

The two commands must receive the same complete puzzle input when their behavior is compared.
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

## Limitations

- The programs assume valid three-digit arguments and do not yet provide full input validation.
- They stop at the first solution and do not test whether that solution is unique.

