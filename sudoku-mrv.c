#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include<time.h>

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
#define EMPTY 0
#define ALL_CANDIDATES 0x1ff  // 二进制 111111111 表示候选数1-9

typedef struct {
    int value;          // 当前值（0表示未填）
    int candidates;     // 位掩码表示的候选数（bit0~8对应数字1~9）
} Cell;

Cell grid[N][N];        // 数独网格
int solved = 0;         // 解决标志

// 打印数独盘面
// void print_grid() {
//     for(int r=0; r<N; r++) {
//         for(int c=0; c<N; c++) {
//             printf("%d ", grid[r][c].value);
//         }
//         printf("\n");
//     }
// }

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
            int cell = grid[i][j].value;
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

// 检查指定位置是否有效
int is_valid(int row, int col, int num) {
    // 检查行
    for(int c=0; c<N; c++)
        if(grid[row][c].value == num) return 0;
    
    // 检查列
    for(int r=0; r<N; r++)
        if(grid[r][col].value == num) return 0;
    
    // 检查宫格
    int box_row = row - row%3;
    int box_col = col - col%3;
    for(int r=0; r<3; r++)
        for(int c=0; c<3; c++)
            if(grid[box_row+r][box_col+c].value == num)
                return 0;
    
    return 1;
}

// 更新候选数
int update_candidates() {
    int updated = 0;
    for(int r=0; r<N; r++) {
        for(int c=0; c<N; c++) {
            if(grid[r][c].value != EMPTY) continue;
            
            int old_candidates = grid[r][c].candidates;
            grid[r][c].candidates = ALL_CANDIDATES;
            
            // 根据行约束过滤
            for(int col=0; col<N; col++) {
                if(grid[r][col].value != EMPTY)
                    grid[r][c].candidates &= ~(1 << (grid[r][col].value - 1));
            }
            
            // 根据列约束过滤
            for(int row=0; row<N; row++) {
                if(grid[row][c].value != EMPTY)
                    grid[r][c].candidates &= ~(1 << (grid[row][c].value - 1));
            }
            
            // 根据宫格约束过滤
            int box_row = r - r%3;
            int box_col = c - c%3;
            for(int i=0; i<3; i++) {
                for(int j=0; j<3; j++) {
                    if(grid[box_row+i][box_col+j].value != EMPTY)
                        grid[r][c].candidates &= ~(1 << (grid[box_row+i][box_col+j].value - 1));
                }
            }
            
            if(grid[r][c].candidates != old_candidates) {
                updated = 1;
                // 如果候选数为空，说明矛盾
                if(grid[r][c].candidates == 0) return -1;
            }
        }
    }
    return updated;
}

// 应用约束传播（前向检查）
int propagate_constraints() {
    int updated;
    do {
        updated = update_candidates();
        if(updated == -1) return 0;  // 发现矛盾
    } while(updated);
    return 1;
}

// 找到MRV启发式选择的下一个空格
int find_mrv_cell(int *row, int *col) {
    int min_candidates = INT_MAX;
    
    for(int r=0; r<N; r++) {
        for(int c=0; c<N; c++) {
            if(grid[r][c].value == EMPTY && 
               grid[r][c].candidates != 0) {
                int count = __builtin_popcount(grid[r][c].candidates);
                if(count < min_candidates) {
                    min_candidates = count;
                    *row = r;
                    *col = c;
                }
            }
        }
    }
    
    return (min_candidates != INT_MAX);
}

// 递归求解函数
int solve() {
    // 先进行约束传播
    if(!propagate_constraints()) return 0;
    
    // 检查是否完成
    int complete = 1;
    for(int r=0; r<N; r++) {
        for(int c=0; c<N; c++) {
            if(grid[r][c].value == EMPTY) {
                complete = 0;
                goto check_done;
            }
        }
    }
check_done:
    if(complete) {
        solved = 1;
        return 1;
    }
    
    int row, col;
    if(!find_mrv_cell(&row, &col)) return 0;  // 无可用候选
    
    // 保存当前状态（用于回溯）
    int saved_candidates[N][N];
    for(int r=0; r<N; r++) {
        for(int c=0; c<N; c++) {
            saved_candidates[r][c] = grid[r][c].candidates;
        }
    }
    
    // 尝试每个候选值
    int candidates = grid[row][col].candidates;
    for(int num=1; num<=N; num++) {
        if((candidates & (1 << (num-1))) == 0) continue;
        
        // 设置值
        grid[row][col].value = num;
        
        // 递归求解
        if(solve() && solved) return 1;
        
        // 回溯
        grid[row][col].value = EMPTY;
        for(int r=0; r<N; r++) {
            for(int c=0; c<N; c++) {
                grid[r][c].candidates = saved_candidates[r][c];
            }
        }
    }
    
    return 0;
}

// 初始化数独网格
// void init_grid(int input[N][N]) {
//     for(int r=0; r<N; r++) {
//         for(int c=0; c<N; c++) {
//             grid[r][c].value = input[r][c];
//             grid[r][c].candidates = ALL_CANDIDATES;
//         }
//     }
// }

/* Processes the program arguments. Each argument is assumed to be a string
with three digits row-col-number, 1-based, representing the known cells in the
Sudoku. For example, "123" means there is a 3 in the cell (0, 1). */
void init_grid(size_t count, char** cells)
{
    for (int c = 0; c < count; ++c) {
        char* cell = cells[c];
        int i, j, n;
        if (sscanf(cell, "%1d%1d%1d", &i, &j, &n)) {
            grid[i-1][j-1].value = n;
            grid[i-1][j-1].candidates = ALL_CANDIDATES;
        } else {
            printf("bad input token: %s\n", cell);
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char** argv) {
    // 示例输入（0表示空格）
    // int input[N][N] = {
    //     {9, 0, 0, 3, 4, 0, 8, 1, 0},
    //     {0, 2, 0, 0, 0, 0, 0, 0, 9},
    //     {1, 3, 0, 0, 9, 0, 0, 0, 0},
    //     {0, 0, 0, 0, 5, 7, 0, 0, 6},
    //     {5, 0, 0, 0, 0, 4, 3, 9, 0},
    //     {0, 6, 0, 0, 0, 3, 0, 0, 0},
    //     {2, 1, 0, 0, 3, 0, 0, 0, 0},
    //     {0, 0, 3, 0, 7, 9, 2, 0, 1},
    //     {0, 0, 9, 0, 0, 2, 0, 7, 0}
    // };
    init_grid(argc-1, argv+1);
    print_grid();
    
    //init_grid(input);
    
    Timer timer;
    timer_start(&timer);
    if(solve()) {
        timer_stop(&timer);
        timer_print(&timer);
        printf("Solution found:\n");
        print_grid();
    } else {
        printf("No solution exists.\n");
    }
    
    return 0;
}