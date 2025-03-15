#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <signal.h>   // signal, SIGINT


#include "../lib/utils.h"

#define FoodCnt 5
#define RelativeOffsetWidth 4   // 相对偏移宽度
#define RelativeOffsetHeight 2  // 相对偏移高度

enum Direction {
    ERRDIR, UP, DOWN, LEFT, RIGHT
};

enum PointType {
    NONE, SNAKE_HEAD, SNAKE_BODY, FOOD
};

struct Point{
    uint8_t x, y;
    struct Point *next;
} ;
typedef struct Point Point;

typedef struct {
    Point *snake;
    Point *foods;
    uint8_t *matrix;
    enum Direction dir;
    uint8_t width, height;
} Board;

Board *board;
Point *tmp_point;

bool point_equals(Point *p1, Point *p2) {
    return p1->x == p2->x && p1->y == p2->y;
}

bool list_contains(Point *li, Point *p) {
    while(li != NULL) {
        if(li->x == p->x && li->y == p->y) {
            return true;
        }
        li = li->next;
    }
    return false;
}

bool list_remove(Point **li, Point *p) {
    Point *tmp = *li;
    if(point_equals(tmp, p)) {
        *li = tmp->next;
        free(tmp);
        return true;
    }
    while(tmp->next != NULL) {
        if(point_equals(tmp->next, p)) {
            tmp_point = tmp->next;
            tmp->next = tmp_point->next;
            free(tmp_point);
            return true;
        }
        tmp = tmp->next;
    }
    return false;
}

int list_length(Point *li) {
    int len = 0;
    while(li != NULL) {
        len++;
        li = li->next;
    }
    return len;
}

Point* create_point(uint8_t x, uint8_t y) {
    Point *p = (Point *)malloc(sizeof(Point));
    p->x = x;
    p->y = y;
    p->next = NULL;
    return p;
}

void destroy_point(Point *point){
    while(point != NULL){
        tmp_point = point;
        point = point->next;
        free(tmp_point);
    }
}

void destroy_board() {
    if(board == NULL) return;
    destroy_point(board->snake);
    destroy_point(board->foods);
    free(board->matrix);
    free(board);
}

void add_new_food() {
    Point *food = create_point(rand() % board->width, rand() % board->height);
    while(list_contains(board->snake, food) || list_contains(board->foods, food)) {
        food->x = rand() % board->width;
        food->y = rand() % board->height;
    }
    food->next = board->foods;
    board->foods = food;

}

void init_board(uint8_t width, uint8_t height) {
    destroy_board();
    board = (Board *)malloc(sizeof(Board));
    board->width = width;
    board->height = height;

    board->matrix = (uint8_t *)malloc(sizeof(uint8_t) * width * height);

    board->dir = RIGHT;
    board->snake = create_point(2, height/2);
    board->snake->next = create_point(1, height/2);

    int cnt = 0;
    for(int i=0; i<FoodCnt; i++) {
        add_new_food();
    }
}

void update_matrix() {
    memset(board->matrix, 0, sizeof(uint8_t) * board->width * board->height);

    board->matrix[board->snake->y * board->width + board->snake->x] = SNAKE_HEAD;
    tmp_point = board->snake->next;
    while(tmp_point != NULL) {
        board->matrix[tmp_point->y * board->width + tmp_point->x] = SNAKE_BODY;
        tmp_point = tmp_point->next;
    }
    tmp_point = board->foods;
    while(tmp_point != NULL) {
        board->matrix[tmp_point->y * board->width + tmp_point->x] = FOOD;
        tmp_point = tmp_point->next;
    }
}

void update_direction(enum Direction dir) {
    if((dir == UP && board->dir == DOWN) || (dir == DOWN && board->dir == UP) ||
       (dir == LEFT && board->dir == RIGHT) || (dir == RIGHT && board->dir == LEFT) ||
       dir == ERRDIR) {
        return;
    }
    board->dir = dir;
}

bool snake_move(){
    uint8_t x = board->snake->x, y = board->snake->y;
    switch(board->dir) {
        case UP: y--; break;
        case DOWN: y++; break;
        case LEFT: x--; break;
        case RIGHT: x++; break;
    }
    if(x < 0 || x >= board->width || y < 0 || y >= board->height) {
        return false;
    }
    Point *new_head = create_point(x, y);
    if(list_contains(board->snake, new_head)) {
        free(new_head);
        return false;
    }

    new_head->next = board->snake;
    board->snake = new_head;
    if(list_contains(board->foods, new_head)) {
        list_remove(&board->foods, new_head);
        add_new_food();
    } else {
        tmp_point = board->snake;
        while(tmp_point->next->next != NULL) {
            tmp_point = tmp_point->next;
        }
        free(tmp_point->next);
        tmp_point->next = NULL;
    }
    return true;
}


void draw_board(){
    update_matrix();
    int X = RelativeOffsetWidth, Y = RelativeOffsetHeight;
    Goto(Y, X);
    printf("Snake Game %*s score: %d", 2*board->width - 20, "", list_length(board->snake) - 2);
    Goto(++Y, X);
    PrintNCh("▄", 2 * (board->width + 1) );
    for(int i=0; i<board->height; i++) {
        Goto(++Y, X);
        printf("█");
        for(int j=0; j<board->width; j++) {
            switch(board->matrix[i * board->width + j]) {
                case NONE: printf("  "); break;
                case SNAKE_HEAD:
                    // PrintColor(0, 12,"  ");
                    printf("██");
                    break;
                case SNAKE_BODY: PrintColor(0, 208,"  "); break;
                case FOOD: PrintColor(226, 0, "❤ "); break;
            }
        }
        printf("█");
    }
    Goto(++Y, X);
    PrintNCh("▀", 2 * (board->width + 1));
    Goto(Y+1, X);
    fflush(stdout);
}

uint8_t tick=0;
bool success = false;
int ch;
bool do_tick() {
    success = true;
    if(++tick > 30){
        tick = 0;
        success = snake_move();
        draw_board();
    }
    return success;
}

void run_loop(){
    enum Direction dir = ERRDIR;
    while(do_tick()) {
        dir = ERRDIR;
        usleep(10000);
        ch = getch();
        switch(ch) {
            case 'w':
            case 'k':
            case 'A':
                dir = UP; break;
            case 's':
            case 'j':
            case 'B':
                dir = DOWN; break;
            case 'a':
            case 'h':
            case 'D':
                dir = LEFT; break;
            case 'd':
            case 'l':
            case 'C':
                dir = RIGHT; break;
            case 'q': return;
            case ' ': while(getch(100000) != ' ');
        }
        if(dir == board->dir)
            tick = 30;
        update_direction(dir);

    }
}

void signal_callback_handler(int signum)
{
	setBufferedInput(true);
	// make cursor visible, reset all modes
    printf("\033[?25h\033[m");
    printf("Game Over! Your score is: %d\n", list_length(board->snake) - 2);
    destroy_board();
	exit(signum);
}

int main()
{
    srand(time(NULL));
    init_board(20, 20);
    printf("\033[?25l\033[2J");  // make cursor invisible, erase entire screen
	signal(SIGINT, signal_callback_handler);

    setBufferedInput(false);
    draw_board();
    run_loop();
    signal_callback_handler(0);
    return 0;
}