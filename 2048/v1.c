#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include "../lib/utils.h"


#define SIZE 4
#define RelativeOffsetWidth 4
#define RelativeOffsetHeight 2

enum Direction{ NODIR, UP, DOWN, LEFT, RIGHT };
enum Clockwise{ CLOCKWISE90 = 1, ANTICLOCKWISE90 };

uint8_t board[SIZE][SIZE], tmp_board[SIZE][SIZE], tmp, x, y;
int score = 0;
uint8_t fg=15, bg=0, bgs[] = {251, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
uint8_t new_x, new_y;  // 用于记录新生成的数字的位置

void random_num(){
    tmp = (rand() & 1) + 1;
    x = rand() % (SIZE*SIZE);
    y = 0;
    for(int i=0; true; i++){
        i %= (SIZE*SIZE);
        if(board[i/SIZE][i%SIZE] == 0){
            y++;
        }
        if(y == x){
            new_x = i/SIZE;
            new_y = i%SIZE;
            board[new_x][new_y] = tmp;
            // printf("(%d, %d), %d\n", new_x, new_y, tmp);
            break;
        }
    }
}

void init_board(){
    memset(board, 0, sizeof(board));
    random_num();
    random_num();
    new_x = new_y = -1;
}

void board_rotate_90(enum Clockwise ck){
    for(int i = 0; i < SIZE; i++){
        for(int j = 0; j < SIZE; j++){
            if(ck == CLOCKWISE90)
                tmp_board[i][j] = board[SIZE - j - 1][i];
            else
                tmp_board[i][j] = board[j][SIZE - i - 1];
        }
    }
    memcpy(board, tmp_board, sizeof(board));
}

bool slideArray(uint8_t arr[SIZE]) {
    bool changed = false;
    int newIndex = 0, newMergeIndex = -1;

    // 去除 0 元素并合并相邻同类项
    for (int i = 0; i < SIZE; i++) {
        if (arr[i] != 0) {
            if(newIndex != i)
                changed = true;

            if(newIndex == 0){
                arr[newIndex++] = arr[i];
            } else if(arr[newIndex - 1] == arr[i] && newMergeIndex != newIndex - 1) {
                arr[newIndex - 1] += 1;
				score += (1 << arr[newIndex - 1]);
                newMergeIndex = newIndex - 1;
                changed = true;
            } else {
                arr[newIndex++] = arr[i];
            }
        }
    }
    // 剩余位置填充 0
    for (int i = newIndex; i < SIZE; i++) {
        arr[i] = 0;
    }

    return changed;
}

bool board_left(){
    bool changed = false;
    for(int i = 0; i < SIZE; i++){
        changed |= slideArray(board[i]);
    }
    return changed;
}

bool board_right(){
    board_rotate_90(CLOCKWISE90);
    board_rotate_90(CLOCKWISE90);
    bool changed = board_left();
    board_rotate_90(CLOCKWISE90);
    board_rotate_90(CLOCKWISE90);
    return changed;
}

bool board_up(){
    board_rotate_90(ANTICLOCKWISE90);
    bool changed = board_left();
    board_rotate_90(CLOCKWISE90);
    return changed;
}

bool board_down(){
    board_rotate_90(CLOCKWISE90);
    bool changed = board_left();
    board_rotate_90(ANTICLOCKWISE90);
    return changed;
}

bool slide_board(enum Direction dir){
    bool changed = false;
    switch (dir)
    {
    case UP:
        changed = board_up();
        break;
    case DOWN:
        changed = board_down();
        break;
    case LEFT:
        changed = board_left();
        break;
    case RIGHT:
        changed = board_right();
        break;
    }
    return changed;
}

bool has_ended(uint8_t m[SIZE][SIZE]){
    for(int i = 0; i < SIZE; i++){
        for(int j = 0; j < SIZE; j++){
            if(m[i][j] == 0)
                return false;
            if(i > 0 && m[i][j] == m[i-1][j])
                return false;
            if(j > 0 && m[i][j] == m[i][j-1])
                return false;
        }
    }
    return true;
}

void draw_board(){
    int X = RelativeOffsetWidth, Y = RelativeOffsetHeight;
    char s[20];
    Goto(Y, X);
    printf("2048 Game     score:%4d\n", score);
    Y += 2;
    for(int i = 0; i < SIZE; i++){
        for(int j = 0; j < SIZE; j++){
            bg = bgs[(board[i][j] % sizeof(bgs))];

            int len = snprintf(s, sizeof(s), "%d", 1 << board[i][j]);
            tmp = (6 - len) / 2;
            if(i==new_x && j==new_y)
                sprintf(s, "  %d*  ", 1 << board[i][j]);
            else if(board[i][j] == 0)
                sprintf(s, "  ..  ");
            else
                snprintf(s, sizeof(s), "%*s%d%*s", 6-len-tmp, "", 1 << board[i][j], tmp, "");

            // debug code info
            // Goto(Y+i*SIZE+j, X + 6*SIZE + 6 );
            // printf("(%d, %d): %d, bg: %d", i, j, board[i][j], bg);
            // printf("(%d, %d): |%s|", i, j, s);

            Goto(Y + 3*i, X + 6*j);
            PrintColor(fg, bg, "      ");

            Goto(Y + 3*i + 1, X + 6*j);
            PrintColor(fg, bg, s);

            Goto(Y + 3*i + 2, X + 6*j);
            PrintColor(fg, bg, "      ");

        }

    }
    Goto(Y + 3*SIZE, X);
    fflush(stdout);
}

void run_loop(){
    enum Direction dir;
    char ch;
    while(true){
        dir = NODIR;
        ch = GETCHAR();
        switch(ch){
            case 'w':
            case 'A':
            case 'k':
            case 'H':
                dir = UP; break;
            case 's':
            case 'B':
            case 'j':
            case 'P':
                dir = DOWN; break;
            case 'a':
            case 'D':
            case 'h':
            case 'K':
                dir = LEFT; break;
            case 'd':
            case 'C':
            case 'l':
            case 'M':
                dir = RIGHT; break;
            case 'q':
                return;
        }

        if(dir != NODIR){
            bool changed = slide_board(dir);
            if(has_ended(board)){
                return;
            }
            // printf("changed: %d\n", changed);
            if(changed){
                draw_board();
                usleep(200000);
                random_num();
                draw_board();
                new_x = new_y = -1;
            }
        }
    }
}

void signal_callback_handler(int signum)
{
	setBufferedInput(true);
	// make cursor visible, reset all modes
    printf("\033[?25h\033[m");
    printf("Game Over! Your score is: %4d\n", score);
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
    run_loop();
    signal_callback_handler(0);

    return 0;
}