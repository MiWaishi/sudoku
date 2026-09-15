#include <stdio.h>
#include <stdlib.h>
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

#define N 9

// 打印数独盘面
// void print_grid(int grid[N][N]) {
//     for (int row = 0; row < N; row++) {
//         for (int col = 0; col < N; col++) {
//             printf("%d ", grid[row][col]);
//         }
//         printf("\n");
//     }
// }

// 检查在指定位置放置数字是否有效
bool is_valid(int grid[N][N], int row, int col, int num) {
    // 检查同一行
    for (int x = 0; x < N; x++) {
        if (grid[row][x] == num) {
            return false;
        }
    }
    
    // 检查同一列
    for (int x = 0; x < N; x++) {
        if (grid[x][col] == num) {
            return false;
        }
    }
    
    // 检查3x3宫格
    int box_row = row - row % 3;
    int box_col = col - col % 3;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (grid[box_row + i][box_col + j] == num) {
                return false;
            }
        }
    }
    
    return true;
}

// 寻找空格
bool find_empty(int grid[N][N], int *row, int *col) {
    for (*row = 0; *row < N; (*row)++) {
        for (*col = 0; *col < N; (*col)++) {
            if (grid[*row][*col] == 0) {
                return true;
            }
        }
    }
    return false;
}

// 回溯求解函数
bool solve(int grid[N][N]) {
    int row, col;
    
    // 找到空格，如果没有空格说明已解决
    if (!find_empty(grid, &row, &col)) {
        return true;
    }
    
    // 尝试填入1-9
    for (int num = 1; num <= 9; num++) {
        if (is_valid(grid, row, col, num)) {
            // 如果有效则放置数字
            grid[row][col] = num;
            
            // 递归求解剩余部分
            if (solve(grid)) {
                return true;
            }
            
            // 如果当前数字导致后续无解，则回溯
            grid[row][col] = 0;
        }
    }
    
    // 触发回溯
    return false;
}
int grid[N][N];

void init_grid(size_t count, char** cells)
{
    for (int c = 0; c < count; ++c) {
        char* cell = cells[c];
        int i, j, n;
        if (sscanf(cell, "%1d%1d%1d", &i, &j, &n)) {
            grid[i-1][j-1] = n;
        } else {
            printf("bad input token: %s\n", cell);
            exit(EXIT_FAILURE);
        }
    }
}

/* Utility to print lines and crosses, used by print_matrix. */
void print_separator(void)
{
    for (int i = 0; i < 3; ++i) {
        printf("\e[1;34m+---------\e[0m");
    }
    printf("\e[1;34m+\n\e[0m");
}

/* Prints the matrix using some ANSI escape sequences
to distinguish the originally known numbers. */
void print_grid()
{
    for (int i = 0; i < N; ++i) {
        if ((i % 3) == 0) {
            print_separator();
        }
        for (int j = 0; j < N; j++) {
            int cell = grid[i][j];
            if ((j % 3) == 0) {
                printf("\e[1;34m|\e[0m ");
            } else {
                printf(" ");
            }
            printf("%d ", cell);            
        }
        printf("\e[1;34m|\n");
    }
    print_separator();
}

int main(int argc, char** argv) {
    // 示例输入（0表示空格）
    
    init_grid(argc-1, argv+1);
    // int count = argc - 1;
    // char **cells = argv + 1;
    // for (int c = 0; c < count; ++c) {
    //     char* cell = cells[c];
    //     int i, j, n;
    //     if (sscanf(cell, "%1d%1d%1d", &i, &j, &n)) {
    //         grid[i-1][j-1] = n;
    //     } else {
    //         printf("bad input token: %s\n", cell);
    //         exit(EXIT_FAILURE);
    //     }
    // }
    
    printf("Original Sudoku:\n");
    print_grid();
    printf("\n");
    
    Timer timer;
    timer_start(&timer);
    if (solve(grid)) {
        timer_stop(&timer);
        timer_print(&timer);
        printf("Solution Found:\n");
        print_grid();
    } else {
        printf("No solution exists!");
    }
    
    return 0;
}