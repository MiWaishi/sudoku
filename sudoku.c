#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

// 计时器结构体，用于存储开始和结束时间
typedef struct {
    clock_t start;       // 开始时间（时钟周期）
    clock_t end;         // 结束时间（时钟周期）
    double elapsed_sec;  // 经过的秒数
} Timer;

// 初始化计时器并开始计时
void timer_start(Timer *timer);
// 停止计时器并计算经过的时间
void timer_stop(Timer *timer);
// 打印计时结果
void timer_print(Timer *timer);

// 初始化计时器并开始计时
void timer_start(Timer *timer) {
    if (timer == NULL) return;
    timer->start = clock();
    timer->end = 0;
    timer->elapsed_sec = 0.0;
}

// 停止计时器并计算经过的时间
void timer_stop(Timer *timer) {
    if (timer == NULL) return;
    timer->end = clock();
    // 计算经过的秒数：时钟周期数 / 每秒时钟周期数
    timer->elapsed_sec = (double)(timer->end - timer->start) / CLOCKS_PER_SEC;
}

// 打印计时结果，自动选择合适的单位
void timer_print(Timer *timer) {
    if (timer == NULL) return;
    
    if (timer->elapsed_sec < 1e-6) {
        printf("执行时间: %.3f 纳秒\n", timer->elapsed_sec * 1e9);
    } else if (timer->elapsed_sec < 1e-3) {
        printf("执行时间: %.3f 微秒\n", timer->elapsed_sec * 1e6);
    } else if (timer->elapsed_sec < 1.0) {
        printf("执行时间: %.3f 毫秒\n", timer->elapsed_sec * 1e3);
    } else {
        printf("执行时间: %.6f 秒\n", timer->elapsed_sec);
    }
}

int square(int i, int j);
void set_cell(int i, int j, int n);
int clear_cell(int i, int j);
void init_known(size_t count, char** cells);
bool is_available(int i, int j, int n);
bool advance_cell(int i, int j);
void solve_sudoku(void);
void init_bits(void);
void print_matrix(void);
void print_separator(void);

/* The Sudoku matrix itself. */
int matrix[9][9];

/* Which numbers were given as known in the problem. */
int known[9][9];

/* An array of nine integers, each of which representing a sub-square.
Each integer has its nth-bit on iff n belongs to the corresponding sub-square. */
int squares[9];

/* An array of nine integers, each of which representing a row.
Each integer has its nth-bit on iff n belongs to the corresponding row. */
int rows[9];

/* An array of nine integers, each of which representing a column.
Each integer has its nth-bit on iff n belongs to the corresponding column. */
int cols[9];

/* An array with some powers of 2 to avoid shifting all the time. */
int bits[10];

/* 以下是新增的用MAC优化的代码 */

// 新增：用于存储每个单元格的候选数字（位掩码表示）
int candidates[9][9];

// 新增：初始化候选数字
void init_candidates() {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (matrix[i][j] == 0) {
                candidates[i][j] = 0x1FF; // 二进制 111111111 (1-9 都可用)
            } else {
                candidates[i][j] = 0; // 已有数字的单元格无候选
            }
        }
    }
}

// 新增：更新受影响单元格的候选数字
void update_candidates(int i, int j, int n) {
    // 更新行
    for (int k = 0; k < 9; k++) {
        if (matrix[i][k] == 0) {
            candidates[i][k] &= ~bits[n];
        }
    }
    
    // 更新列
    for (int k = 0; k < 9; k++) {
        if (matrix[k][j] == 0) {
            candidates[k][j] &= ~bits[n];
        }
    }
    
    // 更新九宫格
    int sq = square(i, j);
    int start_i = (sq / 3) * 3;
    int start_j = (sq % 3) * 3;
    for (int di = 0; di < 3; di++) {
        for (int dj = 0; dj < 3; dj++) {
            int r = start_i + di;
            int c = start_j + dj;
            if (matrix[r][c] == 0) {
                candidates[r][c] &= ~bits[n];
            }
        }
    }
}

// 新增：弧一致性检查（AC-3算法）
bool enforce_arc_consistency() {
    // 使用队列存储需要检查的约束（这里简化为检查所有单元格）
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (matrix[i][j] == 0 && candidates[i][j] == 0) {
                return false; // 存在空域，无解
            }
        }
    }
    return true;
}

// 修改后的求解函数（使用MAC方法）
void solve_sudoku_optimization(void) {
    init_candidates(); // 初始化候选数字
    
    // 初始传播：更新已知数字的约束
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (matrix[i][j] != 0) {
                update_candidates(i, j, matrix[i][j]);
            }
        }
    }
    
    // 主求解循环
    int pos = 0;
    while (1) {
        while (pos < 81 && known[pos/9][pos%9]) {
            ++pos; // 跳过已知单元格
        }
        
        if (pos >= 81) break; // 求解完成
        
        int i = pos / 9;
        int j = pos % 9;3
        
        // 使用候选数字而不是盲目尝试1-9
        bool advanced = false;
        if (matrix[i][j] == 0) {
            for (int n = 1; n <= 9; n++) {
                if (candidates[i][j] & bits[n]) { // 只尝试候选数字
                    if (is_available(i, j, n)) {
                        set_cell(i, j, n);
                        advanced = true;
                        break;
                    }
                }
            }
        } else {
            advanced = advance_cell(i, j); // 回溯时尝试下一个候选
        }
        
        if (advanced) {
            // 传播约束
            update_candidates(i, j, matrix[i][j]);
            if (!enforce_arc_consistency()) {
                // 约束传播失败，回溯
                clear_cell(i, j);
                advanced = false;
            } else {
                ++pos; // 前进到下一单元格
            }
        }
        
        if (!advanced) {
            // 回溯
            do {
                --pos;
            } while (pos >= 0 && known[pos/9][pos%9]);
            
            if (pos < 0) break; // 无解
        }
    }
}

/* 新增的用MAC优化的代码结束 */

int main(int argc, char** argv)
{
    Timer timer;

    init_bits();
    init_known(argc-1, argv+1);

    timer_start(&timer);
    solve_sudoku();
    timer_stop(&timer);
    timer_print(&timer);
    print_matrix();

    timer_start(&timer);
    solve_sudoku_optimization();
    timer_stop(&timer);
    timer_print(&timer);
    print_matrix();

    return EXIT_SUCCESS;
}

/* Returns the index of the square the cell (i, j) belongs to. */
int square(int i, int j)
{
    return (i/3)*3 + j/3;
}

/* Stores the number n in the cell (i, j), and turns on the corresponding
bits in rows, cols, and squares. */
void set_cell(int i, int j, int n)
{
    matrix[i][j] = n;
    rows[i] |= bits[n];
    cols[j] |= bits[n];
    squares[square(i, j)] |= bits[n];
}

/* Clears the cell (i, j) and turns off the corresponding bits in rows, cols,
and squares. Returns the number it contained. */
int clear_cell(int i, int j)
{
    int n = matrix[i][j];
    matrix[i][j] = 0;
    rows[i] &= ~bits[n];
    cols[j] &= ~bits[n];
    squares[square(i, j)] &= ~bits[n];
    return n;
}

/* Processes the program arguments. Each argument is assumed to be a string
with three digits row-col-number, 1-based, representing the known cells in the
Sudoku. For example, "123" means there is a 3 in the cell (0, 1). */
void init_known(size_t count, char** cells)
{
    for (int c = 0; c < count; ++c) {
        char* cell = cells[c];
        int i, j, n;
        if (sscanf(cell, "%1d%1d%1d", &i, &j, &n)) {
            set_cell(i-1, j-1, n);
            known[i-1][j-1] = 1;
        } else {
            printf("bad input token: %s\n", cell);
            exit(EXIT_FAILURE);
        }
    }
}

/* Can we put n in the cell (i, j)? */
bool is_available(int i, int j, int n)
{
    return (rows[i] & bits[n]) == 0 && (cols[j] & bits[n]) == 0 && (squares[square(i, j)] & bits[n]) == 0;
}

/* Tries to fill the cell (i, j) with the next available number.
Returns a flag to indicate if it succeeded. */
bool advance_cell(int i, int j)
{
    int n = clear_cell(i, j);
    while (++n <= 9) {
        if (is_available(i, j, n)) {
            set_cell(i, j, n);
            return true;
        }
    }
    return false;
}

/* The main function, a fairly generic backtracking algorithm. */
void solve_sudoku(void)
{
    int pos = 0;
    while (1) {
        while (pos < 81 && known[pos/9][pos%9]) {
            ++pos;
        }
        if (pos >= 81) {
            break;
        }
        if (advance_cell(pos/9, pos%9)) {
            ++pos;
        } else {
            do {
                --pos;
            } while (pos >= 0 && known[pos/9][pos%9]);
            if (pos < 0) {
                break;
            }
        }
    }
}

/* Initializes the array with powers of 2. */
void init_bits(void)
{
    for (int n = 1; n < 10; n++) {
        bits[n] = 1 << n;
    }
}

/* Prints the matrix using some ANSI escape sequences
to distinguish the originally known numbers. */
void print_matrix(void)
{
    for (int i = 0; i < 9; ++i) {
        if ((i % 3) == 0) {
            print_separator();
        }
        for (int j = 0; j < 9; j++) {
            int cell = matrix[i][j];
            if ((j % 3) == 0) {
                printf("\e[1;34m|\e[0m ");
            } else {
                printf(" ");
            }
            if (known[i][j]) {
                printf("\e[1;34m%d\e[0m ", cell);
            } else {
                printf("%d ", cell);
            }
        }
        printf("|\n");
    }
    print_separator();
}

/* Utility to print lines and crosses, used by print_matrix. */
void print_separator(void)
{
    for (int i = 0; i < 3; ++i) {
        printf("\e[1;34m+---------\e[0m");
    }
    printf("\e[1;34m+\n\e[0m");
}
