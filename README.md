# Sudoku Solver Implementations

This repository contains two C implementations used to compare ordinary Sudoku backtracking with a more informed search strategy. Both programs solve a 9 × 9 Sudoku and print the first valid solution they find. They use the same command-line input, so they can be run on the same puzzle for a fair comparison.

## Files

- [`sudoku-bt.c`](sudoku-bt.c) — baseline recursive backtracking.
- [`sudoku-mrv.c`](sudoku-mrv.c) — backtracking with candidate domains and the Minimum Remaining Values (MRV) heuristic.

## Input format

Each known digit is a separate three-digit command-line argument in the form `row-column-number`. Rows and columns are 1-based. For example:

- `115` means “place digit 5 at row 1, column 1”.
- `987` means “place digit 7 at row 9, column 8”.

The following commands compile the programs with GCC or Clang:

```bash
gcc -O2 -std=c11 -o sudoku-bt sudoku-bt.c
gcc -O2 -std=c11 -o sudoku-mrv sudoku-mrv.c
```

Run either program by passing all given cells of the same puzzle:

```bash
./sudoku-bt 115 123 157 216 241 259 265 329 338
./sudoku-mrv 115 123 157 216 241 259 265 329 338
```

The example above is intentionally incomplete; a normal run must include every given cell of the chosen puzzle.

## 1. Baseline solver: `sudoku-bt.c`

The baseline is a depth-first search with backtracking.

1. `find_empty()` scans the board from top-left to bottom-right and returns the first cell containing `0`.
2. `solve()` tries the digits 1 through 9 in that cell.
3. `is_valid()` rejects a digit already present in the same row, column, or 3 × 3 box.
4. If a digit is legal, the program writes it to the board and recursively solves the remaining empty cells.
5. If that recursive branch cannot complete the puzzle, the program changes the cell back to `0` and tries the next digit.
6. If every digit fails, the function returns `false` to make its caller backtrack as well.

This version is a useful reference because every step is easy to follow. Its weakness is the fixed choice of the next cell: it may make several guesses in an unconstrained part of the board before discovering a contradiction elsewhere. With `E` empty cells, the naïve search space has a worst-case upper bound of `O(9^E)`, although row, column, and box checks prune many branches in practice.

## 2. MRV solver: `sudoku-mrv.c`

The optimized solver keeps two pieces of information for every position:

```c
typedef struct {
    int value;       // 0 means the cell is empty
    int candidates;  // a bit mask for the possible digits 1 through 9
} Cell;
```

`ALL_CANDIDATES` is `0x1ff`, whose nine low bits are all 1. Bit 0 represents digit 1, bit 1 represents digit 2, and so on through bit 8 for digit 9. If digit 5 cannot appear in a cell, the program clears bit 4 from that cell's mask.

### Candidate-domain update

`update_candidates()` recomputes the possible digits of every empty cell from the current board:

1. Start with all nine bits enabled.
2. Remove digits already used in the cell's row.
3. Remove digits already used in its column.
4. Remove digits already used in its 3 × 3 box.

If a cell is left with no candidate bits, the current partial board is contradictory and the branch can stop immediately. This is candidate-domain pruning: impossible choices are removed before the program attempts them.

### MRV selection

`find_mrv_cell()` counts the set bits in every empty cell's candidate mask with `__builtin_popcount()`. It selects the cell with the fewest candidates, following the Minimum Remaining Values heuristic.

This changes the order of search. A cell with one or two possible digits is usually more informative than one with six or seven. Trying the most restricted cell first tends to reveal an impossible branch near the top of the search tree, rather than after many later guesses.

### Backtracking and state restoration

Before branching, the MRV solver copies all candidate masks into `saved_candidates`. It then tests only the digits whose bits are set in the selected cell. When a recursive attempt fails, it clears the assigned value and restores the saved masks before trying the next candidate.

Restoring this state is essential. Candidate masks describe one specific partial board; without restoration, values ruled out by a failed branch could incorrectly remain unavailable in a different branch.

### What “constraint propagation” means here

`propagate_constraints()` repeatedly recalculates candidate masks until they are stable and rejects an empty candidate domain. This is a lightweight form of constraint propagation, often called forward checking. It does **not** automatically place a digit when a cell has only one candidate, and it does not include techniques such as hidden singles, pairs, or AC-3 queue propagation. The improvement comes chiefly from early contradiction detection and MRV cell selection.

## 3. Comparing the programs

Both programs use the same Sudoku rules and return the first solution they find. The MRV version does not alter the definition of a valid solution; it changes only the search order and avoids testing digits that are already impossible.

For a meaningful timing comparison:

1. Compile both programs with the same compiler and optimization flags.
2. Use exactly the same complete puzzle input.
3. Run each program multiple times and compare an average, especially for easy puzzles whose execution time is very short.
4. Treat the printed time as CPU time because the programs use C's `clock()` function.

The programs assume valid three-digit arguments and do not currently verify whether a puzzle has a unique solution. They are intended as solver implementations and an algorithmic comparison, rather than a full input-validation application.
