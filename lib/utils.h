#include <stdbool.h>

#define Goto(row, col) printf("\x1b[%d;%dH", row, col)
#define PrintNCh(ch, n) for(int i=0; i<n; i++) printf(ch)
#define PrintColor(fg, bg, s) printf("\033[38;5;%u;48;5;%um%s\033[m", fg, bg, s)

// windows下的stdio::getchar无法处理方向键, 用conio::_getch()代替
#ifdef _WIN32  // Windows 版本
#include <conio.h>
#define GETCHAR _getch
#else
#include <stdio.h>
#define GETCHAR getchar
#endif

int getch();
void setBufferedInput(bool enable);