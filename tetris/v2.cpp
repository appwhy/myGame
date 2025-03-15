/*
    改动:
        在v1的基础上，删除了curses.h，使用了printf(ANSI转义序列)代替curses的函数
*/

#include <time.h>     // time
#include <stdlib.h>   // rand, srand, exit, FD_ZERO, FD_SET, select
#include <string.h>   // memcpy, memset
#include <signal.h>   // signal, SIGINT
#include <stdint.h>	  // uint8_t
#include <stdio.h>	  // printf
#include <termios.h>  // struct termios, tcgetattr, tcsetattr
#include <stdarg.h>   // va_list, va_start, va_end

#define Width 10    // 游戏区域宽
#define Height 20   // 游戏区域高
#define RelativeOffsetWidth 2   // 相对偏移宽度
#define RelativeOffsetHeight 1  // 相对偏移高度
#define BlockW(p, r) (3 & (block[p][r] >> 16))  // 获取方块宽
#define BlockH(p, r) (3 & (block[p][r] >> 18))  // 获取方块高
#define BlockXi(p, r, i) (3 & (block[p][r] >> ((i * 4) + 2)))  // 获取方块i的x坐标
#define BlockYi(p, r, i) (3 & (block[p][r] >> (i * 4)))        // 获取方块i的y坐标
#define Goto(row, col) printf("\x1b[%d;%dH", row, col)
#define PrintNCh(ch, n) for(int i=0; i<n; i++) printf(ch)
#define PrintColor(fg, bg, s) printf("\033[38;5;%u;48;5;%um%s\033[m", fg, bg, s)

// prev_x其实是当前的x, x是要变化的x(方块左右下移动/变换)
int      x = 0,      y = 0,      rotate = 0,      piece_type = 0,
    prev_x = 0, prev_y = 0, prev_rotate = 0,
                            next_rotate = -1, next_piece_type = -1;

int c = 0, tick = 0;
int level = 1, del_row = 0, score = 0;  // speed = 2000/(level+1);

uint8_t board[Height][Width], predict[4][4];

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

uint8_t bgs[7] = { 9, 208, 226, 13, 14, 12, 5}, bg=0, fg=255;


void mvprintf(int row, int col, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf("\033[%d;%dH", row, col);
    vprintf(fmt, args);
    va_end(args);
}

void draw_board() {
    int X = RelativeOffsetWidth, Y = RelativeOffsetHeight;
    int i, j;

    Goto(Y, X);
    PrintNCh("▄", 2*(Width+1));

    for (i = 0; i < Height; i++) {
        mvprintf(Y + 1 + i, X, "█");

        for (j = 0; j < Width; j++) {
            if (board[i][j]) {
                PrintColor(fg, bgs[board[i][j]-1], "  ");
            } else {
                PrintColor(fg, (i + j) % 2 ? 242 : 240, "  ");
            }
        }
        printf("█");
    }

    Goto(Y + Height + 1, X);
    PrintNCh("▀", 2*(Width+1));

    int tmp_Y = Y + Height + 2, tmp_X = X;

    X = X + 2*(Width+1) + 4;
    mvprintf(Y += 2, X, "Score: %d", score);
    mvprintf(Y += 2, X, "Row: %d", del_row);
    mvprintf(Y += 2, X, "Level: %d", level);
    mvprintf(Y += 2, X, "Next:");

    Y += 2;
    for(int i=0; i<4; i++) {
        for(int j=0; j<4; j++) {
            Goto(Y + i, X + 2*j);
            if(predict[i][j]) {
                PrintColor(fg, bgs[next_piece_type], "  ");
            } else
                printf("  ");
        }
    }
    fflush(stdout);
    Goto(tmp_Y, tmp_X);
}


void new_piece() {
    piece_type = (next_piece_type == -1 ? rand() % 7 : next_piece_type);
    next_piece_type = rand() % 7;

    rotate = prev_rotate = (next_rotate == -1 ? rand() % 4 : next_rotate);
    next_rotate = rand() % 4;

    y = prev_y = 0;
    x = prev_x = (10 - BlockW(piece_type, rotate))/2;

    memset(predict, 0, sizeof(predict)*sizeof(predict[0][0]));
    for (int i = 0; i < 4; i++) {
        predict[BlockYi(next_piece_type, next_rotate, i)][BlockXi(next_piece_type, next_rotate, i)] = next_piece_type+1;
    }
}

void set_piece(int x, int y, int rotate, int v) {
    for (int i = 0; i < 4; i++) {
        board[y + BlockYi(piece_type, rotate, i)][x + BlockXi(piece_type, rotate, i)] = v;
    }
}

void update_piece() {
    set_piece(prev_x, prev_y, prev_rotate, 0);
    set_piece(prev_x = x, prev_y = y, prev_rotate = rotate, piece_type + 1);
}

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

int getch(unsigned int timeout_ms=0) {
    struct timeval timeout;
    fd_set fds;
    int ret;
    char c;

    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);

    timeout.tv_sec = timeout_ms / 1000;   // 秒部分
    timeout.tv_usec = (timeout_ms % 1000) * 1000;  // 毫秒转换为微秒

    ret = select(STDIN_FILENO + 1, &fds, NULL, NULL, &timeout);

    if (ret > 0) {
        if (read(STDIN_FILENO, &c, 1) > 0) {
            return (unsigned char)c;  // 返回读取的字符
        }
    }
    return -1; // 超时或错误
}

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

void runloop() {
    int speed = 20000/(level+1);
    while (do_tick()) {
        usleep(speed);
        draw_board();
        c = getch();
        if ((c == 'a' || c == 'D') && x > 0 && !check_hit(x - 1, y, rotate)) {  // 左移
            x--;
        }
        if ((c == 'd' || c == 'C') && x + BlockW(piece_type, rotate) < Width-1 && !check_hit(x + 1, y, rotate)) {
            x++;
        }
        if ((c == 's' || c == 'B') && !check_hit(x, y + 1, rotate)) {  // 下移
            y++;
            tick = 30; // 加快循环
        }
        if (c == '\n') {
            while (!check_hit(x, y + 1, rotate)) {
                y++;
                update_piece();
            }
        }
        if (c == 'w' || c == 'A') {  // A == 上键
            ++rotate %= 4;
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
        draw_board();
    }
}

// 禁用缓冲输入时，用户输入的每个字符都会立即传递给程序，而不需要按Enter(用于实时交互)
void setBufferedInput(bool enable)
{
	static bool enabled = true;
	static struct termios old;
	struct termios new_;

	if (enable && !enabled)
	{
		// restore the former settings
		tcsetattr(STDIN_FILENO, TCSANOW, &old);
		// set the new state
		enabled = true;
	}
	else if (!enable && enabled)
	{
		// get the terminal settings for standard input
		tcgetattr(STDIN_FILENO, &new_);
		// we want to keep the old setting to restore them at the end
		old = new_;
		// disable canonical mode (buffered i/o) and local echo
		new_.c_lflag &= (~ICANON & ~ECHO);
		// set the new_ settings immediately
		tcsetattr(STDIN_FILENO, TCSANOW, &new_);
		// set the new state
		enabled = false;
	}
}

void signal_callback_handler(int signum)
{
	setBufferedInput(true);
	// make cursor visible, reset all modes
    printf("\033[?25h\033[m");
    printf("Game Over! Your score is: %d\n", score);
	exit(signum);
}

int main() {
    srand(time(0));
    new_piece();

	printf("\033[?25l\033[2J");  // make cursor invisible, erase entire screen
	signal(SIGINT, signal_callback_handler);

    setBufferedInput(false);
    draw_board();
    runloop();

    signal_callback_handler(0);
    return 0;
}