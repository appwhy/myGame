/*
    参考: https://github.com/taylorconor/tinytetris.git
    改动:
        可读性更强
        添加了下一个方块的显示
        添加了游戏界面的相对(0,0)的偏移
*/
#include <time.h>
#include <curses.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define Width 10
#define Height 20
#define BlockW(p, r) (3 & (block[p][r] >> 16))  // 获取方块宽
#define BlockH(p, r) (3 & (block[p][r] >> 18))  // 获取方块高
#define BlockXi(p, r, i) (3 & (block[p][r] >> ((i * 4) + 2)))  // 获取方块i的x坐标
#define BlockYi(p, r, i) (3 & (block[p][r] >> (i * 4)))        // 获取方块i的y坐标

// prev_x其实是当前的x, x是要变化的x(方块左右下移动/变换)
int      x = 0,      y = 0,      rotate = 0,      piece_type = 0,
    prev_x = 0, prev_y = 0, prev_rotate = 0,
                            next_rotate = -1, next_piece_type = -1;

int c = 0, tick = 0,  board[Height][Width];
int level = 1, del_row = 0, score = 0;  // speed = 2000/(level+1);

// block layout is: {h-1,w-1}{x0,y0}{x1,y1}{x2,y2}{x3,y3} (two bits each)
int block[7][4] = {
    { 0x69540, 0x92154, 0x69540, 0x92154 },  // Z
    { 0x68451, 0x96510, 0x68451, 0x96510 },  // S
    { 0x55140, 0x55140, 0x55140, 0x55140 },  // O
    { 0x92654, 0x69510, 0x92140, 0x69840 },  // J
    { 0x64951, 0x95210, 0x65840, 0x91654 },  // T
    { 0x3c840, 0xc3210, 0x3c840, 0xc3210 },  // I
    { 0x96210, 0x61840, 0x96540, 0x68951 },  // L
};

// create a new piece, don't remove old one (it has landed and should stick)
void new_piece() {
    piece_type = (next_piece_type == -1 ? rand() % 7 : next_piece_type);
    next_piece_type = rand() % 7;

    rotate = prev_rotate = (next_rotate == -1 ? rand() % 4 : next_rotate);
    next_rotate = rand() % 4;

    y = prev_y = 0;
    x = prev_x = (10 - BlockW(piece_type, rotate))/2;
}

// draw the board and score
// void frame() {
//     for (int i = 0; i < Height; i++) {
//         move(1 + i, 1);  // 留出边框
//         for (int j = 0; j < Width; j++) {
//             board[i][j] && attron(board[i][j] << 8);
//             printw("  ");
//             attroff(board[i][j] << 8);
//         }
//     }
//     move(Height + 1, 1);
//     printw("Score: %d", score);
//     refresh();
// }
void next_frame(int X, int Y){
    X = X + 2*(Width+1) + 4;
    move(Y += 2, X);
    printw("Score: %d", score);
    move(Y += 2, X);
    printw("Row: %d", del_row);
    move(Y += 2, X);
    printw("Level: %d", level);
    move(Y += 2, X);
    printw("Next:");

    int x, y;

    Y += 2;

    for(int i=0; i<4; i++) {
        for(int j=0; j<4; j++) {
            move(Y + i, X + 2*j);
            printw("  ");
            // printw("■");
        }
    }

    for(int i=0; i<4; i++) {
        x = BlockXi(next_piece_type, next_rotate, i);
        y = BlockYi(next_piece_type, next_rotate, i);
        attron((next_piece_type+1) << 8);
        move(Y + y, X + 2*x);
        printw("  ");

        attroff((next_piece_type+1) << 8);
    }


}

void frame() {
    int X = 2;
    int Y = 1;

    int i, j;

    move(Y, X);
    for(i=0; i<Width+1; i++) {
        printw("__");
    }
    for (i = 0; i < Height; i++) {
        move(Y + 1 + i, X);  // 留出边框
        printw("|");
        for (j = 0; j < Width; j++) {
            board[i][j] && attron(board[i][j] << 8);
            printw("  ");
            attroff(board[i][j] << 8);
            // if(board[i][j] == 0) {
            //     printw("  ");
            // } else {
            //     addch(ACS_CKBOARD);
            // }
        }
        printw("|");
    }

    move(Y + Height + 1, X);
    for(i=0; i<Width+1; i++) {
        printw("--");
    }

    next_frame(X, Y);
    refresh();
}



// set the value fo the board for a particular (x,y,rotate) piece
void set_piece(int x, int y, int rotate, int v) {
    for (int i = 0; i < 4; i++) {
        board[y + BlockYi(piece_type, rotate, i)][x + BlockXi(piece_type, rotate, i)] = v;
    }
}

// move a piece from old (piece_type*) coords to new
void update_piece() {
    set_piece(prev_x, prev_y, prev_rotate, 0);
    set_piece(prev_x = x, prev_y = y, prev_rotate = rotate, piece_type + 1);
}

// remove line(s) from the board if they're full
void remove_line() {
    int tmp_score = 0;
    for (int row = y; row <= y + BlockH(piece_type, rotate); row++) {
        c = 1;
        for (int i = 0; i < Width; i++) {
            c *= board[row][i];
        }
        if (!c){
            score += tmp_score;
            tmp_score = 0;
            continue;
        }
        for (int i = row - 1; i > 0; i--) {
            memcpy(&board[i + 1][0], &board[i][0], sizeof(board[0][0]) * Width);
        }
        memset(&board[0][0], 0, sizeof(board[0][0]) * Width);
        del_row++;
        tmp_score = (tmp_score == 0 ? 100 : tmp_score * 2);
    }
    score += tmp_score;
}

// check if placing piece_type at (x,y,rotate) will be a collision
int check_hit(int x, int y, int rotate) {
    if(y + BlockH(piece_type, rotate) >= Height){
        return 1;
    }
    set_piece(prev_x, prev_y, prev_rotate, 0);
    c = 0;
    for (int i = 0; i < 4; i++) {
        board[y + BlockYi(piece_type, rotate, i)][x + BlockXi(piece_type, rotate, i)] && c++;
    }
    set_piece(prev_x, prev_y, prev_rotate, piece_type + 1);
    return c;
}

// slowly tick the piece y position down so the piece falls
int do_tick() {
    if (++tick > 30) {
        tick = 0;
        if (check_hit(x, y + 1, rotate)) {
            if (!y) {
                    return 0;
            }
            remove_line();
            new_piece();
        } else {
            y++;
            update_piece();
        }
    }
    return 1;
}

// main game loop with wasd input checking
void runloop() {
    int speed = 20000/(level+1);
    while (do_tick()) {
        usleep(speed);
        c = getch();
        if ((c == 'a' || c == 'D') && x > 0 && !check_hit(x - 1, y, rotate)) {  // 左移
            x--;
        }
        if ((c == 'd' || c == 'C') && x + BlockW(piece_type, rotate) < Width-1 && !check_hit(x + 1, y, rotate)) {
            x++;
        }
        if (c == 's' || c == 'B') {  // 下移
            while (!check_hit(x, y + 1, rotate)) {
                y++;
                update_piece();
            }
            frame();
            usleep(speed*30);
            remove_line();
            new_piece();
        }
        if (c == 'w' || c == 'A') {  // A == 上键
            ++rotate; rotate %= 4;
            while (x + BlockW(piece_type, rotate) >= Width) {
                x--;
            }
            if (check_hit(x, y, rotate)) {
                x = prev_x;
                rotate = prev_rotate;
            }
        }
        if (c == 'q') {
            return;
        }
        if(c == 32) {  // 暂停
            while(getch() != 32)
                usleep(100000);
        }
        update_piece();
        frame();
    }
}

// init curses and start runloop
int main() {
    srand(time(0));
    initscr();
    start_color();
    // colours indexed by their position in the block
    for (int i = 1; i < 8; i++) {
        init_pair(i, 0, i);
    }
    new_piece();
    // resizeterm(22, 22);
    noecho();
    timeout(0);
    curs_set(0);
    // box(stdscr, 0, 0);
    runloop();
    endwin();

    printf("Game Over! Your score is: %d\n", score);
    return 0;
}