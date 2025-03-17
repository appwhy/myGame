#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <stdarg.h>   // va_list, va_start, va_end

#include "../lib/utils.h"

// 游戏设置，可以根据需要调整
#define SIZE 16
#define DIFFICULTY 2   // 难度, 1-3，分别对应简单、中等、困难


#define PERCENT (DIFFICULTY == 1 ? 12 : (DIFFICULTY == 2 ? 16 : 20))
#define NUM_MINES ((SIZE * SIZE) * PERCENT / 100)  // 地雷数量

#define RelativeOffsetWidth 4
#define RelativeOffsetHeight 2

// 暴露 0-8: number of mines around
// 炸弹 15
// 隐藏 0-8 | 0x10
// 标记 0-8 | 0x20
#define VAR_MINE 0xF  // 地雷值
#define isExposed(i, j) (board[i][j] >= 0 && board[i][j] <= 0xF)
#define isHidden(i, j) (board[i][j] & 0x10)
#define isMarked(i, j) ((board[i][j] & 0x30) == 0x30)
#define isMine(i, j) ((board[i][j] & 0xF) == 0xF)
#define hideCell(i, j) (isExposed(i, j) && (board[i][j] |= 0x10))
#define markCell(i, j) (isHidden(i, j) && (board[i][j] |= 0x20))
#define unmarkCell(i, j) (isMarked(i, j) && (board[i][j] &= 0x1F))
#define exposeCell(i, j) (isHidden(i, j) && (board[i][j] &= 0xF))

uint8_t board[SIZE][SIZE], x, y;
uint8_t row=0, col=0;
bool Win = false;
time_t Start;

// (dx, dy) 周围8个格子的偏移量
uint8_t rowIndexs[8] = {-1, -1, -1,  0, 0,  1, 1, 1};
uint8_t colIndexs[8] = {-1,  0,  1, -1, 1, -1, 0, 1};


void init_board(){
    memset(board, 0, sizeof(board));
    for(int i = 0; i < NUM_MINES; i++){
        do{
            x = rand() % SIZE;
            y = rand() % SIZE;
        }while(isMine(x, y));
        board[x][y] = VAR_MINE;
    }

    // 填充数字
    for(int i = 0; i < SIZE; i++){
        for(int j = 0; j < SIZE; j++){
            if(isMine(i, j))
                continue;
            for(int index=0; index < 8; index ++){
                x = i + rowIndexs[index];
                y = j + colIndexs[index];
                if(x >= 0 && x < SIZE && y >= 0 && y < SIZE && isMine(x, y)){
                    board[i][j]++;
                }
            }
        }
    }
    // 隐藏所有单元格
    for(int i = 0; i < SIZE; i++){
        for(int j = 0; j < SIZE; j++){
            hideCell(i, j);
        }
    }
}

// 暴露board[x][y]的值, 如果board[x][y]是地雷, 返回true
bool expose_cell(int i, int j){
    if(!isHidden(i, j))
        return false;
    exposeCell(i, j);
    if(isMine(i, j))
        return true;

    if(board[i][j] == 0){ // 暴露周围的8个单元格
        for(int index=0; index < 8; index ++){
                x = i + rowIndexs[index];
                y = j + colIndexs[index];
                if(x >= 0 && x < SIZE && y >= 0 && y < SIZE){
                    if(isHidden(x, y))
                        expose_cell(x, y);
                }
        }
    }
    return false;
}

void auto_expose_mines(){
    for(int i = 0; i < SIZE; i++){
        for(int j = 0; j < SIZE; j++){
            // 如果地雷周围的所有单元格都已暴露, 该地雷也自动暴露
            if(isMine(i, j) && isHidden(i, j)){
                bool allExposed = true;
                for(int index=0; index<8; index++){
                    x = i + rowIndexs[index];
                    y = j + colIndexs[index];
                    if(x >= 0 && x < SIZE && y >= 0 && y < SIZE){
                        allExposed &= isExposed(x, y);
                    }
                }
                if(allExposed)
                    exposeCell(i, j);
            }
        }
    }
}

bool game_over(){
    for(int i = 0; i < SIZE; i++){
        for(int j = 0; j < SIZE; j++){
            if(!isMine(i, j) && isHidden(i, j)){
                return false;
            }
        }
    }
    Win = true;
    return true;
}

void draw_board(){
    int X = RelativeOffsetWidth, Y = RelativeOffsetHeight;
    Y += 2;
    int exposed_cnt = 0;
    for(int i = 0; i < SIZE; i++){
        Goto(Y + i, X);
        printf("|");
        for(int j = 0; j < SIZE; j++){
            if(i == row && j == col){
                printf("\033[38;5;%u;48;5;%um", 0, 83);
            }
            if(isExposed(i, j)){
                exposed_cnt++;
                if(isMine(i, j)){
                    printf("💣");
                } else if(board[i][j] == 0){
                    printf("  ");
                } else{
                    printf("%d ", board[i][j]);
                }
            }else if(isMarked(i, j)){
                printf("m ");
            }else{  // 隐藏
                printf("X ");
                // if(i == row && j == col){
                //     printf("  ");
                // }else {
                //     PrintColor(0, 252, "  ");
                // }
            }
            if(i == row && j == col){
                printf("\033[m");
            }
            printf("|");
        }
    }
    Goto(RelativeOffsetHeight, X);
    printf("MineSweeper:%*s%3d/%3d",SIZE*3+1-12-1-3-3, "", exposed_cnt, SIZE*SIZE);
    Goto(Y + SIZE, X);
    fflush(stdout);
}

void run_loop(){
    int ch;
    bool bomb = false, return_flag = false;
    while(true){
        ch = GETCHAR();
        switch (ch)
        {
            case 'w':    // UP
            case 'A':
            case 'k':
            case 'H':
                row = (row + SIZE - 1) % SIZE;
                break;
            case 's':    // DOWN
            case 'B':
            case 'j':
            case 'P':
                row = (row + 1) % SIZE;
                break;
            case 'a':    // LEFT
            case 'D':
            case 'h':
            case 'K':
                col = (col + SIZE - 1) % SIZE;
                break;
            case 'd':
            case 'C':
            case 'l':
            case 'M':
                col = (col + 1) % SIZE;
                break;
            case 'q':
                return_flag = true;
                break;
            case 'm':  // mark
                if(isMarked(row, col)){
                    unmarkCell(row, col);
                }else{
                    markCell(row, col);
                }
                break;
            case 'b':  // 确认该点是地雷
                if(isHidden(row, col)){
                    if(isMine(row, col)){
                        exposeCell(row, col);
                    }else{
                        exposeCell(row, col);
                        return_flag = true;
                    }
                }
                break;
            case '\n':
            case '\r':
            case ' ':
                bomb = expose_cell(row, col);
                if(!bomb)
                    auto_expose_mines();
                if(game_over()){
                    return_flag = true;
                }
                break;

        }
        draw_board();
        if(bomb || return_flag){
            return;
        }
    }
}

void signal_callback_handler(int signum)
{
	setBufferedInput(true);
	// make cursor visible, reset all modes
    printf("\033[?25h\033[m");
    // printf("Game Over! Your score is: %4d\n", score);
    time_t End = time(NULL);
    if(Win){
        printf("Congratulations! You win! Cost time: %ld s\n", End - Start);
    }else{
        printf("Game Fail. Cost time: %ld s\n", End - Start);
    }
	exit(signum);
}

int main()
{
    srand(time(NULL));
    setBufferedInput(false);

    printf("\033[?25l\033[2J");  // make cursor invisible, erase entire screen
	signal(SIGINT, signal_callback_handler);

    init_board();
    draw_board();
    Start = time(NULL);
    run_loop();
    signal_callback_handler(0);

    return 0;
}


