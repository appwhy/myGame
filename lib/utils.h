#include <stdbool.h>

#define Goto(row, col) printf("\x1b[%d;%dH", row, col)
#define PrintNCh(ch, n) for(int i=0; i<n; i++) printf(ch)
#define PrintColor(fg, bg, s) printf("\033[38;5;%u;48;5;%um%s\033[m", fg, bg, s)

// windows下的stdio::getchar无法处理方向键, 用conio::_getch()代替
#ifdef _WIN32  // Windows 版本
#include <conio.h>
#define GETCHAR _getch

// 为了解决Windows下的乱码问题（？不支持Unicode）, 用字符代替
#define BORDER_UP "x"
#define BORDER_DOWN "x"
#define BORDER_LEFT "x"
#define BORDER_RIGHT "x"

#else
#include <stdio.h>
#define GETCHAR getchar

#define BORDER_UP "▄"
#define BORDER_DOWN "▀"
#define BORDER_LEFT "█"
#define BORDER_RIGHT "█"

#endif

int getch();
void setBufferedInput(bool enable);