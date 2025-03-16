#include <stdbool.h>

#define Goto(row, col) printf("\x1b[%d;%dH", row, col)
#define PrintNCh(ch, n) for(int i=0; i<n; i++) printf(ch)
#define PrintColor(fg, bg, s) printf("\033[38;5;%u;48;5;%um%s\033[m", fg, bg, s)

int getch_with_timeout(unsigned int timeout_ms);
int getch();
void setBufferedInput(bool enable);